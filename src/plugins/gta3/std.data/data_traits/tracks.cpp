/*
 * Copyright (C) 2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#include <stdinc.hpp>
#include "../data.hpp"
using namespace modloader;

struct tracks_traits
{
    // Detouring traits
    struct dtraits : modloader::dtraits::ReadAndInterpretTrackFile
    {
        static const char* what() { return "train track"; }
    };
};

template<uintptr_t addr>
using OpenTracksDetour = modloader::ReadAndInterpretTrackFileDetour<addr, tracks_traits::dtraits>;


// Population Cycle Properties Detour
static auto xinit = initializer([](DataPlugin* plugin_ptr)
{
    auto ReloadTracks = injector::cstd<void()>::call<0x6F7440>;
    plugin_ptr->AddDetour("tracks.dat", reinstall_since_load, OpenTracksDetour<0x6F7470>(), gdir_refresh(ReloadTracks));
    plugin_ptr->AddDetour("tracks2.dat", reinstall_since_load, OpenTracksDetour<0x6F74BC>(), gdir_refresh(ReloadTracks));
    plugin_ptr->AddDetour("tracks3.dat", reinstall_since_load, OpenTracksDetour<0x6F7496>(), gdir_refresh(ReloadTracks));
    plugin_ptr->AddDetour("tracks4.dat", reinstall_since_load, OpenTracksDetour<0x6F74E2>(), gdir_refresh(ReloadTracks));
});
