/*
 * Copyright (C) 2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under the MIT License, see LICENSE at top level directory.
 * 
 */
#pragma once
#include <modloader/util/hash.hpp>

//
//  NOTE: This file SHOULD NOT be included from a precompiled header or similar!!!!!!
//        It should be included directly by each data_traits/*.cpp file so it captures it's compilation time!
//

/*
 *  !!!! data_traits important implementation information !!!!
 *
 *   [*] It's better that each data trait (i.e. for each file type) be contained in it's own cpp, for compilation time and datalib caching reasons.

 *   [*] Remember the order of stuff in the either<> object matters, it'll try to match the first type, then the second, and so on.
 *       So be warned because e.g. either<string, float> will always match the string, use either<float, string> instead!
 *
 *   [*] Remember not to use int8, uint8 and so in the data_slice<> thinking it is a integer type, instead it will be readen as a character
 */










// Whenever a serialized data format changes, this identifier should change so the serialized file gets incompatible
// So, let's use the compilation time of each compilation unit to represent this identifier
// NOTICE, this function should be static so it gets one instantiation on each translation unit!!!
static /* <--- YES STATIC */ uint32_t build_identifier()
{
    static const uint32_t version = modloader::hash(__DATE__ " " __TIME__);
    return version;
}
