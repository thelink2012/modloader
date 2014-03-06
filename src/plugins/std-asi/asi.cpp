/*
 * Copyright (C) 2013-2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 *  std-asi -- Standard ASI Loader Plugin for San Andreas Mod Loader
 *      Loads ASI files as libraries
 * 
 */
#include "asi.h"
#include <modloader_util.hpp>
#include <modloader_util_path.hpp>
#include <map>
using namespace modloader;


//
CThePlugin* asiPlugin;
static CThePlugin plugin;

// <lower_case_asi_name, file_size>
static std::map<std::string, size_t> incompatible;

/*
 *  Export plugin object data
 */
extern "C" __declspec(dllexport)
void GetPluginData(modloader_plugin_t* data)
{
    asiPlugin = &plugin;
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
    return "0.11";
}

const char** CThePlugin::GetExtensionTable()
{
    static const char* table[] = {  "asi", "dll", "cleo",
                                    "cm", "cs", "cs3", "cs4", "cs5",
                                    0 };
    return table;
}

/*
 *  Startup / Shutdown
 */
bool CThePlugin::OnStartup()
{
    // Register GTA module for some arg translation
    this->asiList.emplace_front("gta");
    
    // Register incompatibilities
    incompatible.emplace("ragdoll.asi", 198656);
    incompatible.emplace("normalmap.asi", 83456);
    incompatible.emplace("outfit.asi", 88064);
    incompatible.emplace("colormod.asi", 111104);
    incompatible.emplace("dof.asi", 110592);
    incompatible.emplace("bullet.asi", 97280);
    incompatible.emplace("google.asi", 112128);
    incompatible.emplace("airlimit.asi", 79872);
    incompatible.emplace("killlog.asi", 98816);
    incompatible.emplace("weaponlimit.asi", 99840);
    incompatible.emplace("maplimit.asi", 50688);
    
    return true;
}

bool CThePlugin::OnShutdown()
{
    // Free the asi files we loaded
    for(auto& asi : this->asiList) asi.Free();
    return true;
}

/*
 *  Check if the file is the one we're looking for
 */
bool CThePlugin::CheckFile(modloader::ModLoaderFile& file)
{
    if(!file.is_dir)
    {
        char dummy;
        
        if(IsFileExtension(file.filext, "asi") || IsFileExtension(file.filext, "cleo") || IsFileExtension(file.filext, "dll"))
        {
            if(IsFileExtension(file.filext, "asi"))
            {
                // Don't load CLEO neither modloader!
                if(!strcmp(file.filename, "cleo.asi", false) || !strcmp(file.filename, "modloader.asi", false))
                {
                    Log("Warning: Forbidden ASI file found \"%s\"", GetFilePath(file).c_str());
                    return false;
                }
            }
            else if(IsFileExtension(file.filext, "cleo"))
            { }
            else if(strcmp(file.filename, "d3d9.dll", false))
                return false;

            // Check out if the file is incompatible
            std::string filename = file.filename;
            auto it = incompatible.find(modloader::tolower(filename));
            if(it != incompatible.end())
            {
                auto size = modloader::GetFileSize(file.filepath);
                if(size == it->second)
                {
                    Error("Incompatible ASI file found: %s\nPlease install it at the game root directory.\nSkipping it!",
                          file.filename);
                    return false;
                }
            }
            return true;
        }
        else if(IsFileExtension(file.filext, "cm") || CsInfo::GetVersionFromExtension(file.filext, dummy))
        {
            // Cleo script, true
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
    
    // Check if asi plugin (or dll injection...)
    if(IsFileExtension(file.filext, "asi")  || IsFileExtension(file.filext, "cleo") || IsFileExtension(file.filext, "dll"))
        this->asiList.emplace_back(GetFilePath(file));
    else    // Nope? Then it's a cleo script
        this->csList.emplace_back(file);
    
    return true;
}

/*
 * Called after all files have been processed
 */
bool CThePlugin::PosProcess()
{
    // Find CLEO.asi and already loaded .cleo plugins (loaded by CLEO.asi)
    this->LocateCleo();
    
    // Iterate on the asi list loading each asi file
    for(auto& asi : this->asiList)
    {
        scoped_chdir xdir(asi.folder.c_str());
        SetLastError(0);
        
        // ....
        bool bLoaded = asi.Load();
        if(asi.bIsMainExecutable == false)
        {
            Log(bLoaded? "[%s] %s \"%s\"" : "[%s] %s \"%s\"; errcode: 0x%X",                    // Formated string
                (asi.bIsASI? "ASI" : asi.bIsD3D9? "D3D9" : asi.bIsCleo? "CLEO" : "???"),        // What
                (bLoaded? "Module has been loaded:" : "Failed to load module"),                 // Loaded properly?
                asi.path.c_str(),                                                               // Path
                GetLastError());                                                                // [extra] Error code
        }
    }

    return true;
}

