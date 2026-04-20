/*
 * Copyright (C) 2015  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under the MIT License, see LICENSE at top level directory.
 * 
 */
#include <stdinc.hpp>
#include "../data_traits.hpp"
using namespace modloader;

struct ipl_traits
{
    struct dtraits : modloader::dtraits::OpenFile
    {
        static const char* what() { return "scene"; }
    };
};

struct mapzone_traits
{
    struct dtraits : modloader::dtraits::OpenFile
    {
        static const char* what() { return "mapzone"; }
    };
};

using OpenSceneDetour = modloader::OpenFileDetour<0x5B871A, ipl_traits::dtraits>;
using OpenMapZoneDetour = modloader::OpenFileDetour<xIII(0x478570), mapzone_traits::dtraits>;

using namespace std::placeholders;
static auto xinit = initializer([](DataPlugin* plugin_ptr)
{
    if (gvm.IsIII())
    {
        plugin_ptr->AddIplOverrider<OpenSceneDetour, OpenMapZoneDetour>(ipl_merger_name, false, false, true, no_reinstall);
    }
    else
    {
        plugin_ptr->AddIplOverrider<OpenSceneDetour>(ipl_merger_name, false, false, true, no_reinstall);
    }
});
