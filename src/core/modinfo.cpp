/*
 * Copyright (C) 2013-2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
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
        loader.Log("%sing file \"%s\"", action, file.FileBuffer());
    }
    
    ~FileInstallLog()
    {
        if(file.IsInstalled() == revaction)
            loader.Log("Failed to %s file \"%s\"", action, file.FileBuffer());
    }
};


/*
 *  ModInformation::Scan
 *      Scans this mod in search of files added, updated and removed
 *      This method just searches, it doesn't install or uninstall anything, to install call parent->Update()
 */
void Loader::ModInformation::Scan()
{
    scoped_gdir xdir(this->path.c_str());
    Log("\nScanning files at \"%s\"...", this->path.c_str());
    
    static auto modloader_subfolder = NormalizePath("modloader");
    
    // > Status here is Unchanged
    // Mark all current files as removed
    MarkStatus(this->files, Status::Removed);

    // Scan the directory checking out all files
    bool fine = FilesWalk("", "*.*", true, [this](FileWalkInfo& file)
    {
        auto filename = NormalizePath(file.filename);
        auto filepath = this->path + NormalizePath(file.filebuf);

        // Sub Mod Loader folder...
        if(file.is_dir && filename == modloader_subfolder)
        {
            Log("Found sub modloader at \"%s\"", filepath.c_str());
            this->parent.AddChild(filepath);
            file.recursive = false;
        }
        else if(!parent.IsFileIgnored(filename))
        {
            uint64_t uid;
            modloader::file m;
            ref_list<PluginInformation> callme;
            PluginInformation* handler;

            // This buffer setup is tricky but should work fine
            m.buffer       = filepath.data();
            m.pos_eos      = filepath.length();
            m.pos_filepath = this->path.length();
            m.pos_filename = m.pos_filepath + (file.filename - file.filebuf);
            m.pos_filext   = m.pos_filepath + (file.filext - file.filebuf);
            
            // Setup other information
            m.flags   = (std::underlying_type<FileFlags>::type)(file.is_dir? FileFlags::IsDirectory : FileFlags::None);
            m.behaviour = uid = loader.PickUniqueFileId();
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
                                            std::forward_as_tuple(std::move(filename)), 
                                            std::forward_as_tuple(*this, std::move(filepath), m, handler, std::move(callme)));
                
                auto& n = ipair.first->second;

                if(!ipair.second)   // File already exists on the list
                {
                    // If behaviour isn't handled at all, don't let m and n be different!
                    if(m.behaviour == uid) m.behaviour = n.behaviour;
                    
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
}


/*
 *  ModInformation::ExtinguishingNecessaryFiles
 *      Uninstall anything that has the status Removed
 *      This is usually called after a Scan
 */
void Loader::ModInformation::ExtinguishNecessaryFiles()
{
    if(this->status != Status::Unchanged)
    {
        Log("Extinguishing some files from \"%s\"...", this->path.c_str());

        // Remove all files if this mod has been removed
        bool bRemoveAll = (this->status == Status::Removed);
        
        // Finds all removed files in this mod and uninstall them
        for (auto it = this->files.begin(); it != this->files.end(); )
        {
            FileInformation& file = it->second;
            if (bRemoveAll || file.status == Status::Removed)
            {
                // File was removed from the filesystem, uninstall and erase from our internal list
                Log("Extinguishing file \"%s\"", file.FileBuffer());
                if(file.Uninstall()) it = this->files.erase(it);
                else                 ++it;
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
    if(this->status != Status::Unchanged)
    {
        Log("Installing some files for \"%s\"...", this->path.c_str());
        
        for (auto it = this->files.begin(); it != this->files.end(); ++it)
        {
            FileInformation& file = it->second;
            switch (file.status)
            {
                case Status::Added: // File has been added since the last update
                    file.Install(); // TODO think about added here at runtime and priorities
                    break;

                case Status::Updated: // File has been updated since the last update
                    file.Reinstall();
                    break;
            }
        }
    }
}






/*
 *  FileInformation::Update
 *      Updates the state of this file based on the newer @m
 *      Returns whether the file changed since the last update
 */
bool Loader::FileInformation::Update(const modloader::file& m)
{
    if (this->HasFileChanged(m))
    {
        this->time = m.time;
        this->size = m.size;

        // Behaviour cannot change between updates!!!!
        if (this->behaviour != m.behaviour)
        {
            Error("Behaviour of file \"%s\" changed during update.", this->FileBuffer());
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
