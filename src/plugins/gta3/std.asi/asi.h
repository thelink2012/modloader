/*
 * Copyright (C) 2013-2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under the MIT License, see LICENSE at top level directory.
 * 
 */
#ifndef ASI_H
#define	ASI_H

#include <windows.h>    // for HMODULE
#include <string>
#include <list>
#include <vector>
#include <modloader/modloader.hpp>
#include <modloader/util/path.hpp>
using namespace modloader;

// Forward path_translator_base from args_translator.h
struct path_translator_base;

// Cleo iVersion for scripts
enum
{
    CLEO_VER_NONE       = '\0',
    CLEO_VER_3          = '3',
    CLEO_VER_4          = '4',
    CLEO_VER_5          = '5',
};


/*
 *  The plugin object
 */
class ThePlugin : public modloader::basic_plugin
{
    public:
        const info& GetInfo();
        bool OnStartup();
        bool OnShutdown();
        int GetBehaviour(modloader::file&);
        bool InstallFile(const modloader::file&);
        bool ReinstallFile(const modloader::file&);
        bool UninstallFile(const modloader::file&);
        
        /*
         *  Information about asi plugins (and more)
         */
        struct ModuleInfo
        {
            bool bIsASI = false;                // Is this a ASI module?
            bool bIsD3D9 = false;               // Is this a D3D9.dll module?
            bool bIsMainExecutable = false;     // Is this the main executable? (gta_sa.exe etc)
            bool bIsCleo = false;               // Is the main CLEO.asi or any .cleo plugin
            bool bIsMainCleo = false;           // Is the main CLEO.asi
            
            struct      // Hacks that some ASIs will need to work properly
            {
                bool bRyosukeModuleName = false;
                
            } hacks;
            
            HMODULE module = nullptr; // The module handle
            std::string folder;          // Absolute path to where the asi is at, like "modloader/aaa/"
            std::string translationPath; // Absolute path to use for path translations. Will be one directory up from the above if the module is in 'scripts'
            std::vector<path_translator_base*> translators;

            

            ModuleInfo(std::string path, const modloader::file* file, HMODULE mod);
            bool Load();            // Loads the module
            void Free();            // Frees the module
            
            void PatchImports();    // Patches IAT to translate arguments such as path
            void RestoreImports();  // Unpatches the IAT
            
            // Find translator for @symbol and @libname on translators list
            path_translator_base* FindTranslatorFrom(const char* symbol, const char* libname);

            private:
                const modloader::file*  file = nullptr; // The module file (may be null)
        };
        
        /*
         *  Information about cleo scripts 
         */
        struct CsInfo
        {
            char        iVersion = CLEO_VER_NONE;
            bool        bIsMission = false;
            
            const modloader::file* file = nullptr;
            std::string translationPath;                // e.g. "modloader/foo/". Absolute
            
            CsInfo(const modloader::file* file);
            static bool GetVersionFromExtension(const char* ext, char& version);

            static void Initialise();
        };
        
        
        
        /*
         *  Finds a ModuleInfo from an address that's supposed to be inside it 
         */
        ModuleInfo* FindModuleFromAddress(const void* addr)
        {
            // Find HMODULE by @addr
            HMODULE hModule;
            if(GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                           (char*)addr, &hModule))
            {
                // Find the ModuleInfo in our list of modules
                for(auto& asi : this->asiList)
                    if(asi.module == hModule) return &asi;
            }
            
            // Not found
            return nullptr;
        }

        
        typedef std::list<ModuleInfo>   ModuleInfoList;
        typedef std::list<CsInfo>       CsInfoList;
        
        // CLEO.ASI version
        int iCleoVersion;
        bool bHasNoCleoFolder;
        
        // List of asi files need to load (or loaded)
        ModuleInfoList asiList;  // It's called asiList but it's not limited to .asi files!
        
        // List of CLEO scripts (.cs, .cs3, .cs4, .cs5, .cm)
        CsInfoList     csList;   // It's called cs but it's not limited to .cs files (e.g. cm files works)
        
        // Find all cleo plugins already loaded and push them into asi list 
        void LocateCleo();

        
};

#endif	/* ASI_H */

