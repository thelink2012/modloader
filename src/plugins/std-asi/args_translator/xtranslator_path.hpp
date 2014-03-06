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
inline bool  path_translator_base::CallInfo::CxBuildPath(T* p, const M& module, const char* currdir, const T*& arg, F build_path, bool bForce)
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
    // In case of the need of an full path, do it on the prefix
    else if(this->bAbsolutePath)
    {
        sprintf(tmp, "%s%s", asiPlugin->modloader->gamepath, prefix);
        prefix = tmp;
    }
    
    // Build the path
    if(auto* path = build_path(p, prefix, currdir, arg))
    {
        // Check if path "prefix + currdir + arg" exists, if yes, signalyze it
        if(bForce || IsPath(path))
        {
            arg = path;
            return true;
        }

    }
    
    return false;
}


// Helper for proper GetCurrentDir
template<class T>
inline bool  path_translator_base::CallInfo::GetCurrentDir(const T*& arg, char type, const char*& currdir, char buffer[MAX_PATH])
{
    bool bCheckModules = false;

    if(this->bAbsolutePath) // This argument is an absolute path?
    {
        // Get current directory assuming argument is the fullpath
        currdir = GetCurrentDir( (const char*) arg, asiPlugin->modloader->gamepath, -1);

        // If could get the currdir, set up some stuff and go ahead on the translation
        // For SetDir, if currdir is empty, that means it should chdir into base dir, so don't touch it
        if(currdir && (!bSetDir || currdir[0] != 0))
        {
            arg = (T*) currdir;
            currdir = 0;
            bCheckModules = true;
        }
    }
    else
    {
        // Get current directory relative to the game path
        currdir = GetCurrentDir(buffer, MAX_PATH);
        bCheckModules = currdir != 0;
    }
    return bCheckModules;
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
        //
        bool bCheckModules = GetCurrentDir(arg, type, currdir, buffer);

        // Check if currdir is alright
        if(bCheckModules)
        {
            bool bSet = false;
            T* p = AllocPath(path_size);

            // Try to translate for ASI files
            auto CheckASI = [&]()
            {
                // Iterate on each asi module trying to find a file in it's path
                if(!bSet)
                {
                    for(auto& module : asiPlugin->asiList)
                    {
                        if(!module.bIsMainExecutable && !module.bIsMainCleo)
                        {
                            if(bSet = CxBuildPath(p, module, currdir, arg, build_path, false))
                                break;
                        }
                    }
                }
            };
            
            // Try to translate for cleo scripts
            auto CheckCleoScripts = [&]()
            {
                // Iterate on each cleo script trying to find a file in it's path
                if(!bSet)
                {
                    for(auto& script : asiPlugin->csList)
                    {
                        if(bSet = CxBuildPath(p, script, currdir, arg, build_path, false))
                            break;
                    }
                }
            };
            
            // Run the tries by priority based on what is the caller
            if(asi->bIsCleo)
            {
                // Cleo scripts first
                CheckCleoScripts();
                CheckASI();
            }
            else
            {
                // ASI first
                CheckASI();
                CheckCleoScripts();
            }
        }
    }
}

template<class T, class F>      // Used when the caller module is any other asi
inline void path_translator_base::CallInfo::TranslatePathForASI(const T*& arg, char type, F build_path)
{
    const int path_size = MAX_PATH * sizeof(T);
    char buffer[MAX_PATH];
    const char* currdir = 0;
    bool bDoTranslation = false;
    
    //
    // Translate path for ASI files in the following manner "$ASI_PATH/$CWD/$ARGUMENT"
    //
    
    // If absolute path only continue the translation if it doesn't exist
    if(this->bAbsolutePath)
    {
        if(type == AR_PATH_INE && !IsPath(arg))
            bDoTranslation = true;
    }
    else
        bDoTranslation = true;
    
    //----
    if(bDoTranslation)
    {
        // Check if working directory is near the game base directory and then get that working directory
        if(GetCurrentDir(arg, type, currdir, buffer))
        {
            // Build the hacked path
            CxBuildPath((T*)(AllocPath(path_size)), *asi, currdir, arg, build_path, true);
        }
    }
    
}

#endif