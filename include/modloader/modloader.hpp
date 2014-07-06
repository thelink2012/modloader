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
#include <modloader/modloader.h>
#include <cstring>
#include <string>

namespace modloader
{
    struct mod  : public modloader_mod_t
    {
    };
    
    struct file : public modloader_file_t
    {
        // TODO RENAME THOSE

        bool IsDirectory() const      { return (flags & MODLOADER_FF_IS_DIRECTORY) != 0; }
        
        //uint64_t Behaviour() const    { return behaviour; }

        uint64_t Size() const         { return size; }
        uint64_t Time() const         { return time; }
        
        std::string FullPath() const;
        std::string& file::FullPath(std::string& out) const;

        const char* FilePath() const  { return FileBuffer(pos_filepath);  }
        const char* FileName() const  { return FileBuffer(pos_filename);  }
        const char* FileExt() const   { return FileBuffer(pos_filext);    }
        
        const char* FileBuffer(size_t idx = 0) const { return &buffer[idx];      }
        size_t FileBufferLength() const              { return (size_t)(pos_eos); }
        
        bool IsExtension(const char* e) const { return !_stricmp(e, FileExt()); }


        // Has file changed in comparation with c
        // Checks only for file size and file time, returns false for directories
        bool HasFileChanged(const file& c) const
        {
            return !IsDirectory() && !(Time() == c.Time() && Size() == c.Size());
        }
    };
    
    struct plugin : public modloader_plugin_t
    {
    };
    
    static_assert(sizeof(file) == sizeof(modloader_file_t), "Invalid file inheritance size");
    static_assert(sizeof(plugin) == sizeof(modloader_plugin_t), "Invalid plugin inheritance size");
    
    class basic_plugin
    {
        public:
            friend struct basic_plugin_wrapper;
            
            struct info
            {
                const char*  name;              // Useless
                const char*  version;
                const char*  author;
                int          default_priority;
                const char** extable;
            };
        
        public:
            modloader_t*                loader;
            plugin*                     data;
            modloader_fLog              Log;
            modloader_fvLog             vLog;
            modloader_fError            Error;

            template<class To>
            To& cast() { return *static_cast<To*>(this); }

        public:
            // Gets the plugin information such as name, version and author
            virtual const info& GetInfo()=0;
            
            // Checkout modloader.h or "doc/Creating Your Own Plugin.txt" for details on those callbacks
            virtual bool OnStartup()    { return true; }
            virtual bool OnShutdown()   { return true; }
            virtual int  GetBehaviour(file&)=0;
            virtual bool InstallFile(const file&)=0;
            virtual bool ReinstallFile(const file&)=0;
            virtual bool UninstallFile(const file&)=0;
            virtual void Update() {}
    };
    
    
#if 1
    // Callbacks wrapper
    struct basic_plugin_wrapper
    {
        static basic_plugin& GetThis(modloader_plugin_t* data)
        {
            return *(basic_plugin*)(data->pThis);
        }
        
        static file& GetFile(const modloader_file_t* f)
        {
            return *(const_cast<file*>(reinterpret_cast<const file*>(f)));
        }
        
        
        
        static const char* GetAuthor(modloader_plugin_t* data)
        {
            return GetThis(data).GetInfo().author;
        }
        
        static const char* GetVersion(modloader_plugin_t* data)
        {
            return GetThis(data).GetInfo().version;
        }
        
        static int OnStartup(modloader_plugin_t* data)
        {
            return !GetThis(data).OnStartup();
        }
        
        static int OnShutdown(modloader_plugin_t* data)
        {
            return !GetThis(data).OnShutdown();
        }
        
        static int GetBehaviour(modloader_plugin_t* data, modloader_file_t* file)
        {
            return GetThis(data).GetBehaviour(GetFile(file));
        }
        
        static int InstallFile(modloader_plugin_t* data, const modloader_file_t* file)
        {
            return !GetThis(data).InstallFile(GetFile(file));
        }
        
        static int ReinstallFile(modloader_plugin_t* data, const modloader_file_t* file)
        {
            return !GetThis(data).ReinstallFile(GetFile(file));
        }
        
        static int UninstallFile(modloader_plugin_t* data, const modloader_file_t* file)
        {
            return !GetThis(data).UninstallFile(GetFile(file));
        }

        static void Update(modloader_plugin_t* data)
        {
            return GetThis(data).Update();
        }
        
    };

    // Attaches the 'interface' with the plugin 'data'
    // This is intended to be called at GetPluginData export
    inline void RegisterPluginData(basic_plugin& interfc, modloader_plugin_t* data)
    {
        int priority = interfc.GetInfo().default_priority;
        
        // Register version this plugin was built in
        data->major = MODLOADER_VERSION_MAJOR;
        data->minor = MODLOADER_VERSION_MINOR;
        data->revision = MODLOADER_VERSION_REVISION;
        
        // Register interface this pointer
        data->pThis = &interfc;

        // Callbacks
        data->GetVersion    = &basic_plugin_wrapper::GetVersion;
        data->GetAuthor     = &basic_plugin_wrapper::GetAuthor;
        data->OnStartup     = &basic_plugin_wrapper::OnStartup;
        data->OnShutdown    = &basic_plugin_wrapper::OnShutdown;
        data->GetBehaviour  = &basic_plugin_wrapper::GetBehaviour;
        data->InstallFile   = &basic_plugin_wrapper::InstallFile;
        data->ReinstallFile = &basic_plugin_wrapper::ReinstallFile;
        data->UninstallFile = &basic_plugin_wrapper::UninstallFile;
        data->Update        = &basic_plugin_wrapper::Update;
        
        // Custom priority
        if(priority != -1) data->priority = priority;
        
        // Get Extension Table
        if(data->extable = interfc.GetInfo().extable)
        {
            for(const char** extable = data->extable; *extable; ++extable)
                ++data->extable_len;
        }
        else
        {
            data->extable_len = 0;
        }
        
        // Mod Loader
        interfc.data      = reinterpret_cast<plugin*>(data);
        interfc.loader    = data->modloader;
        interfc.Error     = interfc.loader->Error;
        interfc.Log       = interfc.loader->Log;
        interfc.vLog      = interfc.loader->vLog;
    }
    
    
    /*
     *  Export plugin object data
     */
    extern basic_plugin* plugin_ptr;
    extern "C" __declspec(dllexport) inline void GetPluginData(modloader_plugin_t* data)
    {
        if(plugin_ptr) modloader::RegisterPluginData(*plugin_ptr, data);
    }

    extern "C" __declspec(dllexport) inline void GetLoaderVersion(uint8_t* major, uint8_t* minor, uint8_t* revision)
    {
        *major = MODLOADER_VERSION_MAJOR;
        *minor = MODLOADER_VERSION_MINOR;
        *revision = MODLOADER_VERSION_REVISION;
    }

    inline std::string& file::FullPath(std::string& out) const
    {
        out.assign(std::string(plugin_ptr->loader->gamepath));
        out.append(this->FileBuffer());
        return out;
    }

    inline std::string file::FullPath() const
    {
        std::string out;
        this->FullPath(out);
        return out;
    }
    
#endif

#define REGISTER_ML_PLUGIN_PTR(ptr) namespace modloader { modloader::basic_plugin* plugin_ptr = ptr; }
#define REGISTER_ML_PLUGIN(plugin)  REGISTER_ML_PLUGIN_PTR(&plugin);
#define REGISTER_ML_NULL()          REGISTER_ML_PLUGIN_PTR(nullptr)

};


#endif	/* MODLOADER_HPP */

