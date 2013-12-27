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
bool CThePlugin::OnStartup()
{
    // Create std-data cache path
    {
        char buf[128], data0[128];
        sprintf(buf, "%s%s\\", this->modloader->cachepath, "std-data");
        sprintf(data0, "%s%s\\", buf, "data_0");
        
        this->cachePath = buf;
        
        DestroyDirectoryA(buf);
        Log("Creating data cache folder...");
        MakeSureDirectoryExistA(buf);
        MakeSureDirectoryExistA(data0); // for dat and cfg files
    }
    
    return true;
}

bool CThePlugin::OnShutdown()
{
#ifdef NDEBUG
    Log("Deleting data cache folder...");
    DestroyDirectoryA(this->cachePath.c_str());
#endif
    return true;
}


/*
 *  Check if the file is the one we're looking for
 */
bool CThePlugin::CheckFile(modloader::ModLoaderFile& file)
{
    /* Check if handlable extension */
    for(const char** p = GetExtensionTable(); *p; ++p)
    {
        if(IsFileExtension(file.filext, *p))
            return true;
    }
    
    // Not found any extension compatible? If .txt (probably the readme), push it into the list of readmes for later procesing
    if(IsFileExtension(file.filext, "txt"))
    {
        file.call_me = true;
    }
    
    return false;
}

/*
 * Process the replacement
 */
bool CThePlugin::ProcessFile(const modloader::ModLoaderFile& file)
{
    const char* filename = file.filename;
    
    if(IsFileExtension(file.filext, "txt"))
    {
        readme.emplace_back(GetFilePath(file));
    }
    if(IsFileExtension(file.filext, "dat"))
    {
        if(!strcmp(filename, "gta.dat", false))
            traits.gta.AddFile(file, "data/gta.dat");
        else if(!strcmp(filename, "default.dat", false))
            traits.gta.AddFile(file, "data/default.dat");
        
        // TODO
        return true;
    }
    else if(IsFileExtension(file.filext, "cfg"))
    {
        if(!strcmp(filename, "handling.cfg", false))
            traits.handling.AddFile(file, "data/handling.cfg");
    }
    else if(IsFileExtension(file.filext, "ide"))
    {
        traits.ide.AddFile(file);
        return true;
    }
    else if(IsFileExtension(file.filext, "ipl") || IsFileExtension(file.filext, "zon"))
    {
        traits.ipl.AddFile(file);
        return true;
    }
    return false;
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
                keys.emplace_back( key );
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
template<class T, class DomFlagsFunctor>
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
    
    /* If there's two members and one is default and the other isn't ... */
    if(list.size() == 2 && list.front().isDefault && !list.back().isDefault)
    {
        /* And it's domflag says to delete entries from default file if doesn't exist in custom file... */
        int flags = GetDomFlags(T::domflags());
        if((flags & flag_RemoveIfNotExistInAnyCustom)
        || (flags & flag_RemoveIfNotExistInOneCustomButInDefault))
        {
            /* ... then just return th custom file */
            return list.back().path;
        }
    }
    
    
    //
    std::string cacheFile;


    /* Findout the type of the list to store keys based on isSorted template parameter
     * Uses std::set if isSorted or std::vector if not ordered */
    typedef std::vector<std::reference_wrapper<const key_type>>                         keys_type1;
    typedef std::set<std::reference_wrapper<const key_type>, std::less<key_type> >      keys_type2;
    typedef typename std::conditional<!T::is_sorted, keys_type1, keys_type2>::type          keys_type;
    
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
            typename std::conditional<!T::is_sorted, typename add::NonSorted, typename add::Sorted>::type func;
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
template<class T>
static std::string BuildDataFile(const char* defaultFile, CDataFS<T>& fs, int domflags)
{
    return BuildDataFile(defaultFile, fs, [domflags](const typename T::key_type&) { return domflags; });
}

/*
 *  File mixer
 *      Hooks a fopen call to mix files in @fs and opens the mixed file instead
 *      @addr is the address to hook, @T is the fs CDataFS type
 *      Note the object is dummy (has no content), all stored information is static
 */
template<uintptr_t addr, class T>
class CFileMixer : function_hooker<addr, void*(const char*, const char*)>
{
    protected:
        typedef function_hooker<addr, void*(const char*, const char*)> super;
        typedef typename super::func_type func_type;
        
        static T*& FSPtr()                      // the pointer to the used fs
        { static T* a; return a; }

        // The fopen hook
        static void* OpenFile(func_type fopen, const char*& filename, const char*& mode)
        {
            typedef typename T::traits_type  traits_type;
            std::string file = BuildDataFile(filename, *FSPtr(), traits_type::domflags());
            dataPlugin->Log("Loading %s \"%s\"", traits_type::what(), file.c_str());
            return fopen(file.c_str(), mode);
        }
        
    public:
        
        /*
         *  Constructs a file mixer. This should be called only once.
         */
        CFileMixer(T& fs) : super(OpenFile)
        {
            FSPtr() = &fs;
        }
        
};

/*
 *  Helper function to create a file mixer, just pass @fs as argument an it's done 
 */
template<uintptr_t at_address, class T>
CFileMixer<at_address, T> make_file_mixer(T& fs)
{
    return CFileMixer<at_address, T>(fs);
}







/*
 *  Called after all files have been processed
 *  Hooks everything needed
 */
bool CThePlugin::PosProcess()
{    
    this->SetChunks(readme.size());
    this->SetChunkLimiter();
    
    // Hook things
    {
        //WriteMemory<const char*>(0x5BD838 + 1, "", true);   // Disable chdir("DATA") for handling.cfg
        //WriteMemory<const char*>(0x5BD84B + 1, "data/handling.cfg", true);  // Change handling.cfg string
        
        make_file_mixer<0x5B8428>(traits.ide);
        make_file_mixer<0x5B871A>(traits.ipl);
        make_file_mixer<0x5B905E>(traits.gta);
        //make_file_mixer<0x5BD850>(traits.handling);
        
    }
    
    // Ignore when files couldn't get open
    if(false)
    {
        OpenFixer<0x5B8428>();  // For IDE files
        OpenFixer<0x5B871A>();  // For IPL files
    }
    
    return true;
}

bool CThePlugin::OnLoad(bool bOnBar)
{
    if(bOnBar)
    {
        // Process readme files
        for(auto& readme : this->readme)
        {
            ProcessReadmeFile(readme.c_str());
            this->NewChunkLoaded();
        }
    }
    return true;
}

