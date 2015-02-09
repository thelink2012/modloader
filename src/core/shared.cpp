/*
 * Copyright (C) 2015  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#include <stdinc.hpp>
#include "loader.hpp"

modloader_shdata_t* Loader::CreateSharedData(const char* name)
{
    if(loader.shdata.count(name) == 0)
    {
        auto& data = loader.shdata[name];
        memset(&data, 0, sizeof(data));
        return &data;
    }
    return nullptr;
}

void Loader::DeleteSharedData(modloader_shdata_t* data)
{
    for(auto it = loader.shdata.begin(); it != loader.shdata.end(); ++it)
    {
        if(&it->second == data)
        {
            loader.shdata.erase(it);
            break;
        }
    }
}

modloader_shdata_t* Loader::FindSharedData(const char* name)
{
    auto it = loader.shdata.find(name);
    if(it != loader.shdata.end()) return &it->second;
    return nullptr;
}
