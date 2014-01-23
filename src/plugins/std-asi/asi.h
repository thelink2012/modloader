/*
 * Copyright (C) 2013  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#ifndef ASI_H
#define	ASI_H

#include <windows.h>    // for HMODULE
#include <string>
#include <list>
#include <vector>
#include <memory>       // for unique_ptr
#include <modloader.hpp>
#include <modloader_util_path.hpp>
using namespace modloader;

// Forward path_translator_base from args_translator.h
struct path_translator_base;

//#include <cstdio>
//#define printf asiPlugin->Log
//#define puts asiPlugin->Log


/*
 *  The plugin object
 */
extern class CThePlugin* asiPlugin;
class CThePlugin : public modloader::CPlugin
{
    public:
        static const int default_priority = 45;
        
        //
        const char* GetName();
        const char* GetAuthor();
        const char* GetVersion();
        bool OnStartup();
        bool OnShutdown();
        bool CheckFile(modloader::ModLoaderFile& file);
        bool ProcessFile(const modloader::ModLoaderFile& file);
        bool PosProcess();
        const char** GetExtensionTable();
        
        //
        struct ModuleInfo
        {
            bool bIsASI;                // Is this a ASI module?
            bool bIsD3D9;               // Is this a D3D9.dll module?
            bool bIsMainExecutable;     // Is this the main executable? (gta_sa.exe etc)
            
            struct      // Hacks that some ASIs will need to work properly
            {
                bool bRyosukeModuleName;
                
            } hacks;
            
            
            std::string name;       // The asi filename, like "bbb.asi"
            std::string path;       // The asi path, like "modloader/aaa/bbb.asi"
            std::string folder;     // The folder where the asi is at, like "modloader/aaa/"
            std::vector<std::unique_ptr<path_translator_base>> translators;

            HMODULE module;         // The module handle

            ModuleInfo(std::string&& path);
            bool Load();
            void Free();
            
            void PatchImports();
            void RestoreImports();
            path_translator_base* FindTranslatorFrom(const char* symbol, const char* libname);
        };
        
        // List of asi fileswe need to load (or loaded)
        std::list<ModuleInfo> asiList;  // It's called asiList but it's not limited to .asi files!
        
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

};

#endif	/* ASI_H */

