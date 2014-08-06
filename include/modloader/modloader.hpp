/* 
 * Mod Loader Plugin Base Header
 * Created by LINK/2012 <dma_2012@hotmail.com>
 * 
 *  Mod Loader plugin interface for C++  -- Binding on the C interface
 *  Take a look at "doc/Creating Your Own Plugin.txt"
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
    /*
        modloader::plugin
            Represents a loaded mod loader plugin
    */
    struct plugin : public modloader_plugin_t
    {
    };

    /*
        modloader::mod
            Represents a mod in mod loader directory tree
    */
    struct mod  : public modloader_mod_t
    {
    };
    
    /*
        modloader::file
            Represents a handleable file in mod loader directory tree
    */
    struct file : public modloader_file_t
    {
        // Checks if this file is a directory
        bool is_dir() const                 { return (flags & MODLOADER_FF_IS_DIRECTORY) != 0; }

        // Checks if this file extension is the same as 'e'
        bool is_ext(const char* e) const    { return !_stricmp(e, filext()); }

        // Gets the full file path (including driver, etc)
        std::string fullpath() const;
        std::string& fullpath(std::string& out) const;

        // Gets the filebuffer that stores the underlying path
        const char* filebuffer(size_t idx = 0) const    { return &buffer[idx];      }
        size_t filebuffer_len() const                   { return (size_t)(pos_eos); }

        // Gets the filepath relative to the game dir
        const char* filepath() const  { return filebuffer(0);             }
        // Gets the filepath relative to the mod folder
        const char* filedir() const   { return filebuffer(pos_filedir);  }
        // Gets the filename
        const char* filename() const  { return filebuffer(pos_filename);  }
        // Gets the file extension
        const char* filext() const    { return filebuffer(pos_filext);    }
        
        // Checks if this file changed compared to 'c'
        // Checks only for file size and file time, returns false for directories
        bool has_changed(const file& c) const
        {
            return !is_dir() && !(time == c.time && size == c.size);
        }

        // Helpers for working with behaviour bits

        template<class T>
        static uint64_t set_mask(uint64_t mask, uint64_t umask, uint32_t shift, T value)
        {
            return ((mask & ~umask) | (uint64_t(value) << shift));
        }

        template<class T>
        static T get_mask(uint64_t mask, uint64_t umask, uint32_t shift)
        {
            return T((mask >> shift) & umask);
        }
    };
    
    // Assert the size of the above wrappers
    static_assert(sizeof(mod) == sizeof(modloader_mod_t), "Invalid mod inheritance size");
    static_assert(sizeof(file) == sizeof(modloader_file_t), "Invalid file inheritance size");
    static_assert(sizeof(plugin) == sizeof(modloader_plugin_t), "Invalid plugin inheritance size");
    
    


    /*
        modloader::basic_plugin
            The base for any custom plugin
    */
    class basic_plugin
    {
        public:
            friend struct basic_plugin_wrapper;
            
            // Stores information about this plugin
            struct info
            {
                const char*  name;              // Useless, name of the plugin
                const char*  version;           // Plugin version
                const char*  author;            // Plugin author
                int          default_priority;  // Plugin default priority (or -1 for mod loader default)
                const char** extable;           // Extension table of possible files this plugin can handle, to speed up lookup
            };
        
        public:
            const modloader_t*  loader;     // Pointer to the Mod Loader basic information
            const plugin*       data;       // Pointer to this plugin data
            modloader_fLog      Log ;       // Log(fmt, ...)
            modloader_fvLog     vLog;       // vLog(fmt, va_list)
            modloader_fError    Error;      // Error(fmt, ...)

            // Casts this base to a derived object
            template<class To>
            To& cast() { return *static_cast<To*>(this); }

        public: // Check out "doc/Creating Your Own Plugin.txt" for detailed information!!!!!!!!!!
            virtual const info& GetInfo()=0;                // Gets the plugin information such as version and author
            virtual bool OnStartup()    { return true; }    // To startup the plugin
            virtual bool OnShutdown()   { return true; }    // To shutdown the plugin
            virtual int  GetBehaviour(file&)=0;             // Gets the behaviour of a specific file in relation to this plugin
            virtual bool InstallFile(const file&)=0;        // Installs a new file
            virtual bool ReinstallFile(const file&)=0;      // Reinstalls a file previosly installed
            virtual bool UninstallFile(const file&)=0;      // Uninstalls a file previosly installed
            virtual void Update() {}                        // Updates the state of the plugin after a serie of install/uninstall/reinstall
    };
    
    

    // Binding the C interface to the C++ interface
    struct basic_plugin_wrapper
    {
        static basic_plugin& GetThis(modloader_plugin_t* data)
        {
            return *(basic_plugin*)(data->pThis);
        }
        
        static file& GetFile(const modloader_file_t* f)
        {
            return *(const_cast<file*>(static_cast<const file*>(f)));
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

        // Attaches the 'interface' using the plugin 'data'
        static void RegisterPluginData(basic_plugin& interfc, modloader_plugin_t* data)
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
        
            // Mod Loader stuff
            interfc.data    = reinterpret_cast<plugin*>(data);
            interfc.loader  = data->modloader;
            interfc.Error   = interfc.loader->Error;
            interfc.Log     = interfc.loader->Log;
            interfc.vLog    = interfc.loader->vLog;
        }
    };

    /*
     *  Plugin object data
     */
    extern basic_plugin* plugin_ptr;

    /*
        Implementation of some modloader::file methods
    */
    inline std::string& file::fullpath(std::string& out) const
    {
        out.assign(std::string(plugin_ptr->loader->gamepath));
        out.append(this->filepath());
        return out;
    }

    inline std::string file::fullpath() const
    {
        std::string out;
        this->fullpath(out);
        return out;
    }
    

    // You need to use those to register the existence of your plugin
    #define REGISTER_ML_PLUGIN(plugin)  REGISTER_ML_PLUGIN_PTR(&plugin);
    #define REGISTER_ML_NULL()          REGISTER_ML_PLUGIN_PTR(nullptr)

    // Backend for REGISTER_ML_PLUGIN
    #define REGISTER_ML_PLUGIN_PTR(ptr) namespace modloader {\
            modloader::basic_plugin* plugin_ptr = ptr;\
            extern "C" __declspec(dllexport) void GetPluginData(modloader_plugin_t* data)\
            {\
                if(plugin_ptr) basic_plugin_wrapper::RegisterPluginData(*plugin_ptr, data);\
            }\
            extern "C" __declspec(dllexport) void GetLoaderVersion(uint8_t* major, uint8_t* minor, uint8_t* revision)\
            {\
                *major = MODLOADER_VERSION_MAJOR;\
                *minor = MODLOADER_VERSION_MINOR;\
                *revision = MODLOADER_VERSION_REVISION;\
            }\
        }


};


#endif
