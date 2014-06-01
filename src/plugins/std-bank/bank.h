/*
 * Copyright (C) 2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#ifndef BANK_H
#define	BANK_H

#include <modloader.hpp>
#include <modloader_util.hpp>
#include <modloader_util_container.hpp>
#include <map>
#include <list>
#include <cstdint>

/*
 *  The plugin object
 */
extern class CThePlugin* bankPlugin;
class CThePlugin : public modloader::CPlugin
{
    public:
        static const int default_priority = 52;
        
        struct WaveInfo
        {
            std::string sfx_path;
            std::string path;
            
            WaveInfo(std::string&& a, std::string&& b)
            : sfx_path(std::move(a)), path(std::move(b))
            {}
        };
        
        typedef std::map<std::string, std::string>  sfx_map;
        
        
        typedef std::map<uint16_t, std::string>     sound_map;
        typedef std::map<uint16_t, sound_map>       bank_map;
        typedef std::map<std::string, bank_map>     pak_map;
        

        std::list<WaveInfo> waves_pre;
        pak_map waves;      // Waves and it's pak, bank and sound indices
        sfx_map sfx;        // SFXPak files and it's replacement
        std::string BankLkup, BankSlot, EventVol, PakFiles;
        
        
    public:
        const char* GetName();
        const char* GetAuthor();
        const char* GetVersion();
        
        bool OnStartup();
        bool OnShutdown();
        
        bool CheckFile(modloader::modloader::file& file);
        bool ProcessFile(const modloader::modloader::file& file);
        bool PosProcess();
        
        //bool OnSplash();
        //bool OnLoad();
        //bool OnReload();
        
        const char** GetExtensionTable();

        // Patches the game code
        void Patch();
        
        // Add wave from @file
        bool AddWave(const modloader::modloader::file& file);

        // Finds a wave at @pak in @bank at @sound
        std::string* FindWave(std::string pak, uint16_t bank, uint16_t sound)
        {
            auto it_pak = waves.find(modloader::tolower(pak));
            if(it_pak != waves.end())
            {
                auto it_bank = it_pak->second.find(bank);
                if(it_bank != it_pak->second.end())
                {
                    auto it_sound = it_bank->second.find(sound);
                    if(it_sound != it_bank->second.end())
                        return &it_sound->second;
                }
            }
            return nullptr;
        }
        
        // Finds a wave at @pak in @bank at @sound, if doesn't exist, create empty sting there
        std::string& FindWaveAlways(std::string pak, uint16_t bank, uint16_t sound)
        {
            if(std::string* p = FindWave(pak, bank, sound))
                return *p;
            else
                return waves[modloader::tolower(pak)][bank][sound];
        }
        
        // Add wave from info at @w
        bool AddWave(const WaveInfo& w);
        
        // Adds wave at @path into @pak in @bank at @sound
        void AddWave(std::string path, std::string pak, uint16_t bank, uint16_t sound)
        {
            char name[256];
            sprintf(name, "%s/bank_%d/sound_%d.raw", pak.c_str(), bank+1, sound+1);
            modloader::RegisterReplacementFile(*this, name, FindWaveAlways(pak, bank, sound), path.c_str());
        }
        
        // Adds all wave on the waves_pre list
        // Must be called after pak files name are known
        void AddWaves()
        {
            for(auto& w : waves_pre) AddWave(w);
            waves_pre.clear();
        }
        
        

        // Adds replacement for @pak at @path
        bool AddPak(const std::string& path, std::string pak)
        {
            modloader::tolower(pak);
            modloader::RegisterReplacementFile(*this, pak.c_str(), sfx[pak], path.c_str());
            return true;
        }
        
        // Checks if SFX pak with name @pakname exists.
        bool HasSFXPak(const char* pakname);
        
};

#endif