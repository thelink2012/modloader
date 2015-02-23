/*
 * Copyright (C) 2013-2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under the MIT License, see LICENSE at top level directory.
 * 
 */
#include <stdinc.hpp>
#include "loader.hpp"
#include <regex/regex.hpp>
#include <intrin.h>
using namespace modloader;
using time_point = std::chrono::steady_clock::time_point;

/*
 *  This file contains some pretty sad win32 code
 *  Watches the filesystem for changes and then sends it to UpdateFromJournal (another .cpp)
 */

// How much time between filesystem changes to execute a reupdate?
static auto refresh_delay = std::chrono::milliseconds(1000);

// Threading variables
static HANDLE hThread = NULL;   // I/O thread used to watch over the file system
static HANDLE hDirectory;       // Handle to modloader/ directory, which we'll be watching
static OVERLAPPED overlapped;   // Watches using async I/O  
static HANDLE hCancelEvent;     // Cancels the I/O operation
static CRITICAL_SECTION mutex;  // Used to avoid data races to 'last_change' and 'journal' variables
static ULONGLONG has_changes = FALSE;// Used to determine if anything changed in the journal (and last_change) (should use atomic operations to set)
static ULONGLONG kill_watcher;       // Should the watcher thread be killed? (should use atomic operations to set)
// ^ use ULONGLONG instead of LONG or something else because of compatibility with mingw-w64

// Journaling changes
static time_point last_change;  // Last time something changed in the filesystem
static Loader::Journal journal; // Journal of unprocessed changes in the filesystem

static DWORD __stdcall WatcherThread(void*);    // I/O thread
static Loader::Journal CheckoutJournal();       // Called by the main thread to deal with the I/O thread

/*
 *  Loader::StartupWatcher
 *      Startups the filesystem watcher to track changes in the modloader directory while the game runs
 */
void Loader::StartupWatcher()
{
    if(this->bAutoRefresh && hThread == NULL)
    {
        this->Log("Starting up filesystem watcher...");

        // Clear common variables
        journal.clear();
        kill_watcher = FALSE;
        has_changes = FALSE;
    
        // Startups the I/O watcher thread....
        hThread = CreateThread(NULL, 0, &WatcherThread, NULL, CREATE_SUSPENDED, NULL);
        if(hThread)
        {
            hCancelEvent = CreateEventA(NULL, FALSE, FALSE, NULL);
            if(hCancelEvent)
            {
                // Takes up the directory handle for modloader...
                hDirectory = CreateFileA((this->gamePath + "modloader/").c_str(),
                                        FILE_LIST_DIRECTORY, (FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE),
                                        NULL, OPEN_EXISTING, (FILE_FLAG_BACKUP_SEMANTICS|FILE_FLAG_OVERLAPPED), NULL);
                if(hDirectory != INVALID_HANDLE_VALUE)
                {
                    InitializeCriticalSection(&mutex);
                    ResumeThread(hThread);
                    return;
                }
                else
                    hDirectory = NULL;

                CloseHandle(hCancelEvent);
            }

            CloseHandle(hThread);
            hThread = NULL;
        }

        this->Log("Failed to startup watcher, automatic refreshing won't work.");
    }
}

/*
 *  Loader::ShutdownWatcher
 *      Terminates the filesystem watcher
 */
void Loader::ShutdownWatcher()
{
    if(hThread)
    {
        this->Log("Shutting down filesystem watcher...");
        SetEvent(hCancelEvent);
        WaitForSingleObject(hThread, INFINITE); // waits for the thread to finish up after the IO cancellation
        CloseHandle(hDirectory);
        CloseHandle(hThread);
        CloseHandle(hCancelEvent);
        DeleteCriticalSection(&mutex);
        journal.clear();
        has_changes = FALSE;
        hThread = NULL;
    }
}

/*
 *  Loader::CheckWatcher
 *      Must be called frequently to check if the watcher reported about any change in the filesystem
 */
void Loader::CheckWatcher()
{
    auto journal = CheckoutJournal();
    if(journal.size())
    {
        bool changed_modloader_ini = std::any_of(journal.begin(), journal.end(), [this](const Journal::value_type& pair) 
                                                                                 { return pair.first == folderConfigFilename; });

        if(changed_modloader_ini) this->LoadFolderConfig();

        if(changed_modloader_ini ||
           std::any_of(journal.begin(), journal.end(), [](const Journal::value_type& pair) {
            return pair.first == ".";
        }))
            this->ScanAndUpdate();  // Complete refresh
        else
            this->UpdateFromJournal(journal);
    }
}



// All those functions should run in a separate thread from the main thread
static void RegisterNotification(FILE_NOTIFY_INFORMATION* notify);
static void RegisterError();
static void NotifyCompleteRefresh();
static void NotifyJournal(std::string modname, int action, bool is_root);
static void NotifyJournalChange();


/*
 *  WatcherThread
 *      I/O Thread used to watch over the filesystem
 */
static DWORD __stdcall WatcherThread(void*)
{
    static const size_t notifies_bufsize = 15750 * sizeof(DWORD);   // 63000 bytes -- 63KB... buffer must be dword aligned and below 64KB
    char* noticies_buf = new char[notifies_bufsize];
    FILE_NOTIFY_INFORMATION* notifies = (FILE_NOTIFY_INFORMATION*)(noticies_buf);
    DWORD bytes;

    // First time using the overlapped structure, zero it up and associate a event object with it
    memset(&overlapped, 0, sizeof(overlapped));
    overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    while(InterlockedAnd(&kill_watcher, TRUE) == FALSE)
    {
        // Watch the next changes in the modloader directory
        if(ReadDirectoryChangesW(hDirectory, notifies, notifies_bufsize, TRUE,
            (FILE_NOTIFY_CHANGE_FILE_NAME|FILE_NOTIFY_CHANGE_DIR_NAME|FILE_NOTIFY_CHANGE_SIZE|FILE_NOTIFY_CHANGE_LAST_WRITE),
            &bytes, &overlapped, NULL))
        {
            // Wait for the changes to come up or the request to be cancelled by the main thread
            HANDLE pHandles[] = { hCancelEvent, overlapped.hEvent  };   // cancel should be the first event
            switch(WaitForMultipleObjects(2, pHandles, FALSE, INFINITE))
            {
                case (WAIT_OBJECT_0 + 0):   // hCancelEvent
                {
                    CancelIo(hDirectory);
                    InterlockedOr(&kill_watcher, TRUE);
                    break;
                }

                case (WAIT_OBJECT_0 + 1):   // hDirectory (overlapped.hEvent)
                {
                    if(GetOverlappedResult(hDirectory, &overlapped, &bytes, FALSE))
                    {
                        if(bytes)
                        {
                            // Pass notifications forward
                            for(auto notify = notifies; notify;  notify = (FILE_NOTIFY_INFORMATION*)(notify->NextEntryOffset? ((char*)notify + notify->NextEntryOffset) : nullptr))
                                RegisterNotification(notify);
                        }
                    }
                    else
                        RegisterError();
                    break;
                }

                default: // this should never happen
                    loader.Log("Warning: Failed to wait for the directory watcher, something is really wrong");
                    break;
            }
        }
        else
            RegisterError();
    }

    // Finish up the thread
    CloseHandle(overlapped.hEvent);
    delete[] noticies_buf;
    return 0;
}


/*
 *  CheckoutJournal
 *      Checks if there's anything pending in the journal in enought time and
 *      then returns the journal content safe to operate without race conditions.
 *
 *      This is the only function that may and should be called from the main thread to communicate with the watcher thread.
 *
 */
static Loader::Journal CheckoutJournal()
{
    if(InterlockedAnd(&has_changes, TRUE))
    {
        scoped_lock xlock(mutex);   // lock for accessing last_change and the journal
        if((std::chrono::steady_clock::now() - last_change) >= refresh_delay)
        {
            InterlockedAnd(&has_changes, FALSE);
            return Loader::Journal(std::move(journal));
        }
    }
    return Loader::Journal();
}

/*
 *  RegisterNotification
 *      Registers a filesystem change notification
 */
static void RegisterNotification(FILE_NOTIFY_INFORMATION* notify)
{
    char buffer[MAX_PATH];
    auto size = WideCharToMultiByte(CP_ACP, 0, notify->FileName, notify->FileNameLength / sizeof(WCHAR), buffer, sizeof(buffer), NULL, NULL);
    if(size != 0)
    {
        static auto regex = make_regex(R"___(^([^\.\\/].*?)([\\/].+)?$)___",  // anything that doesn't begin with '.', match the first dir part
                                       sregex::ECMAScript|sregex::optimize/*|sregex::icase*/);

        auto filepath = NormalizePath(std::string(buffer, size));

        smatch match;
        if(filepath == "modloader.ini" || filepath == ".profiles")
        {
            // Tell the loader to refresh configs
            journal.emplace("modloader.ini", Loader::Status::Updated);
            NotifyJournalChange();
        }
        else if(regex_match(filepath, match, regex))
        {
            if(match.size() == 3)
            {
                // match[1] contains the mod name directory
                // match[2] may contains subdirs or subfiles in the mod name directory
                NotifyJournal(match[1], notify->Action, !match[2].length());
            }
        }
    }
    else
    {
        // Well, something not quite right happened while trying to convert the UTF-16 string to the current locale
        // Let's just refresh everything then
        NotifyCompleteRefresh();
    }
}

/*
 *  RegisterError
 *      Takes care of some errors from the watcher API
 */
static void RegisterError()
{
    switch(auto code = GetLastError())
    {
        case ERROR_NOTIFY_ENUM_DIR:     // This happens when our notification buffer overflows, 
            NotifyCompleteRefresh();    // we then need to enumerate the directory manually to checkout the changes.
            return;                     // Alright, let's do a complete refresh.

        case ERROR_OPERATION_ABORTED:   // This probably happens only during ShutdownWatcher when we call CancelIo
            return;                     // on the watching directory

        default:                        // This should not happen
            loader.Log("Warning: Failed to watch directory changes, error code %u, this may be fatal.", code);
            return;
    }
}


/*
 *  NotifyJournalChange
 *      Adjusts the proper variables after changing something on the journal
 *      (or even to tell something updated even tho the journal didn't need to update for that)
 *
 *      NOTE: mutex should be locked before calling this so we can safely update the vars
 */
static void NotifyJournalChange()
{
    last_change = std::chrono::steady_clock::now();
    InterlockedOr(&has_changes, TRUE);
}

/*
 *  NotifyCompleteRefresh
 *      Adjusts the proper variables after changing something on the journal
 *      (or even to tell something updated even tho the journal didn't need to update for that)
 */
static void NotifyCompleteRefresh()
{
    scoped_lock xlock(mutex);   // lock mutex for update
    journal.emplace(".", Loader::Status::Updated);  // refresh all '.'
    NotifyJournalChange();
}

/*
 *  NotifyJournal
 *      Notifies our journal about some change in the filesystem.
 *      'modname' is the modification that got the change
 *      'action' is a win32 FILE_ACTION
 *      'is_root' tells that the notification happened on the modname folder itself instead of inside it
 */
static void NotifyJournal(std::string modname, int action, bool is_root)
{
    scoped_lock xlock(mutex);   // lock to operate on the journal and it's friends

    auto it = journal.find(modname);
    if(it != journal.end())
    {
        //
        //  This entry has been added to the journal already, let's update it's content
        //

        if(is_root)
        {
            if(action == FILE_ACTION_ADDED || action == FILE_ACTION_RENAMED_NEW_NAME)
            {
                // If the previous state is to be removed and now it's back, change it to added
                if(it->second == Loader::Status::Removed)
                    it->second = Loader::Status::Added;
            }
            else if(action == FILE_ACTION_REMOVED || action == FILE_ACTION_RENAMED_OLD_NAME)
            {
                // No question, just override the previous state with removed
                it->second = Loader::Status::Removed;
            }
            else if(action == FILE_ACTION_MODIFIED
                 && (it->second != Loader::Status::Added && it->second != Loader::Status::Removed))
            {
                // Modified the dir (somehow) and previous state wasn't added/removed... so it can safely be updated
                it->second = Loader::Status::Updated;
            }
        }
        else
        {
            // Something changed INSIDE the mod directory, assume update unless the previous state is Added/Removed
            if(it->second != Loader::Status::Added && it->second != Loader::Status::Removed)
                it->second = Loader::Status::Updated;
        }

        // Notify that the journal is dirty now
        NotifyJournalChange();
    }
    else
    {
        //
        //  This entry has not been added to the journal yet
        //

        auto AddToJournal = [&](Loader::Status status) {
            journal.emplace(modname, status);
            NotifyJournalChange();
        };

        if(is_root)
        {
            // Something changed in the directory itself, not inside it

            bool is_existing_directory = !!IsDirectoryA(std::string(loader.gamepath).append("modloader/").append(modname).c_str());

            if(action == FILE_ACTION_ADDED || action == FILE_ACTION_RENAMED_NEW_NAME)
                AddToJournal(Loader::Status::Added);
            else if(action == FILE_ACTION_REMOVED || action == FILE_ACTION_RENAMED_OLD_NAME)
                AddToJournal(Loader::Status::Removed);
            else if(action == FILE_ACTION_MODIFIED && is_existing_directory)    // allow modified only if the directory already exists
                AddToJournal(Loader::Status::Updated);                          // (i.e. avoid modloader.log and such, although it may pass with ADDED/REMOVED)
        }
        else
        {
            // Something changed inside the mod directory, so the mod just updated
            AddToJournal(Loader::Status::Updated);
        }
    }
}
