/*
 * Copyright (C) 2013-2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#include <stdinc.hpp>
#include <numeric>
#include "loader.hpp"
using namespace modloader;



/*
 *  Loader::ReadBasicConfig
 *       Read the basic configuration file @filename
 */
void Loader::ReadBasicConfig()
{
    linb::ini data;

    Log("Loading basic config file %s", basicConfig.c_str());
    if(data.load_file(gamePath + basicConfig))
    {
        // Read basic stuff from [Config] section
        for(auto& pair : data["Config"])
        {
            if(!compare(pair.first, "EnableMenu", false))
                this->bEnableMenu = to_bool(pair.second);
            else if(!compare(pair.first, "EnablePlugins", false))
                this->bEnablePlugins = to_bool(pair.second);
            else if(!compare(pair.first, "EnableLog", false))
                this->bEnableLog = to_bool(pair.second);
            else if(!compare(pair.first, "ImmediateFlushLog", false))
                this->bImmediateFlush = to_bool(pair.second);
            else if(!compare(pair.first, "MaxLogSize", false))
                this->maxBytesInLog = std::strtoul(pair.second.data(), 0, 0);
            else if(!compare(pair.first, "RefreshKey", false))
                this->vkRefresh = std::stoi(pair.second.data(), 0, 0);
            else if(!compare(pair.first, "AutoRefresh", false))
                this->bAutoRefresh = to_bool(pair.second);
        }
    }
    else
        Log("Failed to load basic config file");
}

/*
 *  Loader::SaveConfig
 *       Saves the basic config file with the current settings applied on this loader
 */
 void Loader::SaveBasicConfig()
 {
     linb::ini ini;
     
     auto& config = ini["Config"];
     config["EnableMenu"]           = modloader::to_string(bEnableMenu);
     config["EnablePlugins"]        = modloader::to_string(bEnablePlugins);
     config["EnableLog"]            = modloader::to_string(bEnableLog);
     config["ImmediateFlushLog"]    = modloader::to_string(bImmediateFlush);
     config["MaxLogSize"]           = std::to_string(maxBytesInLog);
     config["RefreshKey"]           = std::to_string(vkRefresh);
     config["AutoRefresh"]          = modloader::to_string(bAutoRefresh);

     // Log only about failure since we'll be saving every time a entry on the menu changes
     if(!ini.write_file(gamePath + basicConfig))
         Log("Failed to save basic config file");
 }


/*
 *  Loader::ParseCommandLine
 *       Parse command line arguments
 */
void Loader::ParseCommandLine()
{
    char buf[512];
    wchar_t **argv; int argc; 
    argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    
    Log("\nParsing command line");

    if(!argv) // CommandLineToArgvW failed!!
    {
        Log("Failed to parse command line. CommandLineToArgvW() failed.");
        return;
    }
    
    // Converts wchar_t buffer into char* ASCII buffer
    auto toASCII = [](const wchar_t* wstr, char* abuf, size_t asize)
    {
        return !!WideCharToMultiByte(CP_ACP, 0, wstr, -1, abuf, asize, NULL, NULL);
    };
    
    Profile mods_cmd = this->mods.MakeProfile("$CmdLine");
    std::string modprof;
    bool has_nomods_cmd  = false;
    bool has_mod_cmd     = false;
    bool has_modprof_cmd = false;

    for(int i = 0; i < argc; ++i)
    {
        if(argv[i][0] == '-')
        {
            wchar_t *arg = (i+1 < argc? argv[i+1] : nullptr);
            wchar_t *argname = &argv[i][1];

            if(!_wcsicmp(argname, L"nomods"))
            {
                has_nomods_cmd = true;
                Log("Command line ignore received (-nomods)");
            }
            else if(!_wcsicmp(argname, L"mod"))
            {
                if(arg == nullptr)
                {
                    Log("Warning: Failed to read command line because -mod command line is incomplete.");
                    break;
                }
                else if(toASCII(arg, buf, sizeof(buf)))
                {
                    std::string modname = buf;
                    int priority = default_cmd_priority;

                    auto comps = modloader::split(std::string(buf), '=');   // check if specifying priority
                    if(comps.size() == 2)
                    {
                        modname = comps[0];
                        priority = std::strtol(comps[1].c_str(), 0, 0);
                    }

                    mods_cmd.Include(modname);
                    mods_cmd.SetPriority(modname, priority);
                    Log("Command line mod received: \"%s\" with priority %d", modname.c_str(), priority);
                    has_mod_cmd = true;
                }
            }
            else if(!_wcsicmp(argname, L"modprof"))
            {
                // Is argument after mod argument valid?
                if(arg == nullptr)
                {
                    Log("Warning: Failed to read command line because -modprof command line is incomplete.");
                    break;
                }
                else if(toASCII(arg, buf, sizeof(buf)))
                {
                    modprof = buf;
                    has_modprof_cmd = true;
                    Log("Command line mod profile received: \"%s\"", modprof.c_str());
                }
            }
        }
    }


    // Process commands.......
    if(has_nomods_cmd)
    {
        Profile nomods = this->mods.MakeProfile("$CmdLine");
        nomods.SetIgnoreAll(true);
        this->mods.SetAnonymousProfile(nomods, true);
    }
    else if(has_modprof_cmd)
    {
        this->modprof_cmd = std::move(modprof);
    }
    else if(has_mod_cmd)
    {
        mods_cmd.SetExcludeAll(true);
        this->mods.SetAnonymousProfile(mods_cmd, true);
    }

    LocalFree(argv);
}

/*
 *  Loader::UpdateOldConfig
 *       Updates any old ini config into any new format
 */
void Loader::UpdateOldConfig()
{
    // Order matters
    this->UpdateOldConfig_0115_021();
    this->UpdateOldConfig_023_024();
}

/*
 *  Loader::UpdateOldConfig_0115_021
 *       Updates the old ini config (from 0.1.15) into the new ini config format (as from 0.2.1)
 */
void Loader::UpdateOldConfig_0115_021()
{
    linb::ini old, newer;

    struct keydata { const char *section, *key; };

    // Loader independent config, as loaded by this->LoadBasicConfig(), as of 0.2.0 those configs are part of a different ini
    // So just translate them to the new ini file
    auto UpdateBasicConfig = [&]()
    {
        for(auto& pair : old["CONFIG"])
        {
            if(!compare(pair.first, "ENABLE_PLUGINS", false))
                this->bEnablePlugins = to_bool(pair.second);
            else if(!compare(pair.first, "LOG_ENABLE", false))
                this->bEnableLog = to_bool(pair.second);
            else if(!compare(pair.first, "LOG_IMMEDIATE_FLUSH", false))
                this->bImmediateFlush = to_bool(pair.second);
        }
        this->SaveBasicConfig();
    };

    // Takes the key from the old ini and puts in the new ini with new formating
    auto UpdateKey = [&](const keydata& oldkey, const keydata& newkey)
    {
        if(old[oldkey.section].count(oldkey.key))
            newer[newkey.section].emplace(newkey.key, old[oldkey.section][oldkey.key]);
    };

    // Takes a section from the old ini and puts in the new ini with new formating
    auto UpdateSection = [&](const keydata& oldsec, const keydata& newsec)
    {
        if(old.count(oldsec.section))
            newer[newsec.section] = old[oldsec.section];
    };

    if(old.load_file(gamePath + "/modloader/modloader.ini"))
    {
        if(old.count("CONFIG") && old.count("PRIORITY"))
        {
            this->Log("Found 0.1.15 modloader.ini, updating it to 0.2.1 format.");

            // Updates key-values to newer version
            UpdateBasicConfig();
            UpdateSection(keydata { "PRIORITY" }, keydata { "Priority" });
            UpdateSection(keydata { "EXCLUDE_FILES" }, keydata { "IgnoreFiles" });
            UpdateSection(keydata { "INCLUDE_MODS" }, keydata { "IncludeMods" });
            UpdateKey(keydata { "CONFIG", "IGNORE_ALL" },  keydata { "Config", "IgnoreAllFiles" });
            UpdateKey(keydata { "CONFIG", "EXCLUDE_ALL" }, keydata { "Config", "ExcludeAllMods" });

            // Reverse the priority since as from 0.2.1 it's '100 is greater, 1 is lower'
            for(auto& kv : newer["Priority"])
            {
                auto pr = std::stoi(kv.second);
                kv.second = std::to_string(pr > 0 && pr <= 100? 101 - pr : pr);
            }

            newer.write_file(gamePath + "modloader/" +  folderConfigFilename);
        }
    }
}

/*
 *  Loader::UpdateOldConfig_023_024
 *       Updates the old ini config (from 0.2.3) into the new ini config format (as from 0.2.4)
 */
void Loader::UpdateOldConfig_023_024()
{
    modloader_ini old, newer;

    struct keydata { const char *section, *key; };

    // Takes the key from the old ini and puts in the new ini with new formating
    auto UpdateKey = [&](const keydata& oldkey, const keydata& newkey)
    {
        if(old[oldkey.section].count(oldkey.key))
            newer[newkey.section].emplace(newkey.key, old[oldkey.section][oldkey.key]);
    };

    // Takes a section from the old ini and puts in the new ini with new formating
    auto UpdateSection = [&](const keydata& oldsec, const keydata& newsec)
    {
        if(old.count(oldsec.section))
            newer[newsec.section] = old[oldsec.section];
    };

    if(old.load_file(gamePath + "modloader/modloader.ini"))
    {
        if(old.count("Config") && old.count("Priority"))
        {
            this->Log("Found 0.2.3 modloader.ini, updating it to 0.2.4 format.");

            // Updates key-values to newer version
            UpdateSection(keydata { "Config" }, keydata { "Profiles.Default.Config" });
            UpdateSection(keydata { "Priority" }, keydata { "Profiles.Default.Priority" });
            UpdateSection(keydata { "IgnoreFiles" }, keydata { "Profiles.Default.IgnoreFiles" });
            UpdateSection(keydata { "IncludeMods" }, keydata { "Profiles.Default.IncludeMods" });
            UpdateSection(keydata { "IgnoreMods" }, keydata { "Profiles.Default.IgnoreMods" });
            
            // Add new key values
            newer.set("Folder.Config", "Profile", "Default");
            newer["Profiles.Default.ExclusiveMods"];

            newer.write_file(gamePath + "modloader/" +  folderConfigFilename);
        }
    }
}

