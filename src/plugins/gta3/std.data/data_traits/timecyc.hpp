/*
 * Copyright (C) 2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#pragma once
#include <stdinc.hpp>

struct timecyc_traits
{
    // Detouring traits
    struct dtraits : modloader::dtraits::OpenFile
    {
        static const char* what() { return "time cycle properties"; }
    };
};

using OpenTimecycDetour = modloader::OpenFileDetour<0x5BBADE, timecyc_traits::dtraits>;
