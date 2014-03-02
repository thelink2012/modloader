/*
 * Copyright (C) 2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 * Basic information for the translation system
 * 
 */

#ifndef ARGS_TRANSLATOR_XTRANSLATOR_HPP
#define	ARGS_TRANSLATOR_XTRANSLATOR_HPP

// Some symbols need forwarding because we're going to check for them
extern const char aKernel32[];
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
};


#endif
