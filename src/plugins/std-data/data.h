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

#include <modloader.hpp>
#include <modloader_util_path.hpp>
#include "data/traits.h"


using namespace modloader;


/*
 *  This filesystem object stores data about a filesystem
 *  This is a very simple object, not something complex as you'd expect from a filesystem
 */
template<class DataTraisT>  /* DataTraitsT must be of DataTraitsBase */
class CDataFS
{
    public:
        typedef std::list<DataTraisT> TraitsList;
        
        /* map< NormalizedFSPath, ListOfFinalPathData > */
        std::map< std::string, TraitsList > files;
        
        /* TODO hash NormalizeFSPath */
        
        DataTraisT& Add(const char* filepath)
        {
            auto& x = AddNewItemToContainer(files[NormalizePath(filepath)]);
            x.path = filepath;
            return x;
            
        }
        
        DataTraisT& Add(const ModLoaderFile& file)
        {
            auto& x = AddNewItemToContainer(files[NormalizePath(file.filepath)]);
            x.path = GetFilePath(file);
            return x;
        }
        
        TraitsList* GetList(const char* path)
        {
            auto it = files.find(NormalizePath(path));
            if(it == files.end()) return nullptr;
            return &it->second;
        }
        
        static std::string GetCacheFilename(const char* path)
        {
            
        }
        
};

using namespace data;
struct fs_t
{
    CDataFS<TraitsIDE> ide;
    CDataFS<TraitsIPL> ipl;
};
extern fs_t fs;



/*
 *  The plugin object
 */
extern class CThePlugin* dataPlugin;
class CThePlugin : public modloader::CPlugin
{
    public:
        std::string cachePath;
        
        static const int default_priority = 50;
        
        const char* GetName();
        const char* GetAuthor();
        const char* GetVersion();
        int OnStartup();
        int OnShutdown();
        int CheckFile(const modloader::ModLoaderFile& file);
        int ProcessFile(const modloader::ModLoaderFile& file);
        int PosProcess();
        
        const char** GetExtensionTable();

};





#endif	/* DATA_H */

