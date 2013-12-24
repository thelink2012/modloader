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
#include "Injector.h"
#include "modloader_util_injector.hpp"
using namespace modloader;
#include <list>



/*
 *  The plugin object
 */
class CThePlugin* spritesPlugin;
class CThePlugin : public modloader::CPlugin
{
    public:
        const char* GetName();
        const char* GetAuthor();
        const char* GetVersion();
        bool CheckFile(const modloader::ModLoaderFile& file);
        bool ProcessFile(const modloader::ModLoaderFile& file);
        bool PosProcess();
        
        const char** GetExtensionTable();

        std::string loadscs;
        
        struct SSpriteFile
        {
            std::string txdfile;    /* e.g. "file.txd" */
            std::string path;       /* e.g. "modloader/mymod/models/txd/file.txd" */

            SSpriteFile(const ModLoaderFile& file)
            {
                this->txdfile = NormalizePath(file.filename);
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
 *  Check if the file is the one we're looking for
 */
bool CThePlugin::CheckFile(const modloader::ModLoaderFile& file)
{
    if(!file.is_dir)
    {
        if(IsFileExtension(file.filext, "txd"))
        {
            if(IsFileInsideFolder(file.filepath, true, "models/txd"))
                return true;
            else if(!strcmp(file.filename, "loadscs.txd", false))
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
    /* Load screens sprite is used by the game code, not scripts */
    if(!strcmp(file.filename, "loadscs.txd", false))
    {
        if(!RegisterReplacementFile(*this, "loadscs.txd", loadscs, GetFilePath(file).c_str()))
            return false;
    }
    else
    {
        /* Just register this file existence */
        this->sprites.emplace_back(file);
    }
    return true;
}

/*
 * Called after all files have been processed
 */
bool CThePlugin::PosProcess()
{
    /* Patch the game only if there's any custom sprite */
    if(this->sprites.size())
    {
        typedef function_hooker<0x48418A, int(int, const char*)> hooker;
        
        // Hooker function, hooks the CTxdStore::LoadTxd at the script engine
        make_function_hook<hooker>([](hooker::func_type LoadTxd, int& index, const char*& filepath)
        {
            /* Make sure that it's the original game string, no one hooked it */
            if(!strcmp(filepath, "models\\txd\\", 11, false))
            {
                std::string filename = NormalizePath(&filepath[11]);

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
            return LoadTxd(index, filepath);
        });
    }
    
    // loadscs patch
    if(this->loadscs.size())
    {
        /* Replace "loadscs.txd" string with our path */
        WriteMemory<const char*>(0x5900CC + 1, loadscs.data(), true);
            
        /* chdir into root directory instead of "models/txd" */
        WriteMemory<const char*>(0x5900B6 + 1, "", true);
    }
    
    return true;
}
