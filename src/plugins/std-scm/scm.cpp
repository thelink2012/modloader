/*
 * Copyright (C) 2013  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 *  std-scm -- Standard SCM Loader Plugin for San Andreas Mod Loader
 *      This plugin is extremelly simple, made to load a new main.scm file.
 *      It just overrides one or two strings on the game executable.
 *      NOTE: script.img is taken care by the img plugin!
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
        
        std::string mainScm;
        
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

const char** CThePlugin::GetExtensionTable()
{
    /* Put the extensions  this plugin handles on @table */
    static const char* table[] = { "scm", 0 };
    return table;
}

/*
 *  Check if the file is the one we're looking for
 */
int CThePlugin::CheckFile(const modloader::ModLoaderFile& file)
{
    if(!file.is_dir
    && !strcmp(file.filename, "main.scm", false)
    )//&& IsFileInsideFolder(file.filepath, true, "data/script"))
        return MODLOADER_YES;
    return MODLOADER_NO;
}

/*
 * Process the replacement
 */
int CThePlugin::ProcessFile(const modloader::ModLoaderFile& file)
{
    return !RegisterReplacementFile(*this, "main.scm", mainScm, GetFilePath(file).c_str());
}


/*
 * Called after all files have been processed
 */
int CThePlugin::PosProcess()
{
    /* Patch the game only if there's a replacement */
    if(mainScm.size())
    {
        /* chdir into root directory instead of "data/script" */
        WriteMemory<const char*>(0x468EB5+1, "", true);

        /* Replace references to "~/main.scm" string */
        WriteMemory<const char*>(0x468EC4 + 1, mainScm.data(), true);
        WriteMemory<const char*>(0x489A45 + 1, mainScm.data(), true);
    }
    return 0;
}
