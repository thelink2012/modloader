/* 
 * Mod Loader Utilities Headers
 * Created by LINK/2012 <dma_2012@hotmail.com>
 * 
 *  This file provides helpful functions for plugins creators.
 * 
 *  This source code is offered for use in the public domain. You may
 *  use, modify or distribute it freely.
 *
 *  This code is distributed in the hope that it will be useful but
 *  WITHOUT ANY WARRANTY. ALL WARRANTIES, EXPRESS OR IMPLIED ARE HEREBY
 *  DISCLAIMED. This includes but is not limited to warranties of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */
#ifndef MODLOADER_UTIL_DETOUR_HPP
#define	MODLOADER_UTIL_DETOUR_HPP
#pragma once
#include <string>
#include <modloader/modloader.hpp>
#include <modloader/util/injector.hpp>
#include <modloader/util/path.hpp>
#include <tuple>

namespace modloader
{
    // Tag type
    struct tag_detour_t {};
    static const tag_detour_t tag_detour;
    
    struct tag_nofile_t {};
    static const tag_nofile_t tag_nofile;

    /*
     *  File detour
     *      Hooks a call to detour a string argument call to open another file instead of the sent to the function
     *      @addr is the address to hook 
     *      Note the object is dummy (has no content), all stored information is static
     */
    template<class Traits, class MyBase, class Ret, class ...Args>
    struct basic_file_detour : MyBase
    {
        protected:
            typedef MyBase                     function_hooker;
            typedef typename MyBase::func_type func_type;

            // Store for the path (relative to gamedir)
            struct store_type {
                std::string path;
                std::function<std::string(std::string)> transform;
            };

            static store_type& store()
            { static store_type x; return x; }
   
            // The detoured function goes to here
            static Ret hook(func_type fun, Args&... args)
            {
                static const auto  pos  = Traits::arg - 1;
                static const char* what = Traits::what();

                auto& arg = std::get<pos>(std::forward_as_tuple(args...));
                char fullpath[MAX_PATH];
                const char* lpath = nullptr;

                // If has transform functor, use it to get new path
                if(store().transform)
                    store().path = store().transform(arg);

                if(!store().path.empty())   // Has any path to override the original with?
                {
                    copy_cstr(fullpath, fullpath + MAX_PATH, plugin_ptr->loader->gamepath, store().path.c_str());

                    // Make sure the file exists, if it does, change the parameter
                    if(GetFileAttributesA(fullpath) != INVALID_FILE_ATTRIBUTES)
                    {
                        arg = fullpath;
                        lpath = store().path.c_str();
                    }
                }

                // If the file type is known, log what we're loading
                if(what && what[0] && plugin_ptr)
                    plugin_ptr->Log("Loading %s %s \"%s\"",
                                    (lpath? "custom" : "default"), what, (lpath? lpath : arg));

                // Call the function with the new filepath
                return fun(args...);
            }
        
        public:

            // Constructors, move constructors, assigment operators........
            basic_file_detour() = default;
            basic_file_detour(const basic_file_detour&) = delete;
            basic_file_detour(basic_file_detour&& rhs) : MyBase(std::move(rhs)) {}
            basic_file_detour& operator=(const basic_file_detour& rhs) = delete;
            basic_file_detour& operator=(basic_file_detour&& rhs)
            { MyBase::operator=(std::move(rhs)); return *this; }

            // Makes the hook
            void make_call()
            {
                MyBase::make_call(hook);
            }

            void setfile(std::string path)
            {
                make_call();
                store().path = std::move(path);
            }

            void OnTransform(std::function<std::string(std::string)> functor)
            {
                store().transform = std::move(functor);
            }
    };


    /*
        file overrider
            Made to easily override some game file
            Works better in conjunction with a file detour (basic_file_detour)
    */
    template<unsigned int NumInjections = 1>
    struct file_overrider
    {
        public:
            typedef injector::scoped_basic<32> scoped_buffer_type;
            static const auto num_injections = NumInjections;

        protected:
            const modloader::file* file = nullptr;          // The file being used as overrider
            scoped_buffer_type injection_buf[NumInjections];// Buffer to store scoped injection
            bool bCanReinstall = false;                     // Can this overrider get reinstalled?
            bool bCanUninstall = false;                     // Can this overrider get uninstalled?

            // Reinitialization time
            bool bReinitAfterStart = false;                 // Should call Reinit after game startup? (The game screen is there)
            bool bReinitAfterLoad = false;                  // Should call Reinit after game load? (After loading screen)

            // Events
            std::function<void(const modloader::file*)> mOnChange;      // Called when the file changes
            std::function<void(const modloader::file*)> mInstallHook;   // Install (or reinstall, or uninstall if file is null) hook for file
            std::function<void()>                       mReload;        // Reload the file on the game

        public:
            file_overrider() = default;                 
            file_overrider(const file_overrider&) = delete;     // cba to implement those
            file_overrider(file_overrider&&) = delete;          // ^^

            // Get the injection buffer
            injector::scoped_base& GetInjection(unsigned int i = 0)
            {
                return injection_buf[i];
            }
            
            // Checks if at this point of the game execution we can install stuff
            bool CanInstall()
            {
                return (!plugin_ptr->loader->has_game_started || bCanReinstall);
            }

            // Checks if at this point of the game execution we can uninstall stuff
            bool CanUninstall()
            {
                return (!plugin_ptr->loader->has_game_started || bCanUninstall);
            }

            // Call to install/reinstall/uninstall the file (no file should be installed)
            bool InstallFile(const modloader::file& file)
            {
                if(this->CanInstall())
                    return PerformInstall(&file);
                return false;
            }

            //
            bool Refresh()
            {
                return ReinstallFile();
            }

            // Reinstall the currently installed file
            bool ReinstallFile()
            {
                // Can reinstall if the game hasn't started or if we can reinstall this kind
                if(this->CanInstall())
                    return PerformInstall(this->file);
                return true;    // Mark as reinstalled anyway, so we won't happen to have a catastrophical failure
                                // When both Reinstall and Uninstall fails, we have a problem.
            }

            // Uninstall the currently installed file
            bool UninstallFile()
            {
                // Can uninstall if the game hasn't started or if we can uninstall this kind
                if(this->CanUninstall())
                    return PerformInstall(nullptr);
                return false;
            }

            // Set functor to call when it's necessary to install a hook for a file
            void OnHook(std::function<void(const modloader::file*)> fun)
            {
                this->mInstallHook = std::move(fun);
            }

            // Set functor to call when it's necessary to reload the file
            void OnReload(std::function<void()> fun)
            {
                this->mReload = std::move(fun);
            }

            // Set functor to call whenevr the file changes
            void OnChange(std::function<void(const modloader::file*)> fun)
            {
                this->mOnChange = std::move(fun);
            }

            // Set the file detourer (important to call, otherwise no effect will happen)
            // This overrides OnHook
            template<class ...Args> void SetFileDetour(Args&&... detourers_)
            {
                std::reference_wrapper<scoped_base> d[] = { detourers_... };
                for(int i = 0; i < sizeof...(Args); ++i)
                    GetInjection(i) = std::move(d[i].get().template cast<scoped_buffer_type>());

                OnHook([this](const modloader::file* f)
                {
                    Hlp_SetFile<sizeof...(Args), Args..., std::nullptr_t>(f);
                });
            }

        public: // Helpers for construction
            // Pack of basic boolean states to help the initialization of this object
            struct params
            {
                bool bCanReinstall, bCanUninstall, bReinitAfterStart, bReinitAfterLoad;
                
                params(bool bCanReinstall, bool bCanUninstall, bool bReinitAfterStart, bool bReinitAfterLoad) :
                    bCanReinstall(bCanReinstall), bCanUninstall(bCanUninstall), bReinitAfterStart(bReinitAfterStart), bReinitAfterLoad(bReinitAfterLoad)
                {}

                params(std::nullptr_t) : params(false, false, false, false)
                {}
            };

            file_overrider& SetParams(const params& p)
            {
                this->bCanReinstall     = p.bCanReinstall;
                this->bCanUninstall     = p.bCanUninstall;
                this->bReinitAfterStart = p.bReinitAfterStart;
                this->bReinitAfterLoad  = p.bReinitAfterLoad;
                return *this;
            }


            // Constructor - only params, more liberty
            file_overrider(const params& p) :
                bCanReinstall(p.bCanReinstall), bCanUninstall(p.bCanUninstall), bReinitAfterStart(p.bReinitAfterStart), bReinitAfterLoad(p.bReinitAfterLoad)
            {}

            // Constructor - Quickly setup the entire file overrider based on a detourer
            template<class DetourType, class ReloadType>
            file_overrider(tag_detour_t, const params& p, DetourType detour, ReloadType reload) :
                file_overrider(tag_detour, p, std::move(detour))
            {
                this->OnReload(std::move(reload));
            }

            // Constructor - Quickly setup the entire file overrider based on a detourer
            template<class DetourType>
            file_overrider(tag_detour_t, const params& p, DetourType detour) :
                file_overrider(p)
            {
                this->SetFileDetour(std::move(detour));
            }

        protected:
            // Perform a install for the specified file, if null, it will uninstall the currently installed file
            bool PerformInstall(const modloader::file* file)
            {
                this->file = file;
                if(mOnChange) mOnChange(this->file);
                InstallHook(); TryReload();
                return true;
            }

            // Installs the necessary hooking to load the specified file
            void InstallHook()
            {
                if(mInstallHook) mInstallHook(this->file);
            }

            // Tries to reload (if necessary) the current file
            void TryReload()
            {
                // Go ahead only if we have a reload functor and the game has booted up
                if(mReload && plugin_ptr->loader->has_game_started)
                {
                    if(plugin_ptr->loader->has_game_loaded)
                    {
                        // Can reload after game load?
                        if(bReinitAfterLoad) mReload();
                    }
                    else
                    {
                        // Can reload after game startup?
                        if(bReinitAfterStart) mReload();
                    }
                }
            }

        protected: // Because we don't have expression SFINAE on MSVC 2013 we need to do this kind of trick: ( :/ )
                   // Eh, we could actually use integral_constant...

            template<size_t i, class T, class ...Args>
            typename std::enable_if<!std::is_same<T, std::nullptr_t>::value>::type Hlp_SetFile(const modloader::file* f)
            {
                T& detourer = GetInjection(i-1).template cast<T>();
                detourer.setfile(f? f->filepath() : "");
                return Hlp_SetFile<i-1, Args...>(f); 
            }

            template<size_t i, class T, class ...Args>
            typename std::enable_if<std::is_same<T, std::nullptr_t>::value>::type Hlp_SetFile(const modloader::file* f)
            {}  // The end of the args is represented with a nullptr_t

            template<size_t i, class T, class ...Args>
            typename std::enable_if<!std::is_same<T, std::nullptr_t>::value>::type Hlp_OnTransform(const std::function<std::string(std::string)>& fn)
            {
                T& detourer = GetInjection(i-1).template cast<T>();
                detourer.OnTransform(fn);
                return Hlp_OnTransform<i-1, Args...>(fn);
            }

            template<size_t i, class T, class ...Args>
            typename std::enable_if<std::is_same<T, std::nullptr_t>::value>::type Hlp_OnTransform(const std::function<std::string(std::string)>& fn)
            {}  // The end of the args is represented with a nullptr_t

            template<size_t i, class T, class ...Args>
            typename std::enable_if<!std::is_same<T, std::nullptr_t>::value>::type Hlp_MakeCall()
            {
                T& detourer = GetInjection(i-1).template cast<T>();
                detourer.make_call();
                return Hlp_MakeCall<i-1, Args...>(fn);
            }

            template<size_t i, class T, class ...Args>
            typename std::enable_if<std::is_same<T, std::nullptr_t>::value>::type Hlp_MakeCall()
            {}  // The end of the args is represented with a nullptr_t
    };

}


#endif	/* MODLOADER_UTIL_HPP */

