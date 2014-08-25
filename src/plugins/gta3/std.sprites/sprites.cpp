/*
 * Copyright (C) 2013-2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 *  std.sprites -- Standard Sprites Loader Plugin for Mod Loader
 *      This plugin provides sprites for scripts and other parts of the game.
 *      Sprites are placed at "models/txd" folder
 *
 */
#include <map>
#include <modloader/modloader.hpp>
#include <modloader/gta3/gta3.hpp>
#include <modloader/util/hash.hpp>
#include <modloader/util/path.hpp>
using namespace modloader;


/*
 *  The plugin object
 */
class ScriptSpritesPlugin : public modloader::basic_plugin
{
    private:
        std::map<std::string, const modloader::file*> dictionaries;

    public:
        const info& GetInfo();
        bool OnStartup();
        bool OnShutdown();
        int GetBehaviour(modloader::file&);
        bool InstallFile(const modloader::file&);
        bool ReinstallFile(const modloader::file&);
        bool UninstallFile(const modloader::file&);
        
} scr_spr_plugin;

REGISTER_ML_PLUGIN(::scr_spr_plugin);

/*
 *  ScriptSpritesPlugin::GetInfo
 *      Returns information about this plugin 
 */
const ScriptSpritesPlugin::info& ScriptSpritesPlugin::GetInfo()
{
    static const char* extable[] = { "txd", 0 };
    static const info xinfo      = { "std.sprites", get_version_by_date(), "LINK/2012", 49, extable };
    return xinfo;
}





/*
 *  ScriptSpritesPlugin::OnStartup
 *      Startups the plugin
 */
bool ScriptSpritesPlugin::OnStartup()
{
    if(gvm.IsSA())
    {
        typedef function_hooker<0x48418A, int(int, const char*)> hooker;
        
        // Hooker function, hooks the CTxdStore::LoadTxd at the script engine
        make_static_hook<hooker>([this](hooker::func_type LoadTxd, int& index, const char*& filepath)
        {
            std::string filename = filepath;
            filename = &filepath[GetLastPathComponent(filename)];

            // Find replacement for sprite dictionary at filepath
            auto it = dictionaries.find(filename);
            if(it != dictionaries.end()) filepath = it->second->filepath();

            // Jump to the original call to LoadTxd
            Log("Loading script sprite \"%s\"", filepath);
            return LoadTxd(index, filepath);
        });

        return true;
    }
    return false;
}

/*
 *  ScriptSpritesPlugin::OnShutdown
 *      Shutdowns the plugin
 */
bool ScriptSpritesPlugin::OnShutdown()
{
    return true;
}


/*
 *  ScriptSpritesPlugin::GetBehaviour
 *      Gets the relationship between this plugin and the file
 */
int ScriptSpritesPlugin::GetBehaviour(modloader::file& file)
{
    if(!file.is_dir() && file.is_ext("txd") && IsFileInsideFolder(file.filedir(), true, "txd"))
    {
        file.behaviour = file.hash;
        return MODLOADER_BEHAVIOUR_YES;
    }
    return MODLOADER_BEHAVIOUR_NO;
}

/*
 *  ScriptSpritesPlugin::InstallFile
 *      Installs a file using this plugin
 */
bool ScriptSpritesPlugin::InstallFile(const modloader::file& file)
{
    dictionaries.emplace(file.filename(), &file);
    return true;
}

/*
 *  ScriptSpritesPlugin::ReinstallFile
 *      Reinstall a file previosly installed that has been updated
 */
bool ScriptSpritesPlugin::ReinstallFile(const modloader::file& file)
{
    // No need to reinstall, it is always "reinstalled"
    return true;
}

/*
 *  ScriptSpritesPlugin::UninstallFile
 *      Uninstall a previosly installed file
 */
bool ScriptSpritesPlugin::UninstallFile(const modloader::file& file)
{
    dictionaries.erase(file.filename());
    return true;
}
