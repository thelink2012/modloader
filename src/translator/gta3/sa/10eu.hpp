/*
 * Mod Loader - Address Translation Between Game Versions
 * Copyright (C) 2013-2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 *
 */
#pragma once
#include <modloader/util/injector.hpp>
#include <algorithm>
#include <map>
#include "10us.hpp"

// GTA SA 10EU table
static void sa_10eu(std::map<memory_pointer_raw, memory_pointer_raw>& map)
{
    sa_10us(map);
    for(auto it = ++map.begin(), end = --map.end(); it != end;) // first and last are dummy addresses (0x0 and 0xff...)
    {
        uintptr_t addr = it->first.as_int();

        if(it->second >= 0x1556000) // something translated to the [.HOODLUM|.securom] segment
        {
            //
            // Anything on the securom segment must be translated manually, obfuscation is different between the executables.
            //
            addr = 0;
            if(it->second == 0x0156C2FB)        // and     esi, 0FFFFFFh    ; @CdStreamRead
                addr = 0x0156C2DB;
            else if(it->second == 0x0156644F)   // call    _ZN10CStreaming12RequestModelEii ; @CStreaming::RequestFile
                addr = 0x0156643F;
        }
        else if(addr >= 0x746720     // previous function (localization related btw) changes the next offsets
             && addr <  0x857000)    // new segment (_rwcseg), offsets realigned
        {
            if(addr >= 0x7BA940)    // the function before this one had a different alignment in 10EU
                addr += 0x40;
            else
                addr += 0x50;
        }

        // Replace address with 10EU counterpart
        if(addr != 0)
        {
            it->second = addr;
            ++it;
        }
        else
        {
            it = map.erase(it);
            assert(0 && "sa_10eu, addr==nullptr");
        }
    }
}
