/*
 * Copyright (C) 2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under the MIT License, see LICENSE at top level directory.
 * 
 * Basic information for the translation system
 * 
 */

#ifndef ARGS_TRANSLATOR_XTRANSLATOR_HPP
#define	ARGS_TRANSLATOR_XTRANSLATOR_HPP

// Some symbols need forwarding because we're going to check for them
extern const char aBass[];
extern const char aKernel32[];
extern const char aCreateFileA[];
extern const char aSetCurrentDirectoryA[];
extern const char aGetModuleFileNameA[];
extern const char aFindFirstFileA[];
extern const char aFindNextFileA[];
extern const char aFindClose[];

// Argument type
enum eArgsType
{
    AR_DUMMY        = 0,           // Don't touch argument
    AR_PATH_INE,                   // Input path for existing file
    AR_PATH_IN,                    // Input path
    AR_PATH_INEB,                  // Input path for existing folder before the file (e.g. "AA/BB/CC" folder "AA/BB" must exist)
};

// Override INEB, we won't handle it (for now at least)
// Implemeting that wouldn't work very well with FindFirstFileA because it returns a non-relative path on the searched path, grr
#define AR_PATH_INEB    AR_DUMMY


#if defined(_MSC_VER)
#   define GetReturnAddress()      _ReturnAddress()
#else
#   define GetReturnAddress()      __builtin_return_address(0)
#endif



#endif
