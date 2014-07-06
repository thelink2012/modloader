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

    const id_t  dff_start  = 0;
    const id_t& txd_start  = injector::lazy_object<0x5B62CE + 1, uint32_t>::get();
    const id_t& max_models = txd_start;

    enum class EntityType
    {
        Nothing,  Building, Vehicle, Ped, Object, Dummy, NotInPools
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


    static void* GetModelInfo(id_t id)
    {
        auto& p = injector::lazy_object<0x408894 + 3, void**>::get();
        return p[id];
    }

    static id_t GetModelTxdIndex(id_t id)
    {
        if(auto m = GetModelInfo(id)) return ReadOffset<uint16_t>(m, 0x0A);
        return -1;
    }

    static int GetModelUsageCount(id_t id)
    {
        if(auto m = GetModelInfo(id)) return ReadOffset<uint16_t>(m, 0x08);
        return -1;
    }


};
