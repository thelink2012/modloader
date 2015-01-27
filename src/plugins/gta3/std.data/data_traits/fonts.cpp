/*
 * Copyright (C) 2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#include <stdinc.hpp>
#include "../data.hpp"
using namespace modloader;

struct fonts_traits
{
    // Detouring traits
    struct dtraits : modloader::dtraits::OpenFile
    {
        static const char* what() { return "font values"; }
    };
};

using OpenFontsDetour = modloader::OpenFileDetour<0x7187DB, fonts_traits::dtraits>;


// Population Cycle Properties Detour
static auto xinit = initializer([](DataPlugin* plugin_ptr)
{
    auto ReloadFonts = injector::cstd<void()>::call<0x7187C0>;
    plugin_ptr->AddDetour("fonts.dat", reinstall_since_start, OpenFontsDetour(), gdir_refresh(ReloadFonts));
});

