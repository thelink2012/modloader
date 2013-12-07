/*
 * Copyright (C) 2013  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 *  std-data -- Standard Data Loader Plugin for San Andreas Mod Loader
 *          This plugin is responssible for loading .dat, .cfg, .ide and .ipl files from data/ folder
 *
 */
#include "data.h"
#include <modloader_util.hpp>

#include <set>
#include <vector>

#include <cstdlib>
#include <ctime>

CThePlugin* dataPlugin;
static CThePlugin plugin;

fs_t fs;

#include "Injector.h"


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
    char buf[128];
    sprintf(buf, "%s%s\\", this->modloader->cachepath, "std-data");
    this->cachePath = buf;
    MakeSureDirectoryExistA(buf);
    return 0;
}

int CThePlugin::OnShutdown()
{
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
        fs.ide.Add(file);
        return 1;
    }
    else if(IsFileExtension(file.filext, "ipl"))
    {
        fs.ipl.Add(file);
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

static int BuiltDataFilesCount = 0;

// TODO LOG
typedef void (*load_t)(const char*);

template<class T>
static std::string BuildDataFile(const char* fdefault, CDataFS<T>& fs)
{
    auto* pList = fs.GetList(fdefault);
    auto Log = dataPlugin->Log;
    if(!pList) return fdefault;
    
    struct SValue
    {
        const typename T::key_type* p;      // pointer to an value

        SValue(const typename T::key_type& k)
            : p(&k)
        {}
        
        bool operator<(const SValue& rhs) const
        { return (*this->p < *rhs.p); }
    };    
    
    std::set<SValue> keys;
    std::vector< typename T::pair_ref_type > lines;
    
    fs.Add(fdefault).isDefault = true;
   
    auto& tlist = *pList;
    for(auto& x : tlist)
    {
        if(x.LoadData())
        {
            for(auto& itk : x.map)
            {
                keys.insert( SValue(itk.first) );
            }
        }
    }
        
    
    for(auto& k : keys)
    {
        if(auto* line = FindDominantData<T>(*k.p, tlist.begin(), tlist.end(), flag_dominant_ipl))
        {
            lines.push_back( typename T::pair_ref_type(*k.p, *line) );
        }
    }

    Log("CACHE: %s\n", dataPlugin->cachePath.c_str());
    char xfname[64];
    sprintf(xfname, "%sdata_%d", dataPlugin->cachePath.c_str(), ++BuiltDataFilesCount);
    
    Log("----> %s = %s", fdefault, xfname);
    if(T::Build(xfname, lines))
    {
        Log("OI");
        return xfname;
    }
    Log("POXA");
    return fdefault;
    
}
//4002-0022

static load_t xLoadIPL;
static void LoadIPL(const char* filename)
{
    return xLoadIPL(BuildDataFile(filename, fs.ipl).c_str());
}


/*
 * Called after all files have been processed
 */
int CThePlugin::PosProcess()
{
    xLoadIPL = (load_t) MakeCALL(0x5B92C7, (void*) LoadIPL).p;
    
    return 0;
}
