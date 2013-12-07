/*
 * Copyright (C) 2013  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 *  std-sprites -- Standard Sprites Loader Plugin for San Andreas Mod Loader
 *      This plugin provides sprites for scripts and other parts of the game.
 *      Sprites are placed at "models/txd" folder
 *
 */
#include <modloader.hpp>
#include <modloader_util.hpp>
#include <modloader_util_path.hpp>
#include <list>

#include "Injector.h"
using namespace modloader;

/*
 *  The plugin object
 */
class CThePlugin* spritesPlugin;
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
        
        const char** GetExtensionTable();

        struct SSpriteFile
        {
            std::string txdfile;    /* e.g. "file.txd" */
            std::string path;       /* e.g. "modloader/mymod/models/txd/file.txd" */
            
            void Setup(const ModLoaderFile& file)
            {
                this->txdfile = file.filename;
                this->path    = GetFilePath(file);
            }
        };
        
        std::list<SSpriteFile> sprites;
        
} plugin;

/*
 *  Export plugin object data
 */
extern "C" __declspec(dllexport)
void GetPluginData(modloader_plugin_t* data)
{
    spritesPlugin = &plugin;
    modloader::RegisterPluginData(plugin, data);
    plugin.data->priority = plugin.default_priority;
}



/*
 *  Basic plugin informations
 */
const char* CThePlugin::GetName()
{
    return "std-sprites";
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
    static const char* table[] = { "txd", 0 };
    return table;
}

/*
 *  Startup / Shutdown (do nothing)
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
    && IsFileExtension(file.filext, "txd") && IsFileInsideFolder(file.filepath, true, "models/txd"))
        return MODLOADER_YES;
    return MODLOADER_NO;
}

/*
 * Process the replacement
 */
int CThePlugin::ProcessFile(const modloader::ModLoaderFile& file)
{
    /* Load screens sprite is used by the game code, not scripts */
    if(!strcmp(file.filename, "loadscs.txd", false))
    {
        static std::string loadscs;
        if(RegisterReplacementFile(*this, "loadscs.txd", loadscs, GetFilePath(file).c_str()))
        {
            /* Replace "loadscs.txd" string with our path */
            WriteMemory<const char*>(0x5900B6 + 1, loadscs.data(), true);
            
            /* Avoid chdir */
            MakeNOP(0x5900BB, 5);
        }
    }
    else
    {
        /* Just register this file existence */
        AddNewItemToContainer(this->sprites).Setup(file);
    }    
    return 0;
}

/*
 * Called after all files have been processed
 */
int CThePlugin::PosProcess()
{
    /* Patch the game only if there's any custom sprite */
    if(true || this->sprites.size())
    {
        typedef int (*CTxdStore__LoadTxd_t)(int index, const char* filepath);
        static CTxdStore__LoadTxd_t CTxdStore__LoadTxd;
 
        static auto MyLoadTxd = (CTxdStore__LoadTxd_t)([](int index, const char* filepath)
        {
            /* Make sure that it's the original game string, no one hooked it */
            if(!strcmp(filepath, "models\\txd\\", 11, false))
            {
                const char* filename = &filepath[11];

                /* See if we have a replacement for this texture request */
                for(auto& sprite : spritesPlugin->sprites)
                {
                    if(sprite.txdfile == filename)
                    {
                        filepath = sprite.path.data();
                        break;
                    }
                }
            }

            /* Jump to the original call to LoadTxd */
            spritesPlugin->Log("Loading script sprite \"%s\"", filepath);
            return CTxdStore__LoadTxd(index, filepath);
        });
        
        /* Make the hook */
        CTxdStore__LoadTxd = (CTxdStore__LoadTxd_t) MakeCALL(0x48418A, (void*) MyLoadTxd).p;
    }
    
    return 0;
}
