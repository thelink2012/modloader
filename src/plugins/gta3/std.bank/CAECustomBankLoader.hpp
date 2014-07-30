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
static const uint64_t type_mask_base  = 0x0007;                 // Mask for type without any shifting
static const uint64_t bank_mask_base  = 0xFFFF;                 // Mask for bank without any shifting
static const uint64_t sound_mask_base = 0x03FF;                 // Mask for sound without any shifting
static const uint32_t type_mask_shf  = 32;                      // Takes 3 bits, starts from 33th bit because first 32th bits is a hash
static const uint32_t bank_mask_shf  = type_mask_shf + 3;       // Takes 16 bits
static const uint32_t sound_mask_shf = bank_mask_shf + 16;      // Takes 10 bits

// Actual maskes with shifting
static const uint64_t hash_mask     = 0x00000000FFFFFFFF;                   // Hash mask
static const uint64_t type_mask     = (type_mask_base  << type_mask_shf);   // Type of this behaviour
static const uint64_t bank_mask     = (bank_mask_base  << bank_mask_shf);   // Sound bank related to this behaviour
static const uint64_t sound_mask    = (sound_mask_base << sound_mask_shf);   // Sound number related to this behaviour

template<class T>
inline uint64_t SetMask(uint64_t mask, T value, uint32_t shift)
{
    return (mask | (uint64_t(value) << shift));
}

inline uint64_t SetType(uint32_t hash, Type type)
{
    return SetMask(uint64_t(hash), type, type_mask_shf);
}

inline uint64_t SetSound(uint64_t mask, uint16_t sound)
{
    return SetMask(mask, sound, sound_mask_shf);
}

inline uint64_t SetBank(uint64_t mask, uint16_t bank)
{
    return SetMask(mask, bank, bank_mask_shf);
}

template<class T>
inline T GetMask(uint64_t mask, uint64_t smask, uint32_t shift)
{
    return T((mask & smask) >> shift);
}

inline uint32_t GetPakHash(uint64_t mask)
{
    return GetMask<uint32_t>(mask, hash_mask, 0);
}

inline Type GetType(uint64_t mask)
{
    return GetMask<Type>(mask, type_mask, type_mask_shf);
}

inline uint16_t GetSound(uint64_t mask)
{
    return GetMask<uint16_t>(mask, sound_mask, sound_mask_shf);
}

inline uint16_t GetBank(uint64_t mask)
{
    return GetMask<uint16_t>(mask, bank_mask, bank_mask_shf);
}


#include <modloader/util/hash.hpp>


class CAECustomBankLoader;


class CAbstractBankLoader
{
    // TODO MUST LOCK OR FLUSH IN TIME

    protected:
        struct SoundInfo
        {
            const modloader::file*  file;
            uint32_t                sound_offset;
            uint32_t                sound_size;
            uint16_t                sample_rate;
        };

        using PakHash_t = size_t;
        using bank_t    = uint16_t;
        using sound_t   = uint16_t;
        using SndMap_t  = std::map<sound_t, SoundInfo>;
        using BnkMap_t  = std::map<bank_t, SndMap_t>;
        using WavMap_t  = std::map<PakHash_t, BnkMap_t>;

        bool                    m_bHasInitialized;
        PakHash_t               m_GENRL; 
        CAECustomBankLoader*    m_pBankLoader;
        WavMap_t                m_Waves;

        

    public:

        void Initialise(CAECustomBankLoader&);

        std::string GetSfxPakFullPath(const char* filename)
        {
            using modloader::plugin_ptr;
            return std::string(plugin_ptr->loader->gamepath) + "AUDIO\\SFX\\" + filename;
        }

        bool AddWave(const modloader::file& f)
        {
            auto pakhash = GetPakHash(f.behaviour);
            auto bank    = GetBank(f.behaviour);
            auto sound   = GetSound(f.behaviour);

            if(m_bHasInitialized)
            {
                if(m_Waves.count(pakhash) == 0)
                {
                    // TODO WARNING
                    pakhash = m_GENRL;
                }
            }

            CWavePCM wave(f.fullpath().data());
            if(wave.HasChunks())
            {
                m_Waves[pakhash][bank][sound-1] = {&f, wave.GetSoundBufferOffset(), wave.GetSoundBufferSize(), wave.GetSampleRate()};
                return true;
            }
            return false;
        }

        // bank is not 0 based
        SndMap_t* GetSoundMap(std::string pakname, bank_t bank)
        {
            auto pakhash = modloader::hash(pakname, ::tolower);
            auto it = m_Waves.find(pakhash);
            if(it != m_Waves.end())
            {
                auto itb = it->second.find(bank);
                if(itb != it->second.end()) return &itb->second;
            }
            return nullptr;
        }

        // 0 based
        static const SoundInfo* FindSound(const SndMap_t& sounds, sound_t snd)
        {
            auto it = sounds.find(snd);
            if(it != sounds.end()) return &it->second;
            return nullptr;
        }

        const SndMap_t* work = nullptr;

        void SetWorkingBank(const SndMap_t* bank)
        {
            work = bank;
        }

        const SndMap_t* GetWorkingBank()
        {
            return work;
        }



#if 0
        bool HasAnyWaveForBank(std::string pakname, bank_t bank)
        {
            auto pakhash = modloader::hash(pakname, ::tolower);

            auto it = m_Waves.find(pakhash);
            if(it != m_Waves.end())
            {
                return it->second.count(bank) > 0;
            }
            return false;
        }
#endif

};
extern CAbstractBankLoader banker;

static_assert(FOPEN_MAX < 400, "Not enought fopens for a bank");




#include <vector>

#endif