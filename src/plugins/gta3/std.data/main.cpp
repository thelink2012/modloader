/*
 * Copyright (C) 2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#include <stdinc.hpp>
#include "data.hpp"
using namespace modloader;

/*
 *  !!!! data_traits important implementation information !!!!
 *
 *   [*] It's better that each data trait (i.e. for each file type) be contained in it's own cpp, see next * for a reason
 *   [*] Note data.hpp / cache.hpp should not be included in a PCH but in the data traits cpp, that because the magic for each cache
 *       is the hash of the compilation time of the trait translation unit, that means it is a static (by unit) function in cache.hpp
 *       meaning including it by PCH (stdinc.hpp) would defeat it purposes of one hash for each translation unit for each time it compiles.
 *
 *   [*] Remember the order of stuff in the either<> object matters, it'll try to match the first type, then the second, and so on.
 *       So be warned because e.g. either<string, float> will always match the string, use either<float, string> instead!
 *
 *       
 *   [*] Remember not to use int8, uint8 and so in the data_slice<> thinking it is a integer type, instead it will be readen as a character
 */

void LazyGtaDatPatch();

DataPlugin plugin;
REGISTER_ML_PLUGIN(::plugin);

CEREAL_REGISTER_RTTI(void); // for DataPlugin::line_data_base


/*
 *  DataPlugin::GetInfo
 *      Returns information about this plugin 
 */
const DataPlugin::info& DataPlugin::GetInfo()
{
    static const char* extable[] = { "dat", "cfg", "ide", "ipl", "zon", "txt", 0 };
    static const info xinfo      = { "std.data", get_version_by_date(), "LINK/2012", 54, extable };
    return xinfo;
}


/*
 *  DataPlugin::OnStartup
 *      Startups the plugin
 */
bool DataPlugin::OnStartup()
{
    if(gvm.IsSA())
    {
        this->readme_magics.reserve(20);

        // Initialise the caching
        if(!cache.Startup())
            return false;

        // Initialises all the merges and overrides (see 'data_traits/' directory for those)
        for(auto& p : initializer::list())
            p->initialise(this);

        // Installs the hooks in any case, so we have the log always logging the loading of data files
        for(auto& pair : this->ovmap)
            pair.second.InstallHook();

        // Makes default.dat/gta.dat load in a lazy way
        LazyGtaDatPatch();

        // Hook allowing us to know when we are ready to know about the model names of the game
        using modelinfo_hook =  function_hooker<0x5B922F, void()>;
        make_static_hook<modelinfo_hook>([this](modelinfo_hook::func_type MatchAllModelStrings)
        {
            this->has_model_info = true;
            return MatchAllModelStrings();
        });

        // Hook after the loading screen to write a readme cache
        using initialise_hook = injector::function_hooker<0x748CFB, void()>;
        make_static_hook<initialise_hook>([this](initialise_hook::func_type InitialiseGame)
        {
            InitialiseGame();
            if(this->changed_readme_data)
            {
                // If we have a empty list of readme data and we previosly had a cached readme, overwrite it with empty data
                // In the case the list is not empty, overwrite with new data
                if(!this->maybe_readme.empty() || this->had_cached_readme)
                    this->WriteReadmeCache();
            }
        });

        // When there's no cache present mark changed_readme_data as true because we'll need to generate a cache
        this->had_cached_readme   = IsPathA(cache.GetCachePath("readme.ld").data());;
        this->changed_readme_data = !had_cached_readme;

        return true;
    }
    return false;
}

/*
 *  DataPlugin::OnShutdown
 *      Shutdowns the plugin
 */
bool DataPlugin::OnShutdown()
{
    cache.Shutdown();
    return true;
}


/*
 *  DataPlugin::GetBehaviour
 *      Gets the relationship between this plugin and the file
 */
int DataPlugin::GetBehaviour(modloader::file& file)
{
    static const files_behv_t* ipl_behv = FindBehv(ipl_merger_name);
    static const files_behv_t* ide_behv = FindBehv(ide_merger_name);

    // Setups the behaviour of a file based on the specified behv object (which can be null for none)
    // Each specific behv should have a unique identifier, for mergable files the filepath is used to identify
    // the file (as many files of the same name can come at us) otherwise the filename is used as an identifier.
    auto setup_behaviour = [](modloader::file& file, const files_behv_t* behv)
    {
        if(behv)
        {
            file.behaviour = behv->canmerge?
                SetType(modloader::hash(file.filepath()), behv->index) :
                SetType(file.hash, behv->index);    // filename hash
            return true;
        }
        return false;
    };

    if(file.is_ext("txt"))
    {
        return MODLOADER_BEHAVIOUR_CALLME;
    }
    else if(file.is_ext("ide"))
    {
        if(setup_behaviour(file, ide_behv))
            return MODLOADER_BEHAVIOUR_YES;
    }
    else if(file.is_ext("ipl") || file.is_ext("zon"))
    {
        if(setup_behaviour(file, ipl_behv))
            return MODLOADER_BEHAVIOUR_YES;
    }
    else if(setup_behaviour(file, FindBehv(file)))
        return MODLOADER_BEHAVIOUR_YES;

    return MODLOADER_BEHAVIOUR_NO;
}

/*
 *  DataPlugin::InstallFile
 *      Installs a file using this plugin
 */
bool DataPlugin::InstallFile(const modloader::file& file)
{
    if(file.is_ext("txt"))
    {
        this->readme_toinstall.emplace(&file, 0);
        this->readme_touninstall.erase(&file);
        return true;
    }
    else
    {
        static const files_behv_t* ipl_behv = FindBehv(ipl_merger_name);
        static const files_behv_t* ide_behv = FindBehv(ide_merger_name);

        auto type = GetType(file.behaviour);
        if((ipl_behv && type == ipl_behv->index) || (ide_behv && type == ide_behv->index))
        {
            auto hash = (type == (ipl_behv? ipl_behv->index : -1)? ipl_merger_hash : ide_merger_hash);
            return this->InstallFile(file, hash, find_gta_path(file.filedir()), file.filepath());
        }
        else
        {
            return this->InstallFile(file, file.hash, file.filename(), file.filepath());
        }
    }
    return false;
}


/*
 *  DataPlugin::ReinstallFile
 *      Reinstall a file previosly installed that has been updated
 */
bool DataPlugin::ReinstallFile(const modloader::file& file)
{
    if(file.is_ext("txt"))
    {
        this->readme_touninstall.emplace(&file, 0);
        this->readme_toinstall.emplace(&file, 0);
        return true;
    }
    else
    {
        static const files_behv_t* ipl_behv = FindBehv(ipl_merger_name);
        static const files_behv_t* ide_behv = FindBehv(ide_merger_name);

        auto type = GetType(file.behaviour);
        if((ipl_behv && type == ipl_behv->index) || (ide_behv && type == ide_behv->index))
        {
            auto hash = (type == (ipl_behv? ipl_behv->index : -1)? ipl_merger_hash : ide_merger_hash);
            return this->ReinstallFile(file, hash);
        }
        else
        {
            return this->ReinstallFile(file, file.hash);
        }
    }
    return true; // Avoid catastrophical failure
}

/*
 *  DataPlugin::UninstallFile
 *      Uninstall a previosly installed file
 */
bool DataPlugin::UninstallFile(const modloader::file& file)
{
    if(file.is_ext("txt"))
    {
        this->readme_touninstall.emplace(&file, 0);
        this->readme_toinstall.erase(&file);
        return true;
    }
    else
    {
        static const files_behv_t* ipl_behv = FindBehv(ipl_merger_name);
        static const files_behv_t* ide_behv = FindBehv(ide_merger_name);

        auto type = GetType(file.behaviour);
        if((ipl_behv && type == ipl_behv->index) || (ide_behv && type == ide_behv->index))
        {
            auto hash = (type == (ipl_behv? ipl_behv->index : -1)? ipl_merger_hash : ide_merger_hash);
            return this->UninstallFile(file, hash, find_gta_path(file.filedir()));
        }
        else
        {
            return this->UninstallFile(file, file.hash, file.filename());
        }
    }
    return false;
}

/*
 *  DataPlugin::Update
 *      Updates the state of this plugin after a serie of install/uninstalls
 */
void DataPlugin::Update()
{
    plugin_ptr->Log("Updating %s state...", this->data->name);

    bool has_readme_changes = (this->readme_toinstall.size() || this->readme_touninstall.size());

    // Perform the Update of readmes before refreshing!
    if(has_readme_changes)
        this->UpdateReadmeState();

    // Refresh every overriden of multiple files right here
    // Note: Don't worry about this being called before the game evens boot up, the ov->Refresh() method takes care of it
    for(auto& ov : this->ovrefresh)
    {
        if(!ov->Refresh())
            plugin_ptr->Log("Warning: Failed to refresh some data file.");   // very useful warning indeed
    }
    this->ovrefresh.clear();

    // Free up the temporary readme_buffer that may have been allocated in ParseReadme()
    this->readme_buffer.reset();

    // If anything changed in the readmes state (i.e. installed, removed or reinstalled a readme)
    // then rewrite it's cache
    if(has_readme_changes)
    {
        if(this->MayWriteReadmeCache()) // write caches on Update() only if we did pass tho the loading screen
            this->WriteReadmeCache();
    }

    plugin_ptr->Log("Done updating %s state.", this->data->name);
}


///////////////////////////


/*
 *  DataPlugin::InstallFile (Effectively)
 *      Installs a file assuming it's Behv and/or Merger hash to be 'merger_hash', 'fspath' the file's path in our virtual filesystem and
 *      fullpath the actual absolute path to the file in disk.
 *
 *      The 'isreinstall' parameter is a helper (defaults to false) to allow both install and reinstall in the same function;
 *      NOTE: fspath and fullpath can safely be empty when isreinstall=true
 */
bool DataPlugin::InstallFile(const modloader::file& file, size_t merger_hash, std::string fspath, std::string fullpath, bool isreinstall)
{
    // NOTE: fspath and fullpath can safely be empty when isreinstall=true

    if(FindBehv(merger_hash)->canmerge)
    {
        auto m = FindMerger(merger_hash);
        if(m->CanInstall())
        {
            if(!isreinstall)
                fs.add_file(std::move(fspath), std::move(fullpath), &file);

            // Delay in-game installs to go through DataPlugin::Update()
            ovrefresh.emplace(m);
            return true;
        }
    }
    else
    {
        if(!isreinstall)
            return FindMerger(merger_hash)->InstallFile(file);
        else
            return FindMerger(merger_hash)->ReinstallFile();
    }
    return false;
}

/*
 *  DataPlugin::ReinstallFile (Effectively)
 *      Reinstalls a file assuming it's Behv and/or Merger hash to be 'merger_hash'
 */
bool DataPlugin::ReinstallFile(const modloader::file& file, size_t merger_hash)
{
    // Just forward the call to InstallFile. It does not need the fspath and fullpath parameter on reinstall.
    if(!this->InstallFile(file, merger_hash, "", "", true))
        return true;    // Avoid catastrophical failure
    return true;
}

/*
 *  DataPlugin::UninstallFile (Effectively)
 *      Uninstalls a file assuming it's Behv and/or Merger hash to be 'merger_hash', 'fspath' the file's path in our virtual filesystem and
 *      fullpath the actual absolute path to the file in disk.
 */
bool DataPlugin::UninstallFile(const modloader::file& file, size_t merger_hash, std::string fspath)
{
    if(FindBehv(merger_hash)->canmerge)
    {
        auto m = FindMerger(merger_hash);
        if(m->CanUninstall())
        {
            // Removing it from our virtual filesystem and possibily refreshing should do it
            if(fs.rem_file(std::move(fspath), &file))
            {
                ovrefresh.emplace(m);
                return true;
            }
        }
    }
    else
    {
        return FindMerger(merger_hash)->UninstallFile();
    }
    return false;
}

/*
 *  DataPlugin::InstallReadme
 *      Makes sure the specified mergers get refreshed
 */
void DataPlugin::InstallReadme(const std::set<size_t>& mergers)
{
    for(auto merger_hash : mergers)
    {
        auto m = FindMerger(merger_hash);
        if(m && m->CanInstall())
            ovrefresh.emplace(m);
    }
}

/*
 *  DataPlugin::UninstallReadme
 *      Uninstalls a readme file which may previosly gave us some data lines
 */
void DataPlugin::UninstallReadme(const modloader::file& file)
{
    // Finds all the mergers related to the specified readme file
    // and signals the updater that they should be refreshed
    for(auto& data : this->maybe_readme[&file])
    {
        if(data.merger_hash)   // maybe has a merger associated with this data?
        {
            auto m = FindMerger(data.merger_hash.get());
            if(m && m->CanUninstall())
                ovrefresh.emplace(m);
        }
    }

    // Remove all the content related to this readme file
    this->RemoveReadmeData(file);
}

/*
 *  DataPlugin::UpdateReadmeState
 *      Installs / Uninstalls / Reinstalls pending readmes
 */
void DataPlugin::UpdateReadmeState()
{
    // First of all, uninstalls all the readmes that needs to be uninstalled
    // Do so first because it may be a reinstall, during a reinstall the file is both in the uninstall and the install list.
    for(auto& r : this->readme_touninstall)
        this->UninstallReadme(*r.first);

    if(readme_toinstall.size())
    {
        readme_listing_type cached_readme_listing = this->ReadCachedReadmeListing();
        readme_listing_type installing_listing;
        readme_data_store   cached_readme_store;
        
        // Builds the listing about the readmes going to be installed...
        installing_listing.reserve(this->readme_toinstall.size());
        std::transform(readme_toinstall.begin(), readme_toinstall.end(), std::back_inserter(installing_listing),
            [](const std::pair<const modloader::file*, int>& file) -> const modloader::file& {
                return *file.first;
        });

        if(cached_readme_listing.size())    // Do we have any cached readme?
        {
            // Yeah, we do have cached readmes!!! Read the cached readmes content, i.e. lines itself and their data
            cached_readme_store = this->ReadCachedReadmeStore();
            if(cached_readme_store.size() != cached_readme_listing.size())
            {
                plugin_ptr->Log("Warning: Cached readme listing seems to not match the cached store, something is really wrong here!");
                cached_readme_listing.clear();  // problems with the cache, so no cached readme ;)
                cached_readme_store.clear();    // ^^
            }
        }

        // Installs the pending readmes, either by parsing the readme file again or by fetching the data from the cache
        auto install_it = readme_toinstall.begin();
        for(size_t i = 0; i < readme_toinstall.size(); ++i, ++install_it)
        {
            auto& file = *install_it->first;
            auto it = std::find(cached_readme_listing.begin(), cached_readme_listing.end(), installing_listing[i]);
            if(it == cached_readme_listing.end())
            {
                this->Log("Parsing readme file \"%s\"", file.filepath());
                this->InstallReadme(ParseReadme(file));
            }
            else
            {
                auto old_state = this->changed_readme_data; // AddReadmeData changes this, but we are over cache
                this->Log("Parsing cached readme data for \"%s\"", file.filepath());
                auto index = std::distance(cached_readme_listing.begin(), it);
                this->InstallReadme(AddReadmeData(file, std::move(cached_readme_store[index])));
                this->changed_readme_data = old_state;
            }
        }
    }

    // Clear the update lists
    this->readme_touninstall.clear();
    this->readme_toinstall.clear();
}


/*
 *  DataPlugin::ParseReadme
 *      Parses the specified readme file and returns a list of mergers that are related to data found in this file
 */
std::set<size_t> DataPlugin::ParseReadme(const modloader::file& file)
{
    static const size_t max_readme_size = 10000; // ~10KB, don't increase too much, gotta read to memory

    if(file.size <= max_readme_size)
    {
        std::ifstream stream(file.fullpath(), std::ios::binary);
        if(stream)
        {
            // Allocate buffer to work with readme files
            // This buffer will be later freed at Update() time
            if(readme_buffer == nullptr)
                readme_buffer.reset(new char[max_readme_size]);

            if(stream.read(&readme_buffer[0], file.size))
                return this->ParseReadme(file, std::make_pair(&readme_buffer[0], &readme_buffer[file.size]));
            else
                this->Log("Warning: Failed to read from \"%s\".", file.filepath());
        }
        else
            this->Log("Warning: Failed to open \"%s\" for reading.", file.filepath());
    }
    else
        this->Log("Ignoring text file \"%s\" because it's too big.", file.filepath());

    return std::set<size_t>();
}

/*
 *  DataPlugin::ParseReadme
 *      Parses the specified readme buffer (the 'buffer' pair represents begin and end respectively) from the readme file
 *      and returns a list of mergers that are related to data found in this file
 */
std::set<size_t> DataPlugin::ParseReadme(const modloader::file& file, std::pair<const char*, const char*> buffer)
{
    std::string line; line.reserve(256);
    std::set<size_t> mergers;
    size_t line_number = 0;

    while(datalib::gta3::getline(buffer, line))
    {
        ++line_number;
        if(datalib::gta3::trim_config_line(line).size())    // remove trailing spaces, comments and replace ',' with ' '
        {
            for(auto& reader_pair : this->readers)
            {
                auto& reader = reader_pair.second;
                if(auto merger_hash = reader(file, line, line_number)) // calls one of the readme files handlers
                {
                    mergers.emplace(merger_hash.get());
                    break;
                }
            }
        }
    }

    return mergers;
}

/*
 *  DataPlugin::VerifyCachedReadme
 *      Reads the cache header and make sure it's compatible with the current build.
 *      Also fetches all the RTTI type indices possibily used by the cache so we can skip them later on.
 */
bool DataPlugin::VerifyCachedReadme(std::ifstream& ss, cereal::BinaryInputArchive& archive)
{
    decltype(this->readme_magics) magics;
    size_t magic;

    archive(magic); // magic for this translation unit in specific
    if(magic == build_identifier())
    {
        block_reader magics_block(ss);
        archive(magics);                    // magic for the other translation units related to the readmes
        if(magics == this->readme_magics)   // notice order matters
            return true;
    }

    this->Log("Warning: Incompatible readme cache version, a new cache will be generated.");
    return false;
}

/*
 *  DataPlugin::ReadCachedReadmeListing
 *      Reads and outputs the readme cache file listing.
 */
auto DataPlugin::ReadCachedReadmeListing() -> readme_listing_type
{
    readme_listing_type cached_readme_listing;

    std::ifstream ss(cache.GetCachePath("readme.ld"), std::ios::binary);
    if(ss.is_open())
    {
        cereal::BinaryInputArchive archive(ss);
        if(VerifyCachedReadme(ss, archive))
        {
            block_reader listing_block(ss);
            archive(cached_readme_listing);
        }
    }
    return cached_readme_listing;
}

/*
 *  DataPlugin::ReadCachedReadmeStore
 *      Reads and outputs the data_line objects stored in the readme cache
 */
auto DataPlugin::ReadCachedReadmeStore() -> readme_data_store
{
    readme_data_store store_lines;

    std::ifstream ss(cache.GetCachePath("readme.ld"), std::ios::binary);
    if(ss.is_open())
    {
        cereal::BinaryInputArchive archive(ss);
        if(VerifyCachedReadme(ss, archive))
        {
            block_reader::skip(ss); // skip listing block
            block_reader lines_block(ss);
            archive(store_lines);
        }
    }
    return store_lines;
}

/*
 *  DataPlugin::WriteReadmeCache
 *      Writes the 'this->maybe_readme' object into a readme cache
 */
void DataPlugin::WriteReadmeCache()
{
    std::ofstream ss(cache.GetCachePath("readme.ld"), std::ios::binary);
    if(ss.is_open())
    {
        cereal::BinaryOutputArchive archive(ss);

        archive(build_identifier());

        // magics
        {
            block_writer magics_block(ss);
            archive(this->readme_magics);
        }

        // readmes listing
        {
            readme_listing_type files;
            files.reserve(maybe_readme.size());

            std::transform(maybe_readme.begin(), maybe_readme.end(), std::back_inserter(files),
                [](const maybe_readme_type::value_type& pair) -> const modloader::file&
                { return *pair.first; });

            block_writer listing_block(ss);
            archive(files);
        }

        // readme data_line stores
        {
            readme_data_store stores;
            stores.reserve(maybe_readme.size());

            // each item in 'stores' stores a list of data from lines in files
            for(auto& m : this->maybe_readme)
            {
                stores.emplace_back();
                stores.back().reserve(m.second.size());
                std::transform(m.second.begin(), m.second.end(), std::back_inserter(stores.back()), [](const line_data& line) {
                    return line.base();
                });
            }

            block_writer store_block(ss);
            archive(stores);
        }

        this->changed_readme_data = false;
        this->had_cached_readme = true;
    }
}
