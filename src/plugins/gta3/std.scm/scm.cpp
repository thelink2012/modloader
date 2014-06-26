/*
 * Copyright (C) 2013-2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 *  std-scm -- Standard SCM Loader Plugin for San Andreas Mod Loader
 *      This plugin is extremelly simple, made to load a new main.scm file.
 *      It just overrides one or two strings on the game executable.
 *      NOTE: script.img is taken care by the img plugin!
 * 
 */
#include <modloader/modloader.hpp>
#include <modloader/util/hash.hpp>
#include <modloader/gta3/gta3.hpp>
using namespace modloader;

/*
 *  The plugin object
 */
class ThePlugin : public modloader::basic_plugin
{
    private:
        uint32_t main_scm;
        file_overrider<2> overrider;

    public:
        const info& GetInfo();
        bool OnStartup();
        bool OnShutdown();
        int GetBehaviour(modloader::file&);
        bool InstallFile(const modloader::file&);
        bool ReinstallFile(const modloader::file&);
        bool UninstallFile(const modloader::file&);
        
} plugin;

REGISTER_ML_PLUGIN(::plugin);

/*
 *  ThePlugin::GetInfo
 *      Returns information about this plugin 
 */
const ThePlugin::info& ThePlugin::GetInfo()
{
    static const char* extable[] = { "scm", 0 };
    static const info xinfo      = { "std.scm", "R0.1", "LINK/2012", -1, extable };
    return xinfo;
}





/*
 *  ThePlugin::OnStartup
 *      Startups the plugin
 */
bool ThePlugin::OnStartup()
{
    if(gvm.IsSA())
    {
        this->main_scm = modloader::hash("main.scm");
        this->overrider.SetParams(file_overrider<2>::params(true, true, true, true));
        this->overrider.SetFileDetour(OpenFileDetour<0x468EC9>(), OpenFileDetour<0x489A4A>());
        return true;
    }
    return false;
}

/*
 *  ThePlugin::OnShutdown
 *      Shutdowns the plugin
 */
bool ThePlugin::OnShutdown()
{
    return true;
}


/*
 *  ThePlugin::GetBehaviour
 *      Gets the relationship between this plugin and the file
 */
int ThePlugin::GetBehaviour(modloader::file& file)
{
    if(file.hash == main_scm)
    {
        file.behaviour = file.hash;
        return MODLOADER_BEHAVIOUR_YES;
    }
    return MODLOADER_BEHAVIOUR_NO;
}

/*
 *  ThePlugin::InstallFile
 *      Installs a file using this plugin
 */
bool ThePlugin::InstallFile(const modloader::file& file)
{
    return overrider.InstallFile(file);
}

/*
 *  ThePlugin::ReinstallFile
 *      Reinstall a file previosly installed that has been updated
 */
bool ThePlugin::ReinstallFile(const modloader::file& file)
{
    return overrider.ReinstallFile();
}

/*
 *  ThePlugin::UninstallFile
 *      Uninstall a previosly installed file
 */
bool ThePlugin::UninstallFile(const modloader::file& file)
{
    return overrider.UninstallFile();
}
