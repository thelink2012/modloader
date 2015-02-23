/* 
 * Copyright (C) 2015  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under the MIT License, see LICENSE at top level directory.
 * 
 */
#include <modloader/modloader.hpp>

// Interface to communicate with the streaming refresher in std.stream.dll

// Capabilities of a refresher 
enum
{
    STREAMREFRESHER_CAPABILITY_MODEL        = 1,
    STREAMREFRESHER_CAPABILITY_TEXDICT      = 2,
    STREAMREFRESHER_CAPABILITY_COLL         = 4,
    STREAMREFRESHER_CAPABILITY_SCENE        = 8,
    STREAMREFRESHER_CAPABILITY_NODES        = 16,
    STREAMREFRESHER_CAPABILITY_ANIM         = 32,
    STREAMREFRESHER_CAPABILITY_RECORDING    = 64,
    STREAMREFRESHER_CAPABILITY_SCRIPT       = 128,
    STREAMREFRESHER_CAPABILITY_ALL          = 0xFFFFFFFF,
};


// The interface returned by StreamRefresherCreate
struct IStreamRefresher
{
    virtual ~IStreamRefresher() {}

    virtual void Release() = 0;                     // Releases this object
    virtual void RequestRefresh(uint32_t id) = 0;   // Adds a id to be refreshed.
    virtual void PrepareRefresh() = 0;              // Prepares to refresh, should be called after the calls to RequestRefresh and before any work.
    virtual void PerformRefresh() = 0;              // PerformRefresh just calls the functions as follow in the same order
    virtual void DestroyEntities() = 0;             // Destroy all entities related to the refresh ids
    virtual void RemoveModels() = 0;                // Removes all models related to the refresh ids
    virtual void RequestModels() = 0;               // Request back the models related to the refresh ids
    virtual void RecreateEntities() = 0;            // Recreate the entities related to the refresh ids

    // Others
    virtual void Clear() = 0;                       // Clears the refresher object so it can perform another refresh
                                                    // (then call RequestRefresh again and so on).
    virtual void RebuildTxdAssociationMap() = 0;    // On construction this interface builds this association map.
                                                    // If txd related to models change, call this method to rebuild the map.
};

// Creates a refresher interface if possible, returns a null pointer on failure.
// The returned pointer should be killed using it->Release();
inline IStreamRefresher* StreamRefresherCreate(uint32_t capabilities, uint32_t flags)  // flags are reserved, should be 0
{
    if(modloader_shdata_t* data = modloader::plugin_ptr->loader->FindSharedData("StreamRefresherCreate"))
    {
        if(data->type == MODLOADER_SHDATA_FUNCTION)
        {
            typedef IStreamRefresher* (*fStreamCreateRefresher)(uint32_t, uint32_t);
            auto StreamCreateRefresher = (fStreamCreateRefresher)(data->f);
            return StreamCreateRefresher(capabilities, flags);
        }
    }
    return nullptr;
}
