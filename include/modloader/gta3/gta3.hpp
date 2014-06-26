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
#ifndef MODLOADER_GTA3_UTIL_HPP
#define	MODLOADER_GTA3_UTIL_HPP
#pragma once

#include <modloader/util/modloader.hpp>

namespace modloader
{
    /*
        Detour helpers
    */
    namespace detours
    {
        template<int Arg>
        struct base
        {
            static const char* what() { return ""; };
            static const int arg = Arg;
        };

        using LoadTxd = base<2>;
        using OpenFile = base<1>;
        using RwStreamOpen = base<3>;
    };

    template<uintptr_t addr>
    using LoadTxdDetour = modloader::basic_file_detour<detours::LoadTxd,
                                            injector::function_hooker<addr, char(int, const char*)>,
                                                                            char, int, const char*>;

    template<uintptr_t addr>
    using OpenFileDetour = modloader::basic_file_detour<detours::OpenFile,
                                            injector::function_hooker<addr, void*(const char*, const char*)>,
                                                                            void*, const char*, const char*>;

    template<uintptr_t addr>
    using RwStreamOpenDetour = modloader::basic_file_detour<detours::RwStreamOpen,
                                            injector::function_hooker<addr, void*(int, int, const char*)>,
                                                                            void*, int, int, const char*>;
}


#endif
