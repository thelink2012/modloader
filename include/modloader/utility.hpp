/* 
 * Mod Loader Plugin Related Utilities
 * Created by LINK/2012 <dma_2012@hotmail.com>
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
#pragma once
#ifndef MODLOADER_UTIL_HPP
#define	MODLOADER_UTIL_HPP
#include <modloader/modloader.hpp>
#include <modloader/util/path.hpp>
#include <functional>

namespace modloader
{
    // Scoped chdir relative to gamedir
    struct scoped_gdir : public modloader::scoped_chdir
    {
        scoped_gdir(const char* newdir) : scoped_chdir(
            (!newdir[0]? (plugin_ptr->loader->gamepath) : ((std::string(plugin_ptr->loader->gamepath) + newdir).data()))
         )
        { }
    };

    template<class FuncT>
    std::function<void()> gdir_refresh(FuncT func)
    {
        return [func]
        {
            scoped_gdir xdir("");
            return func();
        };
    }

    /*
     *  basic_cache
     *      Base utility for accessing the modloader/.data/cache/ directory
     */
    class basic_cache
    {
        protected:
            std::string path, fullpath;     // relative and fullpath to the chosen cache name
            bool initialized = false;

        public:

            // Initializes the managed cache directory named 'name'
            bool Startup(const std::string& name = plugin_ptr->data->name, bool useappdata = true)
            {
                if(useappdata)
                {
                    this->path      = std::string(plugin_ptr->loader->commonappdata).append(name);
                    MakeSureStringIsDirectory(this->path);
                    this->fullpath  = this->path;
                }
                else
                {
                    return false;
                    //this->path      = std::string(plugin_ptr->loader->cachepath).append(name);
                    //MakeSureStringIsDirectory(this->path);
                    //this->fullpath  = std::string(plugin_ptr->loader->gamepath).append(path);
                }

                if(MakeSureDirectoryExistA(fullpath.c_str()))
                {
                    this->initialized = true;
                    return true;
                }
                return false;
            }

            // Unitialises the managed cache directory, destroying such directory if 'destroydir=true'
            void Shutdown(bool destroydir = false)
            {
                if(this->initialized)
                {
                    if(destroydir)
                        DestroyDirectoryA(this->GetCachePath(true).c_str());
                }
                this->path.clear();
                this->fullpath.clear();
                this->initialized = false;
            }

            // Gets path to the managed cache directory
            const std::string& GetCachePath(bool fullpath = true)
            {
                return fullpath? this->fullpath : this->path;
            }

            // Gets path to the specified file 'at' in the managed cache directory
            std::string GetCachePath(const std::string& at, bool fullpath = true)
            {
                return GetCachePath(fullpath) + at;
            }

            // Gets path to the specified directory 'at' in the managed cache directory
            // The difference between GetCachePath and GetCacheDir is that GetCacheDir pushes a slash into the path.
            std::string GetCacheDir(const std::string& at, bool fullpath = true)
            {
                auto result = GetCachePath(at, fullpath);
                result.push_back(cNormalizedSlash);
                return result;
            }

            // Creates the specified directory in the managed cache path
            bool CreateDir(const std::string& at)
            {
                return !!MakeSureDirectoryExistA(GetCacheDir(at, true).c_str());
            }

    };
}

#endif
