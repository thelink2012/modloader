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
#ifndef MODLOADER_GTA3_UTIL_HPP
#define	MODLOADER_GTA3_UTIL_HPP
#pragma once

#include <modloader/util/detour.hpp>

namespace modloader
{
    /*
        Detour helpers
    */
    namespace dtraits
    {
        template<int Arg>
        struct base
        {
            static const char* what() { return ""; };
            static const int arg = Arg;
        };

        using LoadTxd = base<2>;
        using OpenFile = base<1>;
        using LoadFile = base<1>;
        using SaOpenOr3VcLoadFileDetour = base<1>;
        using RwStreamOpen = base<3>;
        using CreateVideoPlayer = base<2>;
        using ReadAndInterpretTrackFile = base<1>;
        using WinCreateFileA = base<1>;
        using LoadAtomic2Return = base<1>;
    };

    template<uintptr_t addr, class Traits = dtraits::LoadTxd>
    using LoadTxdDetour = modloader::basic_file_detour<Traits,
                                            injector::function_hooker<addr, char(int, const char*)>,
                                                                            char, int, const char*>;

    template<uintptr_t addr, class Traits = dtraits::OpenFile>
    using OpenFileDetour = modloader::basic_file_detour<Traits,
                                            injector::function_hooker<addr, void*(const char*, const char*)>,
                                                                            void*, const char*, const char*>;

    template<uintptr_t addr, class Traits = dtraits::LoadFile>
    using LoadFileDetour = modloader::basic_file_detour<Traits,
                                            injector::function_hooker<addr, int(const char*, void*, int, const char*)>,
                                                                            int, const char*, void*, int, const char*>;

    template<uintptr_t addr, class Traits = dtraits::SaOpenOr3VcLoadFileDetour>
    using SaOpenOr3VcLoadFileDetour = LoadFileDetour<addr, Traits>;

    template<uintptr_t addr, class Traits = dtraits::RwStreamOpen>
    using RwStreamOpenDetour = modloader::basic_file_detour<Traits,
                                            injector::function_hooker<addr, void*(int, int, const char*)>,
                                                                            void*, int, int, const char*>;

    template<uintptr_t addr, class Traits = dtraits::CreateVideoPlayer>
    using CreateVideoPlayerDetour = modloader::basic_file_detour<Traits,
                                            injector::function_hooker<addr, void*(int, const char*)>,
                                                                            void*, int, const char*>; 

    template<uintptr_t addr, class Traits = dtraits::ReadAndInterpretTrackFile>
    using ReadAndInterpretTrackFileDetour = modloader::basic_file_detour<Traits,
                                                injector::function_hooker<addr, int(const char*, void**, int*, float*, /* III only -> */ int, int, int, int, int)>,
                                                                                int, const char*, void**, int*, float*,/* III only -> */ int, int, int, int, int>;

    template<uintptr_t addr, class Traits = dtraits::WinCreateFileA>
    using WinCreateFileA = modloader::basic_file_detour<Traits,
                                                injector::function_hooker_stdcall<addr, HANDLE(LPCTSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE)>,
                                                                                        HANDLE, LPCTSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE>;


    template<uintptr_t addr, class Traits = dtraits::LoadAtomic2Return>
    using LoadAtomic2ReturnDetour = modloader::basic_file_detour<Traits,
                                                injector::function_hooker<addr, void*(const char*)>,
                                                                                void*, const char*>; 

    // function prototype is the same, too lazy to copy and paste more stuff :)
    template<uintptr_t addr>
    using Gta3LoadIfpDetour = LoadAtomic2ReturnDetour<addr>;
}


#endif
