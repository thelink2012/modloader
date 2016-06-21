/*
 * Copyright (C) 2016  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under the MIT License, see LICENSE at top level directory.
 * 
 */
#include <stdinc.hpp>
#include "../data_traits.hpp"
using namespace modloader;

struct chase_traits
{
    struct dtraits : modloader::dtraits::OpenFile
    {
        static const char* what() { return "chase nodes"; }
    };
};

template<uintptr_t addr>
using OpenChaseDetour = modloader::OpenFileDetour<addr, chase_traits::dtraits>;

static auto xinit = initializer([](DataPlugin* plugin_ptr)
{
    if(gvm.IsIII())
    {
        // stub, not sure how to implement, see III 0x43571C (TODO)
    }
});
