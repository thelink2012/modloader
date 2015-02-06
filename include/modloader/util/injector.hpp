/* 
 * Mod Loader Utilities Headers
 * Created by LINK/2012 <dma_2012@hotmail.com>
 * 
 *  This file provides helpful functions for plugins creators.
 * 
 *  This source code is offered for use in the public domain. You may
 *  use, modify or distribute it freely.
 *
 *  This code is distributed in the hope that it will be useful but
 *  WITHOUT ANY WARRANTY. ALL WARRANTIES, EXPRESS OR IMPLIED ARE HEREBY
 *  DISCLAIMED. This includes but is not limited to warranties of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */
#ifndef MODLOADER_UTIL_INJECTOR_HPP
#define	MODLOADER_UTIL_INJECTOR_HPP

#if _MSC_VER
#   pragma warning(disable : 4180)  // qualifier applied to function type has no meaning; ignored
#endif

#include <modloader/modloader.hpp>
#include <injector/injector.hpp>
#include <injector/hooking.hpp>
#include <injector/calling.hpp>
#include <injector/utility.hpp>

extern bool trying_address;


//
//  We usually use GTA SA addresses as base for stuff, but in the case any III/VC address is used as base
//  please use the xVc and x3d macros.
//

static const uintptr_t vc_addr_begin   = 0x04000000;
static const uintptr_t gta3_addr_begin = 0x06000000;

#define xVc(addr)  ((addr) + vc_addr_begin)
#define x3d(addr)  ((addr) + gta3_addr_begin)
#define xSa(addr)  ((addr)) // not necessary to use

namespace modloader
{
    using namespace injector;

    // Ref to the game version manager
    static modloader::address_manager& gvm = injector::address_manager::singleton();

    static const uintptr_t winmain_addr   = 0xBA5ECA11;

    static const uintptr_t gta3_specific  = 0xF3000000;
    static const uintptr_t gtavc_specific = 0xF4000000;
    static const uintptr_t gtasa_specific = 0xF5000000;

    inline auto_pointer try_address(memory_pointer addr)
    {
        auto Log = modloader::plugin_ptr? modloader::plugin_ptr->Log : nullptr;

        if(Log) Log("Trying to get optional address 0x%p...", addr.get_raw<void>());
        trying_address = true;
        void* x = addr.get(); 
        trying_address = false;

        return auto_pointer(x);
    }
    
}

#endif

