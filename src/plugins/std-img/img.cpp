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
#include "Injector.h"
#include <modloader_util.hpp>
#include <functional>
using namespace modloader;

extern const char* noImgName;

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
    return "0.7";
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
        /* Check on extension table if the extension is supported */
        for(const char** p = this->GetExtensionTable(); *p; ++p)
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
        if(!strcmp(file.filename, "ped.ifp", false))
            RegisterReplacementFile(*this, "ped.ifp", this->pedIfp, GetFilePath(file).c_str());
        else
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
    // Process img contents
    mainContent.Process();
    for(auto& x : this->imgFiles) x.Process();
    
    // Replace ped.ifp
    if(!this->pedIfp.empty())
        WriteMemory<const char*>(0x4D563D+1, this->pedIfp.data(), true);
    
    return 0;
}

/*
 *  Adds a new file into the img 'img'
 *  filename2 is used on recursion processes (see ProcessImgFolder), for other purposes it may be nullptr
 */
void CThePlugin::AddFileToImg(ImgInfo& img, const ModLoaderFile& file, const char* filename2)
{
    // See function description commentary
    const char* xname = filename2? filename2 : file.filename;   
    if(!filename2) filename2 = "";
    
    //
    auto& f = img.imgFiles[xname];
    if(RegisterReplacementFile(*this, xname, f.path, (GetFilePath(file) + filename2).c_str()))
    {
        f.name = xname;
    }
}

/*
 *  Process the content inside a folder with extension '.img'
 */
bool CThePlugin::ProcessImgFolder(const modloader::ModLoaderFile& file)
{
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

extern "C" {
    extern int* gta3ImgDescriptorNum;
    extern int* gtaIntImgDescriptorNum;
    extern int (*CStreaming__OpenImgFile)(const char* filename, char notPlayerImg);
};

/*
 *  Takes care of .img files placed in the mod folder
 */
bool CThePlugin::ProcessImgFile(const modloader::ModLoaderFile& file)
{
    /*
     *  Registers the img path at buf, if buf is not empty (already has a path), return false and log the error.
     */
    static auto RegisterImgPath = [](std::string& buf, const char* path)
    {
        return RegisterReplacementFile(*imgPlugin, "", buf, path);
    };
    
    /* Replace an pointer at mem with pointer to buf data */
    static auto ReplaceAddr = [](memory_pointer mem, const std::string& buf)
    {
        WriteMemory<const char*>(mem, buf.data(), true);
    };

    /* Push new img item into container and setup it */
    auto& img = AddNewItemToContainer(this->imgFiles);
    img.Setup(file);
    
    /* Bufs */
    static std::string gta3, gta_int, player, cuts;

    /*
     *  If any of the original img files (loaded from game code strings), replace the strings
     *  Note we'll set isOriginal flag from 'img' only if it's path could be registered
     *          (that's, replaced game code strings with it's path string)
     * 
     */
    if(IsFileInsideFolder(file.filepath, true, "models"))
    {
        if(!strcmp(file.filename, "gta3.img", false))
        {
            if(!RegisterImgPath(gta3, GetFilePath(file).c_str())) return false;

            img.isOriginal = true;
            ReplaceAddr(0x408430 + 1, gta3);
            ReplaceAddr(0x406C2A + 1, gta3);
            ReplaceAddr(0x40844C + 1, gta3);
        }
        else if(!strcmp(file.filename, "gta_int.img", false))
        {
            if(!RegisterImgPath(gta_int, GetFilePath(file).c_str())) return false;
            
            img.isOriginal = true;
            ReplaceAddr(0x40846E + 1, gta_int);
            ReplaceAddr(0x40848C + 1, gta_int);
        }
        else if(!strcmp(file.filename, "player.img", false))
        {
            if(!RegisterImgPath(player, GetFilePath(file).c_str())) return false;
            
            img.isOriginal = true;
            ReplaceAddr(0x5A41A4 + 1, player);
            ReplaceAddr(0x5A69F7 + 1, player);
            ReplaceAddr(0x5A80F9 + 1, player);
        }
    }
    else if(IsFileInsideFolder(file.filepath, true, "anim"))
    { 
        if(!strcmp(file.filename, "cuts.img", false))
        {
            if(!RegisterImgPath(cuts, GetFilePath(file).c_str())) return false;
            
            img.isOriginal = true;
            ReplaceAddr(0x4D5EB9 + 1, cuts);
            ReplaceAddr(0x5AFBCB + 1, cuts);
            ReplaceAddr(0x5AFC98 + 1, cuts);
            ReplaceAddr(0x5B07DA + 1, cuts);
            ReplaceAddr(0x5B1423 + 1, cuts);
        }
    }
    
    // If replacing gta3 or gta_int, hook it's loading proc
    if(!gta3.empty() || ! gta_int.empty())
    {
        // We need to do this hook to not hook too much code
         MakeNOP(0x4083E4 + 5, 4);
         MakeJMP(0x4083E4, (void*)((void (*)(void))([]()  // mid replacement for CStreaming::OpenGtaImg
         {
             char* gta3Path   = ReadMemory<char*>(0x40844C + 1, true);
             char* gtaIntPath = ReadMemory<char*>(0x40848C + 1, true);;

             imgPlugin->Log("Overridden gta3 img: %s\nOverridden gta_int img: %s", gta3Path, gtaIntPath);

             *gta3ImgDescriptorNum = CStreaming__OpenImgFile(gta3Path, true);
             *gtaIntImgDescriptorNum = CStreaming__OpenImgFile(gtaIntPath, true);
         })));
    }
             
    return true;
}
