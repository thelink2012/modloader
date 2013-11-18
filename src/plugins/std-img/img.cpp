/*
 * Copyright (C) 2013  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 *  std-img -- Standard IMG Loader Plugin for San Andreas Mod Loader
 *      Creates cached img with all "modloader/*" compatible files and load it on the game, before gta3 and gta_int
 *      Also loads "modloader/*.img" (before gta3 and gta_int too)
 * 
 *  TODO compatible with more exe versions
 * 
 */
#include "img.h"
#include <modloader_util.hpp>
using namespace modloader;



/*
 *  The plugin object
 */
static CThePlugin plugin;
CThePlugin* imgPlugin;

/*
 *  Export plugin object data
 */
extern "C" __declspec(dllexport)
void GetPluginData(modloader_plugin_t* data)
{
    imgPlugin = &plugin;
    modloader::RegisterPluginData(plugin, data);
    plugin.data->priority = plugin.default_priority;
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
    return "0.3";
}

const char** CThePlugin::GetExtensionTable(size_t& len)
{
    /* Put the extensions  this plugin handles on @table
     * SCM disabled because, well, script.img is the standard file for it and is like a 'new' thing
     * May disable RRR too? */
    static const char* table[] = { "img", "dff", "txd", "col", "ipl", "dat", "ifp", "rrr", /*"scm",*/ 0 };
    return (len = GetArrayLength(table), table);
}

/*
 *  Startup / Shutdown
 */

/*
 *  OnStartup
 *      Setup hooks
 */
int CThePlugin::OnStartup()
{
    ApplyPatches();
    return 0;
}

/*
 *  OnShutdown
 *      Do nothing, what should I do?
 */
int CThePlugin::OnShutdown()
{
    return 0;
}

/*
 *  Check if the file is the one we're looking for
 */
int CThePlugin::CheckFile(const modloader::ModLoaderFile& file)
{
    if(IsFileExtension(file.filext, "img"))
    {
        /* Supports both dir and file version */
        return MODLOADER_YES;
    }
    else if(!file.is_dir)
    {
        size_t plen;
        
        /* Check on extension table if the extension is supported */
        for(const char** p = this->GetExtensionTable(plen); *p; ++p)
        {
            if(IsFileExtension(file.filext, *p))
            {
                bool isOkay = true; int dummy;
                std::string str;
                
                /* If dat or r3 file, we need to make sure that they're nodes%d.dat or carrec%d.rrr */
                if(IsFileExtension(file.filext, "dat"))
                    isOkay = (sscanf(tolower(str = file.filename).c_str(), "nodes%d", &dummy) == 1);
                else if(IsFileExtension(file.filext, "rrr"))
                    isOkay = (sscanf(tolower(str = file.filename).c_str(), "carrec%d", &dummy) == 1);
                    
                if(isOkay) return MODLOADER_YES;
            }
        }
    }
    return MODLOADER_NO;
}

/*
 * Process the replacement
 */
int CThePlugin::ProcessFile(const modloader::ModLoaderFile& file)
{
    if(IsFileExtension(file.filext, "img"))
    {
        if(file.is_dir)
            return !this->ProcessImgFolder(file);
        else
            return !this->ProcessImgFile(file);
    }
    else /* Other extensions, dff, txd, etc */
    {
        AddFileToImg(mainContent, file);
        return 0;
    }
    return 1;
}


/*
 * Called after all files have been processed
 */
int CThePlugin::PosProcess()
{
    mainContent.Process();
    for(auto& x : this->imgFiles) x.Process();
    return 0;
}

/*
 *  Adds a new file into the img 'img'
 *  filename2 is used on recursion processes (see ProcessImgFolder), for other purposes it may be nullptr
 */
void CThePlugin::AddFileToImg(ImgInfo& img, const ModLoaderFile& file, const char* filename2)
{
    const char* xname = filename2? filename2 : file.filename;
    if(!filename2) filename2 = "";
    
    auto& f = img.imgFiles[xname];
    if(f.path.empty())
    {
        f.path = GetFilePath(file) + filename2;
        f.name = xname;
    }
    else
        Log("Trying to add new file into std-img but file \"%s\" is already loaded into std-img!\n"
            "\tFirst file path: %s",
            xname, f.path.c_str());
}

/*
 *  Process the content inside a folder with extension '.img'
 */
bool CThePlugin::ProcessImgFolder(const modloader::ModLoaderFile& file)
{
    //auto& img = AddNewItemToContainer(this->imgFiles);
    //img.path = file.filepath;
    auto& img = this->mainContent;
    
    /* Recursivelly iterate on this img folder adding all files into the img list */
    ForeachFile("*.*", true, [this, &img, &file](ModLoaderFile& ff)
    {
        /* ignore directories (recursion will take care of them) */
        if(ff.is_dir == false)
            this->AddFileToImg(img, file, ff.filename);

        return true;
    });
    
    return true;
}

/*
 *  Takes care of .img files placed in the mod folder
 */
bool CThePlugin::ProcessImgFile(const modloader::ModLoaderFile& file)
{
    /*
     *  TODO  loading .img files
     *      The implementation of this function shall:
     *          Replace the img string at the game executable if the filepath is folder example "/models/gta3.img"
     *          In any other case, the file should be ignored, needing gta.dat registering
     *          Well, maybe storing this filepath and looking for it on gta.dat read may be a good idea?
     */
    
    if(IsFileInsideFolder(file.filepath, true, "models"))
    {
    }
    
    return false;
}
