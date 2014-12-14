#pragma once
#include <stdinc.hpp>
#include <unordered_map>

template<typename UData = int>
class vfs
{
    public:
        using multimap  = std::unordered_multimap<std::string, std::pair<std::string, UData>>;
        using iterator  = typename multimap::iterator;
        using size_type = typename multimap::size_type;

    private:
        multimap fs;

    public:
        static std::string normalize(std::string path)
        {
            // TODO since mod loader paths sent to plugins are already normalized should we just return path itself? (YEAHHH CASE SENSITIVE, but still we'd need to normalize)
            // if we do so, may we get values by const-reference at the public functions? 
            // TODO the UData file* has a hash in it, should we use it?
            return modloader::NormalizePath(std::move(path));
        }

    public:

        // TODO ctors, moves, etc

        // undefined behaviour if you add two files to the same vpath pointing to the same real path (see @rem_files)
        iterator add_file(std::string vpath, std::string path, UData userdata = UData())
        {
            return fs.emplace(normalize(std::move(vpath)), 
                              std::make_pair(normalize(std::move(path)), std::move(userdata))
                      );
        }

        bool rem_file(std::string vpath, std::string path)
        {
            path  = normalize(std::move(path));
            vpath = normalize(std::move(vpath));

            auto range = files_at(vpath);
            for(auto it = range.first; it != range.second; ++it)
            {
                if(it->second.first == path)
                {
                    fs.erase(it);
                    return true;
                }
            }

            return false;
        }

        bool rem_file(std::string vpath, const UData& udata)    // << TODO tag for udata
        {
            vpath = normalize(std::move(vpath));

            auto range = files_at(vpath);
            for(auto it = range.first; it != range.second; ++it)
            {
                if(it->second.second == udata)
                {
                    fs.erase(it);
                    return true;
                }
            }

            return false;
        }

        std::pair<iterator, iterator> files_at(std::string vpath)
        {
            return equal_range(std::move(vpath));
        }





        size_type count(std::string vpath)
        {
            return fs.count(normalize(std::move(vpath)));
        }

        std::pair<iterator, iterator> equal_range(std::string vpath)
        {
            return fs.equal_range(normalize(std::move(vpath)));
        }

};


