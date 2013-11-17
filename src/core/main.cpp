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


#include <windows.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <map>
#include <vector>
#include <list>
#include "Injector.h"
#include "GameInfo.h"

#if false && !defined(NDEBUG) && defined(__MINGW32__)
#define LOGFILE_AS_STDOUT
#endif

/*
 * TODO: Plugin Priority Config File
 *       ~~~~~~ Exclusion Config File
 *      Thread-safe ?
 *      Export ReadModf & LoadPlugin / UnloadPlugin
 *      Export FindFileHandler ?
 * 
 *
 */

namespace modloader
{
    static const char* modurl = "<LINK/2012>";/* TODO, web address for mod, gta forums? google code? gta garage? dunno */
    
    // log stream
    static FILE* logfile = 0;
    
    /*
     * Log
     *      Logs something into the log file 'logfile'
     *      @msg: (Format) Message to log
     *      @...: Additional args to format
     */
    static void Log(const char* msg, ...)
    {
        if(logfile)
        {
            va_list va; va_start(va, msg);
            vfprintf(logfile, msg, va);
            fputc('\n', logfile);
            fflush(logfile);
            va_end(va);
        }
    }
    
    /*
     * Error
     *      Displays a error message box
     *      @msg: (Format) Error to display
     *      @...: Additional args to format
     */
    static void Error(const char* msg, ...)
    {
        va_list va; va_start(va, msg);
        char buffer[1024];
        vsprintf(buffer, msg, va);
        MessageBoxA(NULL, buffer, "modloader", MB_ICONERROR); 
        va_end(va);
    }
    
    /*
     * CModLoader
     *      Mod Loader Core Class
     */
    class CModLoader
    {
        private:
            struct ModInfo;
            struct FileInfo;
            
            struct ModInfo
            {
                std::list<FileInfo> files;

                std::string  path;          /* relative to main game dir (ex: "modloader/mymod") */
                std::string  fullPath;      /* full path */
                std::string  name;          /* modname */
                unsigned int id;            /* Note: This id is (probably) unique on each game call */
            };
            
            struct FileInfo
            {
                ModLoaderPlugin* handler;
                ModInfo*         parentMod;
                ModLoaderFile    data;
                
                std::string     fileName;       /* name */
                std::string     fileExtension;  /* ext */
                std::string     filePath;       /* full */
                
                unsigned int    id;         /* Note: This id is (probably) unique on each game call */
                bool            isDir;
            };

        private:
            modloader_t loader;
            
            std::string gamePath;
            std::string modsPath;
            std::string cachePath;
            
            unsigned int currentModId;
            unsigned int currentFileId;

            std::list<ModInfo> mods;
            std::list<ModLoaderPlugin> plugins;
            std::map<std::string, std::vector<ModLoaderPlugin*>> extMap;
            
        private:
            void LoadPlugins();
            void UnloadPlugins();
            void PerformSearch();
            ModLoaderPlugin* FindFileHandler(const ModLoaderFile& file);
            
            bool UnloadPlugin(ModLoaderPlugin& plugin, bool bRemoveFromList = false);
            
            void PosProcess();
            
            /*
             *  Freeup memory taken by mods information
             */
            void ClearFilesData()
            {
                mods.clear();
            }
            
            /*
             *  Handle all files readen
             */
            void HandleFiles()
            {
                Log("\nHandling files...");
                SetCurrentDirectoryA("modloader\\");
                for(auto& mod : this->mods)
                {
                    SetCurrentDirectoryA((mod.name + "\\").c_str());
                    {
                        for(auto& file : mod.files)
                        {
                            this->HandleFile(file);
                        }
                    }
                    SetCurrentDirectoryA("..\\");
                }
                SetCurrentDirectoryA("..\\");
            }
            
            /*
             *  Sorts plugins by priority and name order
             */
            void SortPlugins()
            {
                this->plugins.sort([](const ModLoaderPlugin& a, const ModLoaderPlugin& b)
                {
                    return (a.priority != b.priority?
                            a.priority < b.priority :
                            strcmp(a.name, b.name) < 0);
                });
            }
            
            /*
             *  Builds extensions std::map, used in the file checking algorithm
             */
            void BuildExtensionMap()
            {
                for(auto& plugin : this->plugins)
                {
                    for(size_t i = 0, n = plugin.extable_len;
                        i < n; ++i)
                        this->extMap[plugin.extable[i]].push_back(&plugin);
                }
            }
         
            /*
             *  Call plugin OnStartup method
             */
            void StartupPlugin(ModLoaderPlugin& data)
            {
                if(data.OnStartup && data.OnStartup(&data))
                {
                    Log("Failed to startup plugin '%s', unloading it.", data.name);
                    this->UnloadPlugin(data, true);
                }
            }
            
            /*
             *  Call plugins OnStartup method
             */
            void StartupPlugins()
            {
                for(auto& plugin : this->plugins)
                    StartupPlugin(plugin);
            }
            
            
            
        public:
            CModLoader() : currentModId(0), currentFileId(0)
            { }
            
            void Patch();
            bool Startup();
            bool Shutdown();
            
            void ReadModf(const char* modfolder);
            void ReadFile(ModLoaderFile& file, CModLoader::ModInfo& mod);
            bool HandleFile(FileInfo& file);
            
            bool LoadPlugin(const char* pluginPath, bool bDoStuffNow);
            bool UnloadPlugin(const char* pluginName);
            
    };
    

}

using namespace modloader;

// Singleton
static CModLoader loader;


/* Plugins are equal only (and only if) they point to the same data space */
bool operator==(const ModLoaderPlugin& a, const ModLoaderPlugin& b)
{
    return (&a == &b);
}


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
            bResult = loader.Shutdown();
            break;
    }
    return bResult;
}


namespace modloader
{
    /*
     * CModLoader::Startup
     *      Start ups modloader, loading plugins and other stuffs
     */
    bool CModLoader::Startup()
    {
        // If modloader directory does not exist, do nothing
        if(IsDirectoryA("modloader"))
        {
            char gameFolder[MAX_PATH * 2];
            
            /* Open log file */
#ifndef LOGFILE_AS_STDOUT
            if(!logfile) logfile = fopen(".\\modloader\\modloader.log", "w");
#else
            logfile = stdout;
#endif
            
            /* Log header, with version number and isdev information */
            Log("========================== modloader %d.%d.%d %s==========================\n",
                MODLOADER_VERSION_MAJOR, MODLOADER_VERSION_MINOR, MODLOADER_VERSION_REVISION,
                MODLOADER_VERSION_ISDEV? "Development Build " : "");
            
            /* Register modloader methods */
            this->loader.Log   = &Log;
            this->loader.Error = &Error;       

            /* --- */
            GetCurrentDirectoryA(sizeof(gameFolder), gameFolder);
            this->gamePath = gameFolder;
            MakeSureStringIsDirectory(this->gamePath);
            
            /* "/modloader/.plugins" is used to store plugins .dll */
            Log("Making sure directory \"\\modloader\\.plugins\\\" exists...");
            if(!MakeSureDirectoryExistA("modloader/.plugins")) Log("...it didn't, created it.");
           
            /* "/modloader/.cache" is used for faster loading */
            Log("Making sure directory \"\\modloader\\.cache\\\" exists...");
            if(!MakeSureDirectoryExistA("modloader/.cache")) Log("...it didn't, created it.");
            
            /* get full paths */
            this->modsPath = this->gamePath + "modloader\\";
            this->cachePath = this->modsPath + ".cache\\";
            
            // Do init
            this->LoadPlugins();        /* Load plugins, such as std-img.dll at /modloader/.plugins */
            this->StartupPlugins();     /* Call Startup methods from plugins */
            this->PerformSearch();      /* Search for mods at /modloader, but do not install them yet */
            this->HandleFiles();        /* Install mods found on search above */
            this->PosProcess();         /* After mods are installed, notify all plugins, they may want to do some pos processing */
            this->ClearFilesData();     /* Clear files data (such as paths) freeing memory */
            
            // =)
            Log("\nGame is ready!\n");

            /* Just make sure we're in the main folder before going back */
            SetCurrentDirectoryA(gameFolder);
        }
        else { /* Can't even log */ }
        
        return true;

    }

    /*
     * CModLoader::OnShutdown
     *  Shutdowns modloader, unloading plugins and other stuffs
     */
    bool CModLoader::Shutdown()
    {
        SYSTEMTIME time;
        GetLocalTime(&time);
        
        Log("Shutdowing modloader...");
        this->UnloadPlugins();
        Log("modloader has been shutdown.");
        
		Log(
			"\n*********************************\n"
			"> Logging finished: %.2d:%.2d:%.2d\n"
			"  Powered by modloader (%s)\n"
			"*********************************\n",
			time.wHour, time.wMinute, time.wSecond,
            modurl);

        
#ifndef LOGFILE_AS_STDOUT       
        if(logfile) { fclose(logfile); logfile = 0; }
#endif
        return true;
    }

    /*
     * CModLoader::Patch
     *      Patches the game code
     */
    
    static GameInfo gameInfo;
    
    
    extern "C" void CALL_Startup() { loader.Startup(); }
    extern "C" void HOOK_Init();
    extern "C" void* _gtasa_try;
    
    void CModLoader::Patch()
    {
        gameInfo.PluginName = "modloader";
        
        // I can't use LoadLibrary at DllMain, so use it somewhere at the beggining, before the startup of the game engine
        gameInfo.DelayedDetect([](GameInfo& info)
        {
            // Shit, we still don't support a delayed style of detection, how to handle it? Help me!
            if(info.IsDelayed())
                Error("Modloader does not support this executable version [DELAYED_DETECT_ERROR]");
            //--
            else if(info.GetGame() != info.SA)
                Error("Modloader was built for GTA San Andreas! This game is not supported.");
            else if(info.GetMajorVersion() != 1 && info.GetMinorVersion() != 0)
                Error("Modloader still do not support other versioons than HOODLUM GTA SA 1.0");
            else
            {
                _gtasa_try = MakeCALL(0x824577, (void*) &HOOK_Init);    
            }
            
        }, true);
    }

    /*
     * CModLoader::PosProcess
     *      Call all 'plugin.PosProcess' methods
     */   
    void CModLoader::PosProcess()
    {
        Log("\nPos processing...");
        for(auto& plugin : this->plugins)
        {
            Log("Pos processing plugin \"%s\"", plugin.name);
            if(plugin.PosProcess(&plugin))
                Log("Plugin \"%s\" failed to pos process", plugin.name);
        }
    }
    
    /*
     * CModLoader::LoadPlugins
     *      Loads all plugins
     */
    void CModLoader::LoadPlugins()
    {
        Log("\nLooking for plugins...");
        
        /* Goto plugins folder */
        if(SetCurrentDirectoryA("modloader\\.plugins\\"))
        {       
            ForeachFile("*.dll", false, [this](ModLoaderFile& file)
            {
                LoadPlugin(file.filepath, false);
                return true;
            });

            this->SortPlugins();
            this->BuildExtensionMap();
            
            /* Go back to main folder */
            SetCurrentDirectory("..\\..\\");
        }
    }

    /*
     * CModLoader::UnloadPlugins
     *      Unloads all plugins
     */
    void CModLoader::UnloadPlugins()
    {
        // Unload one by one, calling OnShutdown callback before the unload
        for(auto& plugin : this->plugins)
        {
            this->UnloadPlugin(plugin, true);
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

        // Load plugin module
        if(module = LoadLibraryA(modulename))
        {
            Log("Loading plugin module '%s'", modulename);
            GetPluginData = (modloader_fGetPluginData)(GetProcAddress(module, "GetPluginData"));

            // Setup new's plugin data and fill plugin data from the plugin module
            memset(&data, 0, sizeof(data));
            data.modloader = &this->loader;
            data.pModule   = module;
            data.priority  = 50;
            
            // Check if plugin has been loaded sucessfully
            if(GetPluginData)
            {
                GetPluginData(&data);

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
                    data.author = data.GetAuthor? data.GetAuthor(&data) : "NOAUTHOR";
                    Log("Plugin module '%s' loaded as %s %s by %s", modulename, data.name, data.version, data.author);
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
                this->plugins.push_back(data);
                if(bDoStuffNow)
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
            {
                bool result = this->UnloadPlugin(*it, false);
                this->plugins.erase(it);
                return result;
            }
        }
        
        Log("Could not unload plugin named '%s' because it was not found", pluginName);
        return false;
    }
    
    /*
     *  CModLoader::UnloadPlugin
     *      Unloads a specific plugin
     *      @plugin: The plugin instance
     *      @bRemoveFromList: Removes plugin from this->plugins list
     */
    bool CModLoader::UnloadPlugin(ModLoaderPlugin& plugin, bool bRemoveFromList)
    {
        if(plugin.OnShutdown) plugin.OnShutdown(&plugin);
        if(plugin.pModule) FreeLibrary((HMODULE)(plugin.pModule));
        
        Log("Plugin \"%s\" Unloaded", plugin.name);
        if(bRemoveFromList) this->plugins.remove(plugin);
        
        return true;
    }
    

    /*
     * CModLoader::PeformSearch
     *      Search for mods
     */
    void CModLoader::PerformSearch()
    {
        /* Iterate on all folders at /modloader/ dir, and treat them as a mod entity */
        Log("\nLooking for mods...");
        SetCurrentDirectoryA("modloader\\");
        {
            ForeachFile("*.*", false, [this](ModLoaderFile& file)
            {
                /* Must be a directory to be a mod */
                if(file.is_dir) this->ReadModf(file.filepath);
                return true;
            });
        }
        SetCurrentDirectoryA("..\\");
    }

    
    /*
     * CModLoader::ReadModf
     *      Read a mod
     *      @modfolder: Mod folder
     */
    void CModLoader::ReadModf(const char* modfolder_cc)
    {
        std::string modfolder_str = modfolder_cc;
        //if(modfolder_str.compare(, 2, ".\\");
        const char* modfolder = modfolder_str.c_str();
        
        
        /* Go into the mod folder to work inside it */
        SetCurrentDirectoryA(modfolder);
        {
            char buffer[MAX_PATH * 2];
            GetCurrentDirectoryA(sizeof(buffer), buffer);
 
            /* Push a new modification into the mods list */
            auto& mod = AddNewItemToContainer(this->mods);
            mod.name = GetLastPathComponent(modfolder);
            mod.id = this->currentModId++;
            mod.path = std::string("modloader\\") + modfolder;
            mod.fullPath = buffer;
            
            Log("Reading mod \"%s\" (%d) at \"%s\"...", mod.name.c_str(), mod.id, modfolder);
            
            ForeachFile("*.*", true, [this, &mod](ModLoaderFile& file)
            {
                this->ReadFile(file, mod);
                return true;
            });
        }
        SetCurrentDirectoryA("..\\");        
    }
    
    
    /*
     *  CModLoader::ReadFile
     *      Read file
     */
    void CModLoader::ReadFile(ModLoaderFile& file, CModLoader::ModInfo& mod)
    {
        /* Continue the 'file' setup */
        file.modname = mod.name.data();
        file.mod_id = mod.id;
        file.file_id = this->currentFileId++;
        file.gamepath = this->gamePath.data();
        file.modpath = mod.path.data();
        file.modfullpath = mod.fullPath.data();      
        
        /* See if we can find a handler for this file */
        ModLoaderPlugin* handler = this->FindFileHandler(file);
        
        if(handler)
        {
            /* If this is a directory and a handler for it has been found,
            * don't go inside this folder (recursive search). */
            file.recursion = false;
            
            /* Setup fileInfo */
            auto& fileInfo = AddNewItemToContainer(mod.files);
            fileInfo.id = file.file_id;
            fileInfo.parentMod = &mod;
            fileInfo.handler = handler;
            fileInfo.isDir = file.is_dir;

            /* Continue setupping fileInfo and resetup 'file' to contain pointers to fileInfo (static) */
            file.filename = (fileInfo.fileName = file.filename).data();
            file.filepath = (fileInfo.filePath = file.filepath).data();
            file.filext   = (fileInfo.fileExtension = file.filext).data();
            
            /* Copy C POD data into fileInfo */
            memcpy(&fileInfo.data, &file, sizeof(file));
        }
            
        if(handler)
            Log("Handler for file \"%s\" (%d) is \"%s\"", file.filepath, file.file_id, handler->name);
        else
            Log("Handler for file \"%s\" (%d) not found", file.filepath, file.file_id);    
        

        
    }
    
    /*
     *  CModLoader::ReadFile
     *      Read file
     */
    bool CModLoader::HandleFile(CModLoader::FileInfo& file)
    {
        // Process file using the handler
        if(auto* handler = file.handler)
        {
            const char* pluginName = handler->name;
            const char* filePath   = file.filePath.data(); 
            
            Log("Handling file \"%s\\%s\" by plugin \"%s\"", file.parentMod->name.c_str(), filePath, pluginName);
            if(handler->ProcessFile && !handler->ProcessFile(handler, &file.data))
            {
                /* TODO */
                return true;
            }
            else
                // That's not my fault, hen
                Log("Handler \"%s\" failed to process file \"%s\"", pluginName, filePath);
        }
        return false;
    }
    
    /*
     * IsFileHandlerForFile
     *      Checks if an plugin can check a specific file. It also sets the @plugin.checked flag.
     *      @plugin: The plugin to handle the file
     *      @file: File to be handled
     */
    inline bool IsFileHandlerForFile(ModLoaderPlugin& plugin, const ModLoaderFile& file)
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
    ModLoaderPlugin* CModLoader::FindFileHandler(const ModLoaderFile& file)
    {
        ModLoaderPlugin* handler = 0;
        
        /* First, search for the handler at the vector for the extension,
         * it is commonly possible that the handler for this file is here and we don't have to call CheckFile on all plugins
         * At this point all 'plugin.checked' are false.
         */
        for(auto& pPlugin : this->extMap[file.filext])
        {
            auto& plugin = *pPlugin;    /* Turn into reference just for common coding style */
            if(IsFileHandlerForFile(plugin, file))
            {
                handler = &plugin;
                break;
            }
        }

        /*
         *  Iterate on all plugins to mark all 'plugin.checked' flags as false,
         *  but before, if no handler has been found and a plugin was not checked yet, check it.
         */
        for(auto& plugin : this->plugins)
        {
            if(!handler && IsFileHandlerForFile(plugin, file))
            {
                handler = &plugin;
            }
                
            /* Set the 'plugin.checked' state to false, so on the next call to FindFileHandler it is false */
            plugin.checked = false;
        }
        
        return handler;
    }

}

