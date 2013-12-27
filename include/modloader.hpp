/* 
 * San Andreas modloader
 * Copyright (C) 2013  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 *  Modloader plugin interface for C++
 *      Just a wrapper around the C interface, please check it out at modloader.h!!!
 * 
 *  Take a look at "doc/Creating Your Own Plugin.txt"
 * 
 */

#ifndef MODLOADER_HPP
#define	MODLOADER_HPP
#pragma once

#include <modloader.h>

namespace modloader
{
    typedef modloader_file_t   ModLoaderFile;
    typedef modloader_plugin_t ModLoaderPlugin;
    
    // Base class for the plugin interface
    class CPlugin
    {
        private:
            friend struct CPluginCallbacks;
            
            int nChunks;
            int nChunksLoaded;
            
            // This is called after the OnLoad to make sure nChunksLoaded == nChunks
            void MakeSureChunksHaveBeenLoaded()
            {
                while(this->nChunksLoaded < this->nChunks)
                {
                    this->NewChunkLoaded();
                }
            }

        public:
                // The loader data
                modloader_plugin_data*      data;
                modloader_data*             modloader;
                modloader_fLog              Log;
                modloader_fError            Error;
                
                // Call this to set the number of chunks this plugin got
                void SetChunks(int num)
                {
                    nChunks = num;
                }
                
                // Call this to increase the number of chunks this plugin got
                void AddChunks(int increaseBy = 1)
                {
                    nChunks += increaseBy;
                }
                
                // Safe wrapper around modloader->NewChunkLoaded
                void NewChunkLoaded()
                {
                    if(nChunksLoaded < nChunks)
                    {
                        ++nChunksLoaded;
                        modloader->NewChunkLoaded();
                    }
                }
                
                // Checkout modloader.h or "doc/Creating Your Own Plugin.txt" for details on those callbacks
                virtual const char* GetName()=0;
                virtual const char* GetAuthor()=0;
                virtual const char* GetVersion()=0;
                virtual bool OnStartup() { return true; }               /* default */
                virtual bool OnShutdown() { return true; }              /* default */
                virtual bool CheckFile(ModLoaderFile& file)=0;
                virtual bool ProcessFile(const ModLoaderFile& file)=0;
                virtual bool PosProcess() { return true; }              /* default */
                virtual bool OnLoad(bool isLoadBar) { return true; }   /* default */
                
                /* Returns the favorable file extensions for this plugin */
                virtual const char** GetExtensionTable() { return nullptr; }
                
                CPlugin() : nChunks(0), nChunksLoaded(0)
                {}
    };
    
    // Callbacks wrapper
    struct CPluginCallbacks
    {
        static CPlugin* GetThis(modloader_plugin_t* data)
        {
            return (CPlugin*)(data->pThis);
        }
        
        static const char* GetName(modloader_plugin_t* data)
        {
            return GetThis(data)->GetName();
        }
        
        static const char* GetAuthor(modloader_plugin_t* data)
        {
            return GetThis(data)->GetAuthor();
        }
        
        static const char* GetVersion(modloader_plugin_t* data)
        {
            return GetThis(data)->GetVersion();
        }
        
        static int OnStartup(modloader_plugin_t* data)
        {
            return !GetThis(data)->OnStartup();
        }
        
        static int OnShutdown(modloader_plugin_t* data)
        {
            return !GetThis(data)->OnShutdown();
        }
        
        static int CheckFile(modloader_plugin_t* data, modloader_file_t* file)
        {
            return !GetThis(data)->CheckFile(*file);
        }  
        
        static int ProcessFile(modloader_plugin_t* data, const modloader_file_t* file)
        {
            return !GetThis(data)->ProcessFile(*file);
        }
        
        static int PosProcess(modloader_plugin_t* data)
        {
            int result = !GetThis(data)->PosProcess();
            data->loadbar_chunks = GetThis(data)->nChunks;
            return result;
        }
        
        static int OnLoad(modloader_plugin_t* data, int isLoadBar)
        {
            int result = !GetThis(data)->OnLoad(isLoadBar);
            if(isLoadBar)
            {
                GetThis(data)->MakeSureChunksHaveBeenLoaded();
            }
            return result;
        }
    };

    // Attaches the 'interface' with the plugin 'data'
    // This is intended to be called at GetPluginData export
    inline void RegisterPluginData(CPlugin& interfc, modloader_plugin_t* data, int priority = -1)
    {
        // Register version this plugin was built in
        data->major = MODLOADER_VERSION_MAJOR;
        data->minor = MODLOADER_VERSION_MINOR;
        data->revision = MODLOADER_VERSION_REVISION;
        
        // Register interface this pointer
        data->pThis = &interfc;

        // Callbacks
        data->GetName = &CPluginCallbacks::GetName;
        data->GetVersion = &CPluginCallbacks::GetVersion;
        data->GetAuthor = &CPluginCallbacks::GetAuthor;
        data->OnStartup = &CPluginCallbacks::OnStartup;
        data->OnShutdown = &CPluginCallbacks::OnShutdown;
        data->CheckFile = &CPluginCallbacks::CheckFile;
        data->ProcessFile = &CPluginCallbacks::ProcessFile;
        data->PosProcess = &CPluginCallbacks::PosProcess;
        data->OnLoad = &CPluginCallbacks::OnLoad;
        
        // Custom priority
        if(priority != -1) data->priority = priority;
        
        // Get Extension Table
        if(data->extable = interfc.GetExtensionTable())
        {
            for(const char** extable = data->extable; *extable; ++extable)
                ++data->extable_len;
        }
        else
        {
            data->extable_len = 0;
        }
        
        // Modloader
        interfc.data      = data;
        interfc.modloader = data->modloader;
        interfc.Error     = interfc.modloader->Error;
        interfc.Log       = interfc.modloader->Log;
    }
    
};


#endif	/* MODLOADER_HPP */

