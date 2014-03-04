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
 *  Helper function
 *  Builds a path and checks if it exists, if yes sets @arg and returns true 
 */
template<class T, class M, class F>
inline bool  path_translator_base::CallInfo::CxBuildPath(T* p, const M& module, const char* currdir, const T*& arg, F build_path)
{
    char tmp[MAX_PATH];
    const char* prefix = module.folder.c_str();

    // We have to take care when CLEO is trying to open a new custom script, it will try to do so
    // from [chdir("CLEO")] so we need to get one level back in the directory tree
    if(asi->bIsMainCleo && base->bCreateFile && !stricmp(currdir, "CLEO"))
    {
        // Cleo is probably trying to open a new custom script
        sprintf(tmp, "..\\%s", prefix);     // Get back in the directory tree
        prefix = tmp;                       //
    }
    
    // Check if path "prefix + currdir + arg" exists, if yes, signalyze it
    if(IsPath(build_path(p, prefix, currdir, arg)))
    {
        arg = p;
        return true;
    }
    
    return false;
}



/*
 * 
 *  Really specific path translators
 * 
 */


template<class T, class F>      // Used when the caller module is CLEO.asi
inline void path_translator_base::CallInfo::TranslatePathForCleo(const T*& arg, char type, F build_path)
{
    return TranslatePathForMainExecutable(arg, type, build_path);
}

template<class T, class F>      // Used when the caller module is the main game executable
inline void path_translator_base::CallInfo::TranslatePathForMainExecutable(const T*& arg, char type, F build_path)
{
    const int path_size = MAX_PATH * sizeof(T);
    char buffer[MAX_PATH];
    const char* currdir;

    // If the argument type is a INPUT THAT EXISTS, check if it exists on base path...
    // ...if not, try on the ASI paths or cleo paths
    if(type == AR_PATH_INE && !IsPath(arg))
    {
        bool bCheckModules = false;
        
        if(this->bSetDir)   // This call is a SetCurrentDirectoryA comming from gta_sa.exe:_chdir
        {
            // Get current directory assuming argument is the fullpath
            currdir = GetCurrentDir( (const char*) arg, asiPlugin->modloader->gamepath, -1);
            
            // If could get the currdir, set up some stuff and go ahead on the translation
            // If currdir is empty, that means it should chdir into base dir, so don't touch it
            if(currdir && currdir[0] != 0)
            {
                arg = (T*) currdir;
                currdir = 0;
                bCheckModules = true;
            }
        }
        else
        {
            // Get current directory relative to the game path
            currdir = GetCurrentDir(buffer, sizeof(buffer));
            bCheckModules = currdir != 0;
        }
        
        
        // Check if currdir is alright
        if(bCheckModules)
        {
            bool bSet = false;
            T* p = AllocPath(path_size);

            // Iterate on each asi module trying to find a file in it's path
            if(!bSet)
            {
                for(auto& module : asiPlugin->asiList)
                {
                    if(!module.bIsMainExecutable && !module.bIsMainCleo)
                    {
                        if(bSet = CxBuildPath(p, module, currdir, arg, build_path))
                            break;
                    }
                }
            }
            
            // Iterate on each cleo script trying to find a file in it's path
            if(!bSet)
            {
                for(auto& script : asiPlugin->csList)
                {
                    if(bSet = CxBuildPath(p, script, currdir, arg, build_path))
                        break;
                }
            }
        }
    }
}

template<class T, class F>      // Used when the caller module is any other asi
inline void path_translator_base::CallInfo::TranslatePathForASI(const T*& arg, char type, F build_path)
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