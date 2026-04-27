/*
 * DMAudio Loader Plugin for Mod Loader
 * Copyright (C) 2013-2026  Silent
 * Licensed under the MIT License, see LICENSE at top level directory.
 * 
 *  std.dmaudio -- Standard DMAudio Loader Plugin for Mod Loader
 *
 */
#include <stdinc.hpp>
using namespace modloader;

enum class Type
{
    SfxRaw          = 0,    // sfx.raw
    SfxSdt          = 1,    // sfx.dat
    Sample          = 2,    // Samples (.wav, .mp3, .adf, .vb)
    Max             = 3,    // Max 2 bits
};


// Basic masks
static const uint64_t hash_mask_base  = 0xFFFFFFFF;
static const uint64_t type_mask_base  = 0x0003;                 // Mask for type without any shifting
static const uint32_t type_mask_shf  = 32;                      // Takes 2 bits, starts from 33th bit because first 32th bits is a hash

// Sets the initial value for a behaviour, by using an filename hash and file type
inline uint64_t SetType(uint32_t hash, Type type)
{
    return modloader::file::set_mask(uint64_t(hash), type_mask_base, type_mask_shf, type);
}

// Gets the behaviour file type
inline Type GetType(uint64_t mask)
{
    return modloader::file::get_mask<Type>(mask, type_mask_base, type_mask_shf);
}


/*
 *  The plugin object
 */
class DMAudioPlugin : public modloader::basic_plugin
{
    private:
        file_overrider sfx_raw_overrider;                       // sfx.raw overrider
        file_overrider sfx_sdt_overrider;                       // sfx.sdt overrider
        std::map<uint32_t, const modloader::file*> streams;     // Stream (hash, file*) map

    public:
         // Standard plugin methods
        const info& GetInfo() override;
        bool OnStartup() override;
        bool OnShutdown() override;
        int GetBehaviour(modloader::file&) override;
        bool InstallFile(const modloader::file&) override;
        bool ReinstallFile(const modloader::file&) override;
        bool UninstallFile(const modloader::file&) override;

    private:
        static char* PatchedStrcat(char* destination, const char* source);

} dmaudio_plugin;

REGISTER_ML_PLUGIN(::dmaudio_plugin);

/*
 *  DMAudioPlugin::GetInfo
 *      Returns information about this plugin 
 */
const DMAudioPlugin::info& DMAudioPlugin::GetInfo()
{
    static const char* extable[] = { "raw", "sdt", "wav", "mp3", "adf", "vb", 0 };
    static const info xinfo      = { "std.dmaudio", get_version_by_date(), "Silent", -1, extable };
    return xinfo;
}

/*
 *  DMAudioPlugin::OnStartup
 *      Startups the plugin
 */
bool DMAudioPlugin::OnStartup()
{
    if(gvm.IsIII() || gvm.IsVC())
    {
        // SFX files are used and loaded constantly, and the game doesn't even support closing sfx.raw.
        // They are actually fopen, but the signatures are compatible
        auto no_reinstall = file_overrider::params(nullptr);
        this->sfx_sdt_overrider.SetParams(no_reinstall).SetFileDetour(OpenFileDetour<xVc(0x5D5B7B)>());
        this->sfx_raw_overrider.SetParams(no_reinstall).SetFileDetour(OpenFileDetour<xVc(0x5D5BC5)>());

        // Patch strcat calls in cSampleManager
        using namespace injector;
        const auto strcatFunc = raw_ptr(&PatchedStrcat);

        MakeCALL(xVc(0x5D6339), strcatFunc); // cSampleManager::StartStreamedFile
        MakeCALL(xVc(0x5D64CD), strcatFunc); // cSampleManager::PreloadStreamedFile
        MakeCALL(xVc(0x5D799B), strcatFunc); // cSampleManager::Initialise
        MakeCALL(xVc(0x5D7B69), strcatFunc); // cSampleManager::Initialise
        //MakeCALL(xVc(0x5D71E3), strcatFunc); // cSampleManager::CheckForAnAudioFileOnCD (pointless)
        if (gvm.IsVC())
        {
            MakeCALL(xVc(0x5D7C1B), strcatFunc); // cSampleManager::Initialise
            MakeCALL(xVc(0x5D7A7B), strcatFunc); // cSampleManager::Initialise
        }

        return true;
    }
    return false;
}

/*
 *  DMAudioPlugin::OnShutdown
 *      Shutdowns the plugin
 */
bool DMAudioPlugin::OnShutdown()
{
    return true;
}


/*
 *  DMAudioPlugin::GetBehaviour
 *      Gets the relationship between this plugin and the file
 */
int DMAudioPlugin::GetBehaviour(modloader::file& file)
{
    if(!file.is_dir())
    {
        static const auto sfx_raw = modloader::hash("sfx.raw");
        static const auto sfx_sdt = modloader::hash("sfx.sdt");
        if(file.hash == sfx_raw)
        {
            file.behaviour = SetType(file.hash, Type::SfxRaw);
        }
        else if(file.hash == sfx_sdt)
        {
            file.behaviour = SetType(file.hash, Type::SfxSdt);
        }
        else if(file.is_ext("wav") || file.is_ext("mp3") || file.is_ext("adf") || file.is_ext("vb"))
        {
            file.behaviour = SetType(file.hash, Type::Sample);
        }
        else
        {
            return MODLOADER_BEHAVIOUR_NO;
        }
        return MODLOADER_BEHAVIOUR_YES;
    }
    return MODLOADER_BEHAVIOUR_NO;
}


/*
 *  DMAudioPlugin::InstallFile
 *      Installs a file using this plugin
 */
bool DMAudioPlugin::InstallFile(const modloader::file& file)
{
    switch(GetType(file.behaviour))
    {
        case Type::SfxRaw: return sfx_raw_overrider.InstallFile(file);
        case Type::SfxSdt: return sfx_sdt_overrider.InstallFile(file);
        case Type::Sample: this->streams[file.hash] = &file; return true;
    }
    return false;
}

/*
 *  DMAudioPlugin::ReinstallFile
 *      Reinstall a file previosly installed that has been updated
 */
bool DMAudioPlugin::ReinstallFile(const modloader::file& file)
{
    switch(GetType(file.behaviour))
    {
        case Type::SfxRaw: return sfx_raw_overrider.ReinstallFile();
        case Type::SfxSdt: return sfx_sdt_overrider.ReinstallFile();
        case Type::Sample: this->streams[file.hash] = &file; return true;
    }
    return false;
}

/*
 *  DMAudioPlugin::UninstallFile
 *      Uninstall a previosly installed file
 */
bool DMAudioPlugin::UninstallFile(const modloader::file& file)
{
    switch(GetType(file.behaviour))
    {
        case Type::SfxRaw: return sfx_raw_overrider.UninstallFile();
        case Type::SfxSdt: return sfx_sdt_overrider.UninstallFile();
        case Type::Sample: this->streams.erase(file.hash); return true;
    }
    return false;
}

char* DMAudioPlugin::PatchedStrcat(char* destination, const char* source)
{
    const auto& plugin = plugin_ptr->cast<DMAudioPlugin>();

    const char* lookupName;
    const char* filename = std::strrchr(source, '\\');
    if (filename != nullptr)
    {
        lookupName = filename+1;
    }
    else
    {
        lookupName = source;
    }
    auto it = plugin.streams.find(modloader::hash(lookupName, ::tolower));
    if (it != plugin.streams.end())
    {
        std::string path;
        return strcpy(destination, it->second->fullpath(path).c_str());
    }
    else
    {
        return strcat(destination, source);
    }
}
