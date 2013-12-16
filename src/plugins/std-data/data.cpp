/*
 * Copyright (C) 2013  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 *  std-data -- Standard Data Loader Plugin for San Andreas Mod Loader
 *          This plugin is responsible for loading .dat, .cfg, .ide and .ipl files from data/ folder
 *
 */
#include "data.h"
#include "Injector.h"

/* Global objects */
CAllTraits traits;
CThePlugin* dataPlugin;
static CThePlugin plugin;




/*
 *  Export plugin object data
 */
extern "C" __declspec(dllexport)
void GetPluginData(modloader_plugin_t* data)
{
    dataPlugin = &plugin;
    modloader::RegisterPluginData(plugin, data);
    plugin.data->priority = plugin.default_priority;
}



/*
 *  Basic plugin informations
 */
const char* CThePlugin::GetName()
{
    return "std-data";
}

const char* CThePlugin::GetAuthor()
{
    return "LINK/2012";
}

const char* CThePlugin::GetVersion()
{
    return "0.1";
}

const char** CThePlugin::GetExtensionTable()
{
    /* Put the extensions  this plugin handles on @table */
    static const char* table[] = { "dat", "cfg", "ide", "ipl", 0 };
    return table;
}

/*
 *  Startup / Shutdown
 */
int CThePlugin::OnStartup()
{
    // Create std-data cache path
    {
        char buf[128];
        sprintf(buf, "%s%s\\", this->modloader->cachepath, "std-data");
        this->cachePath = buf;
        DestroyDirectoryA(buf);
        MakeSureDirectoryExistA(buf);
    }
    
    return 0;
}

int CThePlugin::OnShutdown()
{
    // Cleanup cache folder
    DestroyDirectoryA(this->cachePath.c_str());
    return 0;
}


/*
 *  Check if the file is the one we're looking for
 */
int CThePlugin::CheckFile(const modloader::ModLoaderFile& file)
{
    /* Check if handlable extension */
    for(const char** p = GetExtensionTable(); *p; ++p)
    {
        if(IsFileExtension(file.filext, *p))
            return MODLOADER_YES;
    }
    return MODLOADER_NO;
}

/*
 * Process the replacement
 */
int CThePlugin::ProcessFile(const modloader::ModLoaderFile& file)
{
    if(IsFileExtension(file.filext, "ide"))
    {
        traits.ide.AddFile(file);
        return 1;
    }
    else if(IsFileExtension(file.filext, "ipl"))
    {
        traits.ipl.AddFile(file);
        return 0;
    }
    else if(IsFileExtension(file.filext, "cfg"))
    {
        // TODO
    }
    else if(IsFileExtension(file.filext, "dat"))
    {
        // TODO
    }    
    
    return 1;
}













/*
 *  Builds replacement file for @defaultFile
 *  @fs is the filesystem with custom files
 *  @domflag is the flags for the algorithm FindDominantData()
 */
template<class T>
static std::string BuildDataFile(const char* defaultFile, CDataFS<T>& fs, int domflags)
{
    auto Log = dataPlugin->Log;
    
    /* Get list of files assigned to defaultFile (that's, candidates to replace/mix with it) */
    auto* pList = fs.GetList(defaultFile);
    if(pList == nullptr)
    {
        /* There's no custom file for defaultFile path, use defaultFile itself as data file */
        return defaultFile;
    }
    
    /* Let's put it in a referece for the sake of readability and modernity! */
    auto& list = *pList;

    std::string cacheFile;
    
    /* Vector to store references to all keys present in default and custom files */
    std::vector<std::reference_wrapper<typename T::key_type>> keys;
    
    /* Vector to store reference to pairs of <key, value> of dominant data, to be later built into a new data file  */
    std::vector<typename T::pair_ref_type> lines;
    
    
    
    /* If the default file really exist (that's, not only custom files about it) put it in fs */
    if(IsPathA(defaultFile))
    {
        fs.AddFile(defaultFile);   // Automatically detected as default
    }
        
    /* Iterate on each file on the list... */
    for(auto& f : list)
    {
        /* ...loading it... */
        if(f.LoadData())
        {
            /* and iterating on their data */
            for(auto& it : f.map)
            {
                auto& key = it.first;
                
                /* check if key isn't detected yet... */
                if(std::find_if(keys.begin(), keys.end(), [&key](const typename T::key_type& a)
                {
                    return (a == key);
                }) == keys.end())
                {
                    /* ... if not detected yet, push it to the list of all keys */
                    keys.push_back(key);
                }
            }
        }
    }
     
    /* Iterate on all detected keys, finding the dominant data at list */
    for(auto& key : keys)
    {
        if(auto* data = FindDominantData<T>(key.get(), list.begin(), list.end(), domflags))
        {
            lines.push_back( typename T::pair_ref_type(key.get(), *data) );
        }
    }

    /* Retrieve the cache file path for building defaultFile replacement... */
    cacheFile = fs.GetCacheFileFor(defaultFile);
    
    /* ...and build the replacement file */
    if(T::Build(cacheFile.c_str(), lines))
    {
        return cacheFile;
    }
    else
        return defaultFile;
    
}



/* Types Helpers */
typedef void (*LoadProcType1)(const char*);
typedef void* (*LoadProcType2)(const char*, const char*);
/* Macros Helpers */
#define MakeProcHook(xtype, callAddress, func)  (procs.func = (xtype) MakeCALL(callAddress, (void*) func).p)
#define MakeProcHook1(callAddress, func)        MakeProcHook(LoadProcType1, callAddress, func)
#define MakeProcHook2(callAddress, func)        MakeProcHook(LoadProcType2, callAddress, func)

/* Stores pointer to original game functions that loads stuff */
struct LoadProcs
{
    LoadProcType1 LoadIPL;
    LoadProcType2 LoadGTAConfig;
    
} procs;

/* ------------------------- Loading hooks ------------------------- */

static void LoadIPL(const char* filename)
{
    std::string file = BuildDataFile(filename, traits.ipl, 0);
    dataPlugin->Log("Loading IPL file \"%s\"", file.c_str());
    return procs.LoadIPL(file.c_str());
}

/*
static void* LoadGTAConfig(const char* filename, const char* mode)
{
    std::string file = BuildDataFile(filename, traits.gta, 0);
    dataPlugin->Log("Loading game config \"%s\"", file.c_str());
    return procs.LoadGTAConfig(file.c_str(), mode);
}
*/

/*
 *  Called after all files have been processed
 *  Hooks everything needed
 */
int CThePlugin::PosProcess()
{
    MakeProcHook1(0x5B92C7, LoadIPL);
    //MakeProcHook2(0x5B905E, LoadGTAConfig);

    return 0;
}
