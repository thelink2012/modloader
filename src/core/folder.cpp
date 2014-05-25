/*
 * Copyright (C) 2013-2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#include "loader.hpp"
#include <shlwapi.h>

// TODO const std::string

template<class L>
void BuildGlobString(const L& list, std::string& glob)
{
    glob.clear();
    for(auto& g : list) glob.append(g).push_back(';');
}

bool MatchGlob(const std::string& name, const std::string& glob)
{
    return PathMatchSpecA(name.c_str(), glob.c_str()) != 0;
}



/*
 *  FolderInformation::Clear
 *      Clears this object
 */
void Loader::FolderInformation::Clear()
{
    mods.clear();
    childs.clear();
    mods_priority.clear();
    include_mods.clear();
    exclude_files.clear();
    RebuildExcludeFilesGlob();
    RebuildIncludeModsGlob();
}


/*
 *  FolderInformation::Clear
 *      Checks if the specified mod should be ignored
 */
bool Loader::FolderInformation::IsIgnored(std::string name)
{
    // If excluse all is under effect, check if we are included, otherwise check if we are excluded.
    if(flags.bExcludeAll || flags.bForceExclude)
        return (MatchGlob(name, include_mods_glob) == false);
    else
    {
        auto it = mods_priority.find(std::move(name));
        return (it != mods_priority.end() && it->second == 0);
    }
}

// Checks if the specified file should be ignored
bool Loader::FolderInformation::IsFileIgnored(std::string name)
{
    return MatchGlob(name, exclude_files_glob);
}


// Adds a child mod list folder
auto Loader::FolderInformation::AddChild(std::string path) -> FolderInformation&
{
    //childs.emplace(path, this);
    //return childs.back();
    
    auto ipair = childs.emplace(std::piecewise_construct,
                        std::forward_as_tuple(path),
                        std::forward_as_tuple(*this));
    
    return ipair.first->second;
}

// Adds a mod to the container
auto Loader::FolderInformation::AddMod(std::string name) -> ModInformation&
{
    // Acquiere variables
    //mods.emplace_back(std::move(name), *this, loader.PickUniqueModId());
    //return mods.back();
    
    auto ipair = mods.emplace(std::piecewise_construct,
                        std::forward_as_tuple(name),
                        std::forward_as_tuple(name, *this, loader.PickUniqueModId()));
    
    return ipair.first->second;
}

// Adds a priority to a mod. If priority is zero, it ignores the mod.
void Loader::FolderInformation::SetPriority(std::string name, int priority)
{
    mods_priority.emplace(std::move(name), priority);
}

int Loader::FolderInformation::GetPriority(std::string name)
{
    auto it = mods_priority.find(std::move(name));
    return (it == mods_priority.end()? default_priority : it->second);
}

// Adds a mod to the mod inclusion list
void Loader::FolderInformation::Include(std::string name)
{
    include_mods.emplace(std::move(name));
    RebuildIncludeModsGlob();
}

// Makes a specified file ignored
void Loader::FolderInformation::IgnoreFileGlob(std::string glob)
{
    exclude_files.emplace(std::move(glob));
    RebuildExcludeFilesGlob();
}

// Sets flags
void Loader::FolderInformation::SetIgnoreAll(bool bSet)
{
    flags.bIgnoreAll = bSet;
}

void Loader::FolderInformation::SetExcludeAll(bool bSet)
{
    flags.bExcludeAll = bSet;
}

void Loader::FolderInformation::SetForceExclude(bool bSet)
{
    flags.bForceExclude = bSet;
}



// Gets me and my childs in a reference list
auto Loader::FolderInformation::GetAll() -> FolderInformationRefList
{
    FolderInformationRefList list = { *this };
    for(auto& pair : this->childs) list.splice(list.end(), pair.second.GetAll());
    return list;
}







void Loader::FolderInformation::RebuildExcludeFilesGlob()
{
    BuildGlobString(this->exclude_files, this->exclude_files_glob);
}

void Loader::FolderInformation::RebuildIncludeModsGlob()
{
    BuildGlobString(this->include_mods, this->include_mods_glob);
}















void Loader::ModInformation::Scan()
{
    scoped_gdir xdir(this->path.c_str());
    Log("Scanning files at \"%s\"...", this->path.c_str());
    
    // Mark all current files as removed
    MarkStatus(this->files, Status::Removed);

    // Scan the directory checking out all files
    bool fine = FilesWalk("", "*.*", true, [this](FileWalkInfo& file)
    {
        auto filename = NormalizePath(file.filename);

        if(!parent.IsFileIgnored(filename))
        {
            ModLoaderFile m;
            PluginVector callme;
            auto filepath = this->path + NormalizePath(file.filebuf);
            
            // This buffer setup is tricky but should work fine
            m.buffer       = filepath.data();
            m.pos_eos      = filepath.length();
            m.pos_filepath = this->path.length();
            m.pos_filename = m.pos_filepath + (file.filename - file.filebuf);
            m.pos_filext   = m.pos_filepath + (file.filext - file.filebuf);
            
            // Setup other information
            m.flags   = (std::underlying_type<FileFlags>::type)(file.is_dir? FileFlags::IsDirectory : FileFlags::None);
            m.file_id = loader.PickUniqueFileId();
            m.mod_id  = this->id;
            m.size    = file.size;
            m.time    = file.time;
            m._rsv2   = 0;

            // Find a handler for this file
            if(PluginInformation* handler = loader.FindHandlerForFile(m, callme))
            {
                file.recursive = false;     // Avoid FilesWalk recursion
                
                // Push the new file into our list
                auto ipair = files.emplace( std::piecewise_construct,
                                            std::forward_as_tuple(std::move(filename)), 
                                            std::forward_as_tuple(*this, std::move(filepath), m, *handler, std::move(callme)));
                
                auto& n = ipair.first->second;
                
                // Adjust the file status
                if(ipair.second)
                    n.status = Status::Added;
                else
                    n.status = n.HasFileChanged(m)? Status::Updated : Status::Unchanged;
            }
        }
        
        // TODO Check if SubModLoader
        
        return true;
    });
    
    
    // Find the underlying status of this mod
    FindStatus(*this, this->files, fine);
}

auto Loader::FindHandlerForFile(const ModLoaderFile& m, PluginVector& callme) -> PluginInformation*
{
    PluginInformation* handler = nullptr;
    
    for(auto& plugin : this->plugins)
    {
        auto state = plugin.FindHandleStatus(m);
        
        if(state == HandleStatus::Yes)
        {
            // We found a handler, stop the search immediately, don't check for other CallMe's
            handler = &plugin;
            break;
        }
        else if(state == HandleStatus::CallMe)
        {
            // This plugin requests this file to be sent OnInstall for some reason (readme files?)
            callme.emplace_back(&plugin);
        }
    }
    
    return handler;
}



