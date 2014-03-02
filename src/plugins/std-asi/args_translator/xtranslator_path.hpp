/*
 * Copyright (C) 2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 * Arguments Translation System 
 *      Base and basic classes
 * 
 */

#ifndef ARGS_TRANSLATOR_XTRANSLATOR_PATH_HPP
#define	ARGS_TRANSLATOR_XTRANSLATOR_PATH_HPP

//
#include "translator_basic.hpp"


/*
 * 
 *  Really specific path translators
 * 
 */


template<class T, class F>      // Used when the caller module is the main game executable
void path_translator_base::CallInfo::TranslatePathForMainExecutable(const T*& arg, char type, F build_path)
{
    const int path_size = MAX_PATH * sizeof(T);
    char buffer[MAX_PATH];

    // If the argument type is a INPUT THAT EXISTS, check if it exists on base path...
    // ...if not, try on the ASI paths.
    if(type == AR_PATH_INE && !IsPath(arg))
    {
        // Get current directory relative to the game path
        if(auto* currdir = GetCurrentDir(buffer, sizeof(buffer)))
        {
            T* p = AllocPath(path_size);

            // Iterate on each asi module trying to find a file in it's path
            for(auto& module : asiPlugin->asiList)
            {
                if(!module.bIsMainExecutable)
                {
                    if(IsPath(build_path(p, module.folder.c_str(), currdir, arg)))
                    {
                        arg = p;
                        break;
                    }
                }
            }
        }
    }
    
}

        
template<class T, class F>      // Used when the caller module is CLEO.asi
void path_translator_base::CallInfo::TranslatePathForCleo(const T*& arg, char type, F build_path)
{
    const int path_size = MAX_PATH * sizeof(T);
    char buffer[MAX_PATH];
    
    //
    // TODO
    //
    
}


template<class T, class F>      // Used when the caller module is any other asi
void path_translator_base::CallInfo::TranslatePathForASI(const T*& arg, char type, F build_path)
{
    const int path_size = MAX_PATH * sizeof(T);
    char buffer[MAX_PATH];

    //
    // Translate path for ASI files in the following manner "$ASI_PATH/$CWD/$ARGUMENT"
    //
    
    // Check if working directory is near the game base directory and then get that working directory
    if(auto* currdir = GetCurrentDir(buffer, sizeof(buffer)))
    {
        // Build the hacked path
        arg = build_path(AllocPath(path_size), asi->folder.c_str(), currdir, arg);
    }
    
}

#endif
