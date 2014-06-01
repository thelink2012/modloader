/*
 * Copyright (C) 2013-2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */

#include "loader.hpp"
#include <modloader_util_injector.hpp>
#include <modloader_util_path.hpp>
#include <modloader_util_ini.hpp>

REGISTER_ML_NULL();

// TODO take care of ini.0
// TODO no process priority
// TODO build extension map

//
extern int LogException(char* buffer, LPEXCEPTION_POINTERS pException);



// Previous exception filter
static LPTOP_LEVEL_EXCEPTION_FILTER PrevFilter = nullptr;


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
 *  MyUnhandledExceptionFilter
 *      Logs an unhandled exception and shutdowns the loader
 */
static LONG CALLBACK MyUnhandledExceptionFilter(LPEXCEPTION_POINTERS pException)
{
    // Log this exception into the log file
    loader.LogException(pException);

    // Let's shutdown our loader anyway
    loader.Shutdown();

    // Continue exception propagation
    return (PrevFilter? PrevFilter(pException) : EXCEPTION_CONTINUE_SEARCH);  // I'm not really sure about this return
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
        //
        typedef function_hooker_stdcall<0x8246EC, int(HINSTANCE, HINSTANCE, LPSTR, int)> winmain_hook;
        typedef function_hooker<0x53C6DB, void(void)>           onreload_hook;
                
        // Hook WinMain to run mod loader
        make_function_hook<winmain_hook>([](winmain_hook::func_type WinMain,
                                                    HINSTANCE& hInstance, HINSTANCE& hPrevInstance, LPSTR& lpCmdLine, int& nCmdShow)
        {
            // Setup exception filter, we need (whenever possible) to call shutdown before a crash or something
            PrevFilter = SetUnhandledExceptionFilter(MyUnhandledExceptionFilter);

            // Startup the loader and call WinMain, Shutdown the loader after WinMain.
            loader.Startup();
            auto result = WinMain(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
            loader.Shutdown();

            return result;
        });
        
        //  Hook CGame::ReInitObjectVariables to notify a loadgame
        make_function_hook<onreload_hook>([](onreload_hook::func_type func)
        {
            loader.OnReload();
            return func();
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
    const char* basic_config = "modloader/modloader.ini";
    
    // If not running yet and 'modloader' folder exists, let's start up
    if(!this->bRunning && IsDirectoryA("modloader"))
    {
        // Initialise configs and counters
        this->bRunning       = false;
        this->bEnableLog     = true;
        this->bEnablePlugins = true;
        this->currentModId   = 0;
        this->currentModId   = 0;
        
        // Open the log file
        OpenLog();
        
        // Make sure we have the modloader.ini file
        CopyFileA("modloader/.data/modloader.ini", basic_config, TRUE);
        
        // Read the basic information present in the main ini file
        ReadBasicConfig(basic_config);
        
        // Check if logging is disabled by the basic config file
        if(!this->bEnableLog)
        {
            Log("Logging is disabled. Closing log file...");
            CloseLog();
        }
        
        // Setup path variables
        std::string data  = "modloader/.data";
        this->pluginPath  = "modloader/.data/plugins";
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

        // Initialise sub systems (TODO)
        this->ParseCommandLine();   // Parse command line arguments
        this->LoadPlugins();        // Load plugins at /modloader/.data/plugins
        this->ScanAndInstallFiles();// Search and install mods at /modloader
 
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
 *  Loader::OnReload
 *       Called each time the game loads a [new game/save game]
 */
void Loader::OnReload()
{
    // TODO ALREADY CALLED
}




void Loader::ScanAndInstallFiles()
{
    mods.Scan();
    mods.Update();
    // mods.UninstallNecessary
    // mods.InstallNecessary
}
















void Loader::FolderInformation::Scan()
{
    scoped_gdir xdir(this->path.c_str());
    Log("\nScanning mods at \"%s\"...", this->path.c_str());

    bool fine = true;
    
    // TODO this->LoadConfigFromINI("modloader.ini"); ONLY ONCE?

    // Mark all current mods as removed
    MarkStatus(this->mods, Status::Removed);
    
    // Walk on this folder to find mods
    if (flags.bIgnoreAll == false)
    {
        fine = FilesWalk("", "*.*", false, [this](FileWalkInfo & file)
        {
            if(file.is_dir)
            {
                if (IsIgnored(NormalizePath(file.filename)))
                    Log("Ignoring mod at \"%s\"", file.filepath);
                else
                    this->AddMod(file.filename).Scan();
            }
            return true;
        });
    }
    
    // Find the underlying status of this folder
    FindStatus(*this, this->mods, fine);
    
    // TODO search childs
}

void Loader::FolderInformation::Update()
{
    // TODO mods by priority
    
    // Uninstall all removed files since the last update...
    for(auto& pair : this->mods)
    {
        ModInformation& mod = pair.second;
        mod.UninstallNecessaryFiles();
    }
    
    // Install all updated and added files since the last update...
    for(auto& pair : this->mods)
    {
        ModInformation& mod = pair.second;
        mod.InstallNecessaryFiles();
    }
}







void Loader::ModInformation::UninstallNecessaryFiles()
{
    // Finds all removed file in this mod and uninstall them
    for(auto it = this->files.begin(); it != this->files.end();  )
    {
        FileInformation& file = it->second;
        if(file.status == Status::Removed)
        {
            // File was removed from the filesystem, uninstall and erase from our internal list
            Log("Extinguishing file '%s'", file.FileBuffer());
            file.Uninstall();
            it = this->files.erase(it);
        }
        else
            ++it;
    }

}

void Loader::ModInformation::InstallNecessaryFiles()
{
    for(auto it = this->files.begin(); it != this->files.end(); ++it)
    {
        FileInformation& file = it->second;
        switch(file.status)
        {
            case Status::Added:     // File has been added since the last update
                file.Install();
                break;
                
            case Status::Updated:   // File has been updated since the last update
                file.Reinstall();
                break;
        }
        
        file.status = Status::Unchanged;
    }
}








struct FileInstallLog
{
    const Loader::FileInformation& file;
    const char* action;
    bool revaction;
    
    FileInstallLog(const Loader::FileInformation& file, const char* action, bool revaction = false) :
                file(file), action(action), revaction(revaction)
    {
        loader.Log("%sing file '%s'", action, file.FileBuffer());
    }
    
    ~FileInstallLog()
    {
        if(file.installed == revaction)
            loader.Log("Failed to %s file '%s'", action, file.FileBuffer());
    }
    
};

void Loader::FileInformation::Install()
{
    // TODO throw if already installed
    
    FileInstallLog xlog(*this, "Install");
    
    // Install with the main handler and with callme handlers
    if(this->handler) this->installed = handler->Install(*this, true);
    for(auto& p : this->callme) p->Install(*this, false);
}

void Loader::FileInformation::Reinstall()
{
    if(this->installed)
    {
        FileInstallLog xlog(*this, "Reinstall");
        
        // Reinstall both on main handler and on callme handlers
        if(this->handler) this->installed = handler->Reinstall(*this, true);
        for(auto& p : this->callme) p->Reinstall(*this, false);
    }
}

void Loader::FileInformation::Uninstall()
{
    if(this->installed)
    {
        FileInstallLog xlog(*this, "Uninstall", true);
        
        // Uninstall both on main handler and on callme handlers
        if(this->handler) this->installed = !handler->Uninstall(*this, true);
        for(auto& p : this->callme) p->Uninstall(*this, false);
    }
}






bool Loader::PluginInformation::Install(FileInformation& file, bool isMainHandler)
{
    if(isMainHandler)
    {
        if(FileInformation* old = this->FindFileWithBehaviour(file.behaviour))
            old->Uninstall();
        
        auto it = behv.emplace(file.behaviour, &file);
        if(!it.second && it.first->second != &file)
                /* TODO THROW */;
    }
    
    return this->Install(file);
}

bool Loader::PluginInformation::Uninstall(FileInformation& file, bool isMainHandler)
{
    // TODO safeness check
    if(isMainHandler)
    {
        behv.erase(file.behaviour);
    }
    
    return this->Uninstall(file);
}


bool Loader::PluginInformation::Reinstall(FileInformation& file, bool isMainHandler)
{
    // TODO safeness check
    if(isMainHandler)
    {
    }
    
    return this->Reinstall(file);
}









// Mark all as removed
// Scan
// Uninstall old
// Install new


//
// Install/Uni options:
//      [++] Option 1: Let the plugin keep track of all installed files with same behaviour
//      [++] Option 2: Let the plugin send a identifier of the file behaviour, and when one is uninstalled the other is found and installed
//      [--] Option 3: Let the plugin call UninstallFile() on the old file
//
//
// Install 1/x.dff
// Install 2/x.dff
// Install 3/x.dff
// What happens? 3/ gets installed
//
// Uninstall 3/x.dff
// What to do to install /2?
// By option 1:
//              The plugin will know what to do (stacking or something)
// By option 2:
//              PS: When two with same behaviour gets Installed(), the older gets Uninstalled()
//              Find others with same behaviour and Install()
// 
// By option 3:
//              Default will get installed
//
//
//  Reescan finds 4/x.dff
//  What to do to install /4?
//  By option 1:
//              The plugin will know what to do
//
//  By option 2:
//              Others with same behaviour has less priority, Install()
// 
//  By option 3:
//              Simply Install()
//
//
//  Priority of 1/x.dff increases
//  What to do to install /1?
//  By option 1:
//              Reinstall() and the plugin will know what to do
//
//  By option 2:
//              Looking at behaviour and priority, Reinstall() this and Uninstall() old
// 
//  By option 3:
//              ?????????????
//
//
//
//
//
//
//
//
//
// 
//
//
//
//
//
//
//
//
//
//
//
//
// 
//
//
//
//
//
//
//
//
//
// Conclusion:
//      Option 1 will complicate plugin creation
//      Option 3 is mind blowing, will break many things for sure
//      Option 2 looks like the most viable and helpful
//






