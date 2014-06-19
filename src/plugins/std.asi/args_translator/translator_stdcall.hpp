/*
 * Copyright (C) 2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 * Arguments Translation System 
 *      for stdcall functions
 * 
 */

#ifndef ARGS_TRANSLATOR_STDCALL_HPP
#define	ARGS_TRANSLATOR_STDCALL_HPP

#include "translator_basic.hpp"

#include "hacks/FindCleoScripts.hpp"


/*
 *  Path translator for stdcall functions (WINAPI functions)
 */

template<const char* Symbol, const char* LibName, class Prototype>
struct path_translator_stdcall;

template<const char* Symbol, const char* LibName, class Ret, class... Args>
struct path_translator_stdcall<Symbol, LibName, Ret(Args...)> : public path_translator_basic<Symbol, LibName>
{
    typedef path_translator_basic<Symbol, LibName> super;
    typedef CThePlugin::ModuleInfo ModuleInfo;
    typedef Ret(__stdcall *func_type)(Args...);
    static const int num_args = sizeof...(Args);        // number of Args

    /*
     *  Virtual methods 
     */
    path_translator_base::smart_ptr clone()
    { return path_translator_base::DoClone<path_translator_stdcall>(this); }
    const void* GetWrapper() { return (void*)(&call); }

    // Constructs the object, pass the argument types (arg 0 is return type)
    template<class... ArgsType>  // All args should be 'char'
    path_translator_stdcall(ArgsType... t)
    {
        static_assert(sizeof...(t) == 1 + num_args, "Invalid num arguments on constructor");
        path_translator_base::RegisterPathType(t...);     // Register types
    }
    
    
    // The wrapper function that translates the path
    static Ret __stdcall call(Args... a)
    {
        typename super::CallInfo info;
        void* pReturn;
        Ret result;
        bool bDetoured = false;

        // Get pointer to a address in the caller module...
        pReturn = GetReturnAddress();

        // Find the ASI information from the caller return pointer...
        if(info.FindInfo(pReturn))
        {
            // Check if it was called from CLEO.asi
            if(info.asi && info.asi->bIsMainCleo)
            {
                // If walking on files execute another path
                if(info.base->bFindFirstFile || info.base->bFindNextFile || info.base->bFindClose)
                {
                    // Find all scripts in modloader
                    bDetoured = hacks::FindCleoScripts::Finder<Symbol, Ret>(result, a...);
                }
            }
            
            if(!bDetoured)
                info.TranslateForCall(a...); // Translate the paths
        }
        
        if(!bDetoured)
        {
            // Call the original function
            auto f = (func_type) info.base->fun;
            result = f(a...);
        }
        
        return result;
    }

};

#endif
