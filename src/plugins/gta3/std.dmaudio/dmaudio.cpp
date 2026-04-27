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

/*
 *  The plugin object
 */
class DMAudioPlugin : public modloader::basic_plugin
{
    private:
        size_t sfx_raw = 0;                     // Hash
        size_t sfx_sdt = 0;                     // Hash

        file_overrider sfx_raw_overrider;       // sfx.raw overrider
        file_overrider sfx_sdt_overrider;       // sfx.sdt overrider

    public:
         // Standard plugin methods
        const info& GetInfo() override;
        bool OnStartup() override;
        bool OnShutdown() override;
        int GetBehaviour(modloader::file&) override;
        bool InstallFile(const modloader::file&) override;
        bool ReinstallFile(const modloader::file&) override;
        bool UninstallFile(const modloader::file&) override;

} dmaudio_plugin;

REGISTER_ML_PLUGIN(::dmaudio_plugin);

/*
 *  DMAudioPlugin::GetInfo
 *      Returns information about this plugin 
 */
const DMAudioPlugin::info& DMAudioPlugin::GetInfo()
{
    static const char* extable[] = { "raw", "sdt", 0 };
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
        this->sfx_raw = modloader::hash("sfx.raw");
        this->sfx_sdt = modloader::hash("sfx.sdt");

        // SFX files are used and loaded constantly, and the game doesn't even support closing sfx.raw.
        // They are actually fopen, but the signatures are compatible
        auto no_reinstall = file_overrider::params(nullptr);
        this->sfx_sdt_overrider.SetParams(no_reinstall).SetFileDetour(OpenFileDetour<xVc(0x5D5B7B)>());
        this->sfx_raw_overrider.SetParams(no_reinstall).SetFileDetour(OpenFileDetour<xVc(0x5D5BC5)>());
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
        if(file.hash == this->sfx_raw || file.hash == this->sfx_sdt)
        {
            file.behaviour = file.hash;
            return MODLOADER_BEHAVIOUR_YES;
        }
    }
    return MODLOADER_BEHAVIOUR_NO;
}


/*
 *  DMAudioPlugin::InstallFile
 *      Installs a file using this plugin
 */
bool DMAudioPlugin::InstallFile(const modloader::file& file)
{
    if(file.behaviour == this->sfx_raw)
    {
        return sfx_raw_overrider.InstallFile(file);
    }
    else if(file.behaviour == this->sfx_sdt)
    {
        return sfx_sdt_overrider.InstallFile(file);
    }
    return false;
}

/*
 *  DMAudioPlugin::ReinstallFile
 *      Reinstall a file previosly installed that has been updated
 */
bool DMAudioPlugin::ReinstallFile(const modloader::file& file)
{
    if(file.behaviour == this->sfx_raw)
    {
        return sfx_raw_overrider.ReinstallFile();
    }
    else if(file.behaviour == this->sfx_sdt)
    {
        return sfx_sdt_overrider.ReinstallFile();
    }
    return false;
}

/*
 *  DMAudioPlugin::UninstallFile
 *      Uninstall a previosly installed file
 */
bool DMAudioPlugin::UninstallFile(const modloader::file& file)
{
    if(file.behaviour == this->sfx_raw)
    {
        return sfx_raw_overrider.UninstallFile();
    }
    else if(file.behaviour == this->sfx_sdt)
    {
        return sfx_sdt_overrider.UninstallFile();
    }
    return false;
}
