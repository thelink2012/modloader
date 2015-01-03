/*
 * Copyright (C) 2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#pragma once
#include <modloader/modloader.hpp>
#include <modloader/utility.hpp>
#include <modloader/util/container.hpp>
#include <tuple>
#include <string>
#include <vector>
#include "vfs.hpp"

// Serialization
#include <cereal/archives/binary.hpp>
#include <cereal/types/array.hpp>
#include <cereal/types/base_class.hpp>
#include <cereal/types/bitset.hpp>
#include <cereal/types/boost_variant.hpp>
#include <cereal/types/common.hpp>
#include <cereal/types/list.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/set.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/tuple.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/unordered_set.hpp>
#include <cereal/types/utility.hpp>
#include <cereal/types/vector.hpp>

// Whenever a serialized data format changes, this number should increase so the serialized file gets incompatible
// NOTICE, this function should be static so it gets one instantiation on each translation unit!!!
static /* <--- YES STATIC */ uint32_t current_cereal_data_version()
{
    static const uint32_t version = modloader::hash(__DATE__ " " __TIME__);
    return version;
}

// Some settings for debugging caching
static const bool disable_caching     = false;  // Disables reading from the cache
static const bool cache_force_reading = false;  // Forces reading the cache even when nothing changed
#ifdef NDEBUG
static_assert(!disable_caching && !cache_force_reading, "Wrong release settings for caching");
#endif

template<class StoreType>
class caching_stream;

class data_cache : modloader::basic_cache
{
    private:
        vfs<> fs;

    public:
        using cache_file_tuple = std::tuple<int, std::string, std::string>;

        // Initializes the caching system
        bool Startup()
        {
            if(modloader::basic_cache::Startup())
            {
                if(get<0>(this->AddCacheFile("_STARTUP_", false)) != -1     // Creates /0/ directory
                && get<0>(this->AddCacheFile("_STARTUP_", true)) != -1)     // Creates /1/ directory
                    return true;
            }
            return false;
        }

        // Uninitializes the caching system
        void Shutdown()
        {
            fs.clear();
            return basic_cache::Shutdown(false);
        }

        //
        //  NOTE: cache_id 0 is for unique data files
        //

        // Gets directory for the specified cache id
        std::string GetCacheDir(int cache_id, bool fullpath = true)
        {
            if(cache_id >= 0)
                return basic_cache::GetCacheDir(std::to_string(cache_id), fullpath);
            throw std::runtime_error("");
        }

        // Gets path for cache file in the specified cache id directory
        std::string GetCachePath(int cache_id, const std::string& file, bool fullpath = true)
        {
            if(cache_id >= 0)
            {
                auto path = this->GetCacheDir(cache_id, fullpath);
                path.append(file);
                return path;
            }
            throw std::runtime_error("");
        }

        //  Gets the path for the cache file associated with the specified caching stream
        template<class StoreType>
        std::string GetCachePath(const caching_stream<StoreType>& cs, bool fullpath = true)
        {
            return GetCachePath(cs.cache_id, cs.fsfile, fullpath);
        }



        // Adds and takes the directory to the data file 'file' in the cache directory
        // See the meaning of unique at data.hpp
        // This function should never fail
        cache_file_tuple AddCacheFile(std::string file, bool unique)
        {
            using modloader::plugin_ptr;
            if(unique)
            {
                return AddCacheFile(0, file, true);
            }
            else
            {
                cache_file_tuple tuple;
                for(int i = 1; i < 100; ++i)    // try maximum of /99/ folders
                {
                    tuple = AddCacheFile(i, file, false);
                    if(std::get<0>(tuple) != -1)
                        return tuple;
                }

                plugin_ptr->Error("std.data: data_cache::AddCacheFile failed");
                throw std::runtime_error("AddCacheFile failed");
            }
        }


    public:

        // Finds the cache directory for the specified caching stream
        // That's finds the directory that already contains saved cache for the specified data file
        // Returns failure (get<0>(result) == -1) if there's no saved cache for the specified data file (caching stream)
        template<class StoreType>
        cache_file_tuple FindCachedDataFor(caching_stream<StoreType>& cs)
        {
            if(disable_caching) return cache_file_tuple(-1, "", "");
            return cs.unique? MatchCache(cs, 0) : MatchNonUniqueCache(cs);
        }

        // Reads cached store data for the specified caching stream
        // Apply must have been called already for the caching stream
        template<class StoreType>
        bool ReadCachedStore(caching_stream<StoreType>& cs)
        {
            using namespace std::placeholders;
            using store_list_type   = caching_stream<StoreType>::store_list_type;

            // Maps the index of the store in the cache to the index of the store in the current caching stream
            std::map<size_t, int> cache2current;
            for(size_t i = 0; i < cs.listing.size(); ++i)
                cache2current[i] = -1;  // make sure cache2current has max(cs.listing.size(), cs.cached_listing.size()) indices mapped
            for(size_t i = 0; i < cs.cached_listing.size(); ++i)
            {
                auto it = std::find(cs.listing.begin(), cs.listing.end(), cs.cached_listing[i]);
                cache2current[i] = (it == cs.listing.end()? -1 : std::distance(cs.listing.begin(), it));
            }
            
            auto fLoadStore = std::bind(&data_cache::LoadStore<store_list_type>, _1, _2, std::ref(cs.store), std::ref(cache2current));
            if(cereal_from_file_byfunc(GetCachePath(cs.cache_id, cs.fsfile + ".d"), fLoadStore))
            {
                return true;
            }
            return false;
        }

        // Writes the listing and the store to the cache path associated with the caching stream
        template<class StoreType>
        bool WriteCachedStore(caching_stream<StoreType>& cs)
        {
            using namespace std::placeholders;
            using store_list_type   = caching_stream<StoreType>::store_list_type;

            auto fSaveStore = std::bind(&data_cache::SaveStore<store_list_type>, _1, _2, std::ref(cs.store));

            if(cereal_to_file(GetCachePath(cs.cache_id, cs.fsfile + ".l"), cs.listing)
            && cereal_to_file_byfunc(GetCachePath(cs.cache_id, cs.fsfile + ".d"), fSaveStore))
            {
                return true;
            }
            return false;
        }

    private: // Serialization specialization for store type

        /*
            The store object is saved and loaded from the cache in a different manner than cereal would do by default.
            [*] When saving, the block size of the following serialized data_store is saved, so they can be skipped on load
            [*] When loading, it does not load the entire serialized list of data stores but only those which have not been modified
        */

        // Saves the list of data stores 'store' and put the block size taken by each element before it
        template<class StoreList>
        static void SaveStore(std::ofstream& ss, cereal::BinaryOutputArchive& archive, StoreList& store)
        {
            std::streamsize blocksize;
            archive(static_cast<cereal::size_type>(store.size()));

            for(auto& elem : store)
            {
                auto prevpos = ss.tellp();
                ss.seekp(sizeof(blocksize), std::ios::cur);         // skip to overwrite later on
                archive(elem);                                      // save element
                    
                blocksize = (ss.tellp() - prevpos);                 // the size of the block written since 'prevpos'
                blocksize = blocksize - sizeof(blocksize);          // blocksize should not contain the blocksize variable itself in it
                ss.seekp(prevpos, std::ios::beg);                   // get back to the sparse space we left a while ago...
                archive.saveBinary(&blocksize, sizeof(blocksize));  // ...and overwrite it with the proper blocksize
                ss.seekp(blocksize, std::ios::cur);                 // skip the block so we get again to the place we should write the next block
            }
        }

        // Loads the list of UNMODIFIED data stores into 'store'
        // The map cache2current maps indices from the cache into indices in the actual 'store'
        template<class StoreList>
        static void LoadStore(std::ifstream& ss, cereal::BinaryInputArchive& archive, StoreList& store, std::map<size_t, int>& cache2current)
        {
            std::streamsize blocksize;
            cereal::size_type csize;
            
            // Notice: do not resize the store object, it is the object from caching_stream that passed tho Apply()
            // Just pick the size and use it in the loop to read from the serialized data
            archive(csize);

            for(size_t i = 0, size = size_t(csize); i < size && !ss.fail(); ++i)
            {
                archive.loadBinary(&blocksize, sizeof(blocksize));
                int k = cache2current[i];
                if(k == -1) // no association with the current store, skip this element
                    ss.seekg(blocksize, std::ios::cur);
                else
                    archive(store[k]);
            }
        }


    private: // Serialization, with version information

        // Unserializes (reads) the data from a file into a object
        template<class TOut>
        static bool cereal_from_file(std::string filepath, TOut& out)
        {
            return do_cereal<false>(filepath, out);
        }

        // Serializes (writes) the data from a object into a file
        template<class TOut>
        static bool cereal_to_file(std::string filepath, TOut& out)
        {
            return do_cereal<true>(filepath, out);
        }

        // Unserializes (reads) the data from a file into a object
        template<class TFunc>
        static bool cereal_from_file_byfunc(std::string filepath, TFunc fun)
        {
            return do_cereal_byfunc<false>(filepath, fun);
        }

        // Serializes (writes) the data from a object into a file
        template<class TFunc>
        static bool cereal_to_file_byfunc(std::string filepath, TFunc fun)
        {
            return do_cereal_byfunc<true>(filepath, fun);
        }

        // Performs serialization (either read or write, depending on 'bool Output')
        template<bool Output, class TOut>
        static bool do_cereal(std::string filepath, TOut& out)
        {
            using stream_type  = typename std::conditional<Output, std::ofstream, std::ifstream>::type;
            using archive_type = typename std::conditional<Output, cereal::BinaryOutputArchive, cereal::BinaryInputArchive>::type;
            return do_cereal_byfunc<Output>(std::move(filepath), [&out](stream_type&, archive_type& ar) { ar(out); });
        }

        template<bool Output, class TFunc>
        static bool do_cereal_byfunc(std::string filepath, TFunc func)
        {
            static const size_t buffer_size = 512000; // 512KiB
            using stream_type  = typename std::conditional<Output, std::ofstream, std::ifstream>::type;
            using archive_type = typename std::conditional<Output, cereal::BinaryOutputArchive, cereal::BinaryInputArchive>::type;

            stream_type ss;

            std::unique_ptr<char[]> buffer(new char[buffer_size]);
            ss.rdbuf()->pubsetbuf(buffer.get(), buffer_size);

            ss.open(filepath, std::ios::binary);
            if(ss.is_open())
            {
                uint32_t version = current_cereal_data_version(); // This variable won't change in the case of a output stream,
                                                                // but will in the case of a input stream, in any case default initialize for the output case
                archive_type archive(ss);
                archive(version);
                if(version == current_cereal_data_version())      // Make sure serialization version matches
                {
                    func(ss, archive);
                    return true;
                }
                else
                    plugin_ptr->Log("Warning: Incompatible cache version, a new cache will be generated.");
            }
            return false;
        }

    private: /// XXX refactore those functions
    
        cache_file_tuple AddCacheFile(int cache_id, std::string file, bool unique)
        {
            using tuple_type = std::tuple < int, std::string, std::string > ;

            using modloader::plugin_ptr;

            std::string vdir, vpath;
            vdir.assign(std::to_string(cache_id)).push_back('/');
            vpath.assign(vdir).append(file);

            if(unique || fs.count(vpath) == 0)
            {
                if(fs.count(vdir) == 0)
                {
                    if(this->CreateDir(vdir) == false)
                    {
                        plugin_ptr->Error("Fatal Error: std.data: Failed to create directory for caching: \"%s\"", vdir.c_str());
                        return cache_file_tuple(-1, "", "");
                    }
                    else
                        fs.add_file(vdir, "");
                }

                fs.add_file(vpath, "");

                return cache_file_tuple(
                    cache_id,
                    this->GetCachePath(cache_id, file, false),
                    this->GetCachePath(cache_id, file, true)
                    );
            }


            return cache_file_tuple(-1, "", "");
        }

        template<class StoreType>
        cache_file_tuple MatchNonUniqueCache(caching_stream<StoreType>& cs)
        {
            // TODO stop walking the tree every time, get around it
            cache_file_tuple result(-1, "", "");
            modloader::FilesWalk(this->fullpath, "*.*", false, [&](modloader::FileWalkInfo& f)
            {
                if(f.is_dir)
                {
                    uint32_t cache_id;

                    try {
                        cache_id = std::stoul(f.filename);
                        if(cache_id == 0) return true;
                    }
                    catch(const std::exception&) {
                        return true;
                    }

                    auto tuple = MatchCache(cs, cache_id);
                    if(get<0>(tuple) != -1)
                    {
                        result = std::move(tuple);
                        return false;
                    }
                }
                return true;
            });
            return result;
        }

        template<class StoreType>
        cache_file_tuple MatchCache(caching_stream<StoreType>& cs, uint32_t cache_id)
        {
            using namespace modloader;
            cache_file_tuple result(-1, "", "");
            modloader::FilesWalk(this->GetCacheDir(cache_id, true), "*.*", false, [&](modloader::FileWalkInfo& f)
            {
                if(!strcmp(f.filename, cs.fsfile.c_str(), false))
                {
                    if(cereal_from_file(std::string(f.filepath).append(".l"), cs.cached_listing))
                    {
                        if(cs.MatchListing())
                        {
                            get<0>(result) = cache_id;
                            get<1>(result) = this->GetCacheDir(cache_id, false).append(f.filename);
                            get<2>(result) = this->GetCacheDir(cache_id, true).append(f.filename);
                            return false;
                        }
                    }
                }
                return true;
            });
            return result;
        }

};

template<class StoreType>
class caching_stream
{
    public:
        friend class data_cache;

        // Stores file system information about a single data file
        struct info
        {
            bool is_default = false;    // Is this data store a default one? (see data_store.hpp for the meaning of default in store context)
            bool relpath    = false;    // Is this path relative to the current working directory?
            bool rsv1       = false;    // reserved
            bool rsv2       = false;    // reserved
            uint32_t flags  = 0;        // Flags as in modloader::file::flags
            uint64_t size   = 0;        // Size of this file (as in modloader::file::size)
            uint64_t time   = 0;        // Write time of this file (as in modloader::file::time)

            // Compare with another info to check if anything changed in the data file
            bool operator==(const info& rhs) const
            {
                return this->is_default == rhs.is_default
                    && this->relpath    == rhs.relpath
                    && this->rsv1       == rhs.rsv1
                    && this->rsv2       == rhs.rsv2
                    && this->flags      == rhs.flags
                    && this->size       == rhs.size
                    && this->time       == rhs.time;
            }

            // Serializer to load/save this type into a cache
            template <class Archive>
            void serialize(Archive& ar)
            {
                ar(is_default, relpath, rsv1, rsv2, flags, size, time);
            }
        };

        using listing_list_type = std::vector<std::pair<std::string, info>>;
        using store_list_type   = std::vector<StoreType>;

    protected:

        std::string path;       // Path relative to the game dir to the cached data file
        std::string fullpath;   // Fullpath to the above path
        int cache_id;           // Cache directory index that path points to

        std::string fsfile;     // Name of the data file
        bool unique;            // Is this data file unique? (see data.hpp for meaning of unique)
        
        // Cached listing and data store
        listing_list_type   cached_listing;     // The cached listing on disk
        listing_list_type   listing;            // The current listing built from AddFile calls
        store_list_type     store;              // The current store
        
    public:

        caching_stream(std::string fsfile, bool unique) :
            fsfile(std::move(fsfile)), unique(unique), cache_id(-1)
        {}

        // Accessors for variables
        const std::string& FullPath() { return this->fullpath; }
        const std::string& Path()     { return this->path; }
        store_list_type& StoreList()  { return this->store; }

        // Adds information about a data file with path relative to the current working directory
        caching_stream& AddFile(std::string path, bool is_default)
        {
            using namespace modloader;
            if(IsPathA(path.c_str()))
            {
                WIN32_FILE_ATTRIBUTE_DATA attribs;
                path = modloader::NormalizePath(std::move(path));

                auto it = listing.emplace(listing.end(), std::piecewise_construct,
                    std::forward_as_tuple(std::move(path)),
                    std::forward_as_tuple());

                if(GetFileAttributesExA(it->first.c_str(), GetFileExInfoStandard, &attribs))
                {
                    auto& info = it->second;
                    info.is_default = is_default;
                    info.relpath    = true;
                    info.flags     |= (attribs.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)? MODLOADER_FF_IS_DIRECTORY : 0;
                    info.size       = GetLongFromLargeInteger(attribs.nFileSizeLow, attribs.nFileSizeHigh);
                    info.time       = GetLongFromLargeInteger(attribs.ftLastWriteTime.dwLowDateTime, attribs.ftLastWriteTime.dwHighDateTime);
                }
                else
                    listing.erase(it);
            }
            return *this;
        }

        // Adds information about a data file with path relative to the game directory
        caching_stream& AddFile(const modloader::file& file, bool is_default)
        {
            using namespace modloader;
            if(IsPathA(file.fullpath().c_str()))
            {
                auto& info = listing.emplace(listing.end(), std::piecewise_construct,
                    std::forward_as_tuple(file.filepath()),
                    std::forward_as_tuple())->second;

                info.is_default = is_default;
                info.relpath    = false;
                info.flags     |= file.flags;
                info.size       = file.size;
                info.time       = file.time;
            }
            return *this;
        }

        // Applies information received from a serie of AddFile's plus the directory to save the cache (argument) into this cache object
        // The apply operation only takes place if the cache id in the cache tuple is not equal to -1 (none/failure)
        // When the apply operation doesn't take place because of the reason above, false is returned.
        bool Apply(data_cache::cache_file_tuple&& cidpath)
        {
            if(get<0>(cidpath) != -1)
            {
                this->cache_id  = std::move(get<0>(cidpath));
                this->path      = std::move(get<1>(cidpath));
                this->fullpath  = std::move(get<2>(cidpath));

                this->store.resize(this->listing.size());
                for(size_t i = 0; i < this->store.size(); ++i)
                {
                    bool is_default = this->listing[i].second.is_default;
                    this->store[i].set_as_default(is_default);
                }

                return true;
            }
            return false;
        }

        // Checks if any data file has been added/changed/removed by looking into the cached listing
        // and the listing built from the AddFile calls
        bool DidAnythingChange() const
        {
            if(cache_force_reading) return true;
            return !(this->cached_listing == this->listing);
        }

        // Checks if the cached listing has anything to do with the listing built from the AddFile calls
        // This is important for non-unique data files because we could for example have a cache at '/1/a.ipl' and '/2/a.ipl', so which cache should we use?
        bool MatchListing() const
        {
            return std::any_of(cached_listing.begin(), cached_listing.end(), [this](const listing_list_type::value_type& pair)
            {
                return (std::find(this->listing.begin(), this->listing.end(), pair) != this->listing.end());
            });
        }

        // Loads stores from data files that have changed since the last cache-write
        void LoadChangedFiles()
        {
            using namespace modloader;
            for(size_t i = 0; i < this->store.size(); ++i)
            {
                auto& path   = this->listing[i].first;
                bool relpath = this->listing[i].second.relpath;
                bool good    = this->store[i].load_from_file(relpath?
                                                path.c_str() :
                                                std::string(plugin_ptr->loader->gamepath).append(path).c_str()
                                              );

                if(!good)
                    plugin_ptr->Log("Warning: Failed to build data store from data file %d:'%s'", relpath, path.c_str());
            }
        }

};
