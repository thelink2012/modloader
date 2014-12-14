#pragma once
#include <string>
#include "vfs.hpp"

class CDataCache
{
    protected:  // base
        std::string fullpath;
        std::string path;
        bool initialized = false;

    protected: // derived
        vfs<> fs;

    public: // base

        bool Startup(const std::string& name);
        void Shutdown(bool destroydir = false);

        const std::string& GetCachePath(bool fullpath = true);
        std::string GetCachePath(const std::string& at, bool fullpath = true);
        std::string GetCacheDir(const std::string& at, bool fullpath = true);

        bool CreateDir(const std::string& at);

    public: // derived

        std::string GetPathForData(const std::string& at, bool fullpath, bool unique);

};
