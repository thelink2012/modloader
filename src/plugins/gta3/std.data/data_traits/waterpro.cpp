/*
* Copyright (C) 2015  LINK/2012 <dma_2012@hotmail.com>
* Licensed under the MIT License, see LICENSE at top level directory.
*
*/
#include <stdinc.hpp>
#include "../data_traits.hpp"
using namespace modloader;

struct waterpro_traits
{
    struct dtraits : modloader::dtraits::OpenFile
    {
        static const char* what() {
            return "compiled water level";
        }
    };
};

using OpenWaterProDetour = modloader::OpenFileDetour<xVc(0x5C395A), waterpro_traits::dtraits>;

static auto xinit = initializer([](DataPlugin* plugin_ptr)
{
    if(gvm.IsIII() || gvm.IsVC())
    {
        plugin_ptr->AddDetour("waterpro.dat", no_reinstall, OpenWaterProDetour());
    }
});
