/*
 * Copyright (C) 2013-2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#include <modloader_util_path.hpp>
#include <modloader_util_ini.hpp>
#include "loader.hpp"
using namespace modloader;

/*
 * Loader::LoadPlugins
 *      Loads all plugins at plugins directory
 */
void Loader::LoadPlugins()
{
    Log("\nLooking for plugins...");
    
    if(this->bEnablePlugins)
    {
        scoped_gdir xdir(this->pluginPath.c_str());
        
        // Read plugins priority
        {
            modloader::ini ini;

            Log("Reading plugins.ini");
            if(ini.load_file("plugins.ini"))
            {
                // Check out plugins priority overrides
                for(auto& pair : ini["PRIORITY"])
                    plugins_priority.emplace(NormalizePath(pair.first), std::stoi(pair.second));
            }
            else Log("Failed to read plugins.ini");
        }
                
                
        // Iterate on each dll plugin and load it
        FilesWalk("", "*.dll", false, [this](FileWalkInfo& file)
        {
            LoadPlugin(file.filepath);
            return true;
        });
    }
}

/*
 * Loader::UnloadPlugins
 *      Unloads all plugins
 */
void Loader::UnloadPlugins()
{
    std::vector<PluginInformation*> aplugins;
    for(auto& plugin : this->plugins) aplugins.emplace_back(&plugin);
    for(auto& plugin : aplugins) this->UnloadPlugin(*plugin);
}

/*
 * Loader::LoadPlugin
 *      Loads the plugin dll @filename
 */
bool Loader::LoadPlugin(std::string filename)
{
    scoped_gdir xdir(this->pluginPath.c_str());
    HMODULE module;
    bool failed = false;
    int priority = -1;
    const char* modulename = filename.c_str();
        
    // Load the plugin module
    if(module = LoadLibraryA(modulename))
    {
        Log("Loading plugin module '%s'", modulename);
        auto GetPluginData = (modloader_fGetPluginData) GetProcAddress(module, "GetPluginData");
        
        // Allocate a new plugin information structure
        this->plugins.emplace_back(module, modulename, GetPluginData);
        PluginInformation& data = this->plugins.back();

        // Check version incompatibilities
        if (data.minor < 2) // 0.2.x introduces a different API set
        {
            failed = true;
            Log("Failed to load module '%s', plugin was compiled for an old version of Mod Loader.", modulename);
        }
        // Check if plugin was written to a (future) version of modloader, if so, we need to be updated
        else if (data.major > MODLOADER_VERSION_MAJOR
                 || (data.major == MODLOADER_VERSION_MAJOR && data.minor > MODLOADER_VERSION_MINOR))
        {
            failed = true;
            Log("Failed to load module '%s', it requieres a newer version of Mod Loader!\nUpdate yourself at: %s",
                modulename, modurl);
        }
        // Zero priority means "don't load"
        else if (data.priority == 0)
        {
            failed = true;
            Log("Plugin module '%s' will not be loaded. It's priority is zero", modulename);
        }
        else
        {
            // Yep, we managed to load
            data.name = data.GetName ? data.GetName(&data) : "NONAME";
            data.version = data.GetVersion ? data.GetVersion(&data) : "";
            data.author = data.GetAuthor ? data.GetAuthor(&data) : "";
            Log("Plugin module '%s' loaded as %s %s %s %s",
                modulename, data.name, data.version,
                data.author ? "by" : "", data.author);
        }
            
        // On failure, unload module, on success, push plugin to list
        if(failed)
        {
            FreeLibrary(module);
            this->plugins.pop_back();
        }
        else
        {
            if(this->StartupPlugin(this->plugins.back()))
            {
                plugins.sort(PriorityPred<PluginInformation>());
                this->RebuildExtensionMap();
            }
        }
    }
    else
        Log("Could not load plugin module '%s'", modulename);

    return !failed;
}

/*
 *  Loader::UnloadPlugin
 *      Unloads a specific plugin
 */
bool Loader::UnloadPlugin(PluginInformation& plugin)
{
    scoped_gdir xdir(this->pluginPath.c_str());
    
    Log("Unloading plugin \"%s\"", plugin.name);
    plugin.Shutdown();
    FreeLibrary( (HMODULE)(plugin.pModule) );
    this->plugins.remove(plugin);
    this->RebuildExtensionMap();
    
    return true;
}

/*
 * Loader::StartupPlugin
 *      Calls the plugin OnStartup method
 */
bool Loader::StartupPlugin(PluginInformation& plugin)
{
    scoped_gdir xdir(this->pluginPath.c_str());
    Log("Starting up plugin \"%s\"", plugin.name);
    if(!plugin.Startup())
    {
        Log("Failed to startup plugin '%s', unloading it.", plugin.name);
        this->UnloadPlugin(plugin);
        return false;
    }
    return true;
}

/*
 * Loader::RebuildExtensionMap
 *      Builds the extension optimization table
 */
void Loader::RebuildExtensionMap()
{
    // Clear the map and rebuild it
    extMap.clear();
    
    for(auto& plugin : this->plugins)
    {
        if(plugin.extable)
        {
            for(auto i = 0u; i < plugin.extable_len; ++i)
                extMap[plugin.extable[i]].emplace_back(&plugin);
        }
    }
}


/*
 *  PluginInformation methods 
 */
bool Loader::PluginInformation::Startup()
{
    return !(OnStartup && OnStartup(this));
}

bool Loader::PluginInformation::Shutdown()
{
    return !(OnShutdown && OnShutdown(this));
}
