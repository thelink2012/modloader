/* 
 * Copyright (C) 2013-2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 *  Most of things game version dependent (not being addresseses since addr translator) should be handled by the game trait
 *
 */
#pragma once
#include <modloader/util/injector.hpp>
#include <CPool.h>

struct TraitsGTA
{
    template<class T>
    static T& ReadOffset(void* ptr, size_t offset)
    {
        return *(raw_ptr(ptr) + offset).get<T>();
    }
};
