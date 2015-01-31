/*
 * Copyright (C) 2015  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#include <stdinc.hpp>
#include "../data_traits.hpp"
using namespace modloader;

struct tracks_traits
{
    struct dtraits : modloader::dtraits::ReadAndInterpretTrackFile
    {
        static const char* what() { return "train track"; }
    };
};

template<uintptr_t addr>
using OpenTracksDetour = modloader::ReadAndInterpretTrackFileDetour<addr, tracks_traits::dtraits>;

static auto xinit = initializer([](DataPlugin* plugin_ptr)
{
    //auto ReloadTracks = injector::cstd<void()>::call<0x6F7440>;
    plugin_ptr->AddDetour("tracks.dat", no_reinstall, OpenTracksDetour<0x6F7470>());
    plugin_ptr->AddDetour("tracks2.dat", no_reinstall, OpenTracksDetour<0x6F74BC>());
    plugin_ptr->AddDetour("tracks3.dat", no_reinstall, OpenTracksDetour<0x6F7496>());
    plugin_ptr->AddDetour("tracks4.dat", no_reinstall, OpenTracksDetour<0x6F74E2>());
});
