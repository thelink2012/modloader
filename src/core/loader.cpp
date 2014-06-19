/*
 * Copyright (C) 2013-2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */

#include "loader.hpp"
#include <modloader_util_injector.hpp>
#include <modloader_util_path.hpp>
#include <modloader_util_ini.hpp>

extern int InstallExceptionCatcher(void (*cb)(const char* buffer));

#define USE_TEST 1
REGISTER_ML_NULL();

// Mod Loader object
Loader loader;

// TODO Make exception.cpp constants configurable thought a ini


/*
 *
 *
 */
#if USE_TEST
int __stdcall test_winmain(HINSTANCE, HINSTANCE, LPSTR, int) {         int i;
        sscanf("0x1337", "%i", &i);
        printf("%i", i); test(); return 1; }

void test()
{
    while(true)
    {
        auto func = (int(*)())0;
        func();
        Sleep(1000);
        loader.ScanAndUpdate();
    }
}
#endif






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
    auto& gvm = injector::address_manager::singleton();
    gvm.set_name("Mod Loader");     // Mod Loader core
    
    // Check game version to make sure we're compatible
    if(gvm.IsUnknown())
        Error("Mod Loader could not detect your game version.");
    else if(!gvm.IsSA() || gvm.GetMajorVersion() != 1 || gvm.GetMinorVersion() != 0 || gvm.IsUS() == false)
        Error("Mod Loader still do not support game versions other than HOODLUM GTA SA 1.0 US");
    else
    {
        typedef function_hooker_stdcall<0x8246EC, int(HINSTANCE, HINSTANCE, LPSTR, int)> winmain_hook;
        typedef function_hooker<0x53C6DB, void(void)>           onreload_hook;
                
        // Hook WinMain to run mod loader
        make_function_hook<winmain_hook>([](winmain_hook::func_type WinMain,
                                                    HINSTANCE& hInstance, HINSTANCE& hPrevInstance, LPSTR& lpCmdLine, int& nCmdShow)
        {
            // Avoind circular looping forever
            static bool bRan = false;
            if(bRan) return WinMain(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
            bRan = true;

            #if USE_TEST
                MakeCALL(winmain_hook::addr, raw_ptr(test_winmain));
            #endif

            // Install exception filter to log crashes
            InstallExceptionCatcher([](const char* buffer)
            {
                // TODO log the game version
                Log(buffer);
                loader.Shutdown();
            });

            // Startup the loader and call WinMain, Shutdown the loader after WinMain.
            // If any mod hooked WinMain at Startup, no conflict will happen, we're takin' care of that
            loader.Startup();
            WinMain = ReadRelativeOffset(winmain_hook::addr + 1).get();
            auto result = WinMain(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
            loader.Shutdown();

            return result;
        });
    }
}

/*
 *  Loader::Startup
 *      Starts the loader
 */
void Loader::Startup()
{
    char rootPath[MAX_PATH];
    const char* basic_config = "modloader/.data/modloader.ini";
    const char* basic_config_default = "modloader/.data/modloader.ini.0";
    
    // If not running yet and 'modloader' folder exists, let's start up
    if(!this->bRunning && IsDirectoryA("modloader"))
    {
        // Initialise configs and counters
        this->bRunning       = false;
        this->bEnableLog     = true;
        this->bEnablePlugins = true;
        this->maxBytesInLog  = 5242880;     // 5 MiB (TODO ini)
        this->currentModId   = 0;
        this->currentFileId  = 0x8000000000000000;  // File id should have the hibit set
        
        // Open the log file
        OpenLog();
        
        // TODO log game version
        
        // Make sure we have the modloader.ini file
        CopyFileA(basic_config_default, basic_config, TRUE);
        
        // Read the basic information present in the main ini file
        ReadBasicConfig(basic_config);
        
        // Check if logging is disabled by the basic config file
        if(!this->bEnableLog)
        {
            Log("Logging is disabled. Closing log file...");
            CloseLog();
        }
        
        // Setup path variables
        std::string data  = "modloader/.data/";
        this->pluginPath  = "modloader/.data/plugins/";
        this->cachePath   = "modloader/.data/cache/";
        GetCurrentDirectoryA(sizeof(rootPath), rootPath);
        MakeSureStringIsDirectory(this->gamePath = rootPath);
        
        // Make sure the important folders exist
        MakeSureDirectoryExistA(data.c_str());
        MakeSureDirectoryExistA(pluginPath.c_str());
        MakeSureDirectoryExistA(cachePath.c_str());
        
        // Register exported methods and vars
        modloader_t::gamepath        = this->gamePath.data();
        modloader_t::cachepath       = this->cachePath.data();
        modloader_t::Log             = this->Log;
        modloader_t::vLog            = this->vLog;
        modloader_t::Error           = this->Error;

        // Initialise sub systems
        this->ParseCommandLine();   // Parse command line arguments
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
        plugins_priority.clear();
        extMap.clear();
        mods.Clear();
        
        // Close the log file
        CloseLog();
        this->bRunning = false;
    }
}

/*
 *  Loader::ReadBasicConfig
 *       Read the basic configuration file @filename
 */
void Loader::ReadBasicConfig(const char* filename)
{
    modloader::ini data;

    Log("Loading basic config file %s", filename);
    if(data.load_file(filename))
    {
        // Read basic stuff from CONFIG section
        for(auto& pair : data["CONFIG"])
        {
            auto& key = pair.first;
            auto& value = pair.second;

            // Check the option and apply the change
            if(!compare(key.data(), "ENABLE_PLUGINS", false))
                this->bEnablePlugins = to_bool(value);
            else if(!compare(key.data(), "LOG_ENABLE", false))
                this->bEnableLog = to_bool(value);
            else if(!compare(key.data(), "LOG_IMMEDIATE_FLUSH", false))
                this->bImmediateFlush = to_bool(value);
        }
    }
    else
        Log("Failed to load basic config file");
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

            // Mod argument
            if(!_wcsicmp(argname, L"mod"))
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
    
    //
    LocalFree(argv);
    Log("Done reading command line");
}


/*
 *  Loader::ScanAndUpdate
 *       Rescans and Updates the mods
 */
void Loader::ScanAndUpdate()
{
    mods.Scan();
    mods.Update();
}

/*
 *  Loader::FindHandlerForFile
 *       Finds the plugin responssible for handling the file @m,
 *       also the plugins that wants to receive the file at @callme
 */
auto Loader::FindHandlerForFile(modloader::file& m, PluginVector& callme) -> PluginInformation*
{
    PluginInformation* handler = nullptr;
    
    // TODO get list by priority and extmap
    
    for(auto& plugin : this->plugins)
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
            callme.emplace_back(&plugin);
        }
    }
    
    return handler;
}
