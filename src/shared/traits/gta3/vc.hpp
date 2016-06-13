/*
* Copyright (C) 2016  LINK/2012 <dma_2012@hotmail.com>
* Licensed under the MIT License, see LICENSE at top level directory.
*
*  Most of things game version dependent (not being addressed by the addr translator) should be handled by the game trait
*
*/
#pragma once
#include "base.hpp"

struct TraitsVC : TraitsGTA
{
    // Pools
    using VehiclePool_t = CPool<typename std::aligned_storage<0x5DC, 1>::type>;
    using PedPool_t = CPool<typename std::aligned_storage<0x6D8, 1>::type>;
    using ObjectPool_t = CPool<typename std::aligned_storage<0x1A0, 1>::type>;
    using BuildingPool_t = CPool<typename std::aligned_storage<0x64, 1>::type>;
    using DummyPool_t = CPool<typename std::aligned_storage<0x68, 1>::type>;

    // Indices range
    const id_t dff_start = 0;
    const id_t txd_start = injector::lazy_object<0x5B62CF, unsigned int>::get();  // uint32_t not working properly on GCC here
    const id_t col_start = injector::lazy_object<0x5B6314, unsigned int>::get();  // Why?

    const id_t max_models = txd_start;
    const id_t dff_end = txd_start;

    // Type of entities
    enum class EntityType : uint8_t
    {
        Nothing, Building, Vehicle, Ped, Object, Dummy, NotInPools
    };

    // Type of models
    enum class ModelType : uint8_t
    {
        Atomic = 1, Time = 3,
        Weapon = 4, Clump = 5, Vehicle = 6, Ped = 7, _2DFX = 9,
    };

    // Type of CVehicle
    enum class VehicleType : uint8_t
    {
        Automobile = 0, Bike = 5,
    };


    // VTables
    static const size_t vmt_SetModelIndex = 3;
    static const size_t vmt_DeleteRwObject = 6;


    // Gets the entity model id
    static id_t GetEntityModel(void* entity)
    {
        return ReadOffset<uint16_t>(entity, 4+0x58);
    }

    // Gets the entity type
    static EntityType GetEntityType(void* entity)
    {
        return EntityType(ReadOffset<uint8_t>(entity, 4+0x4C) & 0x7);
    }

    // Gets the entity RwObject pointer
    static void* GetEntityRwObject(void* entity)
    {
        return ReadOffset<void*>(entity, 4+0x48);
    }

    // Gets the ped intelligence pointer from a ped entity
    static void*& GetPedIntelligence(void* entity)
    {
        DoesNotExistInThisGame();
    }

    // Gets the ped task manaager pointer from a ped entity
    static void* GetPedTaskManager(void* entity)
    {
        DoesNotExistInThisGame();
    }
    
    // Gets the type of a CVehicle entity.
    static VehicleType GetVehicleType(void* entity)
    {
        return VehicleType(ReadOffset<uint32_t>(entity, 668));
    }


    // Gets the model information structure from it's id
    static void* GetModelInfo(id_t id)
    {
        auto& p = injector::lazy_object<0x408897, void**>::get();
        return p[id];
    }

    // Gets the model type (Building, Ped, Vehicle, etc) from the model id
    static ModelType GetModelType(id_t id)
    {
        auto m = GetModelInfo(id);
        return ModelType(ReadOffset<uint8_t>(m, 25));
    }

    // Gets the model id tex dictionary index
    static id_t GetModelTxdIndex(id_t id)
    {
        if(auto m = GetModelInfo(id)) return ReadOffset<uint16_t>(m, 38);
        return -1;
    }

    // Gets the usage count of the specified model id
    static int GetModelUsageCount(id_t id)
    {
        if(auto m = GetModelInfo(id)) return ReadOffset<uint16_t>(m, 36);
        return 0;
    }

    /*
    // Gets the hash of the name of the specified model
    static uint32_t GetModelKey(id_t id)
    {
        if(auto m = GetModelInfo(id)) return ReadOffset<uint32_t>(m, 0x04);
        return 0;
    }
    */
};
