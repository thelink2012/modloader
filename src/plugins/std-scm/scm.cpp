/*
 * Copyright (C) 2013  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 *  std-scm -- Standard SCM Loader Plugin for San Andreas Mod Loader
 *      This plugin is extremelly simple, made to load a new main.scm file.
 *      It just replaces one or two strings on the game executable.
 *      NOTE: script.img is taken care by the img plugin!
 * 
 */
#include <modloader.hpp>
#include <modloader_util.hpp>
#include "Injector.h"
using namespace modloader;

/*
 *  The plugin object
 */
class CThePlugin : public modloader::CPlugin
{
    public:
        static const int default_priority = 50;
        
        const char* GetName();
        const char* GetAuthor();
        const char* GetVersion();
        int OnStartup();
        int OnShutdown();
        int CheckFile(const modloader::ModLoaderFile& file);
        int ProcessFile(const modloader::ModLoaderFile& file);
        int PosProcess();
        
        const char** GetExtensionTable(size_t& outTableLength);

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
    return "std-scm";
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
    /* Put the extensions  this plugin handles on @table */
    static const char* table[] = { "scm", 0 };
    return (len = GetArrayLength(table), table);
}


/*
 *  Startup / Shutdown
 */
int CThePlugin::OnStartup()
{
    return 0;
}

int CThePlugin::OnShutdown()
{
    return 0;
}

/*
 *  Check if the file is the one we're looking for
 */
int CThePlugin::CheckFile(const modloader::ModLoaderFile& file)
{
    if(!file.is_dir
    && !strcmp(file.filename, "main.scm", false)
    && IsFileInsideFolder(file.filepath, true, "data/script"))
        return MODLOADER_YES;
    return MODLOADER_NO;
}

/*
 * Process the replacement
 */
int CThePlugin::ProcessFile(const modloader::ModLoaderFile& file)
{
    /* Since we only accept main.scm and script.img,
     * we can just check the first letter to see which file is it */
    
    static char mainScm[MAX_PATH];
    
    if(mainScm[0])
    {
        Log("Failed to register main.scm file because there's already one registered\n"
            "\tCurrently registered path: %s", mainScm);
        return false;
    }
    
    /* Register new main.scm buffer */
    strcpy(mainScm, GetFilePath(file).c_str());
    
    /* Avoid chdir into "data/script" */
    MakeNOP(0x468EBA, 5);
    
    /* Replace references to "~~/main.scm" string */
    WriteMemory<const char*>(0x468EC4 + 1, mainScm, true);
    WriteMemory<const char*>(0x489A45 + 1, mainScm, true);
    
    return 0;
}


/*
 * Called after all files have been processed
 */
int CThePlugin::PosProcess()
{
    return 0;
}
