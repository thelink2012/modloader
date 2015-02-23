/*
 * Copyright (C) 2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under the MIT License, see LICENSE at top level directory.
 * 
 * Arguments Translation System
 *      Hacks CLEO.asi file search to inject additional script files
 * 
 */

#ifndef ARGS_TRANSLATOR_HACKS_FINDCLEOSCRIPTS_HPP
#define	ARGS_TRANSLATOR_HACKS_FINDCLEOSCRIPTS_HPP

#include <windows.h>
#include "../xtranslator.hpp"
#include "../../asi.h"

namespace hacks
{
    /*
     *  FindCleoScripts 
     *      This piece of hook is responssible for injecting additional *.cs files into CLEO.asi file search
     *      PS1: This code is not thread-safe, but shouldn't be a concern since CLEO.asi isn't threaded
     *      PS2: Those pseudo handle system is a bit dangerous because it's pointer may conflict with an actual true search handle
     */
    struct FindCleoScripts
    {
        
        // Pseudo search handle returned from GetFirstFileA for *.cs scripts
        struct PseudoHandle
        {
            HANDLE hFind;                                   // Search handle
            char iVersion;                                  // Searching scripts verion... (e.g. cs3 is version 3)
            bool bDidMainSearch;                            // Finished main (WinAPI) search?
            bool bDoingExtraSearch;                         // Working on extra search using iterator?
            bool bDidExtraSearch;                           // Finished the extra search?
            ThePlugin::CsInfoList::iterator iterator;      // Iterator for extra search
            
            // Constructor takes the WinAPI search handle
            PseudoHandle(HANDLE hFind, char iVersion) :
                                        hFind(hFind),
                                        iVersion(iVersion),
                                        bDidMainSearch(false),
                                        bDoingExtraSearch(false),
                                        bDidExtraSearch(false)
            {}
            
            
            // Finishes main search
            void DoneMainSearch()
            {
                this->bDidMainSearch = true;
            }
            
            // Finishes extra search
            void DoneExtaSearch()
            {
                this->bDidExtraSearch = true;
                this->bDoingExtraSearch = false;
            }
            
            // Checks if finished main search
            bool IsMainSearchDone()
            {
                return this->bDidMainSearch;
            }
            
            
            // Moves the iterator to the next extra script and get that file data
            bool NextExtra(LPWIN32_FIND_DATAA lpFindFileData)
            {
                // Iterate on the extra search
                while(this->NextExtra())
                {
                    // Get the file data...
                    if(this->GetFindData(lpFindFileData))
                    {
                        plugin_ptr->Log("Injecting cleo script \"%s\"", this->GetPath());
                        return true;
                    }
                        
                    // Failed to get file data, skip into the next file
                    plugin_ptr->Log("Failed to get find data for cleo script \"%s\"", this->GetPath());
                }
                
                // No more files
                return false;
            }
            
            
            
            
            
            
            // Moves the iterator to the next extra script
            bool NextExtra()
            {
                bool bNoMoreFiles = false;
                bool bJustStarted = false;
                
                // If not doing extra search yet, start it
                if(!bDoingExtraSearch)
                {
                    if(!bDidExtraSearch)    // Work on extra search only once
                    {
                        // Setup fields
                        DoneMainSearch();
                        bDoingExtraSearch = true;
                        bJustStarted = true;
                        iterator = plugin_ptr->cast<ThePlugin>().csList.begin();
                    }
                    else
                    {
                        // No moarrr files on extra search
                        bNoMoreFiles = true;
                    }
                }
                else
                {
                    // Advance the iterator if still doing extra search
                    ++iterator;
                }
                
                // Find next script with compatibility for iVersion and that is not a custom mission (.cm)
                while(iterator != plugin_ptr->cast<ThePlugin>().csList.end() && (iterator->iVersion != this->iVersion || iterator->bIsMission))
                    ++iterator;

                // No more files to work with?
                if(bNoMoreFiles || iterator == plugin_ptr->cast<ThePlugin>().csList.end())
                {
                    // Finish the extra search
                    this->DoneExtaSearch();
                    return false;
                }
                else if(bJustStarted)
                {
                    plugin_ptr->Log("Injecting extra scripts into search %p...", this);
                }
                
                return true;
            }

            // Get WIN32_FIND_DATA structure from the current iterator position
            bool GetFindData(LPWIN32_FIND_DATAA lpFindFileData)
            {
                if(this->bDoingExtraSearch)
                {
                    // Get path for file into lpFindFileData
                    this->GetFullPath(lpFindFileData->cFileName);
                    
                    // Get standard file attributes to give it into lpFindFileData
                    if(GetFileAttributesExA(lpFindFileData->cFileName, GetFileExInfoStandard, lpFindFileData))
                    {
                        // Set lower part of lpFindData
                        this->GetCleoCompatiblePath(lpFindFileData->cFileName);
                        lpFindFileData->cAlternateFileName[0] = 0;          // Not used
                        lpFindFileData->dwReserved0 = 0;                    // ^
                        lpFindFileData->dwReserved1 = 0;                    // ^
                        return true;
                    }
                }
                return false;
            }
            
            // Get full path in iterator
            char* GetFullPath(char* buf)
            {
                sprintf(buf, "%s%s", plugin_ptr->loader->gamepath, GetPath());
                return buf;
            }
            
            // Get path in iterator compatible with what CLEO.asi expects to receive
            char* GetCleoCompatiblePath(char* buf)
            {
                // CLEO 4.3 needs a existing CLEO folder for the path relativity to work
                if(plugin_ptr->cast<ThePlugin>().iCleoVersion > 0x401011E && plugin_ptr->cast<ThePlugin>().bHasNoCleoFolder)
                    return GetFullPath(buf);
                
                sprintf(buf, "..\\%s", GetPath());
                return buf;
            }
            
            // Get relative path in iterator
            const char* GetPath()
            {
                return iterator->file->filepath();
            }
        };
        
        // Get list of open pseudo handles
        static std::list<PseudoHandle>& GetList()
        {
            static std::list<PseudoHandle> x;
            return x;
        }
        
        // Check if hFind is an pseudo handle and returns it's structure object
        static PseudoHandle* FindHandle(HANDLE hFind)
        {
            for(auto& h : GetList())
                if(hFind == &h) return &h;
            return nullptr;
        }
        
        // Add pseudo handle from WinAPI search handle
        static PseudoHandle* AddHandle(HANDLE hFind, char iVersion)
        {
            auto* ph = &(*GetList().emplace(GetList().end(), hFind, iVersion));
            plugin_ptr->Log("Starting injected cleo script search %p for version '%c'...", ph, ph->iVersion? ph->iVersion : '0');
            return ph;
        }
        
        // Checks if hFind is a pseudo handle, if it is delete it and return the original WinAPI handle
        // previoslly attached to it. Returns NULL if hFind isn't a pseudo handle.
        static HANDLE RemoveHandle(HANDLE hFind)
        {
            HANDLE hResult = NULL;
            auto& list = GetList();
            
            // Needs to use iterators because we need to fast-erase
            for(auto it = list.begin(); it != list.end(); ++it)
            {
                // Check if it's this pseudo handle
                if(&(*it) == hFind)
                {
                    // Store original handle and erase this pseudo handle
                    hResult = it->hFind;
                    list.erase(it);
                    plugin_ptr->Log("Finishing injected cleo script search %p...", hFind);
                    break;
                }
            }
            
            // Ret it yey
            return hResult;
        }

        // Finder for some random symbol... Do nothing for this
        template<const char* Symbol, class Ret, class... Args>
        static bool Finder(Ret& result, Args&...)
        {
            return false;
        }
       
    };
  
    /*
     *  Hacked FindFirstFileA
     */
    template<>
    bool FindCleoScripts::Finder<aFindFirstFileA>(HANDLE& result, LPCTSTR& lpFileName, LPWIN32_FIND_DATAA& lpFindFileData)
    {
        char version;
        
        // Find file extension position
        auto* p = strrchr(lpFileName, '.');
        
        // Check if before the extension there's a '*' (for search all with...) and after the extension an 'cs' string (for cs scripts)
        if(p && p != lpFileName && *(p-1) == '*' && ThePlugin::CsInfo::GetVersionFromExtension(p+1, version))
        {
            bool bFailed = false;       // Real failure on FindFirstFileA
            bool bNoMoreFiles = false;  // No more files on FindFirstFileA
            
            if(!plugin_ptr->cast<ThePlugin>().bHasNoCleoFolder)
            {
                // Start the search
                result = FindFirstFileA(lpFileName, lpFindFileData);
                if(result == INVALID_HANDLE_VALUE)
                {
                    // Alrighty, do our search
                    bNoMoreFiles = true;
                }
            }
            else
            {
                // CLEO folder don't exist, don't even try to open a search for it
                result = INVALID_HANDLE_VALUE;
                bNoMoreFiles = true;
            }
            
            // If search started succesfuly (...) inject our pseudo handle into the return
            if(!bFailed)
            {
                // Push our pseudo handle
                PseudoHandle* h = AddHandle(result, version);
                result = h;
                
                // If WinAPI couldn't find any file on the FindFirstFile, start our extra search
                if(bNoMoreFiles)
                {
                    if(!h->NextExtra(lpFindFileData))
                    {
                        // No files on extra search...
                        SetLastError(ERROR_FILE_NOT_FOUND);
                        result = RemoveHandle(result);
                    }
                }
            }

            return true;    // Point out that we hacked this call
        }
        return false;
    }

    
    /*
     *  Hacked FindNextFileA
     */
    template<>
    bool FindCleoScripts::Finder<aFindNextFileA>(BOOL& result, HANDLE& hFindFile, LPWIN32_FIND_DATAA& lpFindFileData)
    {
        // Check out if this handle is a pseudo handle
        if(PseudoHandle* h = FindHandle(hFindFile))
        {
            // If main search isn't done use FindNextFile
            if(!h->IsMainSearchDone())
            {
                result = FindNextFileA(h->hFind, lpFindFileData);

                // Failed because there's no moar files? Finish main search
                if(result == FALSE && GetLastError() == ERROR_NO_MORE_FILES)
                    h->DoneMainSearch();
            }
            
            // Check again if main search is done, if it is, do our extra search
            if(h->IsMainSearchDone())
            {
                // Iterate on the extra search
                if(h->NextExtra(lpFindFileData))
                {
                    // We still have files...
                    result = TRUE;
                }
                else
                {
                    // We are done, no more files
                    SetLastError(ERROR_NO_MORE_FILES);
                    result = FALSE;
                }
            }

            return true;    // Point out that we hacked this call
        }
        return false;
    }

    /*
     *  Hacked FindClose
     */
    template<>
    bool FindCleoScripts::Finder<aFindClose>(BOOL& result, HANDLE& hFindFile)
    {
        // Finish the pseudo handle if it is a pseudo handle (if not the call will return null)
        if(HANDLE h = RemoveHandle(hFindFile))
        {
            // Close the actual WinAPI search
            if(h != INVALID_HANDLE_VALUE)
                result = FindClose(h);

            return true;    // Point out that we hacked this call
        }
        return false;
    }
    
};


#endif
