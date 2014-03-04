/*
 * Copyright (C) 2013  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 * File mixing algorithm
 * 
 */
#ifndef FILE_MIXER_HPP
#define	FILE_MIXER_HPP

#include "data.h"
#include <algorithm>
#include <set>
#include <vector>
#include <type_traits>
#include <modloader_util_injector.hpp>

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
inline std::string BuildDataFile(const char* defaultFile, CDataFS<T>& fs, DomFlagsFunctor domflags)
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
        // And it's domflag says to delete entries from default file if doesn't exist in custom file...
        int flags = GetDomFlags(T::domflags());
        if((flags & flag_RemoveIfNotExistInAnyCustom)
        || (flags & flag_RemoveIfNotExistInOneCustomButInDefault))
        {
            // Let's see if the custom file doesn't point to the same place as the default file
            if(NormalizePath(list.front().path) != NormalizePath(list.back().path))
                return list.back().path;    // Then just return the custom file
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
inline std::string BuildDataFile(const char* defaultFile, CDataFS<T>& fs, int domflags)
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
            Log("Loading %s \"%s\"", traits_type::what(), file.c_str());
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


#endif	/* FILE_MIXER_HPP */

