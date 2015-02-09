/*
 * Copyright (C) 2013-2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#include <stdinc.hpp>
#include "loader.hpp"
using namespace modloader;

/*
 *  FolderInformation::Clear
 *      Clears this object
 */
void Loader::FolderInformation::Clear()
{
    mods.clear();
    profiles.clear();
    current_profile = nullptr;
}

/*
 *  FolderInformation::Profile
 *      Gets the current working profile or temp profile.
 *      ALWAYS access the current profile from this method since it gets a default profile if none is selected and may give a temp/unamed profile.
 */
Loader::Profile& Loader::FolderInformation::Profile()
{
    if(!this->IsUsingAnonymousProfile())
        return this->GetNonAnonProfile();
    return this->GetAnonymousProfile();
}

/*
 *  FolderInformation::NonAnonProfile
 *      Gets the current profile working profile (ignores temp profile)
 */
Loader::Profile& Loader::FolderInformation::GetNonAnonProfile()
{
    if(!this->current_profile)
        this->SwitchToProfile(this->AddProfile(default_profile_name));
    return *this->current_profile;
}

/*
 *  FolderInformation::AddProfile
 *      Adds a new profile named @name or gets a existing one with the same name.
 */
Loader::Profile& Loader::FolderInformation::AddProfile(std::string name)
{
    if(auto* prof = this->FindProfile(name))
        return *prof;
    return (*this->profiles.emplace(this->profiles.end(), *this, std::move(name)));
}

/*
 *  FolderInformation::FindProfile
 *      Finds the profile named @name and returns a pointer to it otherwise returns nullptr.
 */
Loader::Profile* Loader::FolderInformation::FindProfile(const std::string& name)
{
    auto it = std::find_if(this->profiles.begin(), this->profiles.end(), [&](const Loader::Profile& profile) {
        return (profile == name);
    });
    return (it != this->profiles.end()? &(*it) : nullptr);
}

/*
 *  FolderInformation::FindProfile
 *      Finds the profile pointed by @prof_to_find and returns a pointer to it otherwise returns nullptr.
 *      (only works if the profile is registered)
 */
Loader::Profile* Loader::FolderInformation::FindProfile(const Loader::Profile& prof_to_find)
{
    auto it = std::find_if(this->profiles.begin(), this->profiles.end(), [&](const Loader::Profile& profile) {
        return (&profile == &prof_to_find);
    });
    return (it != this->profiles.end()? &(*it) : nullptr);
}

/*
 *  FolderInformation::SwitchToProfile
 *      Switches to the specified profile (only works if the profile is registered)
 */
bool Loader::FolderInformation::SwitchToProfile(Loader::Profile& prof)
{
    if(this->FindProfile(prof))
    {
        this->current_profile = &prof;
        if(this->current_profile && !this->IsUsingAnonymousProfile())
            loader.Log("Using profile named \"%s\".", prof.GetName().c_str());
        return true;
    }
    return false;
}

/*
 *  FolderInformation::SwitchToProfileAsAnonymous
 *      Switches to the specified profile but as an anonymous copy
 */
bool Loader::FolderInformation::SwitchToProfileAsAnonymous(Loader::Profile& prof)
{
    if(this->FindProfile(prof))
    {
        this->SetAnonymousProfile(prof);
        return true;
    }
    return false;
}

/*
 *  FolderInformation::RemoveReferencesToProfile
 *      Removes all references in existing profiles 'n stuff about the profile @rm
 */
void Loader::FolderInformation::RemoveReferencesToProfile(Loader::Profile& rm)
{
    for(auto& prof : this->profiles)
        prof.RemoveReferences(rm);
    if(this->current_profile == &rm)
        this->current_profile = nullptr;
    
    // Should not do this on the anon profile! Would cause a circular destructor!
}

/*
 *  FolderInformation::RemoveProfiles
 *      Removes all the named profiles
 */
void Loader::FolderInformation::RemoveProfiles()
{
    for(auto it = this->profiles.begin(); it != this->profiles.end(); )
        it = this->profiles.erase(it);  // Profile destructor automatically cleans this->current_profile
}

/*
 *  FolderInformation::Profiles
 *      Gets a list of the available profiles
 */
ref_list<Loader::Profile> Loader::FolderInformation::Profiles()
{
    return refs(this->profiles);
}

/*
 *  FolderInformation::SetAnonymousProfile
 *      Sets the anonymous profile to use
 */
void Loader::FolderInformation::SetAnonymousProfile(const Loader::Profile& profile, bool replace)
{
    if(!this->IsUsingAnonymousProfile() || replace)
    {
        this->anon_profile.reset(new Loader::Profile(profile));
        loader.Log("Using anonymous profile.");
    }
}

/*
 *  FolderInformation::RemAnonymousProfile
 *      Removes the working anonymous profile or does nothing if none is set
 */
void Loader::FolderInformation::RemAnonymousProfile()
{
    if(this->IsUsingAnonymousProfile())
    {
        this->anon_profile.reset();
        loader.Log("Not using anonymous profile anymore.");
    }
}

/*
 *  FolderInformation::GetAnonymousProfile
 *      Gets the current temporary profile
 */
Loader::Profile& Loader::FolderInformation::GetAnonymousProfile()
{
    if(this->IsUsingAnonymousProfile())
        return *this->anon_profile;
    throw std::logic_error("Called GetAnonymousProfile while no anonymous profile is set");
}

/*
 *  FolderInformation::IsUsingAnonymousProfile
 *      Checks if is using a temp/unamed profile
 */
bool Loader::FolderInformation::IsUsingAnonymousProfile() const
{
    return this->anon_profile != nullptr;
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
    
    // > Status here is Status::Unchanged
    // Mark all current mods as removed
    MarkStatus(this->mods, Status::Removed);

    // Walk on this folder to find mods
    if(this->Profile().IsIgnoring() == false)
    {
        fine = FilesWalk("", "*.*", false, [this](FileWalkInfo & file)
        {
            if(file.is_dir) this->AddMod(file.filename).Scan();
            return true;
        });
    }
    
    // Find the underlying status of this folder
    UpdateStatus(*this, this->mods, fine);
}

/*
 *  FolderInformation::Scan (from Journal)
 *      Rescans mods at this folder that are present in the change journal
 *      This method only scans, to update using the scanned information, call Update()
 */
void Loader::FolderInformation::Scan(const Journal& journal)
{
    ::scoped_gdir xdir(this->path.c_str());

    if(this->Profile().IsIgnoring() == false)
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

        // Collect garbaged data (mods and childs that are unused atm)
        CollectInformation(this->mods);
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
void Loader::FolderInformation::LoadConfigFromINI()
{
    ::scoped_gdir xdir(this->path.c_str());
    modloader_ini ini;
    CopyFileA(loader.folderConfigDefault.c_str(), loader.folderConfigFilename.c_str(), TRUE);
    
    this->RemoveProfiles();

    // Read the profiles present in the specified ini file
    auto ReadProfilesFromINI = [this](modloader_ini& ini, std::string generator)   // generator must be empty for modloader.ini
    {
        auto prof_filename = std::string(generator.empty()? "modloader.ini" : (loader.profilesPath + generator));
        for(auto& profname : Profile::GetListOfProfilesInIni(ini))
        {
            if(this->FindProfile(profname) == nullptr)  // no profile with this name so add from here
            {
                auto& prof = this->AddProfile(profname);
                prof.LoadConfigFromINI(ini);
                prof.SetGenerator(std::move(generator));
                Log("Reading profile named \"%s\" at \"%s\".", profname.c_str(), prof_filename.c_str());
            }
            else
                Log("Warning: Failed to read profile named \"%s\" at \"%s\". Profile already exists elsewhere.",
                            profname.c_str(), prof_filename.c_str());
        }
    };

    // First take the profiles from modloader.ini
    if(ini.load_file(loader.folderConfigFilename))
        ReadProfilesFromINI(ini, "");
    else
        Log("Warning: Failed to load folder config file");

    // Then from the profiles directory
    if(MakeSureDirectoryExistA((loader.gamePath + loader.profilesPath).c_str()))
    {
        ::scoped_gdir xdir(loader.profilesPath.c_str());
        for(auto& filename : FilesWalk("", "*.ini", false))
            ReadProfilesFromINI(modloader_ini(filename.data()), filename);
    }
    else
        Log("Warning: Failed to access \".profiles/\" directory.");

    // Find out working profile
    if(auto* prof = this->FindProfile(ini.get("Folder.Config", "Profile", default_profile_name)))
        this->SwitchToProfile(*prof);
    Log("Using profile named \"%s\"", this->Profile().GetName().c_str());

    // Transform the parent strings read from the ini into actual parents
    for(auto& prof : this->profiles)
        prof.UpdateInheritance();
    if(this->IsUsingAnonymousProfile())
        this->GetAnonymousProfile().UpdateInheritance();
}

/*
 *  FolderInformation::SaveConfigForINI
 *      Saves configuration specific to this folder to the specified ini file
 */
void Loader::FolderInformation::SaveConfigForINI()
{
    ::scoped_gdir xdir(this->path.c_str());
    modloader_ini ini;

    // Save current profile
    ini.set("Folder.Config", "Profile", this->Profile().GetName());

    // Save all the profiles
    for(auto& profile : this->profiles)
    {
        auto& generator = profile.GetGenerator();

        if(generator.empty())
        {
            // Profile direcly in modloader.ini
            profile.SaveConfigForINI(ini);
        }
        else if(MakeSureDirectoryExistA((loader.gamePath + loader.profilesPath).c_str()))
        {
            // This profile has it's own directory
            ::scoped_gdir xdir(loader.profilesPath.c_str());
            modloader_ini prof_ini;
            profile.SaveConfigForINI(prof_ini);
            if(!prof_ini.write_file(generator))
                Log("Warning: Failed to save profile into file \"%s\"", generator.c_str());
        }
    }

    // Finish Him!
    if(!ini.write_file(loader.folderConfigFilename))
        Log("Warning: Failed to save folder config file");
}
