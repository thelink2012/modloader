/* 
 * Copyright (C) 2015  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#include <modloader/modloader.hpp>

// Interface to communicate with the streaming refresher in std.stream.dll

// The interface returned by StreamRefresherCreate
struct IStreamRefresher
{
    virtual ~IStreamRefresher() {}
    virtual void RequestRefresh(uint32_t id) = 0;   // Adds a id to be refreshed.
    virtual void PrepareRefresh() = 0;              // Prepares to refresh, should be called after the calls to RequestRefresh and before any work.
    virtual void PerformRefresh() = 0;              // PerformRefresh just calls the functions as follow in the same order
    virtual void DestroyEntities() = 0;             // Destroy all entities related to the refresh ids
    virtual void RemoveModels() = 0;                // Removes all models related to the refresh ids
    virtual void RequestModels() = 0;               // Request back the models related to the refresh ids
    virtual void RecreateEntities() = 0;            // Recreate the entities related to the refresh ids
};

// Creates a refresher interface if possible, returns a null pointer on failure.
// The returned pointer should be killed using the keyword 'delete' on it.
inline IStreamRefresher* StreamRefresherCreate(uint32_t flags = 0)  // flags are reserved
{
    if(modloader_shdata_t* data = modloader::plugin_ptr->loader->FindSharedData("StreamRefresherCreate"))
    {
        if(data->type == MODLOADER_SHDATA_FUNCTION)
        {
            typedef IStreamRefresher* (*fStreamCreateRefresher)(uint32_t);
            auto StreamCreateRefresher = (fStreamCreateRefresher)(data->f);
            return StreamCreateRefresher(flags);
        }
    }
    return nullptr;
}




