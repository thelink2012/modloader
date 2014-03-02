/*
 * Copyright (C) 2013-2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 *  std-scm -- Standard SCM Loader Plugin for San Andreas Mod Loader
 *      This plugin is extremelly simple, made to load a new main.scm file.
 *      It just overrides one or two strings on the game executable.
 *      NOTE: script.img is taken care by the img plugin!
 * 
 */
#include <modloader.hpp>
#include <modloader_util.hpp>
#include <modloader_util_path.hpp>
#include <modloader_util_injector.hpp>
using namespace modloader;

/*
 *  The plugin object
 */
class CThePlugin : public modloader::CPlugin
{
    public:
        std::string mainScm;
        
        const char* GetName();
        const char* GetAuthor();
        const char* GetVersion();
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
bool CThePlugin::CheckFile(modloader::ModLoaderFile& file)
{
    if(!file.is_dir
    && !strcmp(file.filename, "main.scm", false)
    )//&& IsFileInsideFolder(file.filepath, true, "data/script"))
        return true;
    return false;
}

/*
 * Process the replacement
 */
bool CThePlugin::ProcessFile(const modloader::ModLoaderFile& file)
{
    return RegisterReplacementFile(*this, "main.scm", mainScm, GetFilePath(file).c_str());
}


/*
 * Called after all files have been processed
 */
bool CThePlugin::PosProcess()
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
    return true;
}
