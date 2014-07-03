/* 
 * San Andreas Mod Loader Utilities Headers
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
#ifndef MODLOADER_UTIL_INJECTOR_ADDRT_HPP
#define	MODLOADER_UTIL_INJECTOR_ADDRT_HPP

#if _MSC_VER
#   pragma warning(disable : 4180)  // qualifier applied to function type has no meaning; ignored
#endif

//Defined on CMake now
//#define INJECTOR_GVM_PLUGIN_NAME        "Mod Loader Plugin"     // (for Mod Loader plugins, in dll)
//#define INJECTOR_GVM_HAS_TRANSLATOR

#include <modloader/modloader.hpp>
#include <injector/injector.hpp>
#include <injector/hooking.hpp>
#include <injector/calling.hpp>

namespace modloader
{
    using namespace injector;

    // Ref to the game version manager
    static modloader::address_manager& gvm = injector::address_manager::singleton();

    static const uintptr_t gta3_specific  = 0xF3000000;
    static const uintptr_t gtavc_specific = 0xF4000000;
    static const uintptr_t gtasa_specific = 0xF5000000;

    inline auto_ptr_cast try_address(memory_pointer addr)
    {
        auto Log = modloader::plugin_ptr? modloader::plugin_ptr->Log : nullptr;

        if(Log) Log("Trying to get optional address 0x%p...", addr.get_raw<void>());
        void* x = addr.get(); 
        //if(!x && Log) Log("Ignore the warning above");

        return auto_ptr_cast(x);
    }
    
}

#endif

