/*
 * Copyright (C) 2013  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 *  std-data -- Standard Data Loader Plugin for San Andreas Mod Loader
 *          This plugin is responsible for loading .dat, .cfg, .ide and .ipl files from data/ folder
 *
 */
#include "data.h"
#include "readme.hpp"
#include "file_detour.hpp"
#include "file_mixer.hpp"

/* Global objects */
CAllTraits traits;
CThePlugin* dataPlugin;
static CThePlugin plugin;

void Log(const char* msg, ...)
{
    va_list va; va_start(va, msg);
    plugin.vLog(msg, va);
    va_end(va);
}



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
    return "0.2 blue";
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
    scoped_chdir xdir(this->modloader->gamepath);
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
        // Put in the front of the container, so the modloader overriding rule applies
        readme.emplace_front(GetFilePath(file));
    }
    if(IsFileExtension(file.filext, "dat"))
    {
        std::string filepath_buffer = GetFilePath(file);
        const char* filepath = filepath_buffer.c_str();
        // TODO
        
        if(!strcmp(filename, "gta.dat", false))
            traits.gta.AddFile(file, "data/gta.dat");
        else if(!strcmp(filename, "default.dat", false))
            traits.gta.AddFile(file, "data/default.dat");
        else if(!strcmp(filename, "carmods.dat", false))
            traits.carmods.AddFile(file, "data/carmods.dat");
        else if(!strcmp(filename, "timecyc.dat", false))
            RegisterReplacementFile(*this, "timecyc.dat", traits.timecyc, filepath);
        else if(!strcmp(filename, "popcycle.dat", false))
            RegisterReplacementFile(*this, "popcycle.dat", traits.popcycle, filepath);
        else if(!strcmp(filename, "plants.dat", false))
            traits.plants.AddFile(file, "data/plants.dat");
        else
            return false;

        return true;
    }
    else if(IsFileExtension(file.filext, "cfg"))
    {
        if(!strcmp(filename, "handling.cfg", false))
        {
            traits.handling.AddFile(file, "data/handling.cfg");
            return true;
        }
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
 *  Called after all files have been processed
 *  Hooks everything needed
 */
bool CThePlugin::PosProcess()
{
    // Disable some chdirs
    WriteMemory<const char*>(0x5BC09A + 1, "", true);   // Disable chdir("DATA") for popcycle.dat
    WriteMemory<const char*>(0x5BBACA + 1, "", true);   // Disable chdir("DATA") for timecyc.dat
    WriteMemory<const char*>(0x5BD838 + 1, "", true);   // Disable chdir("DATA") for handling.cfg
    WriteMemory<const char*>(0x5DD3BA + 1, "", true);   // Disable chdir("DATA") for plants.dat
    WriteMemory<const char*>(0x5BD84B + 1, "data/handling.cfg", true);  // Change "handling.cfg" string
    WriteMemory<const char*>(0x5DD75F + 1, "data/plants.dat", true);    // Change "plants.dat" string
    
    
    // Mixers
    make_file_mixer<0x5B8428>(traits.ide);
    make_file_mixer<0x5B871A>(traits.ipl);
    make_file_mixer<0x5B905E>(traits.gta);
    make_file_mixer<0x5BD850>(traits.handling);
    make_file_mixer<0x5B65BE>(traits.carmods);
    make_file_mixer<0x5DD3D1>(traits.plants);
        
    // Detours
    make_file_detour<0x5BBADE>(traits.timecyc.c_str(),   "time cycle file");
    make_file_detour<0x5BC0AE>(traits.popcycle.c_str(),  "population cycle file");

    return true;
}

/*
 *  Called on the splash screen 
 */
bool CThePlugin::OnSplash()
{
    // Read the readme files
    if(true)
    {
        // Please organize the following tuple in a order from the most used to the least used
        // You might also consider the cost of the detection
        auto tpair = std::make_tuple(
                        std::make_pair(std::ref(traits.ide),        "data/vehicles.ide"),
                        std::make_pair(std::ref(traits.ide),        "data/maps/veh_mods.ide"),
                        std::make_pair(std::ref(traits.handling),   "data/handling.cfg"),
                        std::make_pair(std::ref(traits.gta),        "data/gta.dat"),
                        std::make_pair(std::ref(traits.carmods),    "data/carmods.dat")
                    );
        
        // Read readme files and detect lines that are interesting for us
        for(auto& readme : this->readme)
        {
            ReadmeReader reader;
            reader(readme.c_str(),  tpair);
        }
        
        // Allow data finalization in handling traits (we're done with readme files)
        TraitsHandling::CanFinalizeData() = true;
    }
    return true;
}


