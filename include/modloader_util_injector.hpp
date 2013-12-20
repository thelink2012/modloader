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
    template<size_t addr>
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
    
}

#endif

