/*
 * Copyright (C) 2013-2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 *  std.scm -- Standard SCM Loader Plugin for Mod Loader
 *
 */
#include <modloader/modloader.hpp>
#include <modloader/util/hash.hpp>
#include <modloader/gta3/gta3.hpp>
using namespace modloader;

/*
 *  The plugin object
 */
class ScmPlugin : public modloader::basic_plugin
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
        
} scm_plugin;

REGISTER_ML_PLUGIN(::scm_plugin);

/*
 *  ScmPlugin::GetInfo
 *      Returns information about this plugin 
 */
const ScmPlugin::info& ScmPlugin::GetInfo()
{
    static const char* extable[] = { "scm", 0 };
    static const info xinfo      = { "std.scm", "R0.1", "LINK/2012", -1, extable };
    return xinfo;
}





/*
 *  ScmPlugin::OnStartup
 *      Startups the plugin
 */
bool ScmPlugin::OnStartup()
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
 *  ScmPlugin::OnShutdown
 *      Shutdowns the plugin
 */
bool ScmPlugin::OnShutdown()
{
    return true;
}


/*
 *  ScmPlugin::GetBehaviour
 *      Gets the relationship between this plugin and the file
 */
int ScmPlugin::GetBehaviour(modloader::file& file)
{
    if(!file.is_dir() && file.hash == main_scm)
    {
        file.behaviour = file.hash;
        return MODLOADER_BEHAVIOUR_YES;
    }
    return MODLOADER_BEHAVIOUR_NO;
}

/*
 *  ScmPlugin::InstallFile
 *      Installs a file using this plugin
 */
bool ScmPlugin::InstallFile(const modloader::file& file)
{
    return overrider.InstallFile(file);
}

/*
 *  ScmPlugin::ReinstallFile
 *      Reinstall a file previosly installed that has been updated
 */
bool ScmPlugin::ReinstallFile(const modloader::file& file)
{
    return overrider.ReinstallFile();
}

/*
 *  ScmPlugin::UninstallFile
 *      Uninstall a previosly installed file
 */
bool ScmPlugin::UninstallFile(const modloader::file& file)
{
    return overrider.UninstallFile();
}
