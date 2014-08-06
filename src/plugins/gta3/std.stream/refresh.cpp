/* 
 * Standard Streamer Plugin for Mod Loader
 * Copyright (C) 2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#include "streaming.hpp"
#include <set>
#include <CPool.h>
#include <traits/gta3/sa.hpp>
using namespace modloader;

class CAnimBlendAssociation;
class CTask;
struct tag_player_t {} tag_player;
struct tag_ped_t {} tag_ped;


/*
 *  Refresher
 *      Refreshes the streaming, essentially refresh models that have been added, modified or deleted on Mod Loader.
 */
template<class T>   // T = Traits (see traits/gta3)
class Refresher : private T
{
    public:
        // To run the refresher, just construct it passing the streaming as argument
        Refresher(CAbstractStreaming& s);
        Refresher(tag_player_t, CAbstractStreaming& s);

    private:
        // Stuf from T we're going to use.... be explict... C++ compilant code needs this
        using T::txd_start;
        using T::GetEntityType;
        using T::GetEntityModel;
        using T::GetEntityRwObject;
        using T::GetPedTaskManager;
        using typename T::PedPool_t;
        using typename T::VehiclePool_t;
        using typename T::BuildingPool_t;
        using typename T::ObjectPool_t;


        using hash_t        = CAbstractStreaming::hash_t;
        using id_t          = CAbstractStreaming::id_t;
        using EntityType    = typename T::EntityType;

        // Events that happens to an entity during the refresh
        struct EntityEvent
        {
            std::function<void(void* entity)> Recreate; // Recreate the model
            std::function<void(void* entity)> Destroy;  // Destroy the model
        };

        // Some stuff that must be saved on the entity between the destroy and the recreation
        union EntitySave
        {
            struct {
                CAnimBlendAssociation* mBlendAssoc;
            } ped;
        };

        // Refreshing information for a specific model
        struct RefreshInfo
        {
            bool bShallLoadBack = false;
        };


        // Reference to the streaming object
        CAbstractStreaming& streaming;
        
        std::map<void*, EntitySave>         mSavedEntityData;   // Saves entity stuff

        // Association maps
        std::map<id_t, std::vector<void*>>  mEntityAssoc;   // Map of association between a model id and many game entities
        std::map<id_t,  std::vector<id_t>>  mTxdAssoc;      // Map of association between a txd and many dffs
        std::map<EntityType, EntityEvent>   mEntityEvents;  // Map of association between type of entities and their refreshing events
        std::map<id_t, RefreshInfo>         mToRefresh;     // List of resources that needs to be refreshed (dff/txd/col/ifp/ipl/etc)


        // Initialization
        void BuildRefreshMap();
        void BuildTxdAssociationMap();
        void BuildEntitiesAssociationMap();
        void SetupEntityEvents();

        // Actual refreshing
        void DestroyEntities();
        void RemoveModels();
        void ProcessImportList();
        void RequestModels();
        void RecreateEntities();

        // Returns an vector of models that uses the specified txd index (starts from 0)
        // This should be ran only after the txd association map is already built
        const std::vector<id_t>& GetModelsUsingTxdIndex(id_t index);

        // Adds entities from the pool at @addr where the type of pool is @PoolType into the entities association map
        // This should be ran after the refresh map is already built
        template<uintptr_t addr, class PoolType>
        void BuildEntitiesAssociationForPool()
        {
            auto* pEntityPool = lazy_object<addr, PoolType*>::get();
            for(int i = 0; i < pEntityPool->m_Size; ++i)
            {
                if(auto* pEntity = pEntityPool->GetAt(i))
                {
                    if(GetEntityRwObject(pEntity))  // Has a RwObject attached to it?
                    {
                        // If this entity id is on the refresh list, place it on the entity association map
                        auto id = GetEntityModel(pEntity);
                        auto it = mToRefresh.find(id);
                        if(it != mToRefresh.end())
                        {
                            if(!it->second.bShallLoadBack) it->second.bShallLoadBack = ShallLoadBackEntity(pEntity);
                            this->mEntityAssoc[id].emplace_back((void*)(pEntity));
                        }
                    }
                }
            }
        }

        // Checks if a specific entity should have it's RwObject loaded back after it gets unloaded
        // In other words, returns true for anything other than Buildings and Objects.
        bool ShallLoadBackEntity(void* entity)
        {
            auto type = GetEntityType(entity);
            return !(type == EntityType::Building || type == EntityType::Object || type == EntityType::Dummy);
        }

        // Adds a saved entity slot and returns it
        EntitySave* AddSavedEntityData(void* entity)
        {
            return &(this->mSavedEntityData.emplace(std::piecewise_construct,
                std::forward_as_tuple(entity),
                std::forward_as_tuple()).first->second);
        }

        // Gets a saved entity slot, returning null if not found
        EntitySave* GetSavedEntityData(void* entity)
        {
            auto it = this->mSavedEntityData.find(entity);
            if(it != mSavedEntityData.end()) return &it->second;
            return nullptr;
        }
};


/*
 *  CAbstractStreaming::ProcessRefreshes
 *      Forwards the call to the correct Refresher
 */
void CAbstractStreaming::ProcessRefreshes()
{
    if(this->to_import.size())
    {
        if(gvm.IsSA()) Refresher<TraitsSA>(*this);
        this->to_import.clear();
    }
    
    if(this->to_rebuild_player)
    {
        Refresher<TraitsSA>(tag_player, *this);
        this->to_rebuild_player = false;
    }
}


/*
 *  Refresher
 *      Refreshes the streaming with some new information
 */
template<class T> 
Refresher<T>::Refresher(CAbstractStreaming& s) 
    : streaming(s)
{
    plugin_ptr->Log("Refreshing necessary models...");

    // Before we do anything we shouldn't have anything on the streaming bus
    streaming.FlushChannels();

    // Remove all unused models on the streaming, so our reloading process will be less painful (faster),
    // we won't end up reloading unused models
    streaming.RemoveUnusedResources();

    // Setup the refresher (order matters)
    this->BuildTxdAssociationMap();
    this->BuildRefreshMap();
    this->BuildEntitiesAssociationMap();
    this->SetupEntityEvents();
    
    // Do the refreshing proccess
    this->DestroyEntities();                // Destroy anything that will be refreshed on the existing entities
    this->RemoveModels();                   // Remove all models that needs refreshing from the streaming
    this->ProcessImportList();              // Process the import list
    this->RequestModels();                  // Request all models previosly removed
    this->RecreateEntities();               // Recreate the pieces of the entities we destroyed previosly

    plugin_ptr->Log("Successfully refreshed models.");
}

/*
 *  Refresher
 *      Refreshes the streaming with some new player clothing information
 */
template<class T> 
Refresher<T>::Refresher(tag_player_t, CAbstractStreaming& s) 
    : streaming(s)
{
    plugin_ptr->Log("Refreshing player...");

    injector::scoped_write<5> always;                   // Always refresh the player clump
    always.write<void*>(0x5A8346 + 1, nullptr, true);   // even when ""nothing"" changed in it

    auto ped = injector::cstd<void*(int)>::call<0x56E210>(-1);      // FindPlayerPed
    injector::cstd<void(void*, char)>::call<0x5A82C0>(ped, false);  // CClothes::RebuildPlayer              

    plugin_ptr->Log("Successfully refreshed player.");
}



/*
 *  Refresher::GetModelsUsingTxdIndex
 *      Gets all models using the specified txd index (starts from zero)
 */
template<class T> auto Refresher<T>::GetModelsUsingTxdIndex(id_t index) -> const std::vector<id_t>&
{
    return this->mTxdAssoc[index];
}





/*
 *  Refresher::BuildTxdAssociationMap
 *      Builds an association map of txd vs. dff for fast lookup
 */
template<class T> void Refresher<T>::BuildTxdAssociationMap()
{
    for(auto i = 0u; i < this->max_models; ++i)
    {
        auto txd = this->GetModelTxdIndex(i);
        if(txd != -1) mTxdAssoc[txd].emplace_back(i);
    }
}


/*
 *  Refresher::BuildRefreshMap
 *      Builds the map of resource indices that needs to be refreshed
 */
template<class T> void Refresher<T>::BuildRefreshMap()
{
    // Helper func to add a model that needs refreshing
    // It filters out the model if it doesn't need the refresh
    auto AddRefresh = [this](id_t id)
    {
        // Go ahead only if this model is valid and is present on the streaming (loaded)
        if(id != -1 && streaming.IsModelOnStreaming(id))
        {
            this->mToRefresh.emplace(std::piecewise_construct,
                std::forward_as_tuple(id),
                std::forward_as_tuple());
            return true;
        }
        return false;
    };

    // For each model that needs reimporting.....
    for(auto& imp : streaming.to_import)
    {
        auto id = streaming.FindModelFromHash(imp.first);
        auto type = streaming.GetIdType(id);

        // Cannot refresh some kinds of resources just in time (In fact I'm not giving any effort to refresh those)
        // TODO COL and IPL needs recalculation on the bounding box
        if(type != ResType::VehRecording && type != ResType::StreamedScript
        && type != ResType::Nodes && type != ResType::AnimFile)
        {
            if(AddRefresh(id))
            {
                // Handle some peculiarities
                switch(type)
                {
                    // For txd files we should also reload it's associated dff files so the RwObject material list is corrected
                    case ResType::TexDictionary:
                    {
                        for(auto& mid : GetModelsUsingTxdIndex(id - txd_start)) AddRefresh(mid);
                        break;
                    }
                }
            }
        }
    }
}

/*
 *  Refresher::BuildEntitiesAssociationMap
 *      Builds association map with entities that needs to be refreshed
 */
template<class T> void Refresher<T>::BuildEntitiesAssociationMap()
{
    BuildEntitiesAssociationForPool<0xB74490, PedPool_t>();         // Ped entities
    BuildEntitiesAssociationForPool<0xB74494, VehiclePool_t>();     // Vehicle entities
    BuildEntitiesAssociationForPool<0xB74498, BuildingPool_t>();    // Static entities
    BuildEntitiesAssociationForPool<0xB7449C, ObjectPool_t>();      // Dynamic entities
}


/*
 *  Refresher::DestroyEntities
 *      Destroy the necessary piece of entities so it can be refreshed later
 */
template<class T> void Refresher<T>::DestroyEntities()
{
    // Iterate by model....
    for(auto& assoc : this->mEntityAssoc)
    {
        // For each entity with this model...
        for(auto entity : assoc.second)
        {
            // Destroy necessary pieces for refreshing
            auto& Destroy = mEntityEvents[GetEntityType(entity)].Destroy;
            if(Destroy) Destroy(entity);
        }
    }
}

/*
 *  Refresher::RemoveModels
 *      Removes the mods to refresh from the streaming
 */
template<class T> void Refresher<T>::RemoveModels()
{
    for(auto& pair : this->mToRefresh) streaming.RemoveModel(pair.first);
    streaming.RemoveUnusedResources();
}

/*
 *  Refresher::ProcessImportList
 *      Process the list of models to get imported or unimported
 *      This is sent by the abstract streaming for us to take care of
 */
template<class T> void Refresher<T>::ProcessImportList()
{
    ref_list<const modloader::file*> import;    // To import
    std::vector<hash_t> unimport;               // To unimport
    auto size = streaming.to_import.size();

    // Setup the lists
    import.reserve(size);
    unimport.reserve(size);
    for(auto& pair : streaming.to_import)
    {
        // If has file associated, we need to import otherwise export
        if(pair.second) import.emplace_back(std::ref(pair.second));
        else            unimport.emplace_back(pair.first);
    }

    // Order maters, so, first unimport the models
    for(auto hash : unimport)
    {
        auto id = streaming.FindModelFromHash(hash);
        if(id != -1) streaming.UnimportModel(id);
    }

    // ...and then import
    streaming.ImportModels(import);
}

/*
 *  Refresher::RequestModels
 *      Requests back all models that needs to be refreshed
 */
template<class T> void Refresher<T>::RequestModels()
{
    // Do the requests
    for(auto& pair : mToRefresh)
    {
        auto& model = *streaming.InfoForModel(pair.first);
        if(model.flags || pair.second.bShallLoadBack) // Has any importance to the streaming?
            streaming.RequestModel(pair.first, model.flags);
    }

    // Stream those models in now!
    streaming.LoadAllRequestedModels();
}

/*
 *  Refresher::RecreateEntities
 *      Recreates the piece of entities we previosly destroyed for refreshing
 */
template<class T> void Refresher<T>::RecreateEntities()
{
    for(auto& assoc : this->mEntityAssoc)
    {
        for(auto entity : assoc.second)
        {
            auto& Recreate = mEntityEvents[GetEntityType(entity)].Recreate;
            if(Recreate) Recreate(entity);
        }
    }
}



/*
 *  Refresher::SetupEntityEvents
 *      Setup events to refresh entities
 */
template<class T> void Refresher<T>::SetupEntityEvents()
{
    auto& ped       = mEntityEvents[EntityType::Ped];
    auto& vehicle   = mEntityEvents[EntityType::Vehicle];
    auto& object    = mEntityEvents[EntityType::Object];
    auto& building  = mEntityEvents[EntityType::Building];
    
    // Calls CEntity::SetModelIndex to recreate the entity model
    static auto RecreateModel = [](void* entity)
    {
        injector::thiscall<void(void*, int)>::vtbl<5>(entity, GetEntityModel(entity));  // CEntity::SetModelIndex
    };

    // Calls CEntity::DeleteRwObject to destroy the entity model
    static auto DestroyModel = [](void* entity)
    {
        injector::thiscall<void(void*)>::vtbl<8>(entity);   // CEntity::DeleteRwObject
    };

    // Peds refreshing (based on script command @BUILD_PLAYER_MODEL)
    if(true)
    {
        // We need to restore the animation association after the recreatioon process
        ped.Recreate = [this](void* entity)
        {
            // XXX RpAnimBlendClumpGiveAssociations may requiere a custom implementation in Vice City!
            // Study the RpAnimBlendClump family of functions in Vice City and rebuild this one by hand.
            auto RpAnimBlendClumpGiveAssociations = injector::cstd<void(void*, CAnimBlendAssociation*)>::call<0x4D6C30>;

            // Recreate model and give anim associations back
            RecreateModel(entity);
            auto save = this->GetSavedEntityData(entity);
            RpAnimBlendClumpGiveAssociations(GetEntityRwObject(entity), save->ped.mBlendAssoc);
        };

        // Save ped anim blend association and make inverse kinematics task abortable, a crash is imminent if we don't do this
        ped.Destroy = [this](void* entity)
        {
            // XXX RpAnimBlendClumpGiveAssociations may requiere a custom implementation in Vice City!
            // Study the RpAnimBlendClump family of functions in Vice City and rebuild this one by hand.
            auto RpAnimBlendClumpExtractAssociations = injector::cstd<CAnimBlendAssociation*(void*)>::call<0x4D6BE0>;

            // Save anim association on the clump (R* extension)
            auto save = this->AddSavedEntityData(entity);
            save->ped.mBlendAssoc = RpAnimBlendClumpExtractAssociations(GetEntityRwObject(entity));

            // Make IK task abortable
            if(auto task = injector::thiscall<CTask*(void*, int)>::call<0x681810>(GetPedTaskManager(entity), 5))   // CTaskManager::GetTaskSecondary
                injector::thiscall<void(CTask*, void*, int, int)>::vtbl<6>(task, entity, 2, 0); // CTask::MakeAbortable

            DestroyModel(entity);
        };
    }

    // Vehicles refreshes
    if(true)
    {
        // Vehicles need to recalculate the suspension lines after the model recreation
        vehicle.Recreate = [](void* entity)
        {
            RecreateModel(entity);
            injector::thiscall<void(void*)>::vtbl<48>(entity);  // CVehicle::SetupSuspensionLines
        };

        // Destroying is pretty standard
        vehicle.Destroy  = DestroyModel;
    }
    
    // Other refreshes
    if(true)
    {
        // Buildings and objects do not need a "Recreate", they'll be recreated automatically by the streaming on demand
        building.Destroy = DestroyModel;
        object.Destroy   = DestroyModel;
    }
}
