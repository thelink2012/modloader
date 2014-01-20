/*
 * Copyright (C) 2013  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 *  std-img -- Standard IMG Loader Plugin for San Andreas Mod Loader
 *      Loads all compatible files with imgs (on game request),
 *      it loads directly from disk not by creating a cache or virtual img.
 * 
 */
#include "img.h"
#include <modloader.hpp>
#include <modloader_util.hpp>
#include <modloader_util_path.hpp>
#include <modloader_util_hash.hpp>
#include <modloader_util_injector.hpp>
#include <list>
#include <map>
#include "CdStreamInfo.h"

using namespace modloader;

CThePlugin* imgPlugin;
static CThePlugin plugin;

/*
 *  Export plugin object data
 */
extern "C" __declspec(dllexport)
void GetPluginData(modloader_plugin_t* data)
{
    imgPlugin = &plugin;
    modloader::RegisterPluginData(plugin, data, plugin.default_priority);
}



/*
 *  Basic plugin informations
 */
const char* CThePlugin::GetName()
{
    return "std-img";
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
    /* Put the extensions  this plugin handles on @table
     * SCM disabled because, well, script.img is the standard file for it and is like a 'new' thing
     * May disable RRR too? */
    static const char* table[] = { "img", "dff", "txd", "col", "ipl", "dat", "ifp", "rrr", /*"scm",*/ 0 };
    return table;
}

/*
 *  Startup / Shutdown (do nothing)
 */
bool CThePlugin::OnStartup()
{
    return true;
}

bool CThePlugin::OnShutdown()
{
    return true;
}

/*
 *  Check if the file is the one we're looking for
 */
bool CThePlugin::CheckFile(modloader::ModLoaderFile& file)
{
    if(IsFileExtension(file.filext, "img")) // For directories or actual files
        return true;
    else if(file.is_dir == false)
    {
        /* Check on extension table if the extension is supported */
        for(const char** p = this->GetExtensionTable(); *p; ++p)
        {
            if(IsFileExtension(file.filext, *p))
            {
                int dummy; std::string str;
                
                /* If dat or r3 file, we need to make sure that they're nodes%d.dat or carrec%d.rrr */
                if(IsFileExtension(file.filext, "dat"))
                    return (sscanf(tolower(str=file.filename).c_str(), "nodes%d", &dummy) == 1);
                else if(IsFileExtension(file.filext, "rrr"))
                    return (sscanf(tolower(str=file.filename).c_str(), "carrec%d", &dummy) == 1);
                else
                    return true;
            }
        }
    }
    return false;
}

/*
 * Process the replacement
 */
bool CThePlugin::ProcessFile(const modloader::ModLoaderFile& file)
{
    std::string filepath = GetFilePath(file);
    
    if(IsFileExtension(file.filext, "img"))
    {
        if(file.is_dir)     // If it's a directory, go inside it and take all files
            ReadImgFolder(file);
        else                // Otherwise just push this file into imgFiles list
                            // Put on the front for modloader overriding rule
            imgFiles.emplace_front(file.filename, filepath.c_str());
    }
    else if(!strcmp(file.filename, "ped.ifp", false))
    {
        // Ped.ifp replacement
        RegisterReplacementFile(*this, "ped.ifp", this->pedIfp, filepath.c_str());
    }
    else
    {
        // So it's any kind of handleable file (.dff, .txd, .col, etc)
        // Let's push it into our list for later post processing
        this->modelsFiles.emplace_back(file.filename, filepath.c_str());
    }
    return true;
}

/*
 * Called after all files have been processed
 */
bool CThePlugin::PosProcess()
{
    // Hook ped.ifp string if necessary
    if(this->pedIfp.empty() == false)
        WriteMemory<const char*>(0x4D563D+1, this->pedIfp.c_str(), true);
    
    // Build fast lookup map for models files
    this->BuildModelsMap();
    
    // Replace standard img files if necessary
    this->ReplaceStandardImg();
    
    // Patch the game
    this->StreamingPatch();
    
    return true;
}

/*
 *  Called when the user does a new game or load game
 */
bool CThePlugin::OnReload()
{
    static bool bFirstTime = true;
    if(!bFirstTime) this->ReloadModels();
    bFirstTime = false;
}






/*
 *  Read all files from the folder @file and puts on @modelsFiles list 
 */
void CThePlugin::ReadImgFolder(const modloader::ModLoaderFile& cfile)
{
    ModLoaderFile file = cfile; // Modifiable file structure

    //
    ForeachFile(file.filepath, "*.*", true, [this, &file](const ModLoaderFile& forfile)
    {
        if(forfile.is_dir == false)
        {
            file.filename = forfile.filename;  // Just for convenience
            file.filepath = forfile.filepath;  // Change filepath pointer for proper GetFilePath
            this->modelsFiles.emplace_back(file.filename, GetFilePath(file).c_str());
        }
        return true;
    });
}

/*
 * Builds @models map based on @modelsFiles list
 */
void CThePlugin::BuildModelsMap()
{
    for(auto& file : this->modelsFiles)
    {
        std::string oldpath;    // Trick
                
        auto it = models.find(file.hash);
        if(it != models.end())
        {
            // If already exist on the models list, override
            oldpath = it->second.get().path;
            it->second = std::ref(file);
        }
        else
        {
            // Don't exist on the list, push it now
            models.emplace(file.hash, std::ref(file));
        }
                
        // Register our replacement
        RegisterReplacementFile(*this, file.name.c_str(), oldpath, file.path.c_str(), false);
    }
}

/*
 * Replaces standard img files if necessary, such as gta3.img 
 */
void CThePlugin::ReplaceStandardImg()
{
    // Buffers
    static std::string gta3, gta_int, player, cuts;
    
    struct TableItem
    {
        const char* name;       // img name
        std::string& buf;       // path buf
        uintptr_t pushes[5];    // 'push'es to replace address
    };

    TableItem table[] =
    {
        { "gta3.img",       gta3,    { 0x408430, 0x406C2A, 0x40844C }                       },
        { "gta_int.img",    gta_int, { 0x40846E, 0x40848C }                                 },
        { "player.img",     player,  { 0x5A41A4, 0x5A69F7, 0x5A80F9 }                       },
        { "cuts.img",       cuts,    { 0x4D5EB9, 0x5AFBCB, 0x5AFC98, 0x5B07DA, 0x5B1423 }   }
    };
    
    
    // Iterate in reverse order because of the overriding rule
    for(auto it = this->imgFiles.rbegin(); it != this->imgFiles.rend(); ++it)
    {
        for(TableItem& item : table)
        {
            if(!compare(item.name, it->name, false))
            {
                RegisterReplacementFile(*this, item.name, item.buf, it->path.c_str());
                break;
            }
        }
    }
    
    // Replace all 'push'es for img paths
    for(TableItem& item : table)
    {
        if(item.buf.size()) // Has replacement path...
        {
            // Replace all addresses from push instructions indicated by the pushes array
            for(uintptr_t p : item.pushes)
            {
                if(p == 0) break;
                WriteMemory<const char*>(p + 1, item.buf.c_str(), true);
            }
        }
    }
    
    // If replacing gta3 or gta_int, hook it's loading proc
    if(!gta3.empty() || !gta_int.empty())
    {
        // We need to do this hook to not hook too much code
         MakeNOP(0x4083E4 + 5, 4);
         MakeJMP(0x4083E4, raw_ptr((void*)((void (*)(void))([]()  // mid replacement for CStreaming::OpenGtaImg
         {
             int (*CStreaming__AddImageToList)(const char* filename, char notPlayerImg)
                        = memory_pointer(0x407610).get();
             
             CdStreamInfo& cdinfo = *memory_pointer(0x8E3FEC).get<CdStreamInfo>();
             char* gta3Path       = ReadMemory<char*>(0x40844C + 1, true);
             char* gtaIntPath     = ReadMemory<char*>(0x40848C + 1, true);;

             imgPlugin->Log("Opening gta3 img: %s\nOpening gta_int img: %s", gta3Path, gtaIntPath);

             cdinfo.gta3_id   = CStreaming__AddImageToList(gta3Path, true);
             cdinfo.gtaint_id = CStreaming__AddImageToList(gtaIntPath, true);
         }))));
    }
}
