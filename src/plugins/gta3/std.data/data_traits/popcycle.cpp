/*
 * Copyright (C) 2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under the MIT License, see LICENSE at top level directory.
 * 
 */
#include <stdinc.hpp>
#include "../data_traits.hpp"
using namespace modloader;

struct popcycle_traits
{
    struct dtraits : modloader::dtraits::OpenFile
    {
        static const char* what() { return "population cycle properties"; }
    };
};

using OpenPopCycleDetour = modloader::OpenFileDetour<0x5BC0AE, popcycle_traits::dtraits>;

static auto xinit = initializer([](DataPlugin* plugin_ptr)
{
    if(!gvm.IsSA())
        return;

    auto ReloadPopCycle = []
    {
        injector::cstd<void()>::call<0x5BC090>();   // CPopCycle::Initialise
        // XXX there are some vars at the end of that function body which should not be reseted while the game runs...
        // doesn't cause serious issues but well... shall we take care of them?
    };

    plugin_ptr->AddDetour("popcycle.dat", reinstall_since_start, OpenPopCycleDetour(), gdir_refresh(ReloadPopCycle));
});

