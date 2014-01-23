/*
 * Copyright (C) 2013  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#ifndef DATA_H
#define	DATA_H

#include <set>
#include <map>
#include <list>
#include <cassert>
#include <type_traits>
#include "traits.hpp"
#include <modloader.hpp>
#include <modloader_util.hpp>
using namespace modloader;
using namespace data;           /* from traits.hpp */



/*
 *  The plugin object
 */
extern class CThePlugin* dataPlugin;
class CThePlugin : public modloader::CPlugin
{
    public:
        std::string cachePath;
        std::list<std::string>      readme;
        std::map<int, std::set<std::string>>  cacheFiles;
        
        CThePlugin()
        {}
        
        const char* GetName();
        const char* GetAuthor();
        const char* GetVersion();
        bool OnStartup();
        bool OnShutdown();
        bool CheckFile(modloader::ModLoaderFile& file);
        bool ProcessFile(const modloader::ModLoaderFile& file);
        bool PosProcess();
        bool OnSplash();
        
        const char** GetExtensionTable();
        
        
        std::string GetCacheFileFor(const char* path)
        {
            // Initialize the cacheFiles map if it wasn't initialized yet
            if(cacheFiles.size() == 0) cacheFiles[0];
            
            // Get path last component (filename) and turn into lower case for case insensitiveness...
            std::string filename = &path[GetLastPathComponent(path)];
            modloader::tolower(filename);
            
            // Get current working cache id and it's files
            auto id = cacheFiles.size() - 1;
            auto& cache = cacheFiles[id];
            
            // Check if file exists in this cache...
            auto it = cache.find(filename);
            if(it == cache.end())
            {
                // It does not!!! Let's push this file into this cache.
                cache.emplace(filename);
                
                // Find out this cache path...
                char buf[MAX_PATH];
                sprintf(buf, "%s\\data_%d\\", cachePath.c_str(), id);
                
                // If the cache is being initialized (first file pushed into it),
                // then Create the cache folder
                if(cache.size() == 1) MakeSureDirectoryExistA(buf);
                
                // Return the path for the cache file...
                return std::move(filename.insert(0, buf));
            }
            else
            {
                // Oh well, this cache already contains this specific filename, let's create another empty cache
                cacheFiles[++id];
                return GetCacheFileFor(path);
            }
        }
};





/*
 *  This [filesystem + stack + something] object stores data about a fake filesystem
 *  This is a very simple object, not something complex as you'd expect from a filesystem
 *  Let's call it a container...
 */
template<class DataTraitsType>  /* DataTraitsType must be of DataTraitsBase */
class CDataFS
{
    private:
        static const bool IsIdeOrIpl
            = std::is_same<TraitsIDE, DataTraitsType>::value || std::is_same<TraitsIPL, DataTraitsType>::value;
        
        // Returns hashed normalized path with DataTraitsType specific transformation
        static size_t NormalizeAndHash(const char* path)
        {
            /*
             *  If DataTraitsType is either SDataIDE or SDataIPL perform intelligent path finding
             *  That's, if the path is "Folder1/Folder2/data/a.ipl", It's of course "data/a.ipl" that we want
             */
            const char* transform = nullptr;
            if(IsIdeOrIpl)
            {
                transform = "data\\";   // transform = NormalizePath("data/");
            }
            return modloader::hash(GetProperlyPath(path, transform));
        }
        
    public:
        typedef DataTraitsType traits_type;
        typedef std::list<DataTraitsType> TraitsList;
        
        /* map< hash_FSPath, ListOfPhysicalPaths > */
        std::map< size_t, TraitsList > files;
        
        // List of all readme traits, normally the size of this list is either 1 or 0
        // The list contains pointers to traits in @files map
        std::list<DataTraitsType*> readmeTraits;

        CDataFS()
        {}
        
    public:
        /* Returns an pointer to the list of files assigned with @path or nullptr if no file is assigned to it */
        TraitsList* GetList(const char* path)
        {
            auto it = files.find(NormalizeAndHash(path));
            if(it == files.end() || it->second.empty())
                return nullptr;
            return &it->second;
        }
        
        /* Returns an pointer to the list of files assigned with @path,
         * if no file is assigned to @path returns the empty list */
        TraitsList* GetListAlways(const char* path)
        {
            return &files[NormalizeAndHash(path)];
        }
        
        /* Adds a file to the container, it's assigned path will be @fsPath and the physical path on the computer is @physicalPath 
         * It will be pushed in the front of the list and marked as default if @isDefault is true
         */
        DataTraitsType& AddFile(const char* fsPath, const char* physicalPath, bool isDefault)
        {
            DataTraitsType* p;
            auto& l = *GetListAlways(fsPath);
            
            /* Get space in the list */
            if(isDefault)
            {
                // Default traits must be at the front of the list
                l.emplace_front();
                p = &l.front();
            }
            else
            {
                // Custom traits must after the default traits
                // And following the modloader override rule, put the new trait before the other traits
                
                // Find position to insert the new trait
                typename TraitsList::iterator ipos;
                for(ipos = l.begin(); ipos != l.end(); ++ipos)
                {
                    if(ipos->isDefault == false) break;
                }
                
                // Push the new trait at the desired position and get it's pointer
                p = &(*l.emplace(ipos));
            }
            
            /* Assign information and return */
            p->path = physicalPath;
            p->isDefault = isDefault;
            return *p;
        }
        
        /* Adds a default file (that's, original, from the base game folder) to the container */
        DataTraitsType& AddFile(const char* filepath)
        {
            return AddFile(filepath, filepath, true);
        }
        
        /* Adds a custom file to the container */
        DataTraitsType& AddFile(const ModLoaderFile& file, const char* fsPath = nullptr)
        {
            if(fsPath == nullptr) fsPath = file.filepath;
            return AddFile(fsPath, GetFilePath(file).c_str(), false);
        }

        /* Returns the cache file (used to finally send data to the game) from the assigned @path name
         * (e.g. "DATA/MAPS/VEGAS/VEGASS.IPL" will return something like "$CACHE_PATH/data_10/VEGASS.IPL" */
        std::string GetCacheFileFor(const char* path)
        {
            return dataPlugin->GetCacheFileFor(path);
        }
        
        bool size()
        {
            return files.size();
        }
        
        /*
         *  Returns the reference to the additional data object
         *  Additional data objects are usually used to store readme files data
         * 
         *  PS: @fsPath is path-sensitive, case-sensitive, everything sensitive...
         */
        DataTraitsType& GetReadmeTrait(const char* fsPath)
        {
            // Try to find existing trait with fsPath
            for(DataTraitsType* a : this->readmeTraits)
            {
                if(!strcmp(fsPath, a->path.c_str()))
                    return *a;
            }
            
            // If not found, create new
            return **(this->readmeTraits.emplace(
                        this->readmeTraits.end(),
                        &AddFile(fsPath, fsPath, false)
                    ));
        }

};


/* All data traits we're going to use are inside this structure */
struct CAllTraits
{
    // The following are really traited and runs throught a complex algorithm
    CDataFS<TraitsIDE>      ide;
    CDataFS<TraitsIPL>      ipl;
    CDataFS<TraitsGTA>      gta;
    CDataFS<TraitsHandling> handling;
    CDataFS<TraitsCarmods>  carmods;
    CDataFS<TraitsPlants>   plants;
    CDataFS<TraitsWater>    water;
    
    // The following are not traited, just overrides
    std::string timecyc;
    std::string popcycle;
    std::string fonts;
    std::string roadblox;
    std::string tracks[4];
    
    
    //
    CAllTraits() :
                timecyc("data/timecyc.dat"),
                popcycle("data/popcycle.dat")
                
    { }
    
    
};
extern CAllTraits traits;


#endif	/* DATA_H */
