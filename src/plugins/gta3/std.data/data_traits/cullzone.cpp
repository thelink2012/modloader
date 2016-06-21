/*
* Copyright (C) 2015  LINK/2012 <dma_2012@hotmail.com>
* Licensed under the MIT License, see LICENSE at top level directory.
*
*/
#include <stdinc.hpp>
#include "../data_traits.hpp"
using namespace modloader;

struct cullzone_traits
{
    struct dtraits : modloader::dtraits::OpenFile
    {
        static const char* what() {
            return "cullzones";
        }
    };
};

using OpenCullzoneDetour = modloader::OpenFileDetour<xIII(0x524EDA), cullzone_traits::dtraits>;

static auto xinit = initializer([](DataPlugin* plugin_ptr)
{
    if(gvm.IsIII())
    {
        plugin_ptr->AddDetour("cullzone.dat", no_reinstall, OpenCullzoneDetour());
    }
});
