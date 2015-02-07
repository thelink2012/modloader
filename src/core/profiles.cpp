/*
 * Copyright (C) 2013-2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#include <stdinc.hpp>
#include "loader.hpp"
using namespace modloader;

// TODO GLOBS
// TODO GLOBS IN MODS LIST?

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
 *  Profile::~Profile
 */
Loader::Profile::~Profile()
{
    parent.RemoveReferencesToProfile(*this);
}

/*
 *  Profile::Clear
 *      Clears this object
 */
void Loader::Profile::Clear()
{
    this->bIgnoreAll.first = false;
    this->bExcludeAll.first = false;
    this->mods_priority.clear();
    this->include_mods.clear();
    this->ignore_files.clear();
    this->use_if_module.clear();
    this->ClearInheritance();
    this->RebuildExcludeFilesGlob();
    this->RebuildIncludeModsGlob();
}

/*
 *  Profile::CallHierarchy
 *      Calls the function @fun on me and on my inherited profiles.
 *      If the function fun returns @stop_if the iteration stops.
 *      If no funtion fun returns 'stop_if then @default_return' is returned
 */
bool Loader::Profile::CallHierarchy(bool stop_if, bool default_return, std::function<bool(const Profile&)> fun) const
{
    if(fun(*this) != stop_if)
    {
        for(auto& x : this->inherits)
            if(fun(*x) == stop_if) return stop_if;
        return default_return;
    }
    return stop_if;
}

/*
 *  Profile::AddInheritance
 *      Adds the profile @profile to be inherited from.
 *      Modifies inherits_str if @modify_str = true
 *      The function checks for circular dependency and other invalid inheritance issues.
 */
void Loader::Profile::AddInheritance(Profile& profile, bool modify_str)
{
    if(this != &profile)
    {
        if(profile.IsInheritedFrom(*this))
            loader.Log("Warning: Profile \"%s\" has circular dependency.", this->GetName().c_str());
        else
        {
            this->inherits.emplace(&profile);
            if(modify_str)
            {
                auto name = profile.GetName();
                this->inherits_str.emplace(tolower(name));
            }
        }
    }
    else
        loader.Log("Warning: Profile \"%s\" references itself in inheritance.",this->GetName().c_str());
}


/*
 *  Profile::AddInheritanceToCurrent
 *      Adds the current selected user profile (from the ini) to be inherited from.
 *      Should call UpdateInheritance after this call.
 */
void Loader::Profile::AddInheritanceToCurrent()
{
    this->inherits_str.emplace("$current");
}

/*
 *  Profile::RemoveInheritance
 *      Removes all the references to the specified profile
 */
void Loader::Profile::RemoveInheritance(const Profile& profile, bool modify_str)
{
    auto name = profile.GetName();
    this->inherits.erase(const_cast<Profile*>(&profile));
    if(modify_str) this->inherits_str.erase(tolower(name));
}

/*
 *  Profile::ClearInheritance
 *      Removes all the inheritance hierarchy.
 *      Modifies inherits_str if @modify_str = true
 */
void Loader::Profile::ClearInheritance(bool modify_str)
{
    this->inherits.clear();
    if(modify_str) this->inherits_str.clear();
}

/*
 *  Profile::UpdateInheritance
 *      Updates my hierarchy based on this->inherits_str
 */
void Loader::Profile::UpdateInheritance()
{
    this->ClearInheritance(false);

    for(auto& profname : this->inherits_str)
    {
        if(profname.size() && profname[0] == '$')
        {
            if(profname == "$none")
            {
                this->ClearInheritance(false);
                break;
            }
            else if(profname == "$current")
                this->AddInheritance(parent.Profile(), false);
            else
                loader.Log("Warning: Invalid special parent ('$')");
        }
        else
        {
            if(auto* profptr = parent.FindProfile(profname))
                this->AddInheritance(*profptr, false);
        }
    }
}

/*
 *  Profile::IsInheritedFrom
 *      Checks if we inherit the specified profile in all levels of indirection
 */
bool Loader::Profile::IsInheritedFrom(const Profile& profile) const
{
    for(auto& prof : this->inherits)
    {
        if(prof == &profile || prof->IsInheritedFrom(profile))
            return true;
    }
    return false;
}

/*
 *  Profile::IsIgnored
 *      Checks if the specified mod @name (normalized) should be ignored
 */
bool Loader::Profile::IsIgnored(const std::string& name) const
{
    return (this->IsIgnoredNoExclusive(name) || this->IsExcluded(name));
}

/*
 *  Profile::IsExclusiveToMe
 *      Checks if the mod @name (normalized) is exclusive to this profile
 *      Does not check inherited members!!!!
 */
bool Loader::Profile::IsExclusiveToMe(const std::string& name) const
{
    return exclusive_mods.count(name) > 0;
}

/*
 *  Profile::IsIgnoredNoExclusive
 *      Checks if the specified mod @name (normalized) should be ignored.
 *      This version doesn't check in the exclusive list
 */
bool Loader::Profile::IsIgnoredNoExclusive(const std::string& name) const
{
    return this->CallHierarchy(true, [&name](const Profile& profile)
    {
        // If excluse all is under effect, check if we are included, otherwise check if we are excluded.
        if(profile.IsExcluding())
        {
            return (MatchGlob(name, profile.include_mods_glob) == false);
        }
        else
        {
            if(profile.IsOnIgnoringList(name) == false) // Check if not on ignore mods list
            {
                auto it = profile.mods_priority.find(name);
                return (it != profile.mods_priority.end() && it->second == 0);  // If priority is 0 ignore this mod
            }
            else
                return true;    // it's on the ignore list, ignore.
        }
    });
}

/*
 *  Profile::IsExclusive
 *      Checks if the mod @name (normalized) is excluded from this profile (that's other profile has it as exclusive)
 *      If other profiles has the same exclusive mod as us, ours are used. 
 */
bool Loader::Profile::IsExcluded(const std::string& name) const
{
    if(!this->IsExclusiveToMe(name))
    {
        bool found_exclusivity = false;
        for(const Profile& prof : this->parent.Profiles())
        {
            if(this != &prof && prof.IsExclusiveToMe(name))
            {
                found_exclusivity = true;
                if(this->IsInheritedFrom(prof))
                    return false;
            }
        }
        return found_exclusivity;
    }
    return false;
}


/*
 *  Profile::IsFileIgnored
 *      Checks if the specified file @name (normalized) should be ignored
 */
bool Loader::Profile::IsFileIgnored(const std::string& name) const
{
    return this->CallHierarchy(true, [&name](const Profile& profile) {
        if(MatchGlob(name, profile.ignore_files_glob)) return true;
        return false;
    });
}

/*
 *  Profile::SetPriority
 *      Sets the @priority for the next mods named @name (normalized) over here
 */
void Loader::Profile::SetPriority(std::string name, int priority)
{
    if(priority == default_priority)
        mods_priority.erase(name);
    else
        mods_priority[name] = std::max(std::min(priority, 100), 0); // clamp to 0-100
}

/*
 *  Profile::GetPriority
 *      Gets the @priority for the mod named @name (normalized)
 */
int Loader::Profile::GetPriority(const std::string& name) const
{
    int priority = default_priority;
    if(this->CallHierarchy(true, [&name, &priority](const Profile& profile)
    {
        auto it = profile.mods_priority.find(name);
        if(it != profile.mods_priority.end())
        {
            priority = it->second;
            return true;
        }
        return false;
    }))
        return priority;
    return default_priority;
}

/*
 *  Profile::IsOnIgnoringList
 *      Checks if the mod @name (normalized) is on the ignore list of this profile or it's parents.
 */
bool Loader::Profile::IsOnIgnoringList(const std::string& name) const
{
    return this->CallHierarchy(true, [&name](const Profile& profile) {
        return profile.ignore_mods.count(name) > 0;
    });
}

/*
 *  Profile::IsOnIncludingList
 *      Checks if the mod @name (normalized) is on the inclusion list of this profile or it's parents.
 */
bool Loader::Profile::IsOnIncludingList(const std::string& name) const
{
    return this->CallHierarchy(true, [&name](const Profile& profile) {
        return profile.include_mods.count(name) > 0;
    });
}

/*
*   FolderInformation::Include
*       Adds a file @name (normalized) to the inclusion list, to be included even if ExcludeAllMods=true
*/
void Loader::Profile::Include(std::string name)
{
    include_mods.emplace(std::move(name));
    RebuildIncludeModsGlob();
}

/*
 *  Profile::Uninclude
 *      Removes the file @name (normalized) from the inclusion list
 */
void Loader::Profile::Uninclude(const std::string& name)
{
    include_mods.erase(name);
    RebuildIncludeModsGlob();
}

/*
 *  Profile::AddExclusivity
 *      Adds exclusivity for the mod @name (normalized) to this profile
 */
void Loader::Profile::AddExclusivity(const std::string& mod)
{
    exclusive_mods.emplace(mod);
}

/*
 *  Profile::RemExclusivity
 *      Removes exclusivity for the mod @name (normalized) on this profile
 */
void Loader::Profile::RemExclusivity(const std::string& mod)
{
    exclusive_mods.erase(mod);
}

/*
 *  Profile::IgnoreFileGlob
 *      Adds a file @glob (normalized) to be ignored
 */
void Loader::Profile::IgnoreFileGlob(std::string glob)
{
    ignore_files.emplace(std::move(glob));
    RebuildExcludeFilesGlob();
}

/*
 *  Profile::IgnoreMod
 *      Adds a mod @mod (normalized) to be ignored
 */
void Loader::Profile::IgnoreMod(std::string mod)
{
    ignore_mods.emplace(std::move(mod));
}

/*
 *  Profile::UnignoreMod
 *      Removes the mod @mod (normalized) from the ignored list
 */
void Loader::Profile::UnignoreMod(const std::string& mod)
{
    ignore_mods.erase(mod);
}

/*
 *  Profile::RebuildExcludeFilesGlob  - Rebuilds glob for files exclusion
 *  Profile::RebuildIncludeModsGlob   - Rebuilds glob for files inclusion
 */

void Loader::Profile::RebuildExcludeFilesGlob()
{
    BuildGlobString(this->ignore_files, this->ignore_files_glob);
}

void Loader::Profile::RebuildIncludeModsGlob()
{
    BuildGlobString(this->include_mods, this->include_mods_glob);
}


/*
 *  Profile::SetIgnoreAll     - Ignores all mods 
 *  Profile::SetExcludeAll    - Excludes all mods except the ones being included ([IncludeMods])
 *  Profile::IsIgnoringAll    - Checks if this profile or it's parent is ignoring all mods.
 *  Profile::IsExcludingAll   - Checks if this profile or it's parent are excluding all mods.
 */

void Loader::Profile::SetIgnoreAll(bool bSet)
{
    this->bIgnoreAll.first  = true;
    this->bIgnoreAll.second = bSet;
}

void Loader::Profile::SetExcludeAll(bool bSet)
{
    this->bExcludeAll.first  = true;
    this->bExcludeAll.second = bSet;
}

bool Loader::Profile::IsIgnoringAll() const
{
    if(!this->bIgnoreAll.first)
    {
        return this->CallHierarchy(true, [this](const Profile& profile) {
            if(this != &profile) return profile.IsIgnoringAll();
            return false;
        });
    }
    return this->bIgnoreAll.second;

}

bool Loader::Profile::IsExcludingAll() const
{
    if(!this->bExcludeAll.first)
    {
        return this->CallHierarchy(true, [this](const Profile& profile) {
            if(this != &profile) return profile.IsExcludingAll();
            return false;
        });
    }
    return this->bExcludeAll.second;
}

/*
 *  Profile::GetProfileComps
 *      Gets the three components of a profile section separated by dots or a empty vector if not a profile section.
 */
std::vector<std::string> Loader::Profile::GetProfileComps(const std::string& ini_sect)
{
    auto comps = modloader::split(ini_sect, '.');
    if(comps.size() == 3 && comps[0] == "Profiles")
        return comps;
    return std::vector<std::string>();
}

/*
 *  Profile::LoadConfigFromINI
 *      Loads configuration specific to this profile from the specified ini file
 */
void Loader::Profile::LoadConfigFromINI(const modloader_ini& ini)
{
    // Reads the top [Profiles.ProfileName.Config] section
    auto ReadConfig = [this](const linb::ini::key_container& kv)
    {
        // NOTE: Assumes Profile object is clear!!!!!!!!!!!!!!!!!!!!!!
        for(auto& pair : kv)
        {
            if(!compare(pair.first, "IgnoreAllFiles", false)
            || !compare(pair.first, "IgnoreAllMods", false))
                this->SetIgnoreAll(to_bool(pair.second));
            else if(!compare(pair.first, "ExcludeAllMods", false))
                this->SetExcludeAll(to_bool(pair.second));
            else if(!compare(pair.first, "Parents", false))
            {
                for(auto parent : modloader::split(pair.second, ','))
                {
                    if(trim(parent).size())
                        this->inherits_str.emplace(modloader::tolower(parent));
                }
            }
            else if(!compare(pair.first, "UseIfModule", false))
            {
                this->use_if_module = pair.second;
            }
        }
    };

    // Reads the [Profiles.ProfileName.Priority] section
    auto ReadPriorities = [this](const modloader_ini::key_container& kv)
    {
        this->mods_priority.clear();
        for(auto& pair : kv) this->SetPriority(NormalizePath(pair.first), std::strtol(pair.second.c_str(), 0, 0));
    };

    // Reads the [Profiles.ProfileName.IgnoreMods] section
    auto ReadIgnoreMods = [this](const modloader_ini::key_container& kv)
    {
        this->ignore_mods.clear();
        for(auto& pair : kv) this->IgnoreMod(NormalizePath(pair.first));
    };

    // Reads the [Profiles.ProfileName.IgnoreFiles] section
    auto ReadIgnoreFiles = [this](const modloader_ini::key_container& kv)
    {
        this->ignore_files.clear();
        for(auto& pair : kv) this->IgnoreFileGlob(NormalizePath(pair.first));
    };

    // Reads the [Profiles.ProfileName.IncludeMods] section
    auto ReadIncludeMods = [this](const modloader_ini::key_container& kv)
    {
        this->include_mods.clear();
        for(auto& pair : kv) this->Include(NormalizePath(pair.first));
    };

    // Reads the [Profiles.ProfileName.ExclusiveMods] section
    auto ReadExclusiveMods = [this](const modloader_ini::key_container& kv)
    {
        this->exclusive_mods.clear();
        for(auto& pair : kv) this->AddExclusivity(NormalizePath(pair.first));
    };

    for(auto& section : ini)
    {
        auto comps = GetProfileComps(section.first);
        if(comps.size() && (*this == comps[1]))
        {
            auto& type = comps[2];
            if(type == "Config") ReadConfig(section.second);
            else if(type == "Priority") ReadPriorities(section.second);
            else if(type == "IgnoreMods") ReadIgnoreMods(section.second);
            else if(type == "IgnoreFiles") ReadIgnoreFiles(section.second);
            else if(type == "IncludeMods") ReadIncludeMods(section.second);
            else if(type == "ExclusiveMods") ReadExclusiveMods(section.second);
        }
    }
}

/*
 *  Profile::SaveConfigForINI
 *      Saves configuration specific to this profile to the specified ini file
 */
void Loader::Profile::SaveConfigForINI(modloader_ini& ini)
{
    auto Section = [this](const std::string& name) {
        return "Profiles." + this->GetName() + '.' + name;
    };

    auto& config        = ini[Section("Config")];
    auto& priority      = ini[Section("Priority")];
    auto& ignoremods    = ini[Section("IgnoreMods")];
    auto& ignorefiles   = ini[Section("IgnoreFiles")];
    auto& includemods   = ini[Section("IncludeMods")];
    auto& exclusivemods = ini[Section("ExclusiveMods")];
    
    if(this->bIgnoreAll.first)
        config["IgnoreAllMods"] = modloader::to_string(this->bIgnoreAll.second);
    if(this->bExcludeAll.first)
        config["ExcludeAllMods"] = modloader::to_string(this->bExcludeAll.second);
    
    auto& parents_entry = config["Parents"];

    for(auto& pair : this->mods_priority)
        priority[pair.first] = std::to_string(pair.second);

    for(auto& mod : this->ignore_mods)
        ignoremods[mod];

    for(auto& file : this->ignore_files)
        ignorefiles[file];

    for(auto& mod : this->include_mods)
        includemods[mod];

    for(auto& mod : this->exclusive_mods)
        exclusivemods[mod];

    if(this->inherits.empty())
        parents_entry = "$None";
    else for(auto it = this->inherits_str.begin(); it != this->inherits_str.end(); ++it)
    {
        // Use original case if possible instead of pure lowercase
        auto it_dv = std::find_if(this->inherits.begin(), this->inherits.end(), [&](const Profile* parent) {
            return *parent == *it;
        });
        parents_entry += (it_dv == this->inherits.end()? *it : (*it_dv)->GetName());
        if(std::next(it) != this->inherits_str.end()) parents_entry.append(", ");
    }

    if(this->use_if_module.size())
        config["UseIfModule"] = this->use_if_module;
}

/*
 *  Profile::GetListOfProfilesInIni
 *      Searches a ini for profiles
 */
std::set<std::string> Loader::Profile::GetListOfProfilesInIni(const modloader_ini& ini)
{
    std::set<std::string> list;
    for(auto& section : ini)
    {
        auto comps = modloader::split(section.first, '.');
        if(comps.size() == 3 && comps[0] == "Profiles" &&   // Has three components dot delimited and first component is 'Profiles'
          (comps[1].size() && comps[1].front() != '$'))     // Has no special character
            list.emplace(comps[1]);
    }
    return list;
}

/*
 *  ModLoaderIniSectionPred
 *      Sorts ini sections
 */
bool ModLoaderIniSectionPred::operator()(const std::string& a, const std::string& b) const
{
    static const std::vector<std::string> sortby = {
        "Config", "Priority", "IgnoreFiles", "IgnoreMods", "IncludeMods", "ExclusiveMods",
    };

    // Folder.Config should be at the top
    if(a == "Folder.Config" || b == "Folder.Config")
        return (a == b? false : a == "Folder.Config");

    auto GetIndiceForPred = [](const std::string& key)
    {
        using pair_type = std::pair<std::string, int>;
        auto comps = Loader::Profile::GetProfileComps(key);
        if(comps.size())
        {
            auto it = std::find(sortby.begin(), sortby.end(), comps[2]);
            if(it != sortby.end())
                return pair_type(std::move(comps[1]), std::distance(sortby.begin(), it));
            return pair_type(key, (std::numeric_limits<int>::max)());   // unknown profile section wut
        }
        return pair_type(key, (std::numeric_limits<int>::min)());   // non profile sections should come first
    };

    return GetIndiceForPred(a) < GetIndiceForPred(b);
}
