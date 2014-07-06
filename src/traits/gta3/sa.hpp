/* 
 * Copyright (C) 2013-2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 *  Most of things game version dependent (not being addresseses since addr translator) should be handled by the game trait
 *
 */
#pragma once
#include "base.hpp"

struct TraitsSA : TraitsGTA
{
    using id_t = uint32_t;      // must allow checking for -1
    using VehiclePool_t     = CPool<typename std::aligned_storage<0xA18, 1>::type>;
    using PedPool_t         = CPool<typename std::aligned_storage<0x7C4, 1>::type>;
    using ObjectPool_t      = CPool<typename std::aligned_storage<0x19C, 1>::type>;
    using BuildingPool_t    = CPool<typename std::aligned_storage<0x38, 1>::type>;
    using DummyPool_t       = CPool<typename std::aligned_storage<0x38, 1>::type>;

    const id_t  dff_start   = 0;
    const id_t& txd_start   = injector::lazy_object<0x5B62CE + 1, uint32_t>::get();
    const id_t& max_models  = txd_start;

    const id_t& dff_end     = txd_start;
    const id_t txd_end     = 25000;            // <<<<<<<<<<<<<<<<<<<<<<

    enum class EntityType : uint8_t
    {
        Nothing,  Building, Vehicle, Ped, Object, Dummy, NotInPools
    };

    enum class ModelType : uint8_t
    {
        Atomic = 1, DamageAtomic = 1, Time = 3, Lodtime = 3,
        Weapon = 4, Clump = 5, Vehicle = 6, Ped = 7, LodAtomic = 8
    };

    enum class ResType : uint8_t
    {
        None, Model, TexDictionary, Collision, StreamedScene, Nodes, AnimFile, VehRecording, StreamedScript
    };

    static void*& GetPedIntelligence(void* entity)
    {
        return ReadOffset<void*>(entity, 0x47C);
    }

    static int& GetPedAnimGroup(void* entity)
    {
        return ReadOffset<int>(entity, 0x4D4);
    }
    
    static id_t GetEntityModel(void* entity)
    {
        return ReadOffset<uint16_t>(entity, 0x22);
    }

    static EntityType GetEntityType(void* entity)
    {
        return EntityType(ReadOffset<uint8_t>(entity, 0x36) & 0x7);
    }

    static void* GetEntityRwObject(void* entity)
    {
        return ReadOffset<void*>(entity, 0x18);
    }



    static void* GetModelInfo(id_t id)
    {
        auto& p = injector::lazy_object<0x408894 + 3, void**>::get();
        return p[id];
    }

    static ModelType GetModelType(id_t id)
    {
        auto m = GetModelInfo(id);
        return ModelType(injector::thiscall<uint8_t(void*)>::vtbl<4>(m));
    }

    static id_t GetModelTxdIndex(id_t id)
    {
        if(auto m = GetModelInfo(id)) return ReadOffset<uint16_t>(m, 0x0A);
        return -1;
    }

    static int GetModelUsageCount(id_t id)
    {
        if(auto m = GetModelInfo(id)) return ReadOffset<uint16_t>(m, 0x08);
        return 0;
    }

};
