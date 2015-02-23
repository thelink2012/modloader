/*
 * Copyright (C) 2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under the MIT License, see LICENSE at top level directory.
 * 
 * Arguments Translation System
 *      Hack to give ryosuke plugins a path to itself on GetModuleFileName(NULL) instead of path to gta_sa.exe
 *      It is necessary because Ryosuke normally open files using "<path to gta_sa.exe> + <filename>"
 *      Works only on some of his plugins, when he decides to not open the files directly on DllMain.
 * 
 */

#ifndef ARGS_TRANSLATOR_HACKS_RYOSUKESETMODULE_HPP
#define	ARGS_TRANSLATOR_HACKS_RYOSUKESETMODULE_HPP

#include <windows.h>
#include "../xtranslator.hpp"

namespace hacks
{
    template<class X>       // Dummy for when function first parameter isn't HMODULE
    inline void RyosukeSetModule(X*,HMODULE)  
    { }
    
    template<>              // Actual hacking entity
    inline void RyosukeSetModule(HMODULE* x, HMODULE asi)
    {
        if(*x == NULL) *x = asi;   // If GetModuleFileName module param is NULL, use asi module instead
    }
};

#endif
