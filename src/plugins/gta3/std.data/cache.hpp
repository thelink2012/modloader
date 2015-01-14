/*
 * Copyright (C) 2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#pragma once
#include <modloader/modloader.hpp>
#include <modloader/utility.hpp>
#include <modloader/util/container.hpp>
#include <modloader/util/injector.hpp>
#include <tuple>
#include <string>
#include <vector>
#include "vfs.hpp"
#include <file_block.hpp>

// Serialization
#include <cereal/archives/binary.hpp>
#include <cereal/types/array.hpp>
#include <cereal/types/base_class.hpp>
#include <cereal/types/bitset.hpp>
#include <cereal/types/boost_variant.hpp>
#include <cereal/types/common.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/list.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/set.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/tuple.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/unordered_set.hpp>
#include <cereal/types/utility.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/polymorphic.hpp>

// Custom Types Serialization
#include <cereal/types/boost_optional.hpp>
#include <cereal/types/typeindex.hpp>
#include <cereal/types/boost_any.hpp>

// Allows the specified type to be used in a boost::any
// -> name is a cross-platform name for the type, type is the type itself
#define REGISTER_RTTI_FOR_ANY(type)             \
    CEREAL_REGISTER_RTTI_WITH_NAME(type, #type);\
    CEREAL_REGISTER_FOR_ANY(type);               



// Whenever a serialized data format changes, this identifier should change so the serialized file gets incompatible
// So, let's use the compilation time of each compilation unit to represent this identifier
// NOTICE, this function should be static so it gets one instantiation on each translation unit!!!
static /* <--- YES STATIC */ uint32_t build_identifier()
{
    static const uint32_t version = modloader::hash(__DATE__ " " __TIME__);
    return version;
}

// Maximum number of /0/, /1/, and so on cache directories
// Higher decreases performance since it needs to loop/trytoprocess more
static const int max_cache_dirs = 10;

// Some settings for debugging caching
static const bool disable_caching     = false;  // Disables reading from the cache
static const bool cache_force_reading = false;  // Forces reading the cache even when nothing changed
#ifdef NDEBUG
static_assert(!disable_caching && !cache_force_reading, "Wrong release settings for caching");
#endif


template<class StoreType>
class caching_stream;


// Important information to cache when saving a file listing
struct cached_file_info
{
    protected:
        size_t   path_hash  = 0;    // Hash of the modloader file path
        uint32_t flags      = 0;    // Flags as in modloader::file::flags
        uint64_t size       = 0;    // Size of this file (as in modloader::file::size)
        uint64_t time       = 0;    // Write time of this file (as in modloader::file::time)

    public:
        //
        cached_file_info() = default;

        // Constructs from a modloader::file information
        cached_file_info(const modloader::file& file) :
            path_hash(modloader::hash(file.filepath())), flags(file.flags),
            size(file.size), time(file.time)
        {}

        cached_file_info(const cached_file_info&) = default;
        cached_file_info& operator=(const cached_file_info&) = default;

        // Compares in the a order that false evaluates faster
        bool operator==(const cached_file_info& rhs) const
        {
            return this->path_hash  == rhs.path_hash
                && this->time       == rhs.time
                && this->size       == rhs.size
                && this->flags      == rhs.flags;
            
        }

        //
        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(path_hash, flags, size, time);
        }
};

// Base caching
class data_cache : public modloader::basic_cache
{
    private:
        vfs<> fs;

    public:
        using cache_file_tuple = std::tuple<int, std::string, std::string>;

        // Initializes the caching system
        bool Startup()
        {
            if(modloader::basic_cache::Startup(location::localappdata))
            {
                if(get<0>(this->AddCacheFile("_STARTUP_", false)) != -1     // Creates /0/ directory
                && get<0>(this->AddCacheFile("_STARTUP_", true)) != -1)     // Creates /1/ directory
                {
                    // Setup a hook to delete the not used cache files after the loading screen, so we don't keep trash in there
                    using initialise_hook = injector::function_hooker<0x748CFB, void()>;
                    injector::make_static_hook<initialise_hook>([this](initialise_hook::func_type InitialiseGame)
                    {
                        InitialiseGame();
                        this->DeleteUnusedCaches();
                    });
                    return true;
                }
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

        std::string GetCachePath(const std::string& file, bool fullpath = true)
        {
            return basic_cache::GetCachePath(file, fullpath);
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
                for(int i = 1; i <= max_cache_dirs; ++i)
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
            for(size_t i = 0; i < cs.readme_point; ++i)
                cache2current[i] = -1;  // make sure cache2current has max(cs.readme_point, cs.cached_readme_point) indices mapped
            for(size_t i = 0; i < cs.cached_readme_point; ++i)
            {
                auto end_point = cs.listing.begin() + cs.readme_point;
                auto it = std::find(cs.listing.begin(), end_point, cs.cached_listing[i]);
                cache2current[i] = (it == end_point? -1 : std::distance(cs.listing.begin(), it));
            }
            
            auto fLoadStore = std::bind(&data_cache::LoadStore<store_list_type>, _1, _2, std::ref(cs.store), std::ref(cache2current));
            if(cereal_from_file_byfunc(GetCachePath(cs.cache_id, cs.fsfile + ".d"), fLoadStore))
            {
                return true;
            }
            return false;
        }

        // Caches the current data_store state and deletes the previous listing file ('cuz it was associated with another data_store).
        // Do not write the listing here directly because the merging process may fail and so we don't want a valid cache state on such case.
        template<class StoreType>
        bool WriteCachedStore_DataStore(caching_stream<StoreType>& cs)
        {
            using namespace std::placeholders;
            using store_list_type   = caching_stream<StoreType>::store_list_type;

            auto path = GetCachePath(cs.cache_id, cs.fsfile);
            auto result = cereal_to_file_byfunc(path + ".d",
                std::bind(&data_cache::SaveStore<store_list_type>, _1, _2, std::ref(cs.store), cs.readme_point)
              );
            DeleteFileA((path + ".l").c_str());

            return result;
        }

        // Caches the current file listing (timestamps and sizes)
        template<class StoreType>
        bool WriteCachedStore_Listing(caching_stream<StoreType>& cs)
        {
            using namespace std::placeholders;
            auto path = GetCachePath(cs.cache_id, cs.fsfile);
            return cereal_to_file_byfunc(path + ".l",
                std::bind(&data_cache::SerializeListing<decltype(cs.listing), cereal::BinaryOutputArchive>, _2, std::ref(cs.listing), std::ref(cs.readme_point)));
        }

    private: // Serialization specialization for store type

        /*
            The store object is saved and loaded from the cache in a different manner than cereal would do by default.
            [*] When saving, the block size of the following serialized data_store is saved, so they can be skipped on load
            [*] When loading, it does not load the entire serialized list of data stores but only those which have not been modified
        */

        // Saves the list of data stores 'store' and put the block size taken by each element before it
        template<class StoreList>
        static void SaveStore(std::ofstream& ss, cereal::BinaryOutputArchive& archive, StoreList& store, size_t readme_point)
        {
            using traits_type = typename StoreList::value_type::traits_type;
            traits_type::static_serialize(archive, true, [&]
            {
                archive(static_cast<cereal::size_type>(readme_point));

                for(size_t i = 0; i < readme_point; ++i)
                {
                    block_writer xblock(ss);
                    archive(store[i]);
                }
            });
        }

        // Loads the list of UNMODIFIED data stores into 'store'
        // The map cache2current maps indices from the cache into indices in the actual 'store'
        template<class StoreList>
        static void LoadStore(std::ifstream& ss, cereal::BinaryInputArchive& archive, StoreList& store, std::map<size_t, int>& cache2current)
        {
            using traits_type = typename StoreList::value_type::traits_type;
            traits_type::static_serialize(archive, false, [&]
            {
                cereal::size_type csize;
            
                // Notice: do not resize the store object, it is the object from caching_stream that passed tho Apply()
                // Just pick the size and use it in the loop to read from the serialized data
                archive(csize); // <--- cached readme point

                for(size_t i = 0, size = size_t(csize); i < size && !ss.fail(); ++i)
                {
                    block_reader xblock(ss);
                    int k = cache2current[i];
                    if(k == -1) // no association with the current store, skip this element
                        xblock.skip();
                    else
                        archive(store[k]);
                }
            });
        }

        // Serializes a listing of files into archive.
        // Readme point is how many normal files there is until we reach the readme files in the listing.
        template<class ListingList, class ArchiveType>
        static void SerializeListing(ArchiveType& archive, ListingList& listing, size_t& readme_point)
        {
            archive(readme_point, listing);
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
                uint32_t version = build_identifier(); // This variable won't change in the case of a output stream,
                                                                // but will in the case of a input stream, in any case default initialize for the output case
                archive_type archive(ss);
                archive(version);
                if(version == build_identifier())      // Make sure serialization version matches
                {
                    func(ss, archive);
                    return true;
                }
                else
                    plugin_ptr->Log("Warning: Incompatible cache version, a new cache will be generated.");
            }
            return false;
        }

    private:

        // Tries to add a cache file into the specified cache_id directory
        // You might force the success of the function at all cost by specifying 'force'
        // Returns a little handle for the cache, <0>=id, <1>=path, <2>=fullpath. On failure <0> is equal to -1.
        cache_file_tuple AddCacheFile(int cache_id, std::string file, bool force)
        {
            using modloader::plugin_ptr;
            using tuple_type = std::tuple <int, std::string, std::string>;

            std::string vdir, vpath;
            vdir.assign(std::to_string(cache_id)).push_back('/');   // virtual dir for the cache id
            vpath.assign(vdir).append(file);                        // virtual path for the file in the cache id

            if(force || fs.count(vpath) == 0)
            {
                if(fs.count(vdir) == 0) // if directory still doesn't exist create it
                {
                    if(this->CreateDir(vdir) == false)
                    {
                        // ...ops
                        plugin_ptr->Error("Fatal Error: std.data: Failed to create directory for caching: \"%s\"", vdir.c_str());
                        return cache_file_tuple(-1, "", "");
                    }
                    else
                        fs.add_file(vdir, "");  // make a representation in the vfs about the existence of our directory
                }

                // make a representation in the vfs about our cached file
                fs.add_file(vpath, "");

                return cache_file_tuple(
                    cache_id,
                    this->GetCachePath(cache_id, file, false),
                    this->GetCachePath(cache_id, file, true)
                    );
            }


            return cache_file_tuple(-1, "", "");
        }

        // Finds a cache that we can store the specified file (in the caching stream) trying to get the same place
        // as the previous cache run (not in the same game session!) did, so we can own it's cache.
        // Assumes this file is not unique (i.e. there can be various of the same name, like ipl files)
        // Returns a little handle for the cache, <0>=id, <1>=path, <2>=fullpath. On failure <0> is equal to -1.
        template<class StoreType>
        cache_file_tuple MatchNonUniqueCache(caching_stream<StoreType>& cs)
        {
            // TODO stop walking the tree every time, get around it (?)
            cache_file_tuple result(-1, "", "");
            modloader::FilesWalk(this->fullpath, "*.*", false, [&](modloader::FileWalkInfo& f)
            {
                if(f.is_dir)
                {
                    uint32_t cache_id;

                    try {
                        cache_id = std::stoul(f.filename);
                        if(cache_id == 0) return true;  // don't try with cache id 0, it's for unique files
                    }
                    catch(const std::exception&) {
                        return true;
                    }

                    auto tuple = MatchCache(cs, cache_id);
                    if(get<0>(tuple) != -1)
                    {
                        result = std::move(tuple);
                        return false;   // stop iteration, we are done!
                    }
                }
                return true;
            });
            return result;
        }

        // Tries to match a cache file in the specified caching directory 'cache_id' that may have been associated with the
        // caching stream in the previous game run
        // // Returns a little handle for the cache, <0>=id, <1>=path, <2>=fullpath. On failure <0> is equal to -1.
        template<class StoreType>
        cache_file_tuple MatchCache(caching_stream<StoreType>& cs, uint32_t cache_id)
        {
            // TODO stop walking the tree every time, get around it (?)
            using namespace modloader;
            using namespace std::placeholders;
            cache_file_tuple result(-1, "", "");
            modloader::FilesWalk(this->GetCacheDir(cache_id, true), "*.*", false, [&](modloader::FileWalkInfo& f)
            {
                if(!strcmp(f.filename, cs.fsfile.c_str(), false))
                {
                    // Reads the listing of files for this cache and tries to match it with the current listing
                    if(cereal_from_file_byfunc(std::string(f.filepath).append(".l"),
                        std::bind(&data_cache::SerializeListing<decltype(cs.cached_listing), cereal::BinaryInputArchive>, 
                        _2, std::ref(cs.cached_listing), std::ref(cs.cached_readme_point))))
                    {
                        if(cs.MatchListing())
                        {
                            result = this->AddCacheFile(cache_id, f.filename, true);
                            return false;   // stop iteration, we are done
                        }
                    }
                }
                return true;
            });
            return result;
        }

        // Deletes unused cache files left in the cache directory (i.e. garbage old caches)
        // This in fact just deletes the cache files that weren't used in the current session
        void DeleteUnusedCaches()
        {
            using namespace modloader;
            std::string vdir, vpath, filename;
            for(int cache_id = 0; cache_id <= max_cache_dirs; ++cache_id)
            {
                auto cachedir = this->GetCacheDir(cache_id, true);
                vdir.assign(std::to_string(cache_id)).push_back('/');

                // If this cache id has not even been used, delete it (if it even exists in the OS filesystem)
                if(fs.count(vdir) == 0)
                {
                    if(!IsPathA(cachedir.data()))
                        continue;
                    if(cache_id <= 1 || DestroyDirectoryA(cachedir.data()))
                        continue;
                }

                // Walk over this cache directory finding the unused cache files in it
                std::map<std::string, std::string> to_delete;       // <filename, fullpath>
                modloader::FilesWalk(cachedir, "*.*", false, [&](modloader::FileWalkInfo& f)
                {
                    if(!strcmp(f.filext, "d", false) || !strcmp(f.filext, "l", false))
                        filename.assign(f.filename, (f.filext - f.filename) - 1);   // use the filename without the .d and .l sufix
                    else
                        filename.assign(f.filename);                            // use this filename since it's not special sufixed

                    filename = NormalizePath(std::move(filename));
                    if(to_delete.count(filename) == 0)  // make sure we didn't add this file to the list yet (it may have a .l, .d, etc with same key)
                    {
                        if(fs.count(vpath.assign(vdir).append(filename)) == 0)
                            to_delete.emplace(filename, f.filepath);
                    }

                    return true;
                });

                // Delete the files we found out previosly
                for(auto& pair : to_delete)
                {
                    auto& path = pair.second;
                    DeleteFileA(path.data());
                    DeleteFileA((path + ".d").data());
                    DeleteFileA((path + ".l").data());
                }
            }
        }

};

// Stream of information for data_store caching
template<class StoreType>
class caching_stream
{
    public:
        friend class data_cache;

        // Stores file system information about a single data file
        struct info : public cached_file_info
        {
            protected:
                bool is_default = false;    // Is this data store a default one? (see data_store.hpp for the meaning of default in store context)
                bool is_readme  = false;    // Is readme file
                bool relpath    = false;    // Is this path relative to the current working directory?
                size_t linenum  = 0;        // When is_readme=true specifies in which line the data related to this is at

                friend class caching_stream;

            public:
                //
                info() = default;

                // Builds the information from a modloader file information
                info(const modloader::file& file, bool is_default, bool is_readme, size_t linenum) :
                    cached_file_info(file), is_default(is_default), is_readme(is_readme),
                    relpath(false), linenum(is_readme? linenum : 0)
                {}

                // Build the information from a standard filesystem attribute information
                info(const WIN32_FILE_ATTRIBUTE_DATA& attribs, bool is_default) :
                    cached_file_info(), is_default(is_default), is_readme(false),
                    relpath(true), linenum(0)
                {
                    this->path_hash = 0;
                    this->flags     = (attribs.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)? MODLOADER_FF_IS_DIRECTORY : 0;
                    this->size      = GetLongFromLargeInteger(attribs.nFileSizeLow, attribs.nFileSizeHigh);
                    this->time      = GetLongFromLargeInteger(attribs.ftLastWriteTime.dwLowDateTime, attribs.ftLastWriteTime.dwHighDateTime);
                }

                // Compare with another info to check if anything changed in the data file
                bool operator==(const info& rhs) const
                {
                    return cached_file_info::operator==(rhs)
                        && this->is_default == rhs.is_default
                        && this->is_readme  == rhs.is_readme
                        && this->relpath    == rhs.relpath
                        && this->linenum    == rhs.linenum;
                }

                // Serializer to load/save this type into a cache
                template <class Archive>
                void serialize(Archive& ar)
                {
                    ar(cereal::base_class<cached_file_info>(this), is_default, is_readme, relpath, linenum);
                }
        };

        // Aliases
        using listing_list_type = std::vector<std::pair<std::string, info>>;    // .first is file path, .second is file info
        using store_list_type   = std::vector<StoreType>;
        using readme_data_type  = std::list<std::pair<const modloader::file*, std::pair<size_t, std::reference_wrapper<const StoreType>>>>; // comes from data.hpp

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
        
        readme_data_type    readme_data;        // Data associated with readmes

        size_t cached_readme_point = -1;        // The point where readme files listing begins in 'cached_listing'
        size_t readme_point = -1;               // The point where readme files listing beings in 'listing'

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
                if(GetFileAttributesExA(path.c_str(), GetFileExInfoStandard, &attribs))
                {
                    listing.emplace(listing.end(), std::piecewise_construct,
                        std::forward_as_tuple(std::move(path)),
                        std::forward_as_tuple(attribs, is_default));
                }
            }
            return *this;
        }

        // Adds information about a mod loader file with path relative to the game directory
        caching_stream& AddFile(const modloader::file& file, bool is_default, bool is_readme, size_t linenum = 0)
        {
            using namespace modloader;
            if(IsPathA(file.fullpath().c_str()))
            {
                listing.emplace(listing.end(), std::piecewise_construct,
                    std::forward_as_tuple(file.filepath()),
                    std::forward_as_tuple(file, is_default, is_readme, linenum))->second;
            }
            return *this;
        }

        // Adds information about a data file with path relative to the game directory
        caching_stream& AddFile(const modloader::file& file, bool is_default)
        {
            return this->AddFile(file, is_default, false, 0);
        }

        // Adds information about a set of data present in a readme file that is related to this StoreType
        caching_stream& AddReadmeData(readme_data_type&& data)
        {
            if(data.size())
            {
                this->readme_data = std::move(data);
                for(auto& item : readme_data)
                    this->AddFile(*item.first, false, true, item.second.first);
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

                // put readmes files after normal files
                auto readme_point_it = std::stable_partition(this->listing.begin(), this->listing.end(), 
                    [](const listing_list_type::value_type& a)
                {
                    return !a.second.is_readme;
                });
                this->readme_point = std::distance(this->listing.begin(), readme_point_it);

                this->store.reserve(readme_point + 1);  // reserve one additional elem space for the readme store
                this->store.resize(readme_point);
                for(size_t i = 0; i < readme_point; ++i)
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
            return !(this->cached_listing == this->listing);    // yes ordering matters
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
            for(size_t i = 0; i < readme_point; ++i)
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

        // Builds an additional store that contains data related to readme files
        void MakeReadmeStore()
        {
            assert(this->store.capacity() > this->store.size());
            
            this->store.emplace_back(this->store.front());  // make it be equivalent to the default file
            auto& store = this->store.back();

            for(auto& r : this->readme_data)                // merge content from readme files into the store
                store.force_merge(r.second.second);

            store.set_as_ready(true);
            store.set_as_default(false);
        }

};
