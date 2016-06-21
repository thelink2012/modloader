/*
 * Copyright (C) 2013-2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under the MIT License, see LICENSE at top level directory.
 * 
 *  std.sprites -- Standard Sprites Loader Plugin for Mod Loader
 *      This plugin provides sprites for scripts and other parts of the game.
 *      Sprites are placed at "models/txd" folder
 *
 */
#include <stdinc.hpp>
#include <regex/regex.hpp>
using namespace modloader;

static const uint64_t hash_mask      = 0x00000000FFFFFFFF;  // Mask for the hash on the behaviour
static const uint64_t is_script_mask = 0x0000000100000000;  // Mask for is_script on the behaviour
static const uint64_t is_splash_mask = 0x0000000200000000;  // Mask for is_splash on the behaviour

/*
 *  The plugin object
 */
class ScriptSpritesPlugin : public modloader::basic_plugin
{
    private:
        std::map<std::string, const modloader::file*> script_dicts;
        std::map<std::string, const modloader::file*> splash_dicts;

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
    static const info xinfo      = { "std.sprites", get_version_by_date(), "LINK/2012", 51, extable };
    return xinfo;
}





/*
 *  ScriptSpritesPlugin::OnStartup
 *      Startups the plugin
 */
bool ScriptSpritesPlugin::OnStartup()
{
    // Although III/VC uses "models/", we'll keep the "models/txd" pattern.
    if(gvm.IsIII() || gvm.IsVC() || gvm.IsSA())
    {
        typedef function_hooker<0x48418A, int(int, const char*)> schooker;
        typedef function_hooker<xVc(0x4A6F4E), int(int, const char*)> sphooker;

        // Hooks the CTxdStore::LoadTxd at the script engine
        make_static_hook<schooker>([this](schooker::func_type LoadTxd, int& index, const char*& filepath)
        {
            std::string filename = filepath;
            filename = &filepath[GetLastPathComponent(filename)];

            auto it = script_dicts.find(tolower(filename));
            if(it != script_dicts.end()) filepath = it->second->filepath();

            Log("Loading script sprite \"%s\"", filepath);
            return LoadTxd(index, filepath);
        });

        if(gvm.IsVC() || gvm.IsIII())
        {
            // Hooks the CTxdStore::LoadTxd at the splash loader.
            make_static_hook<sphooker>([this](sphooker::func_type LoadTxd, int& index, const char*& filepath)
            {
                std::string filename = filepath;
                filename = &filepath[GetLastPathComponent(filename)];

                auto it = splash_dicts.find(tolower(filename));
                if(it != splash_dicts.end())
                {
                    filename = it->second->fullpath();
                    filepath = filename.c_str();
                }

                Log("Loading splash sprite \"%s\"", filepath);
                return LoadTxd(index, filepath);
            });
        }

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
        static auto script_regex = make_regex(R"___(^.*models[\\/]txd[\\/]\w{1,8}\.txd$)___",
                                              sregex::ECMAScript|sregex::optimize|sregex::icase);
        if(regex_match(std::string(file.filedir()), script_regex))
            file.behaviour = file.hash | is_script_mask;
        else
            file.behaviour = file.hash | is_splash_mask;
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
    if(file.behaviour & is_script_mask)
        script_dicts.emplace(file.filename(), &file);
    else if(file.behaviour & is_splash_mask)
        splash_dicts.emplace(file.filename(), &file);
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
    if(file.behaviour & is_script_mask)
        script_dicts.erase(file.filename());
    else if(file.behaviour & is_splash_mask)
        splash_dicts.erase(file.filename());
    return true;
}
