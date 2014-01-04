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
//#include "modloader_util_path.hpp"

/*
 *  Injector.h suplement... So, Injector.h must be included before this file
 */

namespace modloader
{
    /*
     * 
     */
    template<uintptr_t addr, class Prototype>
    struct function_hooker;

    template<uintptr_t addr, class Ret, class ...Args>
    struct function_hooker<addr, Ret(Args...)>
    {
        public:
            typedef Ret(*func_type)(Args...);
            typedef Ret(*hook_type)(func_type, Args&...);

        protected:
            
            // Stores the previous function pointer
            static func_type& original()
            { static func_type f; return f; }
            
            // Stores our hook pointer
            static hook_type& hook()
            { static hook_type h; return h; }

            // The hook caller
            static Ret call(Args... a)
            {
                return hook()(original(), a...);
            }

        public:
            // Constructs passing information to the static variables
            function_hooker(hook_type hooker)
            {
                hook() = hooker;
                original() = MakeCALL(addr, (void*) call).get();
            }
            
            // Restores the previous call before the hook happened
            static void restore()
            {
                MakeCALL(addr, (void*) original());
            }
    };
    
    
    
    template<uintptr_t addr, class Prototype>
    struct function_hooker_fastcall;

    template<uintptr_t addr, class Ret, class ...Args>
    struct function_hooker_fastcall<addr, Ret(Args...)>
    {
        public:
            typedef Ret(__fastcall *func_type)(Args...);
            typedef Ret(*hook_type)(func_type, Args&...);

        protected:
            
            // Stores the previous function pointer
            static func_type& original()
            { static func_type f; return f; }
            
            // Stores our hook pointer
            static hook_type& hook()
            { static hook_type h; return h; }

            // The hook caller
            static Ret __fastcall call(Args... a)
            {
                return hook()(original(), a...);
            }

        public:
            // Constructs passing information to the static variables
            function_hooker_fastcall(hook_type hooker)
            {
                hook() = hooker;
                original() = MakeCALL(addr, (void*) call).get();
            }
            
            // Restores the previous call before the hook happened
            static void restore()
            {
                MakeCALL(addr, (void*) original());
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

