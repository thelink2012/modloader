/*
 * Copyright (C) 2013  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 *
 */
#ifndef DATA_H
#define	DATA_H

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
        std::list<std::string>  readme;
        
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
        bool OnLoad(bool bOnBar);
        
        const char** GetExtensionTable();
        
        void ProcessReadmeFile(const char* filename);
        
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
        
        bool bAdditional;
        DataTraitsType* additional;

        CDataFS() : bAdditional(false), additional(0)
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
                l.emplace_front();
                p = &l.front();
            }
            else
            {
                l.emplace_back();
                p = &l.back();
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
            /*
             *  data_0  is reserved for .dat and .cfg files - this folder already exist, created together with cachePath
             *  data_%d is reserved for .ipl and .ide files (randomly, they don't need to be pairs!)
             *  TODO this system could be better!
             */
            
            static int CacheId = IsIdeOrIpl? 1 : 0;

            char dataPath[MAX_PATH];
            
            /* Create path to store the cache file,
             * it's like that because the original filename (last @path component) must be preserved */
            sprintf(dataPath, "%sdata_%d\\", dataPlugin->cachePath.c_str(), CacheId);
            
            // Increase cache id if the trait is for ide or ipl
            if(IsIdeOrIpl)
            {
                ++CacheId;
                MakeSureDirectoryExistA(dataPath);
            }
            
            /* Append the cache path with the file name */
            return std::string(dataPath) + &path[GetLastPathComponent(path)];
        }
        
        bool size()
        {
            return files.size();
        }
        
        DataTraitsType& Additional(const char* fsPath = nullptr)
        {
            if(fsPath == nullptr && bAdditional == false)
            {
                // We've got a problem
                assert(0);
            }
            if(bAdditional == false)
            {
                additional = &AddFile(fsPath, fsPath, false);
                bAdditional = true;
            }
            return *additional;
        }
        
};


/* All data traits we're going to use are inside this structure */
struct CAllTraits
{
    CDataFS<TraitsIDE>      ide;
    CDataFS<TraitsIPL>      ipl;
    CDataFS<TraitsGTA>      gta;
    CDataFS<TraitsHandling> handling;
};
extern CAllTraits traits;


#endif	/* DATA_H */
