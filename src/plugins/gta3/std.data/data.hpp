/*
 * Copyright (C) 2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#pragma once
#include <stdinc.hpp>

#include "vfs.hpp"
#include "cache.hpp"
using boost::optional;

// Type of config file identifier (see files_behv_t)
using Type = std::uint8_t;

// Basical maskes
static const uint64_t hash_mask_base  = 0xFFFFFFFF;
static const uint64_t type_mask_base  = 0xFF;                 // Mask for type without any shifting
static const uint32_t type_mask_shf   = 32;                   // Takes 8 bits, starts from 33th bit because first 32th bits is a hash

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

// Gets the type of a behaviour
inline Type GetType(uint64_t mask)
{
    return modloader::file::get_mask<uint8_t>(mask, type_mask_base, type_mask_shf);
}


// Helper monoid
template<class StoreType>
using maybe_readable = maybe<maybe<StoreType>>;


/*
 *  The plugin object
 */
class DataPlugin : public modloader::basic_plugin
{
    public: // Mod Loader Callbacks

        friend void ProcessGtaDatEntries();

        const info& GetInfo();

        bool OnStartup();
        bool OnShutdown();
        int GetBehaviour(modloader::file&);
        bool InstallFile(const modloader::file&);
        bool ReinstallFile(const modloader::file&);
        bool UninstallFile(const modloader::file&);
        void Update();

    protected:  // Plugin stuff, variables, etc

        // Base for line_data, the base is serializable
        struct line_data_base
        {
            uint32_t                        line_number = -1;// The line number this data is related to
            maybe<size_t>                   merger_hash;  // The hash of the merger this data is related to (or nothing if no related merger yet)
            either<std::string, boost::any> data;         // Data related to the line, either the original string or the data_store (wrapped in a boost::any)
            std::type_index                 owner;        // The StoreType that owns this data (when it points to typeid(void) means no owner)

            line_data_base() : owner(typeid(void)) {}
            line_data_base(maybe<size_t> merger_hash, either<std::string, boost::any> data, size_t line_number, std::type_index owner)
                : merger_hash(std::move(merger_hash)), data(std::move(data)), line_number(line_number), owner(owner)
            {}
            
            // Too lazy to implement those
            //line_data_base(const line_data_base&) = delete;
            //line_data_base(line_data_base&&) = delete;
            //line_data_base& operator=(const line_data_base&) = delete;
            //line_data_base& operator=(line_data_base&&) = delete;

            bool has_owner() const { return owner != typeid(void); }

            //
            template<class Archive>
            void serialize(Archive& archive)
            {
                archive(owner, line_number, merger_hash, data);
            }
        };

        // Line data, represents one piece of queryable important data in one line (from a readme)
        struct line_data : public line_data_base
        {
            const modloader::file&  file;   // File related to this line data
            
            line_data() = delete;

            line_data(const modloader::file& file, maybe<size_t> merger_hash, either<std::string, boost::any> data,
                size_t line_number, std::type_index owner)
                : line_data_base(merger_hash, std::move(data), line_number, owner), file(file)
            {}

            // Too lazy to implement those
            line_data(const line_data&) = delete;
            line_data(line_data&& rhs) = delete;
            line_data& operator=(const line_data&) = delete;
            line_data& operator=(line_data&&) = delete;

            // Gets the reference to our base which can be serialized
            const line_data_base& base() const
            { return *this; }
        };

        // Stores an unique identifier of readme reader
        // This should be used only and only for checking in the serialized format that the readers match
        struct readme_reader_magic_t
        {
            size_t          magic;      // Magic (compilation time) for the specific reader
            std::type_index type;       // StoreType related to it. Must be here (and serialized together the magic) because of the way cereal works
                                        // it does backreferencing to the first type_index of that type found (RTTI-only) and so we need to store
                                        // a first time reference that is not skippable

            readme_reader_magic_t() :  // Default constructor for cereal
                magic(0), type(typeid(void))
            {}

            readme_reader_magic_t(size_t magic, std::type_index type) :
                magic(magic), type(type)
            {}

            // Checks only for the magic, not necessary to check for the type
            bool operator==(const readme_reader_magic_t& rhs) const
            { return this->magic == rhs.magic; }

            template<class Archive>
            void serialize(Archive& archive)
            {
                archive(magic, type);
            }
        };

        struct files_behv_t
        {
            size_t      hash;       // Hash of this kind of file
            bool        canmerge;   // Can many files of this get merged into a single one?
            Type        index;      // Index of this behv... don't use this index to access myself in vbehav[], it mayn't be the same
        };


        using readme_file_info      = cached_file_info;
        using readme_data_store     = std::vector<std::vector<line_data_base>>;
        using readme_listing_type   = std::vector<readme_file_info>;

        // Stores a virtual file system which contains the list of data files we got
        vfs<const modloader::file*> fs;

        // Overriders
        std::map<size_t, modloader::file_overrider> ovmap;        // Map of files overriders and mergers associated with their handling file names hashes
        std::set<modloader::file_overrider*>        ovrefresh;    // Set of mergers to be refreshed on Update() 

        // Info
        std::vector<files_behv_t> vbehav;

        // Temporary buffer used for parsing readmes
        std::unique_ptr<char[]> readme_buffer;

        // stores readme handlers
        using read_handler = std::function<maybe<size_t>(const modloader::file&, const std::string&, either<uint32_t, line_data*>)>;
        std::unordered_multimap<std::type_index, read_handler> readers;

        // Set of readme files that needs to be installed/uninstalled
        linear_map<const modloader::file*, int /*dummy*/> readme_toinstall;
        linear_map<const modloader::file*, int /*dummy*/> readme_touninstall;
        
        // Magic of all the readme readers registered
        std::vector<readme_reader_magic_t> readme_magics;
        
        // List of possibily useful data or lines in readme files
        linear_map<const modloader::file*, std::list<line_data>> maybe_readme;
        using maybe_readme_type = decltype(maybe_readme);

        std::map<std::type_index, const char*> storetype2what;

        bool had_cached_readme = false;
        bool changed_readme_data = false;

    private: // Effective methods

        bool InstallFile(const modloader::file&, size_t merger_hash, std::string fspath, std::string fullpath, bool isreinstall = false);
        bool ReinstallFile(const modloader::file&, size_t merger_hash);
        bool UninstallFile(const modloader::file&, size_t merger_hash, std::string fspath);

        void UpdateReadmeState();
        void InstallReadme(const modloader::file&);
        void InstallReadme(const std::set<size_t>&);
        void UninstallReadme(const modloader::file&);

        std::set<size_t> ParseReadme(const modloader::file&);
        std::set<size_t> ParseReadme(const modloader::file&, std::pair<const char* /*begin*/, const char* /*end*/>);
        
        // Before the game startups we shouldn't write a readme cache, because during the loading screen it's the time
        // the readme query turns strings into data stores, so we only want to save when we have the data stores :)
        bool MayWriteReadmeCache() { return !!this->loader->has_game_loaded; }

        // Cached readme I/O
        bool VerifyCachedReadme(std::ifstream& ss, cereal::BinaryInputArchive& archive);
        readme_listing_type ReadCachedReadmeListing();
        readme_data_store   ReadCachedReadmeStore();
        void WriteReadmeCache();

    public:
        // Caching stuff
        data_cache cache;

        // Game states
        bool has_model_info = false;


        // stores references to queried readme data
        template<class StoreType> // shouldn't contain any data.hpp specific type since it'll be sent over cache.hpp
        using readme_data_list = std::list<std::pair<const modloader::file*,
                                            std::pair<uint32_t, std::reference_wrapper<const StoreType>>>>;

    public:

        // Finds out the hash for the merger related to the specified StoreType
        template<class StoreType>
        static size_t GetMergerHash()
        {
            static const auto hash = modloader::hash(StoreType::traits_type::dtraits::datafile());
            return hash;
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

        // Adds a behaviour type
        void AddBehv(size_t hash, bool canmerge)
        {
            vbehav.emplace_back(files_behv_t { hash, canmerge, vbehav.size()+1 });
            if(vbehav.size() >= type_mask_base) throw std::logic_error("type_mask_base too small");
        }

        // Finds a behaviour type
        files_behv_t* FindBehv(size_t hash)
        {
            for(auto& item : this->vbehav)
                if(item.hash == hash) return &item;
            return nullptr;
        }

        // Finds a behaviour type
        files_behv_t* FindBehv(const modloader::file& f)
        {
            return FindBehv(f.hash);
        }

        // Finds a behaviour type
        files_behv_t* FindBehv(std::string fname)
        {
            return FindBehv(modloader::hash(fname));
        }


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

            AddBehv(hash, true);
            return ov;
        }

        template<class detour_type>
        modloader::file_overrider& AddIplOverrider(std::string fsfile, bool unique, bool samefile, bool complete_path,
                                              const modloader::file_overrider::params& params)
        {
            fsfile = modloader::NormalizePath(std::move(fsfile));
            auto hash = modloader::hash(fsfile);

            auto& ov = ovmap.emplace(std::piecewise_construct,
                std::forward_as_tuple(hash),
                std::forward_as_tuple(tag_detour, params, detour_type())
                ).first->second;

            for(size_t i = 0; i < ov.NumInjections(); ++i)
            {
                // Merges data whenever necessary to open this file. Caching can happen.
                auto& d = static_cast<detour_type&>(ov.GetInjection(i));
                d.OnTransform(std::bind(&DataPlugin::GetIplFile, this, _1, fsfile, unique, samefile, complete_path));
            }

            // Replaces standard OnHook with this one, which just makes the call no setfile (since many files)
            // This gets called during InstallFile/ReinstallFile/UninstallFile
            ov.OnHook([&ov](const modloader::file*)
            {
                for(size_t i = 0; i < ov.NumInjections(); ++i)
                    static_cast<detour_type&>(ov.GetInjection(i)).make_call();
            });

            AddBehv(hash, true);
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

            AddBehv(hash, false);
            return ov;
        }


        // Adds a readme data which contains no line data at all
        void AddDummyReadme(const modloader::file& file)
        {
            this->changed_readme_data = true;
            maybe_readme[&file];
        }

        // Adds a readme data related to the specified 'file' into the query
        void AddReadmeData(const modloader::file& file, maybe<size_t> merger_hash, either<std::string, boost::any> data,
            size_t line_number, std::type_index owner)
        {
            this->changed_readme_data = true;
            maybe_readme[&file].emplace_back(file, merger_hash, std::move(data), line_number, owner);
        }

        // Adds a readme data related to the specified 'file' into the query (from unserialization)
        void AddReadmeData(const modloader::file& file, line_data_base&& base)
        {
            this->changed_readme_data = true;
            maybe_readme[&file].emplace_back(file, base.merger_hash, std::move(base.data), base.line_number, base.owner);

            if(auto* xline = get<std::string>(&base.data))
                this->LogAboutReadmeLineCached(file, *xline, base.line_number, base.owner);
            else
                this->LogAboutReadmeLineCached(file, nothing, base.line_number, base.owner);
        }

        // Adds a readme data related to the specified 'file' into the query (from unserialized)
        // Outputs a set of mergers related to the data in the readme
        std::set<size_t> AddReadmeData(const modloader::file& file, std::vector<line_data_base>&& list)
        {
            std::set<size_t> mergers;
            for(auto& x : list)
            {
                if(x.merger_hash) mergers.emplace(x.merger_hash.get());
                AddReadmeData(file, std::move(x));
            }
            list = std::vector<line_data_base>();
            return mergers;
        }

        // Remove all the content related to this readme file
        void RemoveReadmeData(const modloader::file& file)
        {
            this->changed_readme_data = true;
            this->maybe_readme.erase(&file);
        }
        




        // Adds a readme handler that is related to the specified StoreType
        // The handler takes a string as parameter (the line) and returns a maybe specifying whether the line is handlable by the store type.
        //   If the handler returns a maybe which contains nothing, it means the line has nothing do with this StoreType
        //   If the handler returns a maybe which contains a maybe<StoreType>, it means the line may have something to do with this StoreType
        //   If the handler returns a maybe which contains a StoreType, it means the line have something to do with this StoreType and it know what it is.
        template<class StoreType>
        void AddReader(std::function<maybe_readable<StoreType>(const std::string&)> reader)
        {
            using store_type  = StoreType;
            using traits_type = typename StoreType::traits_type;

            static_assert(cereal::has_rtti<StoreType>::value, "Missing RTTI for StoreType, use REGISTER_RTTI_FOR_ANY macro");


            readme_magics.emplace_back(build_identifier(), typeid(StoreType));
            storetype2what[typeid(StoreType)] = StoreType::traits_type::dtraits::what();

            AddReaderTypeErased(typeid(StoreType),
                [=](const modloader::file& file, const std::string& line, either<uint32_t, line_data*> ref) -> maybe<size_t>
            {
                assert(!empty(ref));
                if(auto maybe_readable = reader(line))
                {
                    auto& maybe_store = maybe_readable.get();
                    if(maybe_store)
                    {
                        // yay we have found a merger for it, remember about it universe!
                        auto merger_hash  = GetMergerHash<StoreType>();
                        if(auto* linenum = get<size_t>(&ref))
                        {
                            // it seems it's the first time we read this line (and we already know it's handler!)
                            this->LogAboutReadmeLine<StoreType>(file, line, *linenum);
                            this->AddReadmeData(file, merger_hash, boost::any(std::move(maybe_store.get())), *linenum, typeid(StoreType));
                        }
                        else if(auto* refx = get<line_data*>(&ref))
                        {
                            // not the first time we read this line, just modify the existing data
                            this->LogAboutReadmeLine<StoreType>(file, line, (*refx)->line_number);
                            (*refx)->merger_hash = merger_hash;
                            (*refx)->owner = typeid(StoreType);
                            (*refx)->data = boost::any(std::move(maybe_store.get()));
                            this->changed_readme_data = true;
                        }
                        else
                            assert(false);
                        return merger_hash;
                    }
                    else
                    {
                        // if it's the first time we read this line, register it, otherwise no need it's already registered
                        if(auto* linenum = get<size_t>(&ref))
                        {
                            this->LogAboutReadmeLine(file, line, *linenum);
                            this->AddReadmeData(file, nothing, line, *linenum, typeid(void));
                        }
                        return nothing; // we may have found a merger for it, but we aren't sure atm, try later
                    }
                }
                return nothing; // didn't found a merger for it
            });
        }

        // Queries all the data related to the specified StoreType that is present in readme files
        // Takes the lines already assigned to the StoreType and tries to assign unassigned lines to the StoreType
        template<class StoreType>
        readme_data_list<StoreType> QueryReadmeData()
        {
            readme_data_list<StoreType> list;
            for(auto& fdata : this->maybe_readme)
            {
                for(auto& line : fdata.second)
                {
                    if(auto maybe = apply_visitor(QueryReadmeDataVisitor<StoreType>(line), line.data))
                        list.emplace_back(std::move(maybe.get()));
                }
            }
            return list;
        }


    private:
        
        // Type erasion for AddReader
        void AddReaderTypeErased(const std::type_index& store_type, read_handler reader)
        {
            readers.emplace(store_type, std::move(reader));
        }

        // Logs about the finding of a readme line directly related to a specific store type
        template<class StoreType>
        void LogAboutReadmeLine(const modloader::file& file, maybe<const std::string&> line, size_t line_number)
        {
            return LogAboutReadmeLineInternal(file, line, line_number, "", StoreType::traits_type::dtraits::what());
        }

        // Logs about the finding of a readme line not related to any store type YET
        void LogAboutReadmeLine(const modloader::file& file, maybe<const std::string&> line, size_t line_number)
        {
            return LogAboutReadmeLineInternal(file, line, line_number, "", nullptr);
        }

        // Logs about the finding of a readme line directly related to a specific store 'type' or just a possible data if it's void
        void LogAboutReadmeLineCached(const modloader::file& file, maybe<const std::string&> line, size_t line_number, std::type_index type = typeid(void))
        {
            return LogAboutReadmeLineInternal(file, line, line_number, "cached ", (type != typeid(void)? storetype2what[type] : nullptr));
        }

        //
        void LogAboutReadmeLineInternal(const modloader::file& file, maybe<const std::string&> line, size_t line_number, const char* descrp = "", const char* what = nullptr)
        {
            if(what)
            {
                if(line)
                    this->Log("Found %s%s line for '%s':%u: %s",
                        descrp, what, file.filepath(), line_number, line.get().c_str());
                else
                    this->Log("Found %s%s line for '%s':%u",
                        descrp, what, file.filepath(), line_number);
            }
            else
                return LogAboutReadmeLineInternal(file, line, line_number, descrp, "possible data");
        }


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

            auto readme_data = traits_type::query_readme_data<StoreType>(filename);

            if(count == 1 && readme_data.empty()) // only one file, so let's just override
                return range.first->second.first;
            else if(count >= 2 || !readme_data.empty())   // any file to merge? we need at least 2 files to be able to do merging
            {
                auto fsfile = filename;
                caching_stream<StoreType> cs(fsfile, unique);
                
                // Add data files we'll work on to the caching stream
                cs.AddFile(file.c_str(), true);
                cs.AddReadmeData(std::move(readme_data));
                for(auto it = range.first; it != range.second; ++it)
                {
                    const modloader::file& f = *(it->second.second);
                    cs.AddFile(f, false);
                }

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
                
                // Rewrite the cached store... Notice we write only the data_store (.d) file on here
                // That's because we cannot do so after the merge since the data store states can have changed (damn side effects)
                bool allow_listing = false;
                if(traits_type::can_cache)
                {
                    if(!cache.WriteCachedStore_DataStore(cs))
                        Log("Warning: Could not write cache at '%s.#'", cs.Path().c_str());
                    else
                        allow_listing = true;
                }

                // Merge all the stored data into a single data file
                cs.MakeReadmeStore();
                if(gta3::merge_to_file<store_type>(cs.FullPath().c_str(), cs.StoreList().begin(), cs.StoreList().end(), traits_type::domflags_fn()))
                {
                    if(allow_listing) cache.WriteCachedStore_Listing(cs);
                    return cs.Path();
                }
                else
                {
                    plugin_ptr->Log("Warning: Failed to merge (%s) data files into \"%s\"", what, cs.Path().c_str());
                }
            }

            return std::string();  // use default file
        }
        
        // Used for IPL merger to not merge IPLs, hax
        std::string GetIplFile(std::string file, std::string fsfile, bool unique, bool samefile, bool complete_path)
        {
            using namespace modloader;
            auto filename     = modloader::NormalizePath(GetPathComponentBack(file));

            // Make sure filename matches if samefile has been specified
            if(samefile && filename != fsfile)
                return std::string(); // use default file

            auto range    = this->fs.files_at(complete_path? file : fsfile);
            auto count    = std::distance(range.first, range.second);

            if(count > 0)
            {
                if(count > 1)
                    plugin_ptr->Log("Warning: More than one file attached to '%s', using the one with higher priority.", (complete_path? file : fsfile).c_str());
                auto it = range.first;
                std::advance(it, count - 1);
                return it->second.first;
            }

            return std::string();  // use default file
        }


        // Queries a reference to the data in a either<string, any> object if it matches the type StoreType
        template<class StoreType>
        struct QueryReadmeDataVisitor : either_static_visitor<maybe<typename readme_data_list<StoreType>::value_type>>
        {
            using maybe_type = maybe<typename readme_data_list<StoreType>::value_type>; // output type
            line_data& ref; // info about the line we are dealing with

            QueryReadmeDataVisitor(line_data& ref) : ref(ref)
            {}
            
            // Check out if this any is an store of our type, if yeah, return it's reference
            maybe_type operator()(const boost::any& some_store) const
            {
                 if(some_store.type() == typeid(StoreType))
                    return std::make_pair(&ref.file, std::make_pair(ref.line_number, std::cref(*boost::any_cast<StoreType>(&some_store))));
                return nothing;
            }

            // Send this string to all the handlers related to this type and see if we can match it
            maybe_type operator()(const std::string& line) const
            {
                auto range = plugin_ptr->cast<DataPlugin>().readers.equal_range(typeid(StoreType));
                for(auto it = range.first; it != range.second; ++it)
                {
                    if(it->second(ref.file, line, &ref))
                        return (*this)(get<boost::any>(ref.data));
                }
                return nothing;
            }

            maybe_type operator()(const either_blank& slice) const
            { throw std::invalid_argument("blank type"); }
        };

};


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


/*
 *  data_traits
 *      Extended traits from gta3::data_traits, adding stuff handled by std.data itself instead of datalib.
 *      
 *      Additional stuff:
 *
 *          static const bool can_cache         -> Can this store get cached?
 *          static const bool is_reversed_kv    -> Does the key contains the data instead of the value in the key-value pair?
 *          static const bool has_sections      -> Does this data file contains sections?
 *          static const bool per_line_section  -> Does the sections of this data file different on each line?
 *
 *          [optional] void static_serialize(Archive, IsSaving, Functor)
 *                                              -> Serializes the static data of a data_traits.
 *                                                 The Archive argument is a reference to the serializer (call it's operator()() to serialize something)
 *                                                 and the Functor is a function that serializes the content of a list of stores (should usually be done)
 *
 *          [optional] List query_readme_data(NormalizedFileName)
 *                                              -> Called to query data stores containing data for the specified store from readme lines
 *                                                 NormalizedFileName is the filename of the data file we are dealing with.
 *
 *
 */
struct data_traits : public gta3::data_traits
{
    static const bool is_ipl_merger = false;

    template<class Archive, class FuncT>
    static void static_serialize(Archive& archive, bool saving, FuncT serialize_store)
    {
        serialize_store();
    }

    template<class StoreType>
    static DataPlugin::readme_data_list<StoreType> query_readme_data(const std::string& filename)
    {
        return plugin_ptr->cast<DataPlugin>().QueryReadmeData<StoreType>();
    }

    // make setbyline output a error on failure
    template<class StoreType, typename TData>
    static bool setbyline(StoreType& store, TData& data, const gta3::section_info* section, const std::string& line, bool allowlog = true)
    {
        if(!gta3::data_traits::setbyline(store, data, section, line))
        {
            if(allowlog) plugin_ptr->Log("Warning: Failed to parse data line: %s", line.c_str());
            return false;
        }
        return true;
    }
};




// Global const params vars for AddMerger/AddDetour
static const auto no_reinstall          = modloader::file_overrider::params(nullptr);
static const auto reinstall_since_start = modloader::file_overrider::params(true, true, true, true);
static const auto reinstall_since_load  = modloader::file_overrider::params(true, true, false, true);


// Makes for example "folder1/folder2/data/a.ipl" turn into "data/a.ipl"
inline std::string find_gta_path(std::string path)
{
    using namespace modloader;
    static const auto data = MakeSureStringIsDirectory(NormalizePath("data/"));
    return GetProperlyPath(std::move(path), data.c_str());
}

// Checks whether model names and other IDE information have been already loaded
inline bool HasModelInfo()
{
    return modloader::plugin_ptr->cast<DataPlugin>().has_model_info;
}

// Tries to find the model info related to the modelname
inline void* MatchModelString(const char* modelname, int* out = nullptr)
{
    return injector::cstd<void*(const char*, int*)>::call<0x4C5940>(modelname, out);
}

// Tries to find the model info related to the modelname
inline void* MatchModelString(const std::string& modelname, int* out = nullptr)
{
    return MatchModelString(modelname.c_str(), out);
}

