/*
 * Copyright (C) 2013-2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */

#include "loader.hpp"
#include <modloader_util_injector.hpp>
#include <modloader_util_path.hpp>
#include <modloader_util_ini.hpp>

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
    gvm.set_name("Mod Loader");
    
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
    Log("Scanning mods at \"%s\"...", this->path.c_str());

    bool fine = true;
    
    // TODO this->LoadConfigFromINI("modloader.ini"); ONLY ONCE?
    // TODO this->PreScan();

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

    // TODO this->PosScan();
    // TODO search childs
}

void Loader::FolderInformation::Update()
{
    for(auto& pair : this->mods)
    {
        ModInformation& mod = pair.second;
        mod.UninstallNecessaryFiles();
    }
    
    for(auto& pair : this->mods)
    {
        ModInformation& mod = pair.second;
        mod.InstallNecessaryFiles();
    }
}

void Loader::ModInformation::UninstallNecessaryFiles()
{
    for(auto it = this->files.begin(); it != this->files.end();  )
    {
        FileInformation& file = it->second;
        if(file.status == Status::Removed)
        {
            // TODO Uninstall()
            it = this->files.erase(it);
        }
        else
            ++it;
    }

}

void Loader::ModInformation::InstallNecessaryFiles()
{
    // TODO GET SORTED LIST BY PRIORITY
    
    for(auto it = this->files.begin(); it != this->files.end(); ++it)
    {
        FileInformation& file = it->second;
        switch(file.status)
        {
            case Status::Added:
                // TODO Install();
                break;
                
            case Status::Updated:
                // TODO Reinstall();
                break;
        }
    }
}









// Mark all as removed
// Scan
// Uninstall old
// Install new
