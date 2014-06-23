/* 
 * San Andreas Mod Loader - Address Translation Between Game Versions
 * Copyright (C) 2013-2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#ifndef ADDRESS_TRANSLATOR_H
#define	ADDRESS_TRANSLATOR_H

#include <modloader/util/injector.hpp>
#include <map>
using namespace injector;

// Tables
#include "gta3/sa/10us.hpp"

// Constants
static const size_t ptr_failure  = 0x0;     // Returns 0x0 in case of failure in address translation
static const size_t max_ptr_dist = 8;       // Max distance to take as a "equivalent" address for modloader
static void init();

// Addresses table
static std::map<memory_pointer_raw, memory_pointer_raw> map;


// Translate pointer from GTA SA 10US offset to this executable offset
void* injector::address_manager::translator(void* p_)
{
    return p_;//TODO
    memory_pointer_raw p = p_;
    memory_pointer_raw result = nullptr;
                
    // Initialize if hasn't initialized yet
    init();

    // Find first element in the map that is greater than or equal to p
    auto it = map.lower_bound(p);
    if(it != map.end())
    {
        // If it's not exactly the address, get back one position on the table
        if(it->first != p) --it;

        auto diff = uintptr_t(p - it->first);       // What's the difference between p and that address?
        if(diff <= max_ptr_dist)                    // Could we live with this difference in hands?
            result = it->second + raw_ptr(diff);    // Yes, we can!
    }
    
    // Print into stdout that the address translation wasn't possible
    // This should never happen! The user won't see this warning on stdout.
    if(!result)
    {
        char buf[128];
        sprintf(buf, "Could not translate address 0x%p!!\n", p.get<void>());
        MessageBoxA(0, buf, injector::game_version_manager::PluginName, 0);
        result = ptr_failure;
    }
    
    return result.get();
}

// Initializes the address translator and it's table
static void init()
{
    static bool bInitialized = false;
    if(bInitialized == false)
    {
        auto& gvm = injector::address_manager::singleton();
        bInitialized = true;
        
        // We're only working with SA addresses on here
        if(gvm.IsSA())
        {
            // Find version and initialize addresses table
            if(gvm.GetMajorVersion() == 1 && gvm.GetMinorVersion() == 0 && gvm.IsUS())
                sa_10us(map);

            // The map must have null pointers at it's bounds
            // So they work properly with lower_bound and stuff
            map.emplace(0x00000000u, 0x00000000u);
            map.emplace(0xffffffffu, 0xffffffffu);
        }
    }
}



/*
 *  ---------------> Address Tables
 */




#endif

