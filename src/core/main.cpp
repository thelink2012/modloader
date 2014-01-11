/* 
 * San Andreas modloader
 * Copyright (C) 2013  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 *  Modloader core, main source file
 *  The core do not handle any file at all, file handling is plugin-based
 * 
 */

#include <modloader.hpp>
#include <modloader_util.hpp>
#include <modloader_util_path.hpp>
#include <modloader_util_container.hpp>
#include <modloader_util_ini.hpp>

#include <windows.h>
#include <cstdio>
#include <cstring>
#include <string>

#include "CModLoader.hpp"
#include "modloader_util.hpp"
#include "modloader_util_injector.hpp"

#if 0 && !defined(NDEBUG)
#define LOGFILE_AS_STDOUT
#endif

namespace modloader
{
    static const char* modurl = "https://github.com/thelink2012/sa-modloader";
    
    // log stream
    static FILE* logfile = 0;
    static bool bImmediateFlush = true;     // Flushes the logfile everything something is written to it
    
    
    /*
     * vLog
     *      Logs something into the log file 'logfile'
     *      @msg: (Format) Message to log
     *      @va: Additional args to format in va_list
     */
    void vLog(const char* msg, va_list va)
    {
        if(logfile)
        {
            vfprintf(logfile, msg, va);
            fputc('\n', logfile);
            if(bImmediateFlush) fflush(logfile);
        }
    }
    
    /*
     * Log
     *      Logs something into the log file 'logfile'
     *      @msg: (Format) Message to log
     *      @...: Additional args to format
     */
    void Log(const char* msg, ...)
    {
        if(logfile)
        {
            va_list va; va_start(va, msg);
            vLog(msg, va);
            va_end(va);
        }
    }
    
    /*
     * Error
     *      Displays a error message box
     *      @msg: (Format) Error to display
     *      @...: Additional args to format
     */
    void Error(const char* msg, ...)
    {
        va_list va; va_start(va, msg);
        char buffer[1024];
        vsprintf(buffer, msg, va);
        MessageBoxA(NULL, buffer, "modloader", MB_ICONERROR); 
        va_end(va);
    }

}

using namespace modloader;

// Singleton
static CModLoader loader;





/*
 * DllMain
 *      Second entry-point, yeah, really
 */
extern "C" __declspec(dllexport) // Needed on MinGW...
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    // Startup or Shutdown modloader
    bool bResult = true; char buffer[256];
    
    switch(fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            loader.Patch();
            break;
            
        case DLL_PROCESS_DETACH:
            bResult = loader.Shutdown();    /* Shutdown the loader if it hasn't shutdown yet */
            break;
    }
    return bResult;
}


namespace modloader
{
    static LPTOP_LEVEL_EXCEPTION_FILTER PrevFilter = 0;
    
    /*
     *  Our unhandled exception handler 
     */
    static LONG CALLBACK modloader_UnhandledExceptionFilter(LPEXCEPTION_POINTERS pException)
    {
        LogException(pException);
        
        // We should shutdown our loader at all cost
        Log("Calling loader.Shutdown();");
        loader.Shutdown();
        
        // Continue exception propagation
        return (PrevFilter? PrevFilter(pException) : EXCEPTION_CONTINUE_SEARCH);  // I'm not really sure about this return
    }
    
    /*
     * CModLoader::ParseCommandLine
     *      Parses the gta_sa.exe (or whatever the executable name is) command line 
     */
    void CModLoader::ParseCommandLine()
    {
        char buf[512];
        wchar_t **argv; int argc; 
        argv = CommandLineToArgvW(GetCommandLineW(), &argc);
        
        Log("\nReading command line");

        if(!argv) // CommandLineToArgvW failed!!
        {
            Log("Failed to read command line. CommandLineToArgvW() failed.");
            return;
        }
        
        /* Converts wchar_t buffer into char* ASCII buffer (not UTF-8, alright?) */
        auto toASCII = [](const wchar_t* wstr, char* abuf, size_t asize)
        {
            return !!WideCharToMultiByte(CP_ACP, 0, wstr, -1, abuf, asize, NULL, NULL);
        };
        
        /* Iterate on the arguments */
        for(int i = 0; i < argc; ++i)
        {
            /* If an actual argument... */
            if(argv[i][0] == '-')
            {
                wchar_t *arg = (i+1 < argc? argv[i+1] : nullptr);
                wchar_t *argname = &argv[i][1];
                
                /* Mod argument */
                if(!_wcsicmp(argname, L"mod"))
                {
                    // Is argument after mod argument valid?
                    if(arg == nullptr)
                    {
                        // ...nope
                        Log("Failed to read command line. Mod command line is incomplete.");
                        break;
                    }
                    else
                    {
                        // ...yep
                        if(toASCII(arg, buf, sizeof(buf)))
                        {
                            // Force exclusion and include the specified mod
                            modsfolder.flags.bForceExclude = true;
                            modsfolder.AddIncludedMod(buf);
                            modsfolder.AddPriority(buf, 80);
                            
                            //
                            Log("Command line argument received: -mod \"%s\"", buf);
                        }
                    }
                }
            }
        }
        
        LocalFree(argv);
        Log("Done reading command line");
    }
     
    
    /*
     * CModLoader::Startup
     *      Start ups modloader, loading plugins and other stuffs
     */
    bool CModLoader::Startup()
    {
        // If already started up or modloader directory does not exist, do nothing
        if(this->bWorking == false && IsDirectoryA("modloader"))
        {
            char gameFolder[MAX_PATH * 2];
            
            // First of all of everything else in the world, open the log file
            this->OpenLog();
            
            // Read basic config from "modloader/modloader.ini"
            // This will tell us if we need to log stuff
            this->ReadBasicConfig("modloader/modloader.ini");

            // Basic logging happened, close log if logging is disabled
            if(!this->bEnableLog)
            {
                Log("Logging is disabled. Closing log file...");
                this->CloseLog();
            }
            
            /* --- */
            GetCurrentDirectoryA(sizeof(gameFolder), gameFolder);
            this->gamePath = gameFolder;
            MakeSureStringIsDirectory(this->gamePath);

            /* Make sure the main paths do exist */
            {
                static const char* folders[] =
                {
                    "modloader/.data",
                    "modloader/.data/plugins",
                    "modloader/.data/cache",
                    0
                };
                
                std::string modsPath = "modloader\\";
                this->cachePath = modsPath + ".data\\cache\\";
                this->pluginsPath = modsPath + ".data\\plugins\\";
                
                for(const char** folder = folders; *folder; ++folder)
                {
                    Log("Making sure directory \"%s\" exists...", *folder);
                    if(!MakeSureDirectoryExistA(*folder)) Log("\t...it didn't, created it.");
                }
            }
            
            /* Register modloader methods and vars */
            this->loader.gamepath  = this->gamePath.c_str();
            //this->loader.modspath  = this->modsPath.c_str();
            this->loader.modspath = nullptr;
            this->loader.cachepath = this->cachePath.c_str();
            this->loader.pluginspath = this->pluginsPath.c_str();
            this->loader.Log   = &Log;
            this->loader.vLog  = &vLog;
            this->loader.Error = &Error;
            this->loader.NewChunkLoaded = memory_pointer(0x590D00).get();
            
            
            // Do init
            this->ParseCommandLine();   // Parse command line arguments, such as "-mod modname"
            this->LoadPlugins();        // Load plugins, such as std-img.dll at /modloader/.data/plugins
            this->StartupPlugins();     // Call Startup methods from plugins
            this->PerformSearch();      // Search for mods at /modloader, but do not install them yet
            this->HandleFiles();        // Install mods found on search above
            this->PosProcess();         // After mods are installed, notify all plugins, they may want to do some pos processing
            this->SetupLoadbarChunks(); // Setups how much the game loading bar should go forward
            this->ClearFilesData();     // Clear files data (such as paths) freeing memory
            
            Log("\nGame is ready!\n");

            /* Just make sure we're in the main folder before going back */
            SetCurrentDirectoryA(gameFolder);

            this->bWorking = true;
            return true;
        }
        else { return false; }
    }

    /*
     * CModLoader::OnShutdown
     *  Shutdowns modloader, unloading plugins and other stuffs
     */
    bool CModLoader::Shutdown()
    {
        /* Don't shutdown if not started up or already shut down */
        if(this->bWorking == false)
            return true;
        
        Log("\nShutdowing modloader...");
        this->UnloadPlugins();
        Log("modloader has been shutdown.");
        this->CloseLog();
        
        this->bWorking = false;
        return true;
    }

    /*
     * CModLoader::OpenLog
     *      Open modloader log file
     */
    void CModLoader::OpenLog()
    {
        /* Open log file */
#ifndef LOGFILE_AS_STDOUT
        if(!logfile) logfile = fopen("modloader/modloader.log", "w");
#else
        logfile = stdout;
#endif
        
        /* Log header, with version number and isdev information */
        Log("========================== modloader %d.%d.%d %s==========================\n",
                MODLOADER_VERSION_MAJOR, MODLOADER_VERSION_MINOR, MODLOADER_VERSION_REVISION,
                MODLOADER_VERSION_ISDEV? "Development Build " : "");
    }
    
    /*
     * CModLoader::CloseLog
     *      Closes modloader log file
     */
    void CModLoader::CloseLog()
    {
        SYSTEMTIME time;
        GetLocalTime(&time);
        
        Log(
			"\n******************************************************************************\n"
			"> Logging finished: %.2d:%.2d:%.2d\n"
			"  Powered by sa-modloader (%s)\n"
			"******************************************************************************\n",
			time.wHour, time.wMinute, time.wSecond,
            modurl);

        if(logfile && logfile != stdout)
        {
            fclose(logfile);
            logfile = 0;
        }
    }
    
    /*
     * CModLoader::Patch
     *      Patches the game code
     */
    void CModLoader::Patch()
    {
        injector::address_manager::set_name("modloader");
        
        // I can't use LoadLibrary at DllMain, so use it somewhere at the beggining, before the startup of the game engine
        if(true)
        {
            auto& gvm = injector::address_manager::singleton();
            
            // Find the game version
            gvm.init_gvm();
            
            // Check if we're going into the right game version
            if(gvm.IsUnknown())
                Error("Modloader could not detect your game executable version.");
            else if(gvm.IsSA() == false)
                Error("Modloader was built for GTA San Andreas! This game is not supported.");
            else if(gvm.GetMajorVersion() != 1 || gvm.GetMinorVersion() != 0 || gvm.IsUS() == false)
                Error("Modloader still do not support game versions other than HOODLUM GTA SA 1.0 US");
            else
            {
                typedef function_hooker_stdcall<0x8246EC, int(HINSTANCE, HINSTANCE, LPSTR, int)> winmain_hook;
                typedef function_hooker<0x53E58E, void(const char*)>    loadbar_hook;
                typedef function_hooker<0x53C6DB, void(void)>           onreload_hook;
                typedef function_hooker<0x748C30, char(void)>           onsplash_hook;
                
                /* Hook WinMain to do modloader stuff */
                make_function_hook<winmain_hook>([](winmain_hook::func_type WinMain,
                                                    HINSTANCE& hInstance, HINSTANCE& hPrevInstance, LPSTR& lpCmdLine, int& nCmdShow)
                {
                    // Setup exception filter, we need (whenever possible) to call shutdown before a crash or something
                    PrevFilter = SetUnhandledExceptionFilter(modloader_UnhandledExceptionFilter);

                    // Startup the loader and call original WinMain... Shutdown the loader after WinMain.
                    ::loader.Startup();
                    int result = WinMain(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
                    ::loader.Shutdown();

                    return result;
                });
                
                /* Hook CGame::InitialiseEssentialsAfterRW to notify about the main splash */
                make_function_hook<onsplash_hook>([](onsplash_hook::func_type func)
                {
                    ::loader.OnSplash();
                    return func();
                });
                
                /* Hook CGame::Initialise to notify about load bar being started */
                make_function_hook<loadbar_hook>([](loadbar_hook::func_type func, const char*& gtadat)
                {
                    ::loader.OnLoadBar();
                    return func(gtadat);
                });
                
                /* Hook CGame::ReInitObjectVariables to notify a loadgame */
                make_function_hook<onreload_hook>([](onreload_hook::func_type func)
                {
                    ::loader.OnReload();
                    return func();
                });
                
            }
        }
    }


    /*
     * CModLoader::LoadPlugins
     *      Loads all plugins
     */
    void CModLoader::LoadPlugins()
    {
        Log("\nLooking for plugins...");

        // Load basic plugins only if bEnablePlugin is true
        if(this->bEnablePlugins)
        {
            // Goto plugins folder
            scoped_chdir xdir("modloader\\.data\\plugins");
            {
                // Read plugins priority
                {
                    modloader::ini ini;
                    
                    Log("Reading plugins.ini");
                    if(ini.load_file("plugins.ini"))
                    {
                        // Check out plugins priority overrides
                        for(auto& pair : ini["PRIORITY"])
                        {
                            auto& key = pair.first;
                            auto& value = pair.second;
                            plugins_priority.emplace(NormalizePath(key), std::stoi(value));
                        }
                    }
                    else Log("Failed to read plugins.ini");
                }
                
                
                // Iterate on each dll plugin
                ForeachFile("*.dll", false, [this](ModLoaderFile& file)
                {
                    LoadPlugin(file.filepath, false);
                    return true;
                });

                // Finish the load
                this->SortPlugins();
                this->BuildExtensionMap();
            }
        }
        else
            Log("Plugins ignored!");
    }

    /*
     * CModLoader::UnloadPlugins
     *      Unloads all plugins
     */
    void CModLoader::UnloadPlugins()
    {
        // Unload one by one, calling OnShutdown callback before the unload
        for(auto it = this->plugins.begin(); it != this->plugins.end();   )
        {
            this->UnloadPlugin(*it, false);
            it = this->plugins.erase(it);
        }
        // Clear plugin list after all
        this->plugins.clear();
    }
    
    /*
     * CModLoader::LoadPlugin
     *      Load a new plugin
     *      @pluginPath: Plugin path
     */
    bool CModLoader::LoadPlugin(const char* pluginPath, bool bDoStuffNow)
    {
        const char* modulename = pluginPath;
        modloader_fGetPluginData GetPluginData;
        modloader_plugin_t data;
        bool bFail = false;
        HMODULE module;
        int priority = -1;

        // Check if module is on plugins overriding priority list
        {
            auto it = this->plugins_priority.find(NormalizePath(pluginPath));
            if(it != this->plugins_priority.end())
            {
                priority = it->second;
            }
        }
        
        // Load plugin module
        if(module = LoadLibraryA(modulename))
        {
            Log("Loading plugin module '%s'", modulename);
            GetPluginData = (modloader_fGetPluginData)(GetProcAddress(module, "GetPluginData"));

            // Setup new's plugin data and fill plugin data from the plugin module
            memset(&data, 0, sizeof(data));
            data.modloader = &this->loader;
            data.pModule   = module;
            data.priority  = priority == -1? 50 : priority;
            
            // Check if plugin has been loaded sucessfully
            if(GetPluginData)
            {
                GetPluginData(&data);

                // Override data.priority if requested to
                if(priority != -1)
                {
                    Log("Overriding priority, from %d to %d", data.priority, priority);
                    data.priority = priority;
                }
                
                // Check version incompatibilities (ignore for now, no incompatibilities)
                if(false && (data.major && data.minor && data.revision))
                {
                    bFail = true;
                    Log("Failed to load module '%s', version incompatibility detected.", modulename);
                }
                // Check if plugin was written to a (future) version of modloader, if so, we need to be updated
                else if(data.major  > MODLOADER_VERSION_MAJOR
                     ||(data.major == MODLOADER_VERSION_MAJOR && data.minor > MODLOADER_VERSION_MINOR))
                     // We don't check VERSION_REVISION because on revisions we don't intent to break structures, etc
                {
                    bFail = true;
                    Log("Failed to load module '%s', it requieres a newer version of modloader!\n"
                "Update yourself at: %s",
                modulename, modurl);
                }
                else if(data.priority == 0)
                {
                    bFail = true;
                    Log("Plugin module '%s' will not be loaded. It's priority is 0", modulename);
                }
                else
                {
                    data.name = data.GetName? data.GetName(&data) : "NONAME";
                    data.version = data.GetVersion? data.GetVersion(&data) : "NOVERSION";
                    data.author = data.GetAuthor? data.GetAuthor(&data) : "";
                    Log("Plugin module '%s' loaded as %s %s %s %s",
                        modulename,
                        data.name, data.version,
                        data.author? "by" : "", data.author);
                }
            }
            else
            {
                bFail = true;
                Log("Could not call GetPluginData() for module '%s'", modulename);
            }
                
            // On failure, unload module, on success, push plugin to list
            if(bFail)
                FreeLibrary(module);
            else
            {
                // Push plugin to list
                this->plugins.emplace_back(std::move(data));
                if(bDoStuffNow) // Start plugin and sort modloader extension table?
                {
                    this->StartupPlugin(this->plugins.back());
                    this->SortPlugins();
                }
            }
        }
        else
            Log("Could not load plugin module '%s'", modulename);

        return !bFail;
    }
    

    
    /*
     * CModLoader::UnloadPlugin
     *      Unloads a specific plugin -- (remove plugin from plugin list)
     *      @pluginName: The plugin name
     */
    bool CModLoader::UnloadPlugin(const char* pluginName)
    {
        for(auto it = this->plugins.begin(); it != this->plugins.end(); ++it)
        {
            if(!strcmp(it->name, pluginName, true))
                return this->UnloadPlugin(*it, true);
        }
        
        Log("Could not unload plugin named '%s' because it was not found", pluginName);
        return false;
    }
    
    /*
     *  CModLoader::UnloadPlugin
     *      Unloads a specific plugin
     *      @plugin: The plugin instance
     *      @bRemoveFromList: Removes plugin from this->plugins list,
     *                        when true be warned that iterators to this element will get invalidated!
     */
    bool CModLoader::UnloadPlugin(ModLoaderPlugin& plugin, bool bRemoveFromList)
    {
        scoped_chdir xdir(this->loader.pluginspath);
        
        Log("Unloading plugin \"%s\"", plugin.name);
        
        // Call the plugin shutdown notification
        if(plugin.OnShutdown) plugin.OnShutdown(&plugin);
        // Free the module
        if(plugin.pModule) FreeLibrary((HMODULE)(plugin.pModule));
        // Remove from plugins list if requested (invalidating some iterators)
        if(bRemoveFromList) this->plugins.remove(plugin);
        
        return true;
    }
   
    /*
     * CModLoader::StartupPlugins
     *      Call plugin OnStartup method
     */
    bool CModLoader::StartupPlugin(ModLoaderPlugin& data)
    {
        scoped_chdir xdir(this->loader.pluginspath);

        Log("Starting up plugin \"%s\"", data.name);
        if(data.OnStartup && data.OnStartup(&data))
        {
            Log("Failed to startup plugin '%s', unloading it.", data.name);
            this->UnloadPlugin(data, false);
            return false;
        }
        return true;
    }
            
    /*
     * CModLoader::StartupPlugins
     *      Call plugins OnStartup method
     */
    void CModLoader::StartupPlugins()
    {
        Log("\nStarting up plugins...");
        for(auto it = this->plugins.begin(); it != this->plugins.end();   )
        {
            if(StartupPlugin(*it))
                ++it;
            else
                it = this->plugins.erase(it);
        }
    }

    
    /*
     * CModLoader::PeformSearch
     *      Search for mods
     */
    void CModLoader::PerformSearch()
    {
        this->PerformSearch(this->modsfolder);
    }
    
    
    /*
     * CModLoader::PeformSearch
     *      Search for mods at @modfolder
     */
    void CModLoader::PerformSearch(ModFolderInfo& modfolder)
    {
        //  Iterate on all folders at modfolder dir, and treat them as a mod entity
        Log("\nLooking for mods at \"%s\"...", modfolder.path.c_str());
        {
            scoped_chdir xdir(modfolder.path.c_str());
            {
                // Load this config
                LoadConfigFromINI("modloader.ini", modfolder);
                
                if(modfolder.flags.bIgnoreAll == false)
                {
                    // Iterate on all files, to load mods
                    ForeachFile("*.*", false, [this, &modfolder](ModLoaderFile& file)
                    {
                        // Must be a directory to be a mod
                        if(file.is_dir)
                        {
                            // If the file is on the exclusion list, don't load it
                            if(!modfolder.IsModIgnored(file))
                            {
                                this->ReadModf(modfolder, file.filepath);
                            }
                            else
                                Log("Ignoring mod at \"%s\"", file.filepath);
                        }
                        return true;
                    });
                }
                else
                    Log("Ignoring all mods\n");
            }
        }
        
        // Perform search on the child modloader folders we found above
        for(auto& child : modfolder.childs)
        {
            PerformSearch(child);
        }
        
        modfolder.ProcessPriorities();
    }

    /*
     * CModLoader::ReadBasicConfig
     *      Loads main config file from @filename
     */
    void CModLoader::ReadBasicConfig(const char* filename)
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

               if(!compare(key, "ENABLE_PLUGINS", false))
                    this->bEnablePlugins = to_bool(value);
                else if(!compare(key, "LOG_ENABLE", false))
                    this->bEnableLog = to_bool(value);
                else if(!compare(key, "LOG_IMMEDIATE_FLUSH", false))
                    bImmediateFlush = to_bool(value);
            }
            
        }
        else
            Log("Failed to load basic config file");
    }
    
    /*
     * CModLoader::LoadConfigFromINI
     *      Loads ini config from @filename into @modx 
     */
    void CModLoader::LoadConfigFromINI(const char* filename, ModFolderInfo& modx)
    {
        modloader::ini data;
        
        Log("Loading modloader config file %s", filename);
        if(data.load_file(filename))
        {
            // Read CONFIG section
            for(auto& pair : data["CONFIG"])
            {
                auto& key = pair.first;
                auto& value = pair.second;
                
                if(!compare(key, "IGNORE_ALL", false))
                    modx.flags.bIgnoreAll = to_bool(value);
                else if(!compare(key, "EXCLUDE_ALL", false))
                    modx.flags.bExcludeAll = to_bool(value);
            }
            
            // Read excluded files and extensions list
            for(auto& pair : data["EXCLUDE_FILES"])
            {
                auto& exclude = pair.first;
                modx.AddIgnoredFile(exclude.c_str());
            }
            
            // Read mods that must be loaded even when EXCLUDE_ALL is TRUE
            for(auto& pair : data["INCLUDE_MODS"])
            {
                auto& include = pair.first;
                modx.AddIncludedMod(include.c_str());
            }
            
            // Read priority list
            for(auto& pair : data["PRIORITY"])
            {
                auto& mod      = pair.first;
                auto& priority = pair.second;
                modx.AddPriority(mod.c_str(), std::stoi(priority));
            }
            
        }
        else
            Log("Failed to load modloader config file");
    }
    
    /*
     * CModLoader::ReadModf
     *      Read a mod
     *      @modsbase: The modloader folder that the mod is at, normally "modloader\\"
     *      @modfolder_cc: Mod folder
     */
    void CModLoader::ReadModf(ModFolderInfo& modsbase, const std::string& modfolder)
    {
        /* Go into the mod folder to work inside it */
        {
            scoped_chdir xdir(modfolder.c_str());

            char buffer[MAX_PATH];
            GetCurrentDirectoryA(sizeof(buffer), buffer);
 
            // Push a new modification into the mods list
            modsbase.mods.emplace_back();
            auto& mod = modsbase.mods.back();
            mod.name = &modfolder.c_str()[GetLastPathComponent(modfolder)];
            mod.id = this->currentModId++;
            mod.path = modsbase.path + modfolder;
            mod.fullPath = buffer;
            
            Log("Reading mod \"%s\" (%d) at \"%s\"...", mod.name.c_str(), mod.id, modfolder.c_str());
            
            // Iterate on the mod files...
            ForeachFile("*.*", true, [this, &mod, &modsbase](ModLoaderFile& file)
            {
                // Checkout if this file/filetype is being ignored, otherwise continue the load...
                if(!modsbase.IsFileIgnored(file))
                {
                    this->ReadFile(modsbase, file, mod);
                }
                return true;
            });
        }     
    }
    
    
    /*
     *  CModLoader::ReadFile
     *      Read file
     */
    void CModLoader::ReadFile(ModFolderInfo& modsbase, ModLoaderFile& file, CModLoader::ModInfo& mod)
    {
        decltype(FileInfo::callMe) callMe;
        
        /* Continue the 'file' setup */
        file.modname = mod.name.data();
        file.mod_id = mod.id;
        file.file_id = this->currentFileId++;
        file.modpath = mod.path.data();
        file.modfullpath = mod.fullPath.data();      
        
        /* See if we can find a handler for this file */
        ModLoaderPlugin* handler = this->FindFileHandler(file, callMe);

        if(handler || callMe.size())
        {
            /* If this is a directory and a handler for it has been found,
            * don't go inside this folder (recursive search). */
            if(handler) file.recursion = false;
            
            /* Setup fileInfo */
            mod.files.emplace_back();
            auto& fileInfo = mod.files.back();
            fileInfo.id = file.file_id;
            fileInfo.parentMod = &mod;
            fileInfo.handler = handler;
            fileInfo.isDir = file.is_dir;
            fileInfo.callMe = std::move(callMe);

            /* Continue setupping fileInfo and resetup 'file' to contain pointers to fileInfo (static) */
            file.filename = (fileInfo.fileName = file.filename).data();
            file.filepath = (fileInfo.filePath = file.filepath).data();
            file.filext   = (fileInfo.fileExtension = file.filext).data();
            
            /* Copy C POD data into fileInfo */
            memcpy(&fileInfo.data, &file, sizeof(file));
            
            if(handler)
                Log("Handler for file \"%s\" (%d) is \"%s\"", file.filepath, file.file_id, handler->name);
        }
        else if(!strcmp(file.filepath, "modloader\\", false))
        {
            // Child modloader folder!!!
            file.recursion = false;
            modsbase.AddChild(GetFilePath(file));
            Log("Found child modloader folder");
        }
        else
        {
            //
            Log("Handler for file \"%s\" (%d) not found", file.filepath, file.file_id);
        }
    }
    
    /*
     *  CModLoader::ReadFile
     *      Read file
     */
    bool CModLoader::HandleFile(CModLoader::FileInfo& file)
    {
        bool bResult = false;
        
        // Process file using the handler
        if(auto* handler = file.handler)
        {
            const char* pluginName = handler->name;
            const char* filePath   = file.filePath.data(); 
            
            //Log("Handling file \"%s\\%s\" by plugin \"%s\"", file.parentMod->name.c_str(), filePath, pluginName);
            if(handler->ProcessFile && !handler->ProcessFile(handler, &file.data))
            {
                bResult = true;
            }
            else
                // That's not my fault, hen
                Log("Handler \"%s\" failed to process file \"%s\"", pluginName, filePath);
        }
        
        // Call ProcessFile for the plugins that request it, even that hey don't handle the file itself
        if(file.callMe.size())
        {
            for(auto* handler : file.callMe)
            {
                if(handler && handler->ProcessFile)
                    handler->ProcessFile(handler, &file.data);
            }
        }
        
        return bResult;
    }
    
    /*
     * IsFileHandlerForFile
     *      Checks if an plugin can check a specific file. It also sets the @plugin.checked flag.
     *      @plugin: The plugin to handle the file
     *      @file: File to be handled
     */
    inline bool IsFileHandlerForFile(ModLoaderPlugin& plugin, ModLoaderFile& file)
    {
        return( !plugin.checked         /* Has not been checked yet */
             && (plugin.checked = true) /* Mark as checked */
            
             /* call CheckFile */
             &&  plugin.CheckFile  && (plugin.CheckFile(&plugin, &file) == MODLOADER_YES)
            );
    }
    
    /*
     * CModLoader::FindFileHandler
     *      Finds a plugin to handle the file
     *      @file: File to be handled
     *      @return: Plugin to handle the file, or nullptr if no plugin found
     * 
     *      This uses an algorithim that first searches for plugins that commonly use the @file extension
     *      and if no handler for file has been found, it checks on other plugins.
     */
    ModLoaderPlugin* CModLoader::FindFileHandler(ModLoaderFile& file, decltype(FileInfo::callMe)& callMe)
    {
        ModLoaderPlugin* handler = 0;

        auto CheckForHandler = [&file, &handler, &callMe](ModLoaderPlugin& plugin)
        {
            file.call_me = false;
            if(IsFileHandlerForFile(plugin, file))
            {
                handler = &plugin;
                return true;
            }
            else if(file.call_me == true)
            {
                callMe.push_back(&plugin);
            }
            return false;
        };
        
        /* First, search for the handler at the vector for the extension,
         * it is commonly possible that the handler for this file is here and we don't have to call CheckFile on all plugins
         * At this point all 'plugin.checked' are false.
         */
        for(auto& pPlugin : this->extMap[file.filext])
        {
            if(CheckForHandler(*pPlugin)) break;
        }

        /*
         *  Iterate on all plugins to mark all 'plugin.checked' flags as false,
         *  but before, if no handler has been found and a plugin was not checked yet, check it.
         */
        for(auto& plugin : this->plugins)
        {
            if(!handler)
            {
                CheckForHandler(plugin);
            }
                
            /* Set the 'plugin.checked' state to false, so on the next call to FindFileHandler it is false */
            plugin.checked = false;
        }
        
        return handler;
    }
    

    
    /*
     * CModLoader::HandleFiles
     *      Handle all files readen
     */
    void CModLoader::HandleFiles()
    {
        Log("\nHandling files...");

        // Gets all mod folders from modsfolder and all it's childs
        for(ModFolderInfo& folder : modsfolder.GetAllFolders())
        {
            scoped_chdir xdir(folder.path.c_str());
                    
            // Iterate on each modification
            for(auto& mod : folder.mods)
            {
                scoped_chdir xdir((mod.name + "\\").c_str());
                
                // Handle all the files
                for(auto& file : mod.files)
                    this->HandleFile(file);
            }
        }
    }
            
    /*
     * CModLoader::PosProcess
     *      Call all 'plugin.PosProcess' methods
     */   
    void CModLoader::PosProcess()
    {
        scoped_chdir xdir(this->gamePath.c_str());
        
        Log("\nPos processing...");
        for(auto& plugin : this->plugins)
        {
            Log("Pos processing plugin \"%s\"", plugin.name);
            if(plugin.PosProcess && plugin.PosProcess(&plugin))
                Log("Plugin \"%s\" failed to pos process", plugin.name);
        }
    }

    /*
     * CModLoader::OnLoadBar
     *      Call all 'plugin.OnSplash' methods
     */  
    void CModLoader::OnSplash()
    {
        scoped_chdir xdir(this->gamePath.c_str());

        Log("\nSplash screen events...");
        for(auto& plugin : this->plugins)
        {
            if(plugin.OnSplash && plugin.OnSplash(&plugin))
                ;
        }
        Log("Done splash screen events.\n");
    }
    
    /*
     * CModLoader::OnLoadBar
     *      Call all 'plugin.OnLoad' methods
     */  
    void CModLoader::OnLoadBar()
    {
        scoped_chdir xdir(this->gamePath.c_str());

        Log("\nLoad time events...");
        for(auto& plugin : this->plugins)
        {
            if(plugin.OnLoad && plugin.OnLoad(&plugin))
                ;
        }
        Log("Done load time events.\n");
    }
    
    /*
     * CModLoader::OnReload
     *      Call all 'plugin.OnReload' methods
     */  
    void CModLoader::OnReload()
    {
        scoped_chdir xdir(this->gamePath.c_str());

        Log("\nReload time events...");
        for(auto& plugin : this->plugins)
        {
            if(plugin.OnReload && plugin.OnReload(&plugin))
                ;
        }
        Log("Done reload time events.\n");
    }

    /*
     *  CModLoader::SetupLoadbarChunks
     *      Setup load bar count
     */
    void CModLoader::SetupLoadbarChunks()
    {
        // Get original number of chunks...
        int totalChunks = ReadMemory<int>(0x590D67 + 1, true);
        
        // ...and add by the chunk count of each plugin
        for(auto& plugin : this->plugins)
            totalChunks += plugin.loadbar_chunks;
        
        // Patch it
        WriteMemory<int>(0x590D67 + 1, totalChunks, true);
        WriteMemory<int>(0x590D2A + 1, totalChunks, true);
    }
    
}

