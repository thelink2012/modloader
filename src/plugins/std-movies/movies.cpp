/*
 * Copyright (C) 2013  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 *  std-movies -- Standard Movies Plugin for San Andreas Mod Loader
 *      Overrides GTAtitles.mpg" and Logo.mpg files
 * 
 * 
 */
#include <modloader.hpp>
#include <modloader_util_path.hpp>
#include "Injector.h"
#include "modloader_util.hpp"
using namespace modloader;

/*
 *  The plugin object
 */
class CThePlugin : public modloader::CPlugin
{
    public:
        static const int default_priority = 50;

        std::string GTAtitles, Logo;

        const char* GetName();
        const char* GetAuthor();
        const char* GetVersion();
        int CheckFile(const modloader::ModLoaderFile& file);
        int ProcessFile(const modloader::ModLoaderFile& file);
        int PosProcess();
        
        const char** GetExtensionTable();

} plugin;

/*
 *  Export plugin object data
 */
extern "C" __declspec(dllexport)
void GetPluginData(modloader_plugin_t* data)
{
    modloader::RegisterPluginData(plugin, data);
    plugin.data->priority = plugin.default_priority;
}

/*
 *  Basic plugin informations
 */
const char* CThePlugin::GetName()
{
    return "std-movies";
}

const char* CThePlugin::GetAuthor()
{
    return "LINK/2012";
}

const char* CThePlugin::GetVersion()
{
    return "RC1";
}

const char** CThePlugin::GetExtensionTable()
{
    static const char* table[] = { "mpg", 0 };
    return table;
}

/*
 *  Check if the file is the one we're looking for
 */
int CThePlugin::CheckFile(const modloader::ModLoaderFile& file)
{
    if( !file.is_dir && IsFileExtension(file.filext, "mpg") &&
        (  !strcmp(file.filename, "Logo.mpg",  false)                                                
        || !strcmp(file.filename, "GTAtitles.mpg", false))
      )
        return MODLOADER_YES;
    return MODLOADER_NO;
}

/*
 * Process the replacement
 */
int CThePlugin::ProcessFile(const modloader::ModLoaderFile& file)
{
    std::string filepath = GetFilePath(file);
    
    if(file.filename[0] == 'L')
        RegisterReplacementFile(*this, "Logo.mpg", this->Logo, filepath.c_str());
    else if(file.filename[0] == 'G')
        RegisterReplacementFile(*this, "GTAtitles.mpg", this->GTAtitles, filepath.c_str());

    return 0;
}

/*
 *  Called after all files have been processed
 *  Hooks everything needed
 */
int CThePlugin::PosProcess()
{
    // Replace Logo.mpg
    if(this->Logo.size())
    {
        WriteMemory<const char*>(0x748AFA + 1, this->Logo.data(), true);
    }
    
    // Replace GTAtitles.mpg (replace GTAtitlesGER.mpg too)
    if(this->GTAtitles.size())
    {
        WriteMemory<const char*>(0x748BEC + 1, this->GTAtitles.data(), true);    // normal
        WriteMemory<const char*>(0x748BF3 + 1, this->GTAtitles.data(), true);    // GER
    }
    
    return 0;
}

