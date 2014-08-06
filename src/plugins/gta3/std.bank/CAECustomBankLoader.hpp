/*
 * Copyright (C) 2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#ifndef CAECUSTOMBANKLOADER_HPP
#define CAECUSTOMBANKLOADER_HPP
#pragma once

#include <cstdint>
#include <map>
#include <modloader/modloader.hpp>


#include <CWavePCM.hpp>
#include <modloader/util/hash.hpp>
#include <modloader/util/container.hpp>
#include <set>


class CAECustomBankLoader;

enum class Type
{
    Wave        = 0,
    SFXPak      = 1,
    BankLookUp  = 2,
    BankSlot    = 3,
    EventVol    = 4,
    PakFiles    = 5,
    Max         = 7,    // Max 3 bits
};


// Basical maskes
static const uint64_t hash_mask_base  = 0xFFFFFFFF;
static const uint64_t type_mask_base  = 0x0007;                 // Mask for type without any shifting
static const uint64_t bank_mask_base  = 0xFFFF;                 // Mask for bank without any shifting
static const uint64_t sound_mask_base = 0x03FF;                 // Mask for sound without any shifting
static const uint32_t type_mask_shf  = 32;                      // Takes 3 bits, starts from 33th bit because first 32th bits is a hash
static const uint32_t bank_mask_shf  = type_mask_shf + 3;       // Takes 16 bits
static const uint32_t sound_mask_shf = bank_mask_shf + 16;      // Takes 10 bits

#if 0
// Actual maskes with shifting
static const uint64_t hash_mask     = 0x00000000FFFFFFFF;                   // Hash mask
static const uint64_t type_mask     = (type_mask_base  << type_mask_shf);   // Type of this behaviour
static const uint64_t bank_mask     = (bank_mask_base  << bank_mask_shf);   // Sound bank related to this behaviour
static const uint64_t sound_mask    = (sound_mask_base << sound_mask_shf);   // Sound number related to this behaviour
#endif

// Sets the initial value for a behaviour, by using an filename hash and file type
inline uint64_t SetType(uint32_t hash, Type type)
{
    return modloader::file::set_mask(uint64_t(hash), type_mask_base, type_mask_shf, type);
}

// Sets the behaviour sound id bitfield 
inline uint64_t SetSound(uint64_t mask, uint16_t sound)
{
    return modloader::file::set_mask(mask, sound_mask_base, sound_mask_shf, sound);
}

// Sets the behaviour bank id bitfield
inline uint64_t SetBank(uint64_t mask, uint16_t bank)
{
    return modloader::file::set_mask(mask, bank_mask_base, bank_mask_shf, bank);
}

// Gets the behaviour sfx pak filename hash
inline uint32_t GetPakHash(uint64_t mask)
{
    return modloader::file::get_mask<uint32_t>(mask, hash_mask_base, 0);
}

// Gets the behaviour file type
inline Type GetType(uint64_t mask)
{
    return modloader::file::get_mask<Type>(mask, type_mask_base, type_mask_shf);
}

// Gets the behaviour sound id
inline uint16_t GetSound(uint64_t mask)
{
    return modloader::file::get_mask<uint16_t>(mask, sound_mask_base, sound_mask_shf);
}

// Gets the behaviour bank id
inline uint16_t GetBank(uint64_t mask)
{
    return modloader::file::get_mask<uint16_t>(mask, bank_mask_base, bank_mask_shf);
}








class CAbstractBankLoader
{
    public:
        struct SoundInfo
        {
            const modloader::file*  file;           // The wave related to this sound
            uint32_t                sound_offset;   // The sound buffer offset on this wave
            uint32_t                sound_size;     // The sound buffer size on this wave
            uint16_t                sample_rate;    // This wave sampling rate
        };

        // Some typedefzzzzz
        using PakHash_t = uint32_t;                             // Hash for an pak filename hash
        using bank_t    = uint16_t;                             // Bank ID
        using sound_t   = uint16_t;                             // Sound ID
        using SndMap_t  = std::map<sound_t, SoundInfo>;         // WAVE - Map of sound ids and their respective sound info
        using BnkMap_t  = std::map<bank_t, SndMap_t>;           // WAVE - Map of bank ids and their respective sound waves
        using WavMap_t  = std::map<PakHash_t, BnkMap_t>;        // WAVE - Map of pak files and their respective banks
        
        using SoundTarget  = std::tuple<PakHash_t, bank_t, sound_t>;// Tuple of pak file, bank id  and sound id, to specify a target sound
        static const int pak_target = 0;                            // Index of the pak hash on the target tuple
        static const int bnk_target = 1;                            // Index of the bank id on the target tuple
        static const int snd_target = 2;                            // Index of the sound id on the target tuple

    private:
        bool                    bIsUpdating = false;                //
        bool                    m_bHasInitialized = false;          // Whether the CAECustomBankLoader has initialized
        PakHash_t               m_GENRL;                            // Hash to tolower("GENRL") sfxpak
        WavMap_t                m_Waves;                            // Wave files map, notice the key contains all sfxpaks

        std::map<std::string, const modloader::file*> sfxpak;       // SFX Pak for replacement
        std::map<SoundTarget, const modloader::file*> to_import;    // To import during Update()

    public:
        CAECustomBankLoader*    m_pBankLoader = nullptr;            // Pointer to our custom bank loader

    public:

        // Initializes this object with the information in the specified bank loader
        void Initialise(CAECustomBankLoader&);

        // Checks whether this has bene initialized
        bool HasInitialized() { return this->m_bHasInitialized; }

        void Flush();       // Loads all pending sounds
        void Update();      // Update after an install
        static void Patch();// Patches the game to work with this loader


        // Gets the full path to the specified sfxpak, overriders will be returned if necessary
        std::string GetSfxPakFullPath(std::string filename)
        {
            using modloader::plugin_ptr;
            auto it = sfxpak.find(modloader::tolower(filename));    // Has replacement?
            if(it != sfxpak.end())  // Yep, return it
                return it->second->fullpath();
            else
                return std::string(plugin_ptr->loader->gamepath) + "audio/sfx/" + filename;
        }

        // Adds the sfxpak 'f' as the replacement to a sfxpak with the same name
        bool AddSfxPak(const modloader::file& f)
        {
            if(!this->HasInitialized()) // Cannot do it after game has initialized
            {
                sfxpak.emplace(f.filename(), &f);
                return true;
            }
            return false;
        }

        // Removes the sfxpak replacement 'f'
        bool RemSfxPak(const modloader::file& f)
        {
            if(!this->HasInitialized()) // Cannot do it after game has initialized
            {
                sfxpak.erase(f.filename());
                return true;
            }
            return false;
        }



        // Installs the specified wave file, delayed or not
        bool InstallWave(const modloader::file& file)
        {
            if(HasInitialized())
            {
                // If game has initialized, we should install it in a delayed way...
                // Register this and wait until Update() to install it
                this->BeginUpdate();
                this->to_import[GetWaveTarget(file)] = &file;
                return true;
            }
            else
            {
                // Game didn't start up yet, we can safely add the wave now
                return this->AddWave(file);
            }
        }

        // Reinstall the wave file, same process as installing
        bool ReinstallWave(const modloader::file& file)
        {
            return this->InstallWave(file);
        }

        // Uninstalls the wave file
        bool UninstallWave(const modloader::file& file)
        {
            if(HasInitialized())
            {
                // Game has initialized, we should delay the uninstall until Update()
                this->BeginUpdate();
                this->to_import[GetWaveTarget(file)] = nullptr;
                return true;
            }
            else
            {
                // Game didn't start up yet, we can safely remove the wave now
                return this->RemWave(file);
            }
        }


    private:
        // Call this during the Install/Uninstall to begin or continue an update
        void BeginUpdate()
        {
            if(this->IsUpdating() == false)
            {
                this->Flush();              // Bus must be empty
                this->bIsUpdating = true;
            }
        }

        // Call this after the refresh to finish the update
        void EndUpdate()
        {
            this->to_import.clear();
            this->bIsUpdating = false;
        }

        // Checks if an update is happening
        bool IsUpdating()
        {
            return this->bIsUpdating;
        }

        
    public:
        // Gets the map of wave files related to the specified pak file an local bank id (1-based)
        // Returns null if pak/bank has no wave files attached to it
        SndMap_t* GetSoundMap(PakHash_t pakhash, bank_t bank)
        {
            auto it = m_Waves.find(pakhash);
            if(it != m_Waves.end())
            {
                auto it_bnk = it->second.find(bank);
                if(it_bnk != it->second.end() && !it_bnk->second.empty())
                    return &it_bnk->second;
            }
            return nullptr;
        }

        // Finds the specified sound id at the specified sound map... sound is 0 based here...
        static const SoundInfo* FindSound(const SndMap_t& sounds, sound_t sound)
        {
            auto it = sounds.find(sound);
            if(it != sounds.end()) return &it->second;
            return nullptr;
        }

    private:
        void WarnSFXPakDoNotExist(const modloader::file& f, uint32_t pakhash)
        {
            static std::set<uint32_t> pakz;

            // Warn only once about an specific non-existing sfxpak
            if(pakz.emplace(pakhash).second)
            {
                modloader::plugin_ptr->Log("Warning: SFXPak for sound file \"%s\" doesn't exist! Using GENRL as default.", f.filepath());
            }
        }

        void WarnSFXPakDoNotExist()
        {
            static bool bWarned = false;
            if(bWarned == false)
            {
                bWarned = true;
                modloader::plugin_ptr->Log("Warning: SFXPak for some sound file doesn't exist! Using GENRL as default.");
            }
        }


        // Gets the sound target for the wave file 'f', that's the sfxpak, bank id and sound id it is related to
        SoundTarget GetWaveTarget(const modloader::file& f)
        {
            auto pakhash = GetPakHash(f.behaviour);
            auto bank    = GetBank(f.behaviour);
            auto sound   = GetSound(f.behaviour);

            // If has initialized, we are able to check if this sfxpak exists
            // If it doesn't, we should use GENRL
            if(m_bHasInitialized)
            {
                if(m_Waves.count(pakhash) == 0)
                {
                    pakhash = m_GENRL;
                    WarnSFXPakDoNotExist(f, pakhash);
                }
            }

            return SoundTarget(pakhash, bank, sound);
        }

        // Adds the wave file 'f' to the abstract banker
        // Notice it overrides any previous wave with the same sound target
        bool AddWave(const modloader::file& f)
        {
            CWavePCM wave(f.fullpath().data());
            if(wave.HasChunks())
            {
                auto target  = GetWaveTarget(f);
                auto pakhash = std::get<pak_target>(target);
                auto bank    = std::get<bnk_target>(target);
                auto sound   = std::get<snd_target>(target);
                m_Waves[pakhash][bank][sound-1] = {&f, wave.GetSoundBufferOffset(), wave.GetSoundBufferSize(), wave.GetSampleRate()};
                return true;
            }
            return false;
        }

        // Removes the wave file 'f' from the abstract banker
        bool RemWave(const modloader::file& f)
        {
            return this->RemWave(GetWaveTarget(f));
        }

        // Removes the wave file at the specified sound target from the abstract banker
        bool RemWave(const SoundTarget& target)
        {
            auto pakhash = std::get<pak_target>(target);
            auto bank    = std::get<bnk_target>(target);
            auto sound   = std::get<snd_target>(target);
            m_Waves[pakhash][bank].erase(sound-1);
            return true;
        }


};
extern CAbstractBankLoader banker;

#endif