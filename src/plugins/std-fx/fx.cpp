/*
 * Copyright (C) 2013  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 *  std-fx -- Standard GFX Replacement Plugin for San Andreas Mod Loader
 *      Overrides all *.txd, *.fxp and grass*.dff files from models folder
 * 
 */
#include <modloader.hpp>
#include <modloader_util.hpp>
#include <modloader_util_hash.hpp>
#include <modloader_util_path.hpp>
using namespace modloader;

#include <map>
#include "Injector.h"
#include "modloader_util_injector.hpp"


/*
 *  The plugin object
 */
class CThePlugin* fxPlugin;;
class CThePlugin : public modloader::CPlugin
{
    public:
        const char* GetName();
        const char* GetAuthor();
        const char* GetVersion();
        bool OnStartup();
        bool OnShutdown();
        bool CheckFile(modloader::ModLoaderFile& file);
        bool ProcessFile(const modloader::ModLoaderFile& file);
        bool PosProcess();

        const char** GetExtensionTable();

        /*
         * This object represents a handler for a specific file name (for example effectsPC.txd)
         */
        struct handler_t
        {
            typedef void (*fhandler_t)(const handler_t& self, const char* path);
            
            const char* name;   // This handle handles this file...
            fhandler_t func;    // The function that patches the game for this file
            std::string buf;    // Replacement file
            
            /*
             * Setup the handler
             */
            void Set(const char* name, fhandler_t func)
            {
                this->name = name;
                this->func = func;
            }
            
            /*
             * Process the handler 
             */
            bool Process(const char* path)
            {
                if(!RegisterReplacementFile(*fxPlugin, name, buf, path))
                    return false;
                return true;
            }
            
            /*
             * Pos Process the handler 
             */
            void PosProcess()
            {
                if(buf.size()) func(*this, buf.c_str());
            }
        };
        
        
        // Map of file handlers (see above)
        typedef std::map<uint32_t, handler_t> handler_map;
        handler_map handlers;

        // Setup the handlers map
        void SetupHandlers();
        
        // Helper function
        static size_t hash(const char* filename)
        { return modloader::hash(filename, ::toupper); }
        
} plugin;



/*
 *  Export plugin object data
 */
extern "C" __declspec(dllexport)
void GetPluginData(modloader_plugin_t* data)
{
    fxPlugin = &plugin;
    modloader::RegisterPluginData(plugin, data);
}



/*
 *  Basic plugin informations
 */
const char* CThePlugin::GetName()
{
    return "std-fx";
}

const char* CThePlugin::GetAuthor()
{
    return "LINK/2012";
}

const char* CThePlugin::GetVersion()
{
    return "RC1";
}

const char** CThePlugin::GetExtensionTable()
{
    /* Put the extensions  this plugin handles on @table */
    static const char* table[] = { "fxp", "txd", "dff", 0 };
    return table;
}

/*
 *  Startup / Shutdown
 */
bool CThePlugin::OnStartup()
{
    plugin.SetupHandlers();
    return true;
}

bool CThePlugin::OnShutdown()
{
    return true;
}

/*
 *  Check if the file is the one we're looking for
 */
bool CThePlugin::CheckFile(modloader::ModLoaderFile& file)
{
    if(!file.is_dir)
    {
        handler_map::iterator it = this->handlers.end();

        /* Check if txd/dff/fxp extension */
        if((IsFileExtension(file.filext, "txd") || IsFileExtension(file.filext, "dff")
        ||  IsFileExtension(file.filext, "fxp"))
        /*&& (IsFileInsideFolder(file.filepath, true, "models")
        || IsFileInsideFolder(file.filepath, true, "models/grass")
        || IsFileInsideFolder(file.filepath, true, "models/generic"))*/)
        {
            /* Find the filename in the handlers map */
            it = this->handlers.find(hash(file.filename));
        }
        
        /* If handler was found, we can handle it */
        if(it != this->handlers.end())
            return true;
    }
    
    return false;
}

/*
 * Process the replacement
 */
bool CThePlugin::ProcessFile(const modloader::ModLoaderFile& file)
{
    auto it = this->handlers.find(hash(file.filename));  /* Surely this entry exists */
    return it->second.Process(GetFilePath(file).c_str());
}

/*
 * Called after all files have been processed
 */
bool CThePlugin::PosProcess()
{
    // Call each file handler to patch the game
    for(auto& h : this->handlers) h.second.PosProcess();
    return true;
}

/*
 *  Setup replacement handler foreach file type 
 */
void CThePlugin::SetupHandlers()
{
    const char* name;
    auto& h = this->handlers;
    static const char* pEffectsPC = 0;
    
    /*
     *  Replacers:
     */
    
    name = "vehicle.txd";
    h[hash(name)].Set(name, [](const handler_t& h, const char* path)
    {
        WriteMemory<const char*>(0x5B8F58 + 1, path, true);
    });
    
    name = "effects.fxp";
    h[hash(name)].Set(name, [](const handler_t& h, const char* path)
    {
        WriteMemory<const char*>(0x49EA9D + 1, path, true);
    });
    
    name = "effectsPC.txd";
    h[hash(name)].Set(name, [](const handler_t& h, const char* path)
    {
        pEffectsPC = path; /* Why? Continue reading the code... */
    });
    
    name = "fonts.txd";
    h[hash(name)].Set(name, [](const handler_t& h, const char* path)
    {
        WriteMemory<const char*>(0x5BA69E + 1, path, true);
    });
    
    name = "fronten1.txd";
    h[hash(name)].Set(name, [](const handler_t& h, const char* path)
    {
        WriteMemory<const char*>(0x572F18 + 1, path, true);
    });
    
    name = "fronten2.txd";
    h[hash(name)].Set(name, [](const handler_t& h, const char* path)
    {
        WriteMemory<const char*>(0x57313C + 1, path, true);
        WriteMemory<const char*>(0x57303A + 1, path, true);
    });
    
    name = "fronten3.txd";
    h[hash(name)].Set(name, [](const handler_t& h, const char* path)
    {
        WriteMemory<const char*>(0x5731FB + 1, path, true);
    });
    
    name = "fronten_pc.txd";
    h[hash(name)].Set(name, [](const handler_t& h, const char* path)
    {
        WriteMemory<const char*>(0x572FAF + 1, path, true);
    });
    
    name = "hud.txd";
    h[hash(name)].Set(name, [](const handler_t& h, const char* path)
    {
        WriteMemory<const char*>(0x5BA85F + 1, path, true);
    });
    
    name = "particle.txd";
    h[hash(name)].Set(name, [](const handler_t& h, const char* path)
    {
        WriteMemory<const char*>(0x5BF8B1 + 1, path, true);
    });
    
    name = "pcbtns.txd";
    h[hash(name)].Set(name, [](const handler_t& h, const char* path)
    {
        WriteMemory<const char*>(0x5BA7CE + 1, path, true);
    });
    
    name = "plant1.txd";
    h[hash(name)].Set(name, [](const handler_t& h, const char* path)
    {
        WriteMemory<const char*>(0x5DD959 + 1, path, true);
    });
    
    
    
    /* We need to replace the call that loads effectsPC.txd because it's filename is
     * just the path to effects.fxp with the last chars replaced with 'PC.txd' */
    typedef function_hooker<0x5C248F, int(int, const char*)> hooker;
    make_function_hook<hooker>([](hooker::func_type LoadTxd, int& index, const char*& filename)
    {
        return LoadTxd(index, pEffectsPC? pEffectsPC : "models\\effectsPC.txd");
    });
    
    /* Replace even txd's not used by the game */
    {
        static const char* unused_table[] =
        {
            "misc.txd", "wheels.txd", "wheels.dff", "zonecylb.dff", "hoop.dff", "arrow.dff", "air_vlo.dff",
            "plant1.dff", "grass2_1.dff", "grass2_2.dff", "grass2_3.dff", "grass2_4.dff",
                          "grass3_1.dff", "grass3_2.dff", "grass3_3.dff", "grass3_4.dff",
            0,
        };
        for(const char** p = unused_table; *p; ++p)
        {
            name = *p;
            h[hash(name)].Set(name, [](const handler_t&, const char*){});
        }
    }
        
    // grass*.dff replacer
    {
        /* Replace a sprintf that is like "models/grass/%s" with just "%s" */
        WriteMemory<const char*>(0x5DD25C + 1, "%s", true);
        /* Then replace the grass string pointers to have the full path instead of only the name... */
        WriteMemory<const char*>(0x5DDA83 + 4, "models/grass/grass0_1.dff", true);
        WriteMemory<const char*>(0x5DDA8B + 4, "models/grass/grass0_2.dff", true);
        WriteMemory<const char*>(0x5DDA93 + 4, "models/grass/grass0_3.dff", true);
        WriteMemory<const char*>(0x5DDA9B + 4, "models/grass/grass0_4.dff", true);
        WriteMemory<const char*>(0x5DDABF + 4, "models/grass/grass1_1.dff", true);
        WriteMemory<const char*>(0x5DDAC7 + 4, "models/grass/grass1_2.dff", true);
        WriteMemory<const char*>(0x5DDACF + 4, "models/grass/grass1_3.dff", true);
        WriteMemory<const char*>(0x5DDAD7 + 4, "models/grass/grass1_4.dff", true);
        
        /* Replacers: */
        
        name = "grass0_1.dff";
        h[hash(name)].Set(name, [](const handler_t& h, const char* path)
        {
            WriteMemory<const char*>(0x5DDA83 + 4, path, true);
        });
        
        name = "grass0_2.dff";
        h[hash(name)].Set(name, [](const handler_t& h, const char* path)
        {
            WriteMemory<const char*>(0x5DDA8B + 4, path, true);
        });
        
        name = "grass0_3.dff";
        h[hash(name)].Set(name, [](const handler_t& h, const char* path)
        {
            WriteMemory<const char*>(0x5DDA93 + 4, path, true);
        });
        
        name = "grass0_4.dff";
        h[hash(name)].Set(name, [](const handler_t& h, const char* path)
        {
            WriteMemory<const char*>(0x5DDA9B + 4, path, true);
        });
        
        name = "grass1_1.dff";
        h[hash(name)].Set(name, [](const handler_t& h, const char* path)
        {
            WriteMemory<const char*>(0x5DDABF + 4, path, true);
        });
        
        name = "grass1_2.dff";
        h[hash(name)].Set(name, [](const handler_t& h, const char* path)
        {
            WriteMemory<const char*>(0x5DDAC7 + 4, path, true);
        });
        
        name = "grass1_3.dff";
        h[hash(name)].Set(name, [](const handler_t& h, const char* path)
        {
            WriteMemory<const char*>(0x5DDACF + 4, path, true);
        });
        
        name = "grass1_4.dff";
        h[hash(name)].Set(name, [](const handler_t& h, const char* path)
        {
            WriteMemory<const char*>(0x5DDAD7 + 4, path, true);
        });
    }
    
}
