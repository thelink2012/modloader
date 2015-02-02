/*
 * Copyright (C) 2013-2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#include <stdinc.hpp>
#include "loader.hpp"
using namespace modloader;


template<class L>
static void BuildGlobString(const L& list, std::string& glob)
{
    glob.clear();
    for(auto& g : list) glob.append(g).push_back(';');
}

static bool MatchGlob(const std::string& name, const std::string& glob)
{
    return !glob.empty() && PathMatchSpecA(name.c_str(), glob.c_str()) != 0;
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
    ignore_files.clear();
    RebuildExcludeFilesGlob();
    RebuildIncludeModsGlob();
}

/*
 *  FolderInformation::IsIgnored
 *      Checks if the specified mod @name (normalized) should be ignored
 *      This doesn't check parents IsIgnored
 */
bool Loader::FolderInformation::IsIgnored(const std::string& name)
{
    // If excluse all is under effect, check if we are included, otherwise check if we are excluded.
    if(this->IsExcluding())
        return (MatchGlob(name, include_mods_glob) == false);
    else
    {
        if(this->IsOnIgnoringList(name) == false) // Check if not on ignore mods list
        {
            auto it = mods_priority.find(name);
            return (it != mods_priority.end() && it->second == 0);  // If priority is 0 ignore this mod
        }
        else return true;
    }
}

/*
 *  FolderInformation::IsFileIgnored
 *      Checks if the specified file @name (normalized) should be ignored
 *      This also checks on parents
 */
bool Loader::FolderInformation::IsFileIgnored(const std::string& name)
{
    if(MatchGlob(name, ignore_files_glob)) return true;
    return (this->parent? parent->IsFileIgnored(name) : false);
}


/*
 *  FolderInformation::AddChild
 *      Adds or finds a child FolderInformation at @path (normalized) into this
 */
auto Loader::FolderInformation::AddChild(const std::string& path) -> FolderInformation&
{
    auto ipair = childs.emplace(std::piecewise_construct,
                        std::forward_as_tuple(path),
                        std::forward_as_tuple(path, this));
    
    return ipair.first->second;
}

/*
 *  FolderInformation::AddMod
 *      Adds or finds mod at folder @name (non-normalized) into this FolderInformation
 */
auto Loader::FolderInformation::AddMod(const std::string& name) -> ModInformation&
{
    auto ipair = mods.emplace(std::piecewise_construct,
                        std::forward_as_tuple(NormalizePath(name)),
                        std::forward_as_tuple(name, *this, loader.PickUniqueModId()));
    
    return ipair.first->second;
}

/*
 *  FolderInformation::SetPriority
 *      Sets the @priority for the next mods named @name (normalized) over here
 */
void Loader::FolderInformation::SetPriority(std::string name, int priority)
{
    if(priority == default_priority)
        mods_priority.erase(name);
    else
        mods_priority[name] = priority;
}


/*
 *  FolderInformation::GetPriority
 *      Gets the @priority for the mod named @name (normalized)
 */
int Loader::FolderInformation::GetPriority(const std::string& name)
{
    auto it = mods_priority.find(name);
    return (it == mods_priority.end()? default_priority : it->second);
}

/*
 *  FolderInformation::Include
 *      Adds a file @name (normalized) to the inclusion list, to be included even if ExcludeAllMods=true
 */
void Loader::FolderInformation::Include(std::string name)
{
    include_mods.emplace(std::move(name));
    RebuildIncludeModsGlob();
}

/*
 *  FolderInformation::Uninclude
 *      Removes the file @name (normalized) from the inclusion list
 */
void Loader::FolderInformation::Uninclude(const std::string& name)
{
    include_mods.erase(name);
    RebuildIncludeModsGlob();
}

/*
 *  FolderInformation::IgnoreFileGlob
 *      Adds a file @glob (normalized) to be ignored
 */
void Loader::FolderInformation::IgnoreFileGlob(std::string glob)
{
    ignore_files.emplace(std::move(glob));
    RebuildExcludeFilesGlob();
}

/*
 *  FolderInformation::IgnoreMod
 *      Adds a mod @mod (normalized) to be ignored
 */
void Loader::FolderInformation::IgnoreMod(std::string mod)
{
    ignore_mods.emplace(std::move(mod));
}

/*
 *  FolderInformation::UnignoreMod
 *      Removes the mod @mod (normalized) from the ignored list
 */
void Loader::FolderInformation::UnignoreMod(const std::string& mod)
{
    ignore_mods.erase(mod);
}


/*
 *  FolderInformation::RebuildExcludeFilesGlob  - Rebuilds glob for files exclusion
 *  FolderInformation::RebuildIncludeModsGlob   - Rebuilds glob for files inclusion
 */

void Loader::FolderInformation::RebuildExcludeFilesGlob()
{
    BuildGlobString(this->ignore_files, this->ignore_files_glob);
}

void Loader::FolderInformation::RebuildIncludeModsGlob()
{
    BuildGlobString(this->include_mods, this->include_mods_glob);
}


/*
 *  FolderInformation::SetIgnoreAll     - Ignores all mods 
 *  FolderInformation::SetForceIgnore   - Internal IgnoreAll for -nomods command line
 *  FolderInformation::SetExcludeAll    - Excludes all mods except the ones being included ([IncludeMods])
 *  FolderInformation::SetForceExclude  - Internal ExcludeAll for -mod command line
 * 
 */

void Loader::FolderInformation::SetIgnoreAll(bool bSet)
{
    this->bIgnoreAll = bSet;
}

void Loader::FolderInformation::SetForceIgnore(bool bSet)
{
    this->bForceIgnore = bSet;
}

void Loader::FolderInformation::SetExcludeAll(bool bSet)
{
    this->bExcludeAll = bSet;
}

void Loader::FolderInformation::SetForceExclude(bool bSet)
{
    this->bForceExclude = bSet;
}




/*
 *  FolderInformation::GetAll 
 *      Gets all the childs and subchilds FolderInformation including self
 */
auto Loader::FolderInformation::GetAll() -> ref_list<FolderInformation>
{
    ref_list<FolderInformation> list = { *this };  // self
    for(auto& pair : this->childs)
    {
        auto rest = pair.second.GetAll();
        list.insert(list.end(), rest.begin(), rest.end());  // subchilds
    }
    return list;
}

/*
 *  FolderInformation::GetMods 
 *      Gets my mods in no particular oder
 */
auto Loader::FolderInformation::GetMods() -> ref_list<ModInformation>
{
    return refs_mapped(this->mods);
}

/*
 *  FolderInformation::GetModsByPriority 
 *      Gets my mods ordered by priority
 */
auto Loader::FolderInformation::GetModsByPriority() -> ref_list<ModInformation>
{
    auto list = this->GetMods();
    std::sort(list.begin(), list.end(), PriorityPred<ModInformation>());
    return list;
}

/*
 *  FolderInformation::GetModsByName
 *      Gets my mods ordered by name
 */
auto Loader::FolderInformation::GetModsByName() -> ref_list<ModInformation>
{
    auto list = this->GetMods();
    std::sort(list.begin(), list.end(), [](const ModInformation& a, const ModInformation& b)
    {
        return a.GetPath() < b.GetPath();
    });
    return list;
}


/*
 *  FolderInformation::Scan
 *      Scans mods at this and child folders
 *      This method only scans, to update using the scanned information, call Update()
 */
void Loader::FolderInformation::Scan()
{
    ::scoped_gdir xdir(this->path.c_str());
    Log("\n\nScanning mods at \"%s\"...", this->path.c_str());

    bool fine = true;
    
    // Loads the config file only once
    if(!this->gotConfig)
    {
        this->gotConfig = true;
        this->LoadConfigFromINI();
    }

    // > Status here is Status::Unchanged
    // Mark all current mods as removed
    MarkStatus(this->mods, Status::Removed);

    // Walk on this folder to find mods
    if(this->IsIgnoring() == false)
    {
        fine = FilesWalk("", "*.*", false, [this](FileWalkInfo & file)
        {
            if(file.is_dir) this->AddMod(file.filename).Scan();
            return true;
        });
    }
    
    // Find the underlying status of this folder
    UpdateStatus(*this, this->mods, fine);
    
    // Scan on my childs too
    if(this->status != Status::Removed)
    {
        for(auto& pair : this->childs)
        {
            FolderInformation& child = pair.second;
            
            child.Scan();
            if(this->status == Status::Unchanged && child.status != Status::Unchanged)
                this->status = Status::Updated;
        }
    }
}

/*
 *  FolderInformation::Scan (from Journal)
 *      Rescans mods at this folder that are present in the change journal
 *      This method only scans, to update using the scanned information, call Update()
 */
void Loader::FolderInformation::Scan(const Journal& journal)
{
    ::scoped_gdir xdir(this->path.c_str());

    if(this->IsIgnoring() == false)
    {
        for(auto& change : journal)
        {
            if(change.second == Status::Removed)
            {
                auto it = this->mods.find(change.first);
                if(it != this->mods.end()) it->second.status = Status::Removed;
            }
            else if(change.second == Status::Added
                 || change.second == Status::Updated)
            {
                if(IsDirectoryA(change.first.c_str()))  // the journal might contain unrelated files...
                    this->AddMod(change.first).Scan();
            }
        }
    }

    UpdateStatus(*this, this->mods, true);
}

/*
 *  FolderInformation::Update
 *      Updates the state of all mods.
 *      This is normally called after Scan()
 */
void Loader::FolderInformation::Update()
{
    if(this->status != Status::Unchanged)
    {
        Updating xup;
        Log("\nUpdating mods for \"%s\"...", this->path.c_str());

        auto mods = this->GetModsByPriority();

        // Uninstall all removed files since the last update...
        for(ModInformation& mod : mods)
        {
            mod.ExtinguishNecessaryFiles();
        }

        // Install all updated and added files since the last update...
        for(ModInformation& mod : mods)
        {
            mod.InstallNecessaryFiles();
            mod.SetUnchanged();
        }

        // Update my childs
        for(auto& child : this->childs)
        {
            child.second.Update();
        }

        // Collect garbaged data (mods and childs that are unused atm)
        CollectInformation(this->mods);
        CollectInformation(this->childs);
        this->SetUnchanged();
    }
}

/*
 *  FolderInformation::Update
 *      Updates the state of the specified @mod
 */
void Loader::FolderInformation::Update(ModInformation& mod)
{
    Updating xup;
    mod.parent.Update();
}


/*
 *  FolderInformation::LoadConfigFromINI
 *      Loads configuration specific to this folder from the specified ini file
 */
void Loader::FolderInformation::LoadConfigFromINI(const std::string& inifile)
{
    linb::ini cfg;
    CopyFileA(loader.folderConfigDefault.c_str(), inifile.c_str(), TRUE);

    // Reads the top [Config] section
    auto ReadConfig = [this](const linb::ini::key_container& kv)
    {
        for(auto& pair : kv)
        {
            if(!compare(pair.first, "IgnoreAllFiles", false))
                this->SetIgnoreAll(to_bool(pair.second));
            else if(!compare(pair.first, "ExcludeAllMods", false))
                this->SetExcludeAll(to_bool(pair.second));
        }
    };

    // Reads the [Priority] section
    auto ReadPriorities = [this](const linb::ini::key_container& kv)
    {
        this->mods_priority.clear();
        for(auto& pair : kv) this->SetPriority(NormalizePath(pair.first), std::strtol(pair.second.c_str(), 0, 0));
    };

    // Reads the [IgnoreMods] section
    auto ReadIgnoreMods = [this](const linb::ini::key_container& kv)
    {
        this->ignore_mods.clear();
        for(auto& pair : kv) this->IgnoreMod(NormalizePath(pair.first));
    };

    // Reads the [IgnoreFiles] section
    auto ReadIgnoreFiles = [this](const linb::ini::key_container& kv)
    {
        this->ignore_files.clear();
        for(auto& pair : kv) this->IgnoreFileGlob(NormalizePath(pair.first));
    };

    // Reads the [IncludeMods] section
    auto ReadIncludeMods = [this](const linb::ini::key_container& kv)
    {
        this->include_mods.clear();
        for(auto& pair : kv) this->Include(NormalizePath(pair.first));
    };

    // Load the ini and readddddddddddddd
    if(cfg.load_file(inifile))
    {
        ReadConfig(cfg["Config"]);
        ReadPriorities(cfg["Priority"]);
        ReadIgnoreMods(cfg["IgnoreMods"]);
        ReadIgnoreFiles(cfg["IgnoreFiles"]);
        ReadIncludeMods(cfg["IncludeMods"]);
        
    }
    else
        Log("Failed to load folder config file");

}

/*
 *  FolderInformation::SaveConfigForINI
 *      Saves configuration specific to this folder to the specified ini file
 */
void Loader::FolderInformation::SaveConfigForINI(const std::string& inifile)
{
    linb::ini ini;
    auto& config      = ini["Config"];
    auto& priority    = ini["Priority"];
    auto& ignoremods  = ini["IgnoreMods"];
    auto& ignorefiles = ini["IgnoreFiles"];
    auto& includemods = ini["IncludeMods"];
    
    config["IgnoreAllFiles"] = modloader::to_string(this->IsIgnoringAll());
    config["ExcludeAllMods"] = modloader::to_string(this->IsExcludingAll());

    for(auto& pair : this->mods_priority)
        priority[pair.first] = std::to_string(pair.second);

    for(auto& mod : this->ignore_mods)
        ignoremods[mod];

    for(auto& file : this->ignore_files)
        ignorefiles[file];

    for(auto& mod : this->include_mods)
        includemods[mod];

    if(!ini.write_file(inifile))
        Log("Failed to save folder config file");
}

void Loader::FolderInformation::SaveConfigForINI()
{
    ::scoped_gdir xdir(this->path.c_str());
    return this->SaveConfigForINI(loader.folderConfigFilename);
}

void Loader::FolderInformation::LoadConfigFromINI()
{
    ::scoped_gdir xdir(this->path.c_str());
    return this->LoadConfigFromINI(loader.folderConfigFilename);
}
