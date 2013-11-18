/*
 * Copyright (C) 2013  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 *  std-movies -- Standard Movies Plugin for San Andreas Mod Loader
 *      Replaces "GTAtitles.mpg" and "Logo.mpg" strings on the executable
 * 
 * 
 */
#include <modloader.hpp>
#include "modloader_util.hpp"
#include "Injector.h"
using namespace modloader;

/*
 *  The plugin object
 */
class CThePlugin : public modloader::CPlugin
{
    public:
        static const int default_priority = 50;
        
        static const size_t sizeofTitleString = MAX_PATH * 2;
        char GTAtitles[sizeofTitleString], Logo[sizeofTitleString];

        bool SetTitle(const char* saTitleName, char* myTitleBuf, const ModLoaderFile& file);
        
        const char* GetName();
        const char* GetAuthor();
        const char* GetVersion();
        int OnStartup();
        int CheckFile(const modloader::ModLoaderFile& file);
        int ProcessFile(const modloader::ModLoaderFile& file);
        
        const char** GetExtensionTable(size_t& outTableLength);

};

static CThePlugin plugin;

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

const char** CThePlugin::GetExtensionTable(size_t& len)
{
    static const char* table[] = { "mpg", 0 };
    return (len = GetArrayLength(table), table);
}

/*
 *  Startup / Shutdown (do nothing)
 */
int CThePlugin::OnStartup()
{
    this->GTAtitles[0] = this->Logo[0] = 0;
    return 0;
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
    if(!this->SetTitle("Logo.mpg", this->Logo, file)
    && !this->SetTitle("GTAtitles.mpg", this->Logo, file))
        return false;
    
    // Replace Logo.mpg
    if(*this->Logo)
    {
        WriteMemory<char*>(0x748AFA + 1, this->Logo, true);
    }
    
    // Replace GTAtitles.mpg (replace GTAtitlesGER.mpg too)
    if(*this->GTAtitles)
    {
        WriteMemory<char*>(0x748BEC + 1, this->GTAtitles, true);    // normal
        WriteMemory<char*>(0x748BF3 + 1, this->GTAtitles, true);    // GER
    }

    return 0;
}

/*
 *      Set a title internal buffer with it's respective replacement if possible
 */
bool CThePlugin::SetTitle(const char* saTitleName, char* myTitleBuf, const ModLoaderFile& file)
{
    // Matches?
    if(!strcmp(file.filename, saTitleName))
    {
        // No replacement yet? If any, output a error to the log
        if(*myTitleBuf)
            Log("Failed to replace movie '%s' with '%s' because another mod already replaced it!\n"
                "\tFull replacement path: %s",
                saTitleName, file.filepath, myTitleBuf);
        else
        {
            // Copy full path into our buffer
            if(GetFullPathNameA(file.filepath, sizeofTitleString, myTitleBuf, 0))
                return true;
            else
                myTitleBuf[0] = 0;  // make sure the string is unchanged
        }
    }
    return false;
}

