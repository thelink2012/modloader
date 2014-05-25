/* 
 * San Andreas modloader
 * Created by LINK/2012 <dma_2012@hotmail.com>
 * Part of modloader
 * 
 *  Modloader plugin interface for C++
 *      Just a wrapper around the C interface, please check it out at modloader.h!!!
 * 
 *  Take a look at "doc/Creating Your Own Plugin.txt"
 * 
 * 
 *  This source code is offered for use in the public domain. You may
 *  use, modify or distribute it freely.
 *
 *  This code is distributed in the hope that it will be useful but
 *  WITHOUT ANY WARRANTY. ALL WARRANTIES, EXPRESS OR IMPLIED ARE HEREBY
 *  DISCLAIMED. This includes but is not limited to warranties of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE
 * 
 */

#ifndef MODLOADER_HPP
#define	MODLOADER_HPP
#pragma once
#include <modloader.h>
#include <cstdio>

namespace modloader
{
    struct ModLoaderFile : modloader_file_t
    {
        bool IsDirectory() const      { return (flags & MODLOADER_FF_IS_DIRECTORY) != 0; }
        
        uint32_t UniqueId() const     { return file_id; }
        uint32_t ModId() const        { return mod_id; }
        
        uint64_t Size() const         { return size; }
        uint64_t Time() const         { return time; }

        const char* FilePath() const  { return FileBuffer(pos_filepath);  }
        const char* FileName() const  { return FileBuffer(pos_filename);  }
        const char* FileExt() const   { return FileBuffer(pos_filext);    }
        
        const char* FileBuffer(size_t idx = 0) const { return &buffer[idx];      }
        size_t FileBufferLength() const              { return (size_t)(pos_eos); }
        
        // Has file changed in comparation with c
        // Checks only for file size and file time
        bool HasFileChanged(const ModLoaderFile& c) const
        {
            return !(Time() == c.Time() && Size() == c.Size());
        }
    };
    
    struct ModLoaderPlugin : modloader_plugin_t
    {
    };
    
    static_assert(sizeof(ModLoaderFile) == sizeof(modloader_file_t), "Invalid ModLoaderFile inheritance size");
    static_assert(sizeof(ModLoaderPlugin) == sizeof(modloader_plugin_t), "Invalid ModLoaderPlugin inheritance size");
    
#if 0
    // Base class for the plugin interface
    class CPlugin
    {
        private:
            friend struct CPluginCallbacks;

        public:
            // The loader data
            modloader_plugin_data*      data;
            modloader_data*             modloader;
            modloader_fLog              Log;
            modloader_fvLog             vLog;
            modloader_fError            Error;
                
            // Checkout modloader.h or "doc/Creating Your Own Plugin.txt" for details on those callbacks
            virtual const char* GetName()=0;
            virtual const char* GetAuthor()=0;
            virtual const char* GetVersion()=0;
            virtual bool OnStartup()    { return true; }
            virtual bool OnShutdown()   { return true; }
            virtual bool CheckFile(ModLoaderFile& file)=0;
            virtual bool ProcessFile(const ModLoaderFile& file)=0;
            virtual bool PosProcess()   { return true; }
            virtual bool OnSplash()     { return true; }
            virtual bool OnLoad()       { return true; }
            virtual bool OnReload()     { return true; }
            
            // TODO new interface
                
            // Returns the favorable file extensions for this plugin
            virtual const char** GetExtensionTable() { return nullptr; }
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
            return result;
        }
        
        static int OnSplash(modloader_plugin_t* data)
        {
            return !GetThis(data)->OnSplash();
        }
        
        static int OnLoad(modloader_plugin_t* data)
        {
            int result = !GetThis(data)->OnLoad();
            return result;
        }
        
        static int OnReload(modloader_plugin_t* data)
        {
            return !GetThis(data)->OnReload();
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
        data->OnSplash = &CPluginCallbacks::OnSplash;
        data->OnLoad = &CPluginCallbacks::OnLoad;
        data->OnReload = &CPluginCallbacks::OnReload;
        
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
        interfc.vLog      = interfc.modloader->vLog;
    }
#endif
    
};


#endif	/* MODLOADER_HPP */

