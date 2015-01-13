/*
 * Copyright (C) 2013-2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#include <stdinc.hpp>
#include "loader.hpp"
using namespace modloader;

// TODO WATCH THE FILESYSTEM, CHECK OUT http://msdn.microsoft.com/en-us/library/windows/desktop/aa365261%28v=vs.85%29.aspx
// TODO take care of configs on rescan
// TODO ReinstallFile shouldn't Uninstall on failure?
// TODO ^ think about uninstalling in-game to the next run, what if couldn't uninstall a specific file

extern int InstallExceptionCatcher(void (*cb)(const char* buffer));

#define USE_TEST 0
REGISTER_ML_NULL();

// Mod Loader object
Loader loader;


/*
 * DllMain
 *      Entry-point
 */
extern "C"
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    if(fdwReason == DLL_PROCESS_ATTACH) loader.Patch();
    return TRUE;
}


/*
 *  Loader::Patch
 *      Patches the game code to run the loader
 */
void Loader::Patch()
{
    typedef function_hooker_stdcall<0x8246EC, int(HINSTANCE, HINSTANCE, LPSTR, int)> winmain_hook;
    typedef function_hooker<0x53ECBD, void(int)> ridle_hook;
    typedef function_hooker<0x53ECCB, void(int)> rfidle_hook;   // Actually void() but... meh

    auto& gvm = injector::address_manager::singleton();
    gvm.set_name("Mod Loader");
    
    // Check if we have WinMain proc address, otherwise this game isn't supported
    if(try_address(winmain_hook::addr))
    {
        // Hook WinMain to run mod loader
        injector::make_static_hook<winmain_hook>([this](winmain_hook::func_type WinMain,
                                                    HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
        {
            // Avoind circular looping forever
            static bool bRan = false;
            if(bRan) return WinMain(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
            bRan = true;

            // Install exception filter to log crashes
            InstallExceptionCatcher([](const char* buffer)
            {
                Log("\n\n");
                LogGameVersion();
                Log(buffer);
                loader.Shutdown();
            });

            // To be called each frame
            auto CallTick = [this](ridle_hook::func_type Idle, int& i)
            {
                this->Tick();
                return Idle(i);
            };

            // Do tick hook only if possible
            if(try_address(ridle_hook::addr))  make_static_hook<ridle_hook>(CallTick);
            if(try_address(rfidle_hook::addr)) make_static_hook<rfidle_hook>(CallTick);

            // Startup the loader and call WinMain, Shutdown the loader after WinMain.
            // If any mod hooked WinMain at Startup, no conflict will happen, we're takin' care of that
            {
            this->Startup();
            auto WinMain = (winmain_hook::func_type_raw) ReadRelativeOffset(winmain_hook::addr + 1).get();
            auto result = WinMain(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
            this->Shutdown();
            return result;
            }
        });
    }
    else
    {
        char buf[128];
        this->Error("This game is not supported\nGame Info:\n%s", gvm.GetVersionText(buf));
    }
}

/*
 *  Loader::Startup
 *      Starts the loader
 */
void Loader::Startup()
{
    char rootPath[MAX_PATH];
    char appDataPath[MAX_PATH];

    // If not running yet and 'modloader' folder exists, let's start up
    if(!this->bRunning && IsDirectoryA("modloader"))
    {
        // Cleanup the base structure
        memset(this, 0, sizeof(modloader_t));

        // Initialise configs and counters
        this->vkRefresh      = VK_F4;
        this->bRunning       = false;
        this->bEnableMenu    = true;
        this->bEnableLog     = true;
        this->bEnablePlugins = true;
        this->maxBytesInLog  = 5242880;     // 5 MiB
        this->currentModId   = 0;
        this->currentFileId  = 0x8000000000000000;  // File id should have the hibit set
        
        // Open the log file
        OpenLog();
        LogGameVersion();

        // Setup basic path variables
        this->dataPath    = "modloader/.data/";
        this->pluginPath  = "modloader/.data/plugins/";
        //this->cachePath   = "modloader/.data/cache/";
        GetCurrentDirectoryA(sizeof(rootPath), rootPath);
        MakeSureStringIsDirectory(this->gamePath = rootPath);

        // Setup "%ProgramData%/modloader/" variable
        if(SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_COMMON_APPDATA|CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, appDataPath)))
            MakeSureStringIsDirectory(MakeSureStringIsDirectory(this->commonAppDataPath = appDataPath).append("modloader"));

        // Setup "%LocalAppData%/modloader/" variable
        if(SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA|CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, appDataPath)))
            MakeSureStringIsDirectory(MakeSureStringIsDirectory(this->localAppDataPath = appDataPath).append("modloader"));

        // Setup config file names
        this->folderConfigFilename = "modloader.ini";
        this->basicConfig          = dataPath + "config.ini";
        this->pluginConfigFilename = "plugins.ini";
        this->folderConfigDefault  = gamePath + dataPath + "modloader.ini.0";
        this->basicConfigDefault   = gamePath + dataPath + "config.ini.0";
        this->pluginConfigDefault  = gamePath + dataPath + "plugins.ini.0";

        // Make sure the important folders exist
        if(!MakeSureDirectoryExistA(dataPath.c_str())
        || !MakeSureDirectoryExistA(pluginPath.c_str())
        //|| !MakeSureDirectoryExistA(cachePath.c_str())
        || !MakeSureDirectoryExistA(commonAppDataPath.c_str())
        || !MakeSureDirectoryExistA(localAppDataPath.c_str()))
            Log("Warning: Mod Loader important directories could not be created.");
        
        // Before loading inis, we should update from the old ini format to the new ini format (ofc only if the ini format is old)
        this->UpdateOldConfig();

        // Load the basic configuration file
        CopyFileA(basicConfigDefault.c_str(), basicConfig.c_str(), TRUE);
        this->ReadBasicConfig();
        
        // Check if logging is disabled by the basic config file
        if(!this->bEnableLog)
        {
            Log("Logging is disabled. Closing log file...");
            CloseLog();
        }

        // Register exported methods and vars
        modloader_t::has_game_started= false;
        modloader_t::has_game_loaded = false;
        modloader_t::gamepath        = this->gamePath.data();
        //modloader_t::cachepath       = this->cachePath.data();
        modloader_t::commonappdata   = this->commonAppDataPath.data();
        modloader_t::localappdata    = this->localAppDataPath.data();
        modloader_t::Log             = this->Log;
        modloader_t::vLog            = this->vLog;
        modloader_t::Error           = this->Error;

        // Initialise sub systems
        this->ParseCommandLine();   // Parse command line arguments
        this->StartupMenu();
        this->LoadPlugins();        // Load plugins at /modloader/.data/plugins
        this->ScanAndUpdate();      // Search and install mods at /modloader
 
        // Startup successfully
        this->bRunning = true;
        Log("\nMod Loader has started up!\n");
    }
}

/*
 *  Loader::Shutdown
 *      Shut downs the loader
 */
void Loader::Shutdown()
{
    if(this->bRunning)
    {
        // Unload the plugins
        Log("\nShutting down Mod Loader...");
        this->UnloadPlugins();
        Log("Mod Loader has been shutdown.");
        
        // Finish containers
        this->ShutdownMenu();
        this->plugins_priority.clear();
        this->extMap.clear();
        this->mods.Clear();
        
        // Close the log file
        this->CloseLog();
        this->bRunning = false;
    }
}

/*
 *  Loader::Shutdown
 *      This is called every frame
 */
void Loader::Tick()
{
    static int& gGameState = *mem_ptr(0xC8D4C0).get<int>();
    this->has_game_started = (gGameState >= 7);
    this->has_game_loaded  = (gGameState >= 9);
    this->TestHotkeys();
}

/*
 *  Loader::TestHotkeys
 *      Test hotkeys for specific actions
 */
void Loader::TestHotkeys()
{
    static bool prevF4 = false; 
    static bool currF4 = false; 

    // Get current hotkey states
    currF4 = (GetKeyState(vkRefresh) & 0x8000) != 0;

    // Check hotkey states
    if(currF4 && !prevF4)
    {
        this->ScanAndUpdate();
    }

    // Save previous states
    prevF4 = currF4;
}


/*
 *  Loader::ReadBasicConfig
 *       Read the basic configuration file @filename
 */
void Loader::ReadBasicConfig()
{
    linb::ini data;

    Log("Loading basic config file %s", basicConfig.c_str());
    if(data.load_file(gamePath + basicConfig))
    {
        // Read basic stuff from [Config] section
        for(auto& pair : data["Config"])
        {
            if(!compare(pair.first, "EnableMenu", false))
                this->bEnableMenu = to_bool(pair.second);
            else if(!compare(pair.first, "EnablePlugins", false))
                this->bEnablePlugins = to_bool(pair.second);
            else if(!compare(pair.first, "EnableLog", false))
                this->bEnableLog = to_bool(pair.second);
            else if(!compare(pair.first, "ImmediateFlushLog", false))
                this->bImmediateFlush = to_bool(pair.second);
            else if(!compare(pair.first, "MaxLogSize", false))
                this->maxBytesInLog = std::strtoul(pair.second.data(), 0, 0);
            else if(!compare(pair.first, "RefreshKey", false))
                this->vkRefresh = std::stoi(pair.second.data(), 0, 0);
        }
    }
    else
        Log("Failed to load basic config file");
}

/*
 *  Loader::SaveConfig
 *       Saves the basic config file with the current settings applied on this loader
 */
 void Loader::SaveBasicConfig()
 {
     linb::ini ini;
     
     auto& config = ini["Config"];
     config["EnableMenu"]           = modloader::to_string(bEnableMenu);
     config["EnablePlugins"]        = modloader::to_string(bEnablePlugins);
     config["EnableLog"]            = modloader::to_string(bEnableLog);
     config["ImmediateFlushLog"]    = modloader::to_string(bImmediateFlush);
     config["MaxLogSize"]           = std::to_string(maxBytesInLog);
     config["RefreshKey"]           = std::to_string(vkRefresh);

     // Log only about failure since we'll be saving every time a entry on the menu changes
     if(!ini.write_file(gamePath + basicConfig))
         Log("Failed to save basic config file");
 }


/*
 *  Loader::ParseCommandLine
 *       Parse command line arguments
 */
void Loader::ParseCommandLine()
{
    char buf[512];
    wchar_t **argv; int argc; 
    argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    
    Log("\nParsing command line");

    if(!argv) // CommandLineToArgvW failed!!
    {
        Log("Failed to parse command line. CommandLineToArgvW() failed.");
        return;
    }
    
    // Converts wchar_t buffer into char* ASCII buffer
    auto toASCII = [](const wchar_t* wstr, char* abuf, size_t asize)
    {
        return !!WideCharToMultiByte(CP_ACP, 0, wstr, -1, abuf, asize, NULL, NULL);
    };
    
    // Iterate on the arguments
    for(int i = 0; i < argc; ++i)
    {
        // If an actual argument...
        if(argv[i][0] == '-')
        {
            wchar_t *arg = (i+1 < argc? argv[i+1] : nullptr);
            wchar_t *argname = &argv[i][1];

            if(!_wcsicmp(argname, L"nomods"))
            {
                mods.SetForceIgnore(true);
                Log("Command line ignore received (-nomods)");
            }
            else if(!_wcsicmp(argname, L"mod"))
            {
                // Is argument after mod argument valid?
                if(arg == nullptr)
                {
                    Log("Failed to read command line. Mod command line is incomplete.");
                    break;
                }
                else
                {
                    if(toASCII(arg, buf, sizeof(buf)))
                    {
                        // Force exclusion and include the specified mod
                        mods.SetForceExclude(true);
                        mods.Include(buf);
                        mods.SetPriority(buf, default_cmd_priority);
                        Log("Command line mod received: \"%s\"", buf);
                    }
                }
            }
        }
    }
    
    LocalFree(argv);
}

/*
 *  Loader::LogGameVersion
 *      Logs the game version into the logging stream
 */
void Loader::LogGameVersion()
{
    char buffer[128];
    Log("Game version: %s", injector::address_manager::singleton().GetVersionText(buffer));
}


/*
 *  Loader::ScanAndUpdate
 *       Rescans and Updates the mods
 */
void Loader::ScanAndUpdate()
{
    Updating xup;
    mods.Scan();
    mods.Update();
}

/*
 *  Loader::NotifyUpdateForPlugins
 *       Notify the plugins that we've installed/uninstalled stuff
 *       This normally happens after a rescan
 */
void Loader::NotifyUpdateForPlugins()
{
    for(auto& plugin : this->plugins)
        plugin.Update();
}


/*
 *  Loader::FindHandlerForFile
 *       Finds the plugin responssible for handling the file @m,
 *       also the plugins that wants to receive the file at @callme
 */
auto Loader::FindHandlerForFile(modloader::file& m, ref_list<PluginInformation>& callme) -> PluginInformation*
{
    PluginInformation* handler = nullptr;
    
    // Iterate on the plugins to find a handler for it
    for(PluginInformation& plugin : this->GetPluginsBy(m.filext()))
    {
        auto state = plugin.FindBehaviour(m);
        
        if(state == BehaviourType::Yes)
        {
            // We found a handler and behaviour, stop the search immediately, don't check for other callme's
            handler = &plugin;
            break;
        }
        else if(state == BehaviourType::CallMe)
        {
            // This plugin requests this file to be sent for some reason (readme files?)
            callme.emplace_back(plugin);
        }
    }
    
    return handler;
}


/*
 *  Loader::GetPluginsBy
 *       Gets a list of plugins sorted for file-behaviour-search.
 *       That's, sort by priority and extension.
 */
auto Loader::GetPluginsBy(const std::string& extension) -> ref_list<PluginInformation>
{
    SimplePriorityPred<PluginInformation> pred_base;
    auto list = refs(this->plugins);

    // Checks if the specified plugin list contains any plugin 'p'
    auto contains = [](ref_list<PluginInformation>& plugins, const PluginInformation& p)
    {
        return std::any_of(plugins.begin(), plugins.end(), [&p](const PluginInformation& a) { return a == p; });
    };

    // Predicate to execute the sorting
    auto pred = [&, this](const PluginInformation& a, const PluginInformation& b)
    {
        if(a.priority == b.priority)    // If priorities are equal, check for extension!
        {
            auto it = this->extMap.find(extension);
            if(it != extMap.end())  // handleabe extension should have priority over other extensions
            {
                bool ca = contains(it->second, a);
                bool cb = contains(it->second, b);
                if(ca && !cb) return true;          // a has priority over b
                else if(!ca && cb) return false;    // b has priority over a
            }
        }
        return pred_base(a, b);
    };

    // Sort and return
    std::sort(list.begin(), list.end(), pred);
    return list;
}


/*
 *  Loader::UpdateOldConfig
 *       Updates the old ini config (from 0.1.15) into the new ini config format (as from 0.2.1)
 */
void Loader::UpdateOldConfig()
{
    linb::ini old, newer;

    struct keydata { const char *section, *key; };

    // Loader independent config, as loaded by this->LoadBasicConfig(), as of 0.2.0 those configs are part of a different ini
    // So just translate them to the new ini file
    auto UpdateBasicConfig = [&]()
    {
        for(auto& pair : old["CONFIG"])
        {
            if(!compare(pair.first, "ENABLE_PLUGINS", false))
                this->bEnablePlugins = to_bool(pair.second);
            else if(!compare(pair.first, "LOG_ENABLE", false))
                this->bEnableLog = to_bool(pair.second);
            else if(!compare(pair.first, "LOG_IMMEDIATE_FLUSH", false))
                this->bImmediateFlush = to_bool(pair.second);
        }
        this->SaveBasicConfig();
    };

    // Takes the key from the old ini and puts in the new ini with new formating
    auto UpdateKey = [&](const keydata& oldkey, keydata& newkey)
    {
        if(old[oldkey.section].count(oldkey.key))
            newer[newkey.section].emplace(newkey.key, old[oldkey.section][oldkey.key]);
    };

    // Takes a section from the old ini and puts in the new ini with new formating
    auto UpdateSection = [&](const keydata& oldsec, keydata& newsec)
    {
        if(old.count(oldsec.section))
            newer[newsec.section] = old[oldsec.section];
    };

    if(old.load_file(gamePath + "/modloader/modloader.ini"))
    {
        if(old.count("CONFIG") && old.count("PRIORITY"))
        {
            this->Log("Found old modloader.ini, updating to newer version");

            // Updates key-values to newer version
            UpdateBasicConfig();
            UpdateSection(keydata { "PRIORITY" }, keydata { "Priority" });
            UpdateSection(keydata { "EXCLUDE_FILES" }, keydata { "IgnoreFiles" });
            UpdateSection(keydata { "INCLUDE_MODS" }, keydata { "IncludeMods" });
            UpdateKey(keydata { "CONFIG", "IGNORE_ALL" },  keydata { "Config", "IgnoreAllFiles" });
            UpdateKey(keydata { "CONFIG", "EXCLUDE_ALL" }, keydata { "Config", "ExcludeAllMods" });

            // Reverse the priority since as from 0.2.1 it's '100 is greater, 1 is lower'
            for(auto& kv : newer["Priority"])
            {
                auto pr = std::stoi(kv.second);
                kv.second = std::to_string(pr > 0 && pr <= 100? 101 - pr : pr);
            }

            newer.write_file(gamePath + "modloader/" +  folderConfigFilename);
        }
    }
}
