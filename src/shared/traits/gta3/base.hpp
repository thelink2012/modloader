/* 
 * Copyright (C) 2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under the MIT License, see LICENSE at top level directory.
 * 
 */
#pragma once
#include <modloader/util/injector.hpp>
#include <CPool.h>
#include <cstdint>

struct TraitsGTA
{
    using id_t = uint32_t;      // must allow checking for -1

    template<class T>
    static T& ReadOffset(void* ptr, size_t offset)
    {
        return *(injector::raw_ptr(ptr) + offset).get<T>();
    }

    static void DoesNotExistInThisGame /*[[noreturn]]*/()
    {
        *(int*)(0xFFEA2012) = 0;
    }
};
