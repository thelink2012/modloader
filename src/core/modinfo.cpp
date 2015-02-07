/*
 * Copyright (C) 2013-2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#include <stdinc.hpp>
#include "loader.hpp"
using namespace modloader;

/*
 *  Scoped logging for Install/Reinstall/Uninstall
 *  Logs when entering the scope (on construction) about the action starting
 *  and logs when going out of scope (on destructor) about the action success or failure. 
 */
struct FileInstallLog
{
    const Loader::FileInformation& file;    // File taking the action
    const char* action;                     // The action in action
    bool revaction;                         // Action success state
    
    FileInstallLog(const Loader::FileInformation& file, const char* action, bool revaction = false) :
                file(file), action(action), revaction(revaction)
    {
        loader.Log("%sing file \"%s\"", action, file.filepath());
    }
    
    ~FileInstallLog()
    {
        if(file.IsInstalled() == revaction)
            loader.Log("Failed to %s file \"%s\"", action, file.filepath());
    }
};


/*
 *  ModInformation::Scan
 *      Scans this mod in search of files added, updated and removed
 *      This method just searches, it doesn't install or uninstall anything, to install call parent->Update()
 */
void Loader::ModInformation::Scan()
{
    static auto modloader_subfolder = NormalizePath("modloader");
    ::scoped_gdir xdir(this->path.c_str());
    
    if(this->UpdateIgnoreStatus().IsIgnored())
        Log("\nIgnoring mod at \"%s\"", this->path.c_str());
    else
        Log("\nScanning files at \"%s\"...", this->path.c_str());

    // > Status here is Unchanged
    // Mark all current files as removed
    MarkStatus(this->files, Status::Removed);

    // Scan the directory checking out all files
    bool fine = this->IsIgnored()? true : FilesWalk("", "*.*", true, [this](FileWalkInfo& file)
    {
        auto filename = NormalizePath(file.filename);
        auto filepath = this->path + NormalizePath(file.filebuf);

        // Nested Mod Loader folder...
        if(!parent.Profile().IsFileIgnored(filename))
        {
            uint64_t uid;
            modloader::file m;
            ref_list<PluginInformation> callme;
            PluginInformation* handler;

            // This buffer setup is tricky but should work fine
            m.buffer       = filepath.data();
            m.pos_eos      = filepath.length();
            m.pos_filedir  = this->path.length();
            m.pos_filename = m.pos_filedir + (file.filename - file.filebuf);
            m.pos_filext   = m.pos_filedir + (file.filext - file.filebuf);
            m.hash         = modloader::hash(filepath.data() + m.pos_filename);
            
            // Setup other information
            m._rsv1   = 0;
            m.flags   = (std::underlying_type<FileFlags>::type)(file.is_dir? FileFlags::IsDirectory : FileFlags::None);
            m.behaviour = uid = -1;
            m.parent  = this;
            m.size    = file.size;
            m.time    = file.time;

            // Find a handler for this file
            handler = loader.FindHandlerForFile(m, callme);
            if(handler || !callme.empty())
            {
                file.recursive = false;     // Avoid FilesWalk recursion
                
                // Push the new file into our list
                auto ipair = files.emplace( std::piecewise_construct,
                                            std::forward_as_tuple(std::string(m.filedir())), 
                                            std::forward_as_tuple(*this, std::move(filepath), m, handler, std::move(callme)));
                
                auto& n = ipair.first->second;

                if(!ipair.second)
                {
                    // Update status checking if file changed
                    n.status = n.Update(m)? Status::Updated : Status::Unchanged;
                }
                else
                    n.status = Status::Added;
                
                Log("Found file [0x%.16" PRIX64 "] \"%s\" with handler \"%s\"",
                        n.behaviour,
                        file.filebuf,
                        n.handler? n.handler->name : callme.size()? "<callme>" : "<none>");
            }
            else
            {
                // Show no handler only if file isn't a directory, avoid spamming directories on the log
                if(!file.is_dir) Log("No handler or callme for file \"%s\"", file.filebuf);
            }
        }
        else
        {
            Log("Ignoring file \"%s\"", file.filebuf);
        }
        
        return true;
    });
    
    
    // Find the underlying status of this mod
    UpdateStatus(*this, this->files, fine);
    if(this->UpdatePriority() && this->status == Status::Unchanged)
        this->status = Status::Updated;
}


/*
 *  ModInformation::ExtinguishingNecessaryFiles
 *      Uninstall anything that has the status Removed
 *      This is usually called after a Scan
 *      Returns the behaviours successfully UNINSTALLED by this call
 */
void Loader::ModInformation::ExtinguishNecessaryFiles()
{
    if(this->status != Status::Unchanged)
    {
        Updating xup;
        bool logged_ex = false;

        auto LogExtinguishing = [&](const char* filepath)
        {
            if(!logged_ex) Log("Extinguishing some files from \"%s\"...", this->path.c_str());
            Log("Extinguishing file \"%s\"", filepath);
            logged_ex = true;
        };

        // Remove all files if this mod has been removed
        bool bRemoveAll = (this->status == Status::Removed);
        
        // Finds all removed files in this mod and uninstall them
        for (auto it = this->files.begin(); it != this->files.end(); )
        {
            FileInformation& file = it->second;
            BehvSet::value_type file_behv = { file.handler, file.behaviour };
            bool was_installed = file.IsInstalled();

            if(bRemoveAll || file.status == Status::Removed)
            {
                // File was removed from the filesystem, uninstall and erase from our internal list
                LogExtinguishing(file.filepath());
                if(file.Uninstall())
                {
                    it = this->files.erase(it);
                }
                else
                    ++it;
            }
            else
                ++it;
        }
    }
}

/*
 *  ModInformation::InstallNecessaryFiles
 *      Installs / Reinstalls anything that has the status Added or Updated
 *      This is usually called after a Scan and an UninstallNecessaryFiles
 */
void Loader::ModInformation::InstallNecessaryFiles()
{
    if(this->IsIgnored() == false)
    {
        Updating xup;
        Log(this->files.size()? "Updating state for \"%s\"..." : "No files in \"%s\"...", this->path.c_str());

        // Helper closure... Installs taking care of other installed priorities.
        auto TryInstall = [](FileInformation& file)
        {
            FileInformation* installed;
            if(file.handler
            && (installed = file.handler->FindFileWithBehaviour(file.behaviour))
            && (SimplePriorityPred<ModInformation>()(file.parent, installed->parent) == true))
            {
                // Don't install, the currently installed file has priority over this one
            }
            else
            {
                // Install, either there's no file installed with this behaviour or this file has
                // priority over the installed one
                file.Install();
            }
        };

        for (auto it = this->files.begin(); it != this->files.end(); ++it)
        {
            FileInformation& file = it->second;

            if(file.status == Status::Added || file.status == Status::Updated)
            {
                if(file.installed)
                    file.Reinstall();
                else
                    TryInstall(file);
            }
            else if(file.status == Status::Unchanged)
            {
                if(file.handler && !file.installed)
                    TryInstall(file);
            }
        }
    }
}

/*
 *  ModInformation::UpdateIgnoreStatus
 *      Updates the state of 'this->ignore' depending upon the parent folder ignore list.
 */
Loader::ModInformation& Loader::ModInformation::UpdateIgnoreStatus()
{
    this->ignored = parent.Profile().IsIgnored(this->GetName());
    return *this;
}

/*
 *  ModInformation::UpdatePriority
 *      Updates the state of 'this->priority' depending upon the parent folder priority list.
 *      Returns true if the priority changed or false otherwise.
 */
bool Loader::ModInformation::UpdatePriority()
{
    auto priority = parent.Profile().GetPriority(this->name);
    if(this->priority != priority)
    {
        this->priority = priority;
        return true;
    }
    return false;
}







/*
 *  FileInformation::Update
 *      Updates the state of this file based on the newer @m
 *      Returns whether the file changed since the last update
 */
bool Loader::FileInformation::Update(const modloader::file& m)
{
    if (this->has_changed(m))
    {
        this->time = m.time;
        this->size = m.size;

        // Behaviour cannot change between updates!!!!
        if (this->behaviour != m.behaviour)
        {
            Error("Behaviour of file \"%s\" changed during update.", this->filepath());
            return false;
        }
        return true;
    }
    return false;
}


/*
 *  FileInformation::Install
 *      Installs this file into the game
 */
bool Loader::FileInformation::Install()
{
    Updating xup;
    FileInstallLog xlog(*this, "Install");
    
    // Make sure file is not installed, otherwise something is very wrong
    if(this->installed)
        FatalError("Install failed because file is already installed");
    
    // Install with the main handler then with callme handlers
    if((this->installed = this->handler? handler->Install(*this) : true))
    {
        for(auto& p : this->callme) p.get().Install(*this);
    }
    
    // Refresh state
    this->status = Status::Unchanged;
    return this->installed;
}

/*
 *  FileInformation::Reinstall
 *      Reinstalls this file previosly installed but updated
 */
bool Loader::FileInformation::Reinstall()
{
    if(this->installed)
    {
        Updating xup;
        FileInstallLog xlog(*this, "Reinstall");
        
        // Reinstall with the main handler then with callme handlers
        if((this->installed = this->handler? handler->Reinstall(*this) : true))
        {
            for(auto& p : this->callme) p.get().Reinstall(*this);
        }
    }
    else
    {
        Log("Warning: Reinstall called with non installed file, calling Install...");
        return this->Install();
    }
        
    // Refresh state
    this->status = Status::Unchanged;
    return this->installed;
}

/*
 *  FileInformation::Uninstall
 *      Uninstalls this file previosly installed
 */
bool Loader::FileInformation::Uninstall()
{
    if(this->installed)
    {
        Updating xup;
        FileInstallLog xlog(*this, "Uninstall", true);
        
        // Uninstall with the main handler then with callme handlers
        if((this->installed = this->handler? !handler->Uninstall(*this) : false) == false)
        {
            for(auto& p : this->callme) p.get().Uninstall(*this);
        }
    }
    
    // Refresh state
    this->status = Status::Unchanged;
    return !this->installed;
}
