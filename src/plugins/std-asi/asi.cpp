/*
 * Copyright (C) 2013  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 *  std-asi -- Standard ASI Loader Plugin for San Andreas Mod Loader
 *      Loads ASI files as libraries
 * 
 *  TODO:
 *      Replace modules address for some functions imported to translate paths...
 *      ...trying to load the file at the base path and at the asi path.
 * 
 */
#include <windows.h>
#include <modloader.hpp>
#include <modloader_util.hpp>
#include <modloader_util_path.hpp>
#include <string>
#include <list>
using namespace modloader;

/*
 *  The plugin object
 */
class CThePlugin : public modloader::CPlugin
{
    public:
        struct AsiInfo
        {
            std::string path;       // The asi path, like "modloader/aaa/bbb.asi"
            std::string folder;     // The folder where the asi is at, like "modloader/aaa/"
            
            HMODULE module;         // The library module handle

            // The constructor, takes rvalue strings only
            AsiInfo(std::string&& path)
            {
                this->module = 0;
                this->folder = path.substr(0, GetLastPathComponent(path));
                this->path = std::move(path);
            }
            
            // Sort by module base address
            bool operator<(const AsiInfo& b) const
            {
                return ((char*)(this->module) < (char*)(b.module));
            }
            
            // Load the module @path and returns whether the load was successful or not
            bool Load()
            {
                if(!this->module) this->module = LoadLibraryA(path.c_str());
                return this->module != 0;
            }
            
            // Unload the module @path
            void Free()
            {
                if(this->module) FreeLibrary(module);
            }
            
        };
        
        // List of asi files
        std::list<AsiInfo> asiList;
        
        const char* GetName();
        const char* GetAuthor();
        const char* GetVersion();
        bool OnStartup();
        bool OnShutdown();
        bool CheckFile(modloader::ModLoaderFile& file);
        bool ProcessFile(const modloader::ModLoaderFile& file);
        bool PosProcess();
        
        const char** GetExtensionTable();

} plugin;

/*
 *  Export plugin object data
 */
extern "C" __declspec(dllexport)
void GetPluginData(modloader_plugin_t* data)
{
    modloader::RegisterPluginData(plugin, data);
}



/*
 *  Basic plugin informations
 */
const char* CThePlugin::GetName()
{
    return "std-asi";
}

const char* CThePlugin::GetAuthor()
{
    return "LINK/2012";
}

const char* CThePlugin::GetVersion()
{
    return "0.1";
}

const char** CThePlugin::GetExtensionTable()
{
    static const char* table[] = { "asi", 0 };
    return table;
}

/*
 *  Startup / Shutdown
 */
bool CThePlugin::OnStartup()
{
    return true;
}

bool CThePlugin::OnShutdown()
{
    for(auto& asi : this->asiList) asi.Free();
    return true;
}

/*
 *  Check if the file is the one we're looking for
 */
bool CThePlugin::CheckFile(modloader::ModLoaderFile& file)
{
    if(!file.is_dir && IsFileExtension(file.filext, "asi"))
    {
        if(strcmp(file.filename, "cleo.asi", false))    // Don't load CLEO
        {
            return true;
        }
    }
    return false;
}

/*
 * Process the replacement
 */
bool CThePlugin::ProcessFile(const modloader::ModLoaderFile& file)
{
    this->asiList.emplace_back(GetFilePath(file));
    return true;
}

/*
 * Called after all files have been processed
 */
bool CThePlugin::PosProcess()
{
    for(auto& asi : this->asiList)
    {
        scoped_chdir xdir(asi.folder.c_str());
        
        bool bLoaded = asi.Load();
        Log("%s \"%s\"",
            (bLoaded? "ASI file has been loaded:" : "Failed to load ASI file"),
            asi.path.c_str());
    }
    return true;
}
