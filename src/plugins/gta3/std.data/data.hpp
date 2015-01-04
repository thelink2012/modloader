/*
 * Copyright (C) 2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#pragma once
#include <stdinc.hpp>
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

// Non unique files merger fs names
// Since ipls and ides do not have a single file name to be merged (i.e. there are many ides and ipls files to merge) we use the followin'
static const char* ipl_merger_name = "**.ipl";
static const char* ide_merger_name = "**.ide";
static const size_t ipl_merger_hash = modloader::hash(ipl_merger_name);
static const size_t ide_merger_hash = modloader::hash(ide_merger_name);

// Sets the initial value for a behaviour, by using an filename hash and file type
inline uint64_t SetType(uint32_t hash, Type type)
{
    return modloader::file::set_mask(uint64_t(hash), type_mask_base, type_mask_shf, type);
}

// Sets the counter of the behaviour, note it is stored in a 28 bits integer.
inline uint64_t SetCounter(uint64_t behaviour, uint32_t count)
{
    return modloader::file::set_mask(behaviour, count_mask_base, count_mask_shf, count);
}

// Gets the type of a behaviour
inline Type GetType(uint64_t mask)
{
    return modloader::file::get_mask<Type>(mask, type_mask_base, type_mask_shf);
}


/*
 *  The plugin object
 */
class DataPlugin : public modloader::basic_plugin
{
    public: // Mod Loader Callbacks

        const info& GetInfo();

        bool OnStartup();
        bool OnShutdown();
        int GetBehaviour(modloader::file&);
        bool InstallFile(const modloader::file&);
        bool ReinstallFile(const modloader::file&);
        bool UninstallFile(const modloader::file&);
        void Update();

    private: // Effective methods
        bool InstallFile(const modloader::file&, size_t merger_hash, std::string fspath, std::string fullpath, bool isreinstall = false);
        bool ReinstallFile(const modloader::file&, size_t merger_hash);
        bool UninstallFile(const modloader::file&, size_t merger_hash, std::string fspath);

    private:  // Plugin Stuff

        struct files_behv_t
        {
            size_t      hash;       // Hash of this kind of file
            bool        canmerge;   // Can many files of this get merged into a single one?
            uint32_t    count;      // Must be initialized to 0, count of files (for merging, see GetBehaviour)
        };

        // Stores a virtual file system which contains the list of data files we got
        vfs<const modloader::file*> fs;

        // Overriders
        std::map<size_t, modloader::file_overrider> ovmap;        // Map of files overriders and mergers associated with their handling file names hashes
        std::set<modloader::file_overrider*>        ovrefresh;    // Set of mergers to be refreshed on Update() 

        // Info
        std::vector<files_behv_t> vbehav;

    public:
        // Caching stuff
        data_cache cache;

    public:

        // Registers a merger to work during the file verification and installation/uninstallation process
        // The parameter 'fsfile' especifies how is this file identified in the 'DataPlugin::fs' virtual filesystem
        // The boolean parameter 'unique' especifies whether the specified file can have only one instance
        //      (i.e. plants.dat have only one instance at data/ but vegass.ipl could have two at e.g. data/maps/vegas1 and data/maps/vegas2)
        // The boolean parameter 'samefile' specifies whether the filename received should match fsfile
        // The other parameters are familiar enought
        template<class StoreType, class... Args>
        modloader::file_overrider& AddMerger(std::string fsfile, bool unique, bool samefile, bool complete_path,
                                              const modloader::file_overrider::params& params, Args&&... xargs)
        {
            using namespace modloader;
            using namespace std::placeholders;
            using store_type  = StoreType;
            using traits_type = typename store_type::traits_type;
            using detour_type = typename traits_type::detour_type;

            fsfile = modloader::NormalizePath(std::move(fsfile));

            auto hash = modloader::hash(fsfile);

            auto& ov = ovmap.emplace(std::piecewise_construct,
                std::forward_as_tuple(hash),
                std::forward_as_tuple(tag_detour, params, detour_type(), std::forward<Args>(xargs)...)
                ).first->second;

            for(size_t i = 0; i < ov.NumInjections(); ++i)
            {
                // Merges data whenever necessary to open this file. Caching can happen.
                auto& d = static_cast<detour_type&>(ov.GetInjection(i));
                d.OnTransform(std::bind(&DataPlugin::GetMergedData<StoreType>, this, _1, fsfile, unique, samefile, complete_path));
            }

            // Replaces standard OnHook with this one, which just makes the call no setfile (since many files)
            // This gets called during InstallFile/ReinstallFile/UninstallFile
            ov.OnHook([&ov](const modloader::file*)
            {
                for(size_t i = 0; i < ov.NumInjections(); ++i)
                    static_cast<detour_type&>(ov.GetInjection(i)).make_call();
            });

            vbehav.emplace_back(files_behv_t { hash, true, 0 });
            return ov;
        }

        // Adds a detour for the file with the specified file to the overrider system
        // The parameter 'fsfile' especifies how is this file identified in the 'DataPlugin::fs' virtual filesystem
        template<class ...Args>
        modloader::file_overrider& AddDetour(std::string fsfile, Args&&... args)
        {
            fsfile = modloader::NormalizePath(std::move(fsfile));
            auto hash = modloader::hash(fsfile);

            auto& ov = ovmap.emplace(std::piecewise_construct,
                std::forward_as_tuple(hash),
                std::forward_as_tuple(tag_detour, std::forward<Args>(args)...)
                ).first->second;

            vbehav.emplace_back(files_behv_t { hash, false, 0 });
            return ov;
        }

        // Finds overrider/merger for the file with the specified instance in the virtual filesystem, rets null if not found
        modloader::file_overrider* FindMerger(std::string fsfile)
        {
            fsfile = modloader::NormalizePath(std::move(fsfile));
            return FindMerger(modloader::hash(fsfile));
        }

        // Finds overrider/merger for the file with the specified hash, rets null if not found
        modloader::file_overrider* FindMerger(size_t hash)
        {
            auto it = ovmap.find(hash);
            if(it != ovmap.end()) return &it->second;
            return nullptr;
        }

        files_behv_t* FindBehv(size_t hash)
        {
            for(auto& item : this->vbehav)
            {
                if(item.hash == hash)
                    return &item;
            }
            return nullptr;
        }

        files_behv_t* FindBehv(const modloader::file& f)
        {
            return FindBehv(f.hash);
        }

        files_behv_t* FindBehv(std::string fname)
        {
            return FindBehv(modloader::hash(fname));
        }

    private:
        
        //
        // Merges all the files in the virtual filesystem 'fs' which is in the 'fsfile' path, plus the file 'file' assuming the store type 'StoreType'
        // For a explanation of the 'unique' parameter see AddMerger function
        // The boolean parameter 'samefile' specifies whether the filename received should match fsfile
        //
        template<class StoreType>  
        std::string GetMergedData(std::string file, std::string fsfile, bool unique, bool samefile, bool complete_path)
        {
            using namespace modloader;
            using store_type  = StoreType;
            using traits_type = typename store_type::traits_type;
            auto filename     = modloader::NormalizePath(GetPathComponentBack(file));

            // Make sure filename matches if samefile has been specified
            if(samefile && filename != fsfile)
                return std::string(); // use default file

            auto what     = traits_type::dtraits::what();
            auto range    = this->fs.files_at(complete_path? file : fsfile);
            auto count    = std::distance(range.first, range.second);

            if(count == 1) // only one file, so let's just override
                return range.first->second.first;
            else if(count >= 2)   // any file to merge? we need at least 2 files to be able to do merging
            {
                auto fsfile = filename;
                caching_stream<StoreType> cs(fsfile, unique);
                
                // Add data files we'll work on to the caching stream
                cs.AddFile(file.c_str(), true);
                for(auto it = range.first; it != range.second; ++it)
                {
                    const modloader::file& f = *(it->second.second);
                    cs.AddFile(f, false);
                }

                // TODO make can_cache work for when anything changed == false etc
                // To do so just make the store (.d) not be saved and remove traits_type::can_cache from the following if
                // (but leave it at the check after DidAnythingChange)

                if(traits_type::can_cache && cs.Apply(cache.FindCachedDataFor(cs)))
                {
                    // We have a saved cache for this data file.
                    //
                    // Check out if any file has been added, changed or removed since the last cache-write
                    // If it did not change, simply return the previosly generated merged data file,
                    // otherwise read cached stores that didn't change and continue to load changed data files
                    //
                    if(cs.DidAnythingChange())
                    {
                        if(traits_type::can_cache)
                        {
                            if(!cache.ReadCachedStore(cs))
                                Log("Warning: Could not read cached store for '%s', skipping cache...", cs.Path().c_str());
                        }
                    }
                    else
                    {
                        //Log("No data file '%s' changed since last time, using cached data file", fsfile.c_str());
                        if(IsPathA(cs.FullPath().c_str()))
                            return cs.Path();
                        else
                            Log("Warning: Could not find cached data file '%s', skipping cache...", cs.Path().c_str());
                    }
                }
                else
                {
                    // No cache saved for this data file, find a cache directory for this
                    cs.Apply(cache.AddCacheFile(fsfile, unique));
                }

                // Load data files that have been added/changed
                cs.LoadChangedFiles();
                
                // Merge all the stored data into a single data file
                if(gta3::merge_to_file<store_type>(cs.FullPath().c_str(), cs.StoreList().begin(), cs.StoreList().end(), traits_type::domflags_fn()))
                {
                    // Rewrite the cached store/listing
                    if(traits_type::can_cache)
                    {
                        if(!cache.WriteCachedStore(cs))
                            Log("Warning: Could not write cache at '%s'", cs.Path().c_str());
                    }

                    return cs.Path();
                }
                else
                    plugin_ptr->Log("Warning: Failed to merge (%s) data files into \"%s\"", what, cs.Path().c_str());

            }

            return std::string();  // use default file
        }

};
extern DataPlugin plugin;


/*
 *  initializer object
 *      This allows the separation of many merges/overrides in .cpp files, which will improve our compilation speed.
 *      Leaving everything in a single cpp (using headers) would be a pain to compile since we HEAVILY rely on templates and more templates.
 */
class initializer
{
    protected:
        friend class DataPlugin;

        // The initializer callback
        std::function<void(DataPlugin*)> initialise;

        // List of initializers
        static std::vector<initializer*>& list()
        { static std::vector<initializer*> x; return x; }

    public:

        // Construct a initializer. This should be instantiated in a cpp file.
        // When initialization time comes, the specified callback gets called
        template<class FuncT>
        initializer(FuncT cb)
        {
            list().emplace_back(this);
            initialise = [cb](DataPlugin* p) mutable {
                cb(p);
            };
        }
};

// Global const params vars for AddMerger/AddDetour
static const auto no_reinstall          = modloader::file_overrider::params(nullptr);
static const auto reinstall_since_start = modloader::file_overrider::params(true, true, true, true);
static const auto reinstall_since_load  = modloader::file_overrider::params(true, true, false, true);
