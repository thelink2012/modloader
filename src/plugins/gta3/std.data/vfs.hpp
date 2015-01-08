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
        using value_type = typename multimap::value_type;

    private:
        multimap fs;

    public:
        static std::string normalize(std::string path)
        {
            return modloader::NormalizePath(std::move(path));
        }

    public:

        // TODO ctors, moves, etc

        iterator begin() { return fs.begin(); }
        iterator end()   { return fs.end(); }

        iterator erase(iterator it) { return fs.erase(it); }

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

        bool rem_file(std::string vpath, const UData& udata)
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


        iterator insert(value_type&& value)
        {
            return fs.insert(std::move(value));
        }


        size_type count(std::string vpath)
        {
            return fs.count(normalize(std::move(vpath)));
        }

        std::pair<iterator, iterator> equal_range(std::string vpath)
        {
            return fs.equal_range(normalize(std::move(vpath)));
        }

        void clear()
        {
            fs.clear();
        }

        iterator move_file(iterator file_it, std::string dest_vpath)
        {
            auto itx  = this->add_file(std::move(dest_vpath), std::move(file_it->second.first), std::move(file_it->second.second));
            this->erase(file_it);
            return itx;
        }

        template<class FuncT>
        void walk(FuncT func)
        {
            for(auto it = begin(); it != end(); ++it)
            {
                if(func(it) == false)   // yes send iterator
                    break;
            }
        }
};


