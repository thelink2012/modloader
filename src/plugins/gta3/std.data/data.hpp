#pragma once
#include <stdinc.hpp>
#include "datalib.hpp"
#include "vfs.hpp"
#include "cache.hpp"

enum class Type
{
    Data        = 0,    // Its a common kind of data file (Have to check the hash part of the behaviour to see which data file is it)
    Scene       = 1,    // Its a IPL scene
    ObjTypes    = 2,    // Its a IDE def file

    Max         = 7,    // Max 3 bits
};

// Basical maskes
static const uint64_t hash_mask_base  = 0xFFFFFFFF;
static const uint64_t type_mask_base  = 0x0007;                 // Mask for type without any shifting
static const uint64_t count_mask_base = 0xFFFF;                 // Mask for count without any shifting
static const uint32_t type_mask_shf   = 32;                     // Takes 3 bits, starts from 33th bit because first 32th bits is a hash
static const uint32_t count_mask_shf  = type_mask_shf + 3;      // Takes 28 bits


// Sets the initial value for a behaviour, by using an filename hash and file type
inline uint64_t SetType(uint32_t hash, Type type)
{
    return modloader::file::set_mask(uint64_t(hash), type_mask_base, type_mask_shf, type);
}

inline uint64_t SetCounter(uint64_t behaviour, uint32_t count)
{
    if(count == 0)
    {
        modloader::plugin_ptr->Error("std.data: SetCounter cannot have a count of 0, zero is reserved for non-counted files.\n"
                                     "This error should never happen. It's a bug, please report it.");
    }
    return modloader::file::set_mask(behaviour, count_mask_base, count_mask_shf, count);
}

inline Type GetType(uint64_t mask)
{
    return modloader::file::get_mask<Type>(mask, type_mask_base, type_mask_shf);
}


/*
 *  The plugin object
 */
class DataPlugin : public modloader::basic_plugin
{
    public:
        const info& GetInfo();

        bool OnStartup();
        bool OnShutdown();
        int GetBehaviour(modloader::file&);
        bool InstallFile(const modloader::file&);
        bool ReinstallFile(const modloader::file&);
        bool UninstallFile(const modloader::file&);
        void Update();


    protected:
        vfs<const modloader::file*> fs;
        modloader::file_overrider<> ov_plants;

        std::map<size_t, modloader::file_overrider<>> ovmap;
        std::set<modloader::file_overrider<>*> ovrefresh;

        CDataCache cache;

        template<class StoreType, class... Args>
        modloader::file_overrider<>& AddMerger(const std::string& fsfile, bool unique, const modloader::file_overrider<>::params& params, Args&&... xargs)
        {
            using namespace std::placeholders;
            using store_type  = StoreType;
            using traits_type = typename store_type::traits_type;
            using detour_type = typename traits_type::detour_type;

            auto& ov = ovmap.emplace(std::piecewise_construct,
                std::forward_as_tuple(modloader::hash(fsfile)),
                std::forward_as_tuple(tag_detour, params, detour_type(), std::forward<Args>(xargs)...)
                ).first->second;

            
            ov.OnHook(nullptr);

            auto& d = ov.GetInjection().cast<detour_type>();
            d.make_call();  // TODO make some way this is called only on install from a file of this kind
            d.OnTransform(std::bind(&DataPlugin::GetMergedData<StoreType>, this, _1, fsfile, unique));

            return ov;
        }

        // Adds a detour for the file with the specified hash to the overrider subsystem
        template<class ...Args>
        modloader::file_overrider<>& AddDetour(const std::string& fsfile, Args&&... args)
        {
            return ovmap.emplace(std::piecewise_construct,
                std::forward_as_tuple(modloader::hash(fsfile)),
                std::forward_as_tuple(tag_detour, std::forward<Args>(args)...)
                ).first->second;
        }

    public:
        // Finds overrider for the file with the specified hash, rets null if not found
        modloader::file_overrider<>* FindMerger(const std::string& fsfile)
        {
            return FindMerger(modloader::hash(fsfile));
        }

        modloader::file_overrider<>* FindMerger(size_t hash)
        {
            auto it = ovmap.find(hash);
            if(it != ovmap.end()) return &it->second;
            return nullptr;
        }

    private:
        
        template<class StoreType>  
        std::string GetMergedData(std::string file, std::string fsfile, bool unique)
        {
            using store_type  = StoreType;
            using traits_type = typename store_type::traits_type;

            auto range = this->fs.files_at(fsfile);
            auto count = std::distance(range.first, range.second);

            // TODO make warnings display current trait name (identifier, what())

            if(count > 0)
            {
                if(count == 1)  // TODO but what if domflags says to not delete entries from the default file?
                    return range.first->second.first;   // return the only file in the vfs

                std::vector<store_type> store;
                std::string fullpath; fullpath.reserve(MAX_PATH);
                store.reserve(count + 1);

                auto add_store = [&store](const char* path, bool is_default)
                {
                    if(IsPathA(path))
                    {
                        auto st = store.emplace(store.end());
                        st->set_as_default(is_default);
                        if(!st->load_from_file(path))
                        {
                            plugin_ptr->Log("Warning: Failed to read %s data file into store: \"%s\"",
                                            (is_default? "default" : "custom"), path);
                            store.erase(st);
                        }
                    }
                };

                add_store(file.c_str(), true);
                for(auto it = range.first; it != range.second; ++it)
                {
                    auto& f = it->second.second;
                    add_store(f->fullpath(fullpath).c_str(), false);
                }

                if(!store.empty())
                {
                    file = cache.GetPathForData(file, false, unique);
                    fullpath.assign(plugin_ptr->loader->gamepath).append(file);
                    if(gta3::merge_to_file<store_type>(fullpath.c_str(), store.begin(), store.end(), traits_type::domflags_fn()))
                        return file;
                    else
                        plugin_ptr->Log("Warning: Failed to merge (next to load) data files into \"%s\"", file.c_str());
                }
            }

            return "";  // use default file
        }

};

extern DataPlugin plugin;

