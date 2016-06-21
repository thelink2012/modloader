/*
 * Copyright (C) 2015  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under the MIT License, see LICENSE at top level directory.
 * 
 */
#include <stdinc.hpp>
#include "../data_traits.hpp"
using namespace modloader;

struct plane_traits
{
    struct dtraits : modloader::dtraits::ReadAndInterpretTrackFile
    {
        static const char* what() { return "plane path"; }
    };
};

template<uintptr_t addr> // using tracks detour type, it works, cba adding one for flight
using OpenFlightDetour = modloader::ReadAndInterpretTrackFileDetour<addr, plane_traits::dtraits>;

static auto xinit = initializer([](DataPlugin* plugin_ptr)
{
    if(gvm.IsVC() || gvm.IsIII())
    {
        plugin_ptr->AddDetour("flight.dat", no_reinstall, OpenFlightDetour<xVc(0x5B220A)>());
        plugin_ptr->AddDetour("flight2.dat", no_reinstall, OpenFlightDetour<xVc(0x5B2475)>());
        plugin_ptr->AddDetour("flight3.dat", no_reinstall, OpenFlightDetour<xVc(0x5B24AE)>());
        if(gvm.IsIII()) plugin_ptr->AddDetour("flight4.dat", no_reinstall, OpenFlightDetour<xIII(0x54BB7C)>());
    }
});
