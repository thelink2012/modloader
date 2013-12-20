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
#include <modloader_util_injector.hpp>    // Must be included after Injector.h
#include <type_traits>
#include <set>


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
    static const char* table[] = { "dat", "cfg", "ide", "ipl", "zon", 0 };
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
#if NDEBUG
    // Cleanup cache folder
    DestroyDirectoryA(this->cachePath.c_str());
#endif
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
    
    // Not found any extension compatible? If .txt (probably the readme), push it into the list of readmes for later procesing
    if(IsFileExtension(file.filext, "txt"))
    {
        readme.emplace_back(GetFilePath(file));
        // do not mark as MODLOADER_YES !!
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
    else if(IsFileExtension(file.filext, "ipl") || IsFileExtension(file.filext, "zon"))
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
        traits.gta.AddFile(file);
        return 0;
        // TODO
    }
    return 1;
}





/*
 * The following structure is a helper for the BuildDataFile() function
 * It's not contained within the function itself because it has template arguments
 */
template<class T, class K>
struct BuildDataFileKeyInserter
{
    // Used when isSorted == false
    struct NonSorted
    {
        bool operator()(T& keys, const K& key)
        {
            // Check if the key exist in the container...
            if(std::find_if(keys.begin(), keys.end(), [&key](const K& a)
            {
                return (a == key);
            }) == keys.end())
            {
                // It doesn't, push it
                keys.push_back( key );
                return true;
            }
            return false;
        }
    };
    
    // Used when isSorted == true
    struct Sorted
    {
        bool operator()(T& keys, const K& key)
        {
            return keys.insert(key).second;
        }
    };
};


/*
 *  Builds replacement file for @defaultFile
 *  @fs is the filesystem with custom files
 *  @domflag is a functor that returns the flags for the algorithm FindDominantData()
 */
template<bool isSorted, class T, class DomFlagsFunctor>
static std::string BuildDataFile(const char* defaultFile, CDataFS<T>& fs, DomFlagsFunctor domflags)
{
    typedef typename T::key_type    key_type;
    
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

    /* If the default file really exist (that's, not only custom files about it) put it in fs */
    if(IsPathA(defaultFile))
    {
        fs.AddFile(defaultFile);   // Automatically detected as default
    }
    
    /* If there's only one member on the list (probably a single custom element), just return it */
    if(list.size() == 1)
    {
        return list.front().path;
    }
    
    //
    std::string cacheFile;


    /* Findout the type of the list to store keys based on isSorted template parameter
     * Uses std::set if isSorted or std::vector if not ordered */
    typedef std::vector<std::reference_wrapper<const key_type>>                         keys_type1;
    typedef std::set<std::reference_wrapper<const key_type>, std::less<key_type> >      keys_type2;
    typedef typename std::conditional<!isSorted, keys_type1, keys_type2>::type          keys_type;
    
    /* Container to store references to all keys present in default and custom files */
    keys_type keys;
    
    /* Vector to store reference to pairs of <key, value> of dominant data, to be later built into a new data file  */
    std::vector<typename T::pair_ref_type> lines;
    
    auto AddKeysFromContainer = [&keys](typename T::container_type& map)
    {
        /* and iterating on their data */
        for(auto& it : map)
        {
            // Add it key into keys list
            typedef BuildDataFileKeyInserter<keys_type, key_type> add;
            typename std::conditional<!isSorted, typename add::NonSorted, typename add::Sorted>::type func;
            func(keys, it.first);
        }
    };
    
    /* Iterate on each file on the list... */
    for(auto& f : list)
    {
        /* ...loading it... */
        if(f.LoadData())
            AddKeysFromContainer(f.map);
    }
    
    /* Iterate on all detected keys, finding the dominant data at list */
    for(auto& wrapper : keys)
    {
        auto& key = const_cast<key_type&>(wrapper.get());
        if(auto* data = FindDominantData<T>(key, list.begin(), list.end(), domflags(key)))
        {
            lines.emplace_back(key, *data);
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
    {
        return defaultFile;
    }
    
}

/*
 *  Same as BuildDataFile<T,F> but sending the flag directly, no functor 
 */
template<bool isSorted, class T>
static std::string BuildDataFile(const char* defaultFile, CDataFS<T>& fs, int domflags)
{
    return BuildDataFile<isSorted>(defaultFile, fs, [domflags](const typename T::key_type&) { return domflags; });
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
    /* Functor that returns appropriate flags for the IPL key */
    struct
    {
        int operator()(const SDataIPL::key_type& key) const
        {
            if(key.section == IPL_INST) return 0;
            return flag_RemoveIfNotExistInAnyCustom;
        }
        
    } domflags;
    
    std::string file = BuildDataFile<false>(filename, traits.ipl, domflags);
    dataPlugin->Log("Loading IPL file \"%s\"", file.c_str());
    return procs.LoadIPL(file.c_str());
}


static void* LoadGTAConfig(const char* filename, const char* mode)
{
    std::string file = BuildDataFile<true>(filename, traits.gta, flag_RemoveIfNotExistInOneCustomButInDefault);
    dataPlugin->Log("Loading gta config \"%s\"", file.c_str());
    return procs.LoadGTAConfig(file.c_str(), mode);
}










/*
 *  Called after all files have been processed
 *  Hooks everything needed
 */
int CThePlugin::PosProcess()
{
    // Process readme files before anything else!
    this->ProcessReadmeFiles();
    
    // Hook things
    {
        MakeProcHook1(0x5B92C7, LoadIPL);
        MakeProcHook2(0x5B905E, LoadGTAConfig);
    }
    
    // Ignore when files couldn't get open
    {
        OpenFixer<0x5B871A>();  // For IPL files
    }
    
    return 0;
}
