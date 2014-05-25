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

#define INJECTOR_GVM_PLUGIN_NAME        "Mod Loader Plugin"
#define INJECTOR_GVM_HAS_TRANSLATOR

#include <injector/injector.hpp>
#include <injector/hooking.hpp>

namespace modloader
{
    using namespace injector;
}

#endif

