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
#ifndef MODLOADER_UTIL_INJECTOR_SUPL_HPP
#define	MODLOADER_UTIL_INJECTOR_SUPL_HPP

#include <cstdio>
#include "modloader_util_file.hpp"

/*
 *  Injector.h suplement... So, Injector.h must be included before this file
 */

namespace modloader
{
    /* 
     *  fopen fixer for GTA San Andreas
     *      This hook hooks a call to fopen (or CFileMgr__OpenFile, whatever)
     *     in San Andreas to open the file always, that's, if couldn't open the request file, open null device.
     */
    template<uintptr_t addr>
    struct OpenFixer
    {
        typedef void* (*fopen_func)(const char*, const char*);
        
        static fopen_func& fopen()
        {
            static fopen_func x;
            return x;
        }

        static void* OpenAlways(const char* filename, const char* mode)
        {
            if(void* f = fopen()(filename, mode)) return f;
            return fopen()(szNullFile, mode);
        }
    
        OpenFixer()
        {
            fopen() = MakeCALL(addr, (void*) OpenAlways).get();
        }
    };
 
    template<uintptr_t addr, class Prototype>
    struct function_hooker;

    template<uintptr_t addr, class Ret, class ...Args>
    struct function_hooker<addr, Ret(Args...)>
    {
        public:
            typedef Ret(*func_type)(Args...);
            typedef Ret(*hook_type)(func_type, Args&...);

        protected:
            static func_type& original()
            {
                static func_type f;
                return f;
            }

            static hook_type& hook()
            {
                static hook_type h;
                return h;
            }

            static Ret call(Args... a)
            {
                return hook()(original(), a...);
            }

        public:
            function_hooker(hook_type hooker)
            {
                hook() = hooker;
                original() = MakeCALL(addr, (void*) call).get();
            }

    };
    
    template<class T> inline
    T make_function_hook(typename T::hook_type hooker)
    {
        return T(hooker);
    }
    
    template<uintptr_t addr, class T, class U> inline
    function_hooker<addr, T> make_function_hook(U hooker)
    {
        typedef function_hooker<addr, T> type;
        return make_function_hook<type>(hooker);
    }
    
}

#endif

