/*
 * Copyright (C) 2015  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#include <stdinc.hpp>
#include "../data.hpp"
using namespace modloader;

struct clothes_traits
{
    struct dtraits : modloader::dtraits::OpenFile
    {
        static const char* what() { return "clothes rules"; }
    };
};

using OpenClothesDetour = modloader::OpenFileDetour<0x5A7B54, clothes_traits::dtraits>;

static auto xinit = initializer([](DataPlugin* plugin_ptr)
{
    auto ReloadClothes = injector::cstd<void()>::call<0x5A7B30>;
    plugin_ptr->AddDetour("clothes.dat", reinstall_since_load, OpenClothesDetour(), gdir_refresh(ReloadClothes));
});

