/*
 * Copyright (C) 2015  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under the MIT License, see LICENSE at top level directory.
 * 
 */
#include <stdinc.hpp>
#include "../data_traits.hpp"
using namespace modloader;

struct furnitur_traits
{
    struct dtraits : modloader::dtraits::OpenFile
    {
        static const char* what() { return "furniture data"; }
    };
};

using OpenFurniturDetour = modloader::OpenFileDetour<0x5C0297, furnitur_traits::dtraits>;

static auto xinit = initializer([](DataPlugin* plugin_ptr)
{
    if(gvm.IsSA())
    {
        auto ReloadFurnitur = []{};
        plugin_ptr->AddDetour("furnitur.dat", reinstall_since_load, OpenFurniturDetour(), gdir_refresh(ReloadFurnitur));
    }
});

