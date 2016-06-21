/*
 * Copyright (C) 2016  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under the MIT License, see LICENSE at top level directory.
 * 
 */
#include <stdinc.hpp>
#include "../data_traits.hpp"
using namespace modloader;

#if 0

struct spath_traits
{
    struct dtraits : /* modloader::dtraits::OpenFile */
    {
        static const char* what() { return "script path"; }
    };
};

template<uintptr_t addr>
using OpenSPathDetour = modloader::OpenFileDetour<addr, spath_traits::dtraits>;

#endif

static auto xinit = initializer([](DataPlugin* plugin_ptr)
{
    if(gvm.IsVC())
    {
        // stub, not sure how to implement, see VC 0x54DC89 (TODO)
    }
});
