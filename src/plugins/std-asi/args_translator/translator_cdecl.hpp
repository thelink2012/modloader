/*
 * Copyright (C) 2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 * Arguments Translation System 
 *      for cdecl functions
 * 
 */

#ifndef ARGS_TRANSLATOR_CDECL_HPP
#define	ARGS_TRANSLATOR_CDECL_HPP

#include "translator_basic.hpp"

/*
 *  Path translator for cdecl functions (CSTD functions)
 */

template<const char* Symbol, const char* LibName, class Prototype>
struct path_translator_cdecl;

template<const char* Symbol, const char* LibName, class Ret, class... Args>
struct path_translator_cdecl<Symbol, LibName, Ret(Args...)> : public path_translator_basic<Symbol, LibName>
{
    typedef path_translator_basic<Symbol, LibName> super;
    typedef CThePlugin::ModuleInfo ModuleInfo;
    typedef Ret(__cdecl *func_type)(Args...);
    static const int num_args = sizeof...(Args);        // number of Args

    /*
     *  Virtual methods 
     */
    path_translator_base::smart_ptr clone()
    { return path_translator_base::DoClone<path_translator_cdecl>(this); }
    const void* GetWrapper() { return (void*)(&call); }

    // Constructs the object, pass the argument types (arg 0 is return type)
    template<class... ArgsType>  // All args should be 'char'
    path_translator_cdecl(ArgsType... t)
    {
        static_assert(sizeof...(t) == 1 + num_args, "Invalid num arguments on constructor");
        path_translator_base::RegisterPathType(t...);     // Register types
    }
    
    
    // The wrapper function that translates the path
    static Ret __cdecl call(Args... a)
    {
        typename super::CallInfo info;
        void* pReturn;

        // Get pointer to a address in the caller module...
        pReturn = GetReturnAddress();
        
        // Find the ASI information from the caller return pointer...
        if(info.FindInfo(pReturn)) info.TranslateForCall(a...);    // Translate the paths
        
        // Call the original function
        auto f = (func_type) info.base->fun;
        Ret result = f(a...);
        
        return result;
    }

};

#endif

