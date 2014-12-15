#pragma once
#include <stdinc.hpp>
using namespace modloader;

// TODO PROPER CACHING!!!!

bool CDataCache::Startup(const std::string& name)
{
    this->path = std::string(plugin_ptr->loader->cachepath).append(name).append("/");
    this->fullpath = std::string(plugin_ptr->loader->gamepath).append(path);
    if(MakeSureDirectoryExistA(fullpath.c_str()))
    {
        this->GetPathForData("_STARTUP_", false, true);     // create /0/ directory
        this->GetPathForData("_STARTUP_", false, false);    // create /1/ directory
        this->initialized = true;
        return true;
    }

    return false;
}

void CDataCache::Shutdown(bool destroydir)
{
    if(this->initialized)
    {
        if(destroydir)
            DestroyDirectoryA(this->GetCachePath(true).c_str());
    }

    this->path.clear();
    this->fullpath.clear();
    this->fs.clear();
    this->initialized = false;
}

const std::string& CDataCache::GetCachePath(bool fullpath)
{
    return fullpath? this->fullpath : this->path;
}

std::string CDataCache::GetCachePath(const std::string& at, bool fullpath)
{
    return GetCachePath(fullpath) + at;
}

std::string CDataCache::GetCacheDir(const std::string& at, bool fullpath)
{
    auto result = GetCachePath(at, fullpath);
    result.push_back('/');
    return result;
}

bool CDataCache::CreateDir(const std::string& at)
{
    return !!MakeSureDirectoryExistA(GetCacheDir(at, true).c_str());
}

std::string CDataCache::GetPathForData(const std::string& at, bool fullpath, bool unique)
{
    // XXX what if file already exist from before the game execution?
    // No problems when we're talking about serialized data but what when we serialize it?

    std::string vpath, vdir;

    auto check = [&](int i) -> std::string
    {
        vdir.assign(std::to_string(i)).push_back('/');
        vpath.assign(vdir).append(at);

        if(unique || fs.count(vpath) == 0)
        {
            if(fs.count(vdir) == 0)
            {
                if(this->CreateDir(vdir) == false)
                {
                    plugin_ptr->Log("Warning: Failed to create directory at cache: \"%s\"", vdir.c_str());
                    return std::string();
                }
                else
                    fs.add_file(vdir, "");
            }

            fs.add_file(vpath, "");
            return this->GetCachePath(vpath, fullpath);
        }
        return std::string();
    };

    if(unique)
    {
        auto path = check(0);
        if(path.size()) return path;
    }
    else
    {
        for(int i = 1; i < std::numeric_limits<int>::max(); ++i)
        {
            auto path = check(i);
            if(path.size()) return path;
        }
    }

    // should never reach this point
    plugin_ptr->Error("std.data: Failed on GetPathForData.");
    throw std::runtime_error("std.data: Failed on GetPathForData.");
}
