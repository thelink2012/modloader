/*
 * Copyright (C) 2013-2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#include "loader.hpp"
#include <modloader/util/path.hpp>
#include <modloader/util/ini.hpp>
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

            // Make sure we have plugins.ini ....
            CopyFileA(pluginConfigDefault.c_str(), pluginConfigFilename.c_str(), TRUE);

            if(ini.load_file("plugins.ini"))
            {
                Log("Reading plugins.ini");
                // Check out plugins priority overrides
                for(auto& pair : ini["Priority"])
                    plugins_priority.emplace(NormalizePath(pair.first), std::stoi(pair.second));
            }
            else Log("Failed to read plugins.ini");
        }
                
                
        // Iterate on each dll plugin and load it
        FilesWalk("", "*.*", true, [this](FileWalkInfo& file)
        {
            if(!file.is_dir && !strcmp(file.filext, "dll", false))
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
    for(auto& plugin : refs(this->plugins)) this->UnloadPlugin(plugin);
}

/*
 * Loader::LoadPlugin
 *      Loads the plugin dll @filename
 */
bool Loader::LoadPlugin(std::string filename)
{
    scoped_gdir xdir(this->pluginPath.c_str());
    
    uint8_t major, minor, revision;
    const char* modulename = filename.c_str();
    int priority = -1;
    bool failed = false;
    HMODULE module;

    // Validate the plugin version to make sure it's compatible
    auto ValidateVersion = [&modulename, &major, &minor, &revision](modloader_fGetLoaderVersion GetLoaderVersion)
    {
        // Since 0.2.x we have GetLoaderVersion to make sure compatibility is fine
        if(GetLoaderVersion == nullptr)
        {
            Log("Failed to load module \"%s\", missing GetLoaderVersion. Plugin possibily compiled for an older version of Mod Loader.", modulename);
            return false;
        }
        else
        {
            GetLoaderVersion(&major, &minor, &revision);
        }

        // Check version incompatibilities
        if (minor < 2) // 0.2.x introduces a different API set
        {
            Log("Failed to load module \"%s\", plugin was compiled for an older version of Mod Loader.", modulename);
            return false;
        }
        // Check if plugin was written to a (future) version of modloader, if so, we need to be updated
        else if (major > MODLOADER_VERSION_MAJOR
                 || (major == MODLOADER_VERSION_MAJOR && minor > MODLOADER_VERSION_MINOR))
        {
            Log("Failed to load module \"%s\", it requieres a newer version of Mod Loader!\nUpdate yourself at: %s",
                modulename, downurl);
            return false;
        }

        return true;
    };

    // Validates the plugin data to make sure it's usable
    auto ValidatePlugin = [&modulename](const modloader::plugin& data)
    {
        // Zero priority means "don't load"
        if (data.priority == 0)
        {
            Log("Plugin module \"%s\" will not be loaded. It's priority is zero", modulename);
            return false;
        }
        return true;
    };


    // Load the plugin module, use full path because we don't want to conflict with any other plugin with same name but different directory
    if(module = LoadLibraryA((this->gamePath + this->pluginPath + modulename).c_str()))
    {
        Log("Loading plugin module \"%s\"", modulename);
        auto GetLoaderVersion = (modloader_fGetLoaderVersion) GetProcAddress(module, "GetLoaderVersion");
        auto GetPluginData = (modloader_fGetPluginData) GetProcAddress(module, "GetPluginData");

        if(ValidateVersion(GetLoaderVersion))
        {
            // Allocate a new plugin information structure
            this->plugins.emplace_back(module, modulename, GetPluginData);
            PluginInformation& data = this->plugins.back();

            // Validate plugin to make sure it' ok to run it
            if(ValidatePlugin(data))
            {
                // We're almost there, setup some important data
                data.major     = major;
                data.minor     = minor;
                data.revision  = revision;
                data.version   = data.GetVersion? data.GetVersion(&data) : "";
                data.author    = data.GetAuthor? data.GetAuthor(&data) : "";

                Log("Plugin module \"%s\" loaded as %s %s %s %s",
                    modulename, data.name, data.version,
                    data.author ? "by" : "", data.author);

                // Startup and go!!!!
                if(this->StartupPlugin(this->plugins.back()))
                {
                    plugins.sort(PriorityPred<PluginInformation>());
                    this->RebuildExtensionMap();
                }
                return true;    // Here LoadPlugin is successful, even if StartupPlugin fails.
                                // StartupPlugin does it's own cleanup for failure :)
            }

            this->plugins.pop_back();
        }

        FreeLibrary(module);
    }
    else
        Log("Could not load plugin module \"%s\"", modulename);

    return false;
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
        Log("Failed to startup plugin \"%s\", unloading it.", plugin.name);
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
                extMap[plugin.extable[i]].emplace_back(plugin);
        }
    }
}


/*
 *  PluginInformation methods 
 */

// Wrappers around the C library...

bool Loader::PluginInformation::Startup()
{
    return !(OnStartup && OnStartup(this));
}

bool Loader::PluginInformation::Shutdown()
{
    return !(OnShutdown && OnShutdown(this));
}

Loader::BehaviourType Loader::PluginInformation::FindBehaviour(modloader::file& m)
{
    return GetBehaviour? (BehaviourType) (GetBehaviour(this, &m)) : BehaviourType::No;
}

bool Loader::PluginInformation::InstallFile(const modloader::file& m)
{
    return base::InstallFile? !base::InstallFile(this, &m) : false;
}

bool Loader::PluginInformation::ReinstallFile(const modloader::file& m)
{
    return base::ReinstallFile? !base::ReinstallFile(this, &m) : false;
}

bool Loader::PluginInformation::UninstallFile(const modloader::file& m)
{
    return base::UninstallFile? !base::UninstallFile(this, &m) : false;
}

void Loader::PluginInformation::Update()
{
    if(base::Update) base::Update(this);
}


/*
 *  PluginInformation::FindFileWithBehaviour
 *      Finds the currently installed file that has the specified @behaviour  
 */
Loader::FileInformation* Loader::PluginInformation::FindFileWithBehaviour(uint64_t behaviour)
{
    auto it = behv.find(behaviour);
    return it != behv.end() ? it->second : nullptr;
}


/*
 *  PluginInformation::Install 
 *      Installs the specified @file using this plugin.
 */
bool Loader::PluginInformation::Install(FileInformation& file)
{
    if(this->IsMainHandlerFor(file))
    {
        // If any other file with the same behaviour present, uninstall it
        if(FileInformation* old = this->FindFileWithBehaviour(file.behaviour))
        {
            if(!old->Uninstall())
                return false;
        }
            
        // Assign this file to it's behaviour
        if(!behv.emplace(file.behaviour, &file).second)
            FatalError("Behaviour emplace didn't took place at Install");
    }
    
    return this->InstallFile(file);
}

/*
 *  PluginInformation::Uninstall 
 *      Uninstalls the specified @file using this plugin.
 */
bool Loader::PluginInformation::Uninstall(FileInformation& file)
{
    if(EnsureBehaviourPresent(file) && this->UninstallFile(file))
    {
        if(this->IsMainHandlerFor(file)) behv.erase(file.behaviour);
        return true;
    }
    return false;
}

/*
 *  PluginInformation::Reinstall 
 *      Reinstalls the specified @file using this plugin.
 */
bool Loader::PluginInformation::Reinstall(FileInformation& file)
{
    if(EnsureBehaviourPresent(file) && !this->ReinstallFile(file))
    {
        if(this->IsMainHandlerFor(file))
        {
            // Somehow we failed to Reinstall, so Uninstall it instead.
            if(!file.Uninstall())
                FatalError("Catastrophical failure at Reinstall");
        }
        return false;
    }
    return true;
}

/*
 *  PluginInformation::EnsureBehaviourPresent 
 *      Ensures the specified @file behaviour is being used on this plugin.
 *      Always return true, in case of failure terminates the application.
 */
bool Loader::PluginInformation::EnsureBehaviourPresent(const FileInformation& file)
{
    if(behv.find(file.behaviour) == behv.end()) FatalError("EnsureBehaviourPresent failed");
    return true;
}
