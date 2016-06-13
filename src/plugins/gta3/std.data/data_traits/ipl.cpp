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

using OpenSceneDetour = modloader::OpenFileDetour<0x5B871A, ipl_traits::dtraits>;

using namespace std::placeholders;
static auto xinit = initializer([](DataPlugin* plugin_ptr)
{
    if(gvm.IsSA())
    {
        plugin_ptr->AddIplOverrider<OpenSceneDetour>(ipl_merger_name, false, false, true, no_reinstall);
    }
});
