/*
 * Copyright (C) 2013-2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under the MIT License, see LICENSE at top level directory.
 * 
 *  std.asi -- Standard ASI Loader Plugin for Mod Loader
 *      Loads ASI files as libraries and CLEO scripts
 * 
 */
#include <stdinc.hpp>
#include "asi.h"
using namespace modloader;

// TODO THIS PLUGIN NEEDS REVISION -- AFTER THE CORE CHANGE MOST OF THE CODE HERE IS LEGACY


// <normalized_asi_name, file_size>
static std::map<std::string, size_t> incompatible;

static const uint64_t hash_mask   = 0x00000000FFFFFFFF;     // Mask for the hash on the behaviour
static const uint64_t is_asi_mask = 0x0000000100000000;     // Mask for is_asi on the behaviour
static const uint64_t is_cs_mask  = 0x0000000200000000;     // Mask for is_cs on the behaviour

static ThePlugin plugin;
REGISTER_ML_PLUGIN(::plugin);


/*
 *  ThePlugin::GetInfo
 *      Returns information about this plugin 
 */
const ThePlugin::info& ThePlugin::GetInfo()
{
    static const char* extable[] = { "asi", "dll", "cleo", "cm", "cs", "cs3", "cs4", "cs5", 0 };
    static const info xinfo      = { "std.asi", get_version_by_date(), "LINK/2012", -1, extable };
    return xinfo;
}





/*
 *  ThePlugin::OnStartup
 *      Startups the plugin
 */
bool ThePlugin::OnStartup()
{
    // Register GTA module for some arg translation
    this->asiList.emplace_front("gta", nullptr, GetModuleHandleA(0));
    this->asiList.front().PatchImports();
    
    // Find CLEO.asi
    this->LocateCleo();

    // Register incompatibilities
    incompatible.emplace(NormalizePath("ragdoll.asi"),          198656);
    incompatible.emplace(NormalizePath("normalmap.asi"),        83456);
    incompatible.emplace(NormalizePath("outfit.asi"),           88064);
    incompatible.emplace(NormalizePath("colormod.asi"),         111104);
    incompatible.emplace(NormalizePath("dof.asi"),              110592);
    incompatible.emplace(NormalizePath("bullet.asi"),           97280);
    incompatible.emplace(NormalizePath("google.asi"),           112128);
    incompatible.emplace(NormalizePath("airlimit.asi"),         79872);
    incompatible.emplace(NormalizePath("killlog.asi"),          98816);
    incompatible.emplace(NormalizePath("weaponlimit.asi"),      99840);
    incompatible.emplace(NormalizePath("maplimit.asi"),         50688);
    incompatible.emplace(NormalizePath("GTA_IV_HUD.asi"),       50688);
    incompatible.emplace(NormalizePath("HandlingAdder.asi"),    55296);
    return true;
}

/*
 *  ThePlugin::OnShutdown
 *      Shutdowns the plugin
 */
bool ThePlugin::OnShutdown()
{
    for(auto& asi : this->asiList) asi.Free();
    return true;
}


/*
 *  ThePlugin::GetBehaviour
 *      Gets the relationship between this plugin and the file
 */
int ThePlugin::GetBehaviour(modloader::file& file)
{
    if(!file.is_dir())
    {
        char dummy;
        if(file.is_ext("asi") || file.is_ext("cleo") || file.is_ext("dll"))
        {
            if(file.is_ext("asi"))
            {
                // Don't load CLEO neither modloader!
                if(!strcmp(file.filename(), "cleo.asi", false)
                || !strcmp(file.filename(), "iii.cleo.asi", false)
                || !strcmp(file.filename(), "vc.cleo.asi", false)
                || !strcmp(file.filename(), "modloader.asi", false))
                {
                    Log("Warning: Forbidden ASI file found \"%s\"", file.filepath());
                    return MODLOADER_BEHAVIOUR_NO;
                }
            }
            else if(file.is_ext("cleo"))                       // Allow
            { }
            else if(strcmp(file.filename(), "d3d9.dll", false))   // Not D3D9 either?
                return MODLOADER_BEHAVIOUR_NO;

            // Check out if the file is incompatible
            std::string filename = file.filename();
            auto it = incompatible.find(filename);
            if(it != incompatible.end())
            {
                if(file.size == it->second)
                {
                    Error("Incompatible ASI file found: %s\nPlease install it at the game root directory.\nSkipping it!",
                          file.filename());
                    return MODLOADER_BEHAVIOUR_NO;
                }
            }

            file.behaviour = file.hash | is_asi_mask;
            return MODLOADER_BEHAVIOUR_YES;
        }
        else if(file.is_ext("cm") || CsInfo::GetVersionFromExtension(file.filext(), dummy))
        {
            // Cleo script, true
            file.behaviour = file.hash | is_cs_mask;
            return MODLOADER_BEHAVIOUR_YES;
        }
    }
    return MODLOADER_BEHAVIOUR_NO;
}

/*
 *  ThePlugin::InstallFile
 *      Installs a file using this plugin
 */
bool ThePlugin::InstallFile(const modloader::file& file)
{
    if(file.behaviour & is_asi_mask)
    {
        auto& asi = *asiList.emplace(asiList.end(), file.filepath(), &file, nullptr);
        
        if(!asi.Load())
        {
            Log(false ? "[%s] %s \"%s\"" : "[%s] %s \"%s\"; errcode: 0x%X",                     // Formated string
                (asi.bIsASI ? "ASI" : asi.bIsD3D9 ? "D3D9" : asi.bIsCleo ? "CLEO" : "???"),     // What
                (false ? "Module has been loaded:" : "Failed to load module"),                  // Loaded properly?
                file.filedir(),                                                                // Path
                GetLastError());                                                                // [extra] Error code

            asiList.pop_back();
            return false;
        }

        return true;
    }
    else if(file.behaviour & is_cs_mask)
    {
        csList.emplace_back(&file);
        return true;
    }
    return false;
}

/*
 *  ThePlugin::ReinstallFile
 *      Reinstall a file previosly installed that has been updated
 */
bool ThePlugin::ReinstallFile(const modloader::file& file)
{
    if(file.behaviour & is_cs_mask) return true;
    return false;
}

/*
 *  ThePlugin::UninstallFile
 *      Uninstall a previosly installed file
 */
bool ThePlugin::UninstallFile(const modloader::file& file)
{
    if(file.behaviour & is_cs_mask)
    {
        for(auto it = csList.begin(); it != csList.end(); ++it)
        {
            if(it->file == &file)
            {
                csList.erase(it);
                break;
            }
        }
        return true;
    }
    return false;
}
