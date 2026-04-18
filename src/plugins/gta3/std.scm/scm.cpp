/*
 * Copyright (C) 2013-2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under the MIT License, see LICENSE at top level directory.
 * 
 *  std.scm -- Standard SCM Loader Plugin for Mod Loader
 *
 */
#include <stdinc.hpp>
using namespace modloader;

/*
 *  The plugin object
 */
class ScmPlugin : public modloader::basic_plugin
{
    private:
        std::map<uint32_t, const modloader::file*> scm; // SCM Files Map<hash, file>

        OpenFileDetour<0x468EC9> scm_d1;                // CTheScripts::Init detour
        OpenFileDetour<0x489A4A> scm_d2;                // CRunningScript::ProcessCommands1000To1099 detour

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
    static const info xinfo      = { "std.scm", get_version_by_date(), "LINK/2012", -1, extable };
    return xinfo;
}





/*
 *  ScmPlugin::OnStartup
 *      Startups the plugin
 */
bool ScmPlugin::OnStartup()
{
    if(gvm.IsIII() || gvm.IsVC() || gvm.IsSA())
    {
        // Returns the overriden scm file path (relative to gamedir)
        auto transformer = [this](std::string filename)
        {
            auto it = scm.find(modloader::hash(&filename[GetLastPathComponent(filename)], ::tolower));
            return std::string(it != scm.end()? it->second->filepath() : "");
        };

        this->scm_d1.make_call();
        this->scm_d1.OnTransform(transformer);
        this->scm_d2.make_call();
        this->scm_d2.OnTransform(transformer);
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
    if(!file.is_dir() && file.is_ext("scm"))
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
    this->scm[file.hash] = &file;
    return true;
}

/*
 *  ScmPlugin::ReinstallFile
 *      Reinstall a file previosly installed that has been updated
 */
bool ScmPlugin::ReinstallFile(const modloader::file& file)
{
    return InstallFile(file);
}

/*
 *  ScmPlugin::UninstallFile
 *      Uninstall a previosly installed file
 */
bool ScmPlugin::UninstallFile(const modloader::file& file)
{
    this->scm.erase(file.hash);
    return true;
}
