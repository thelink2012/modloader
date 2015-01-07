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



DataPlugin plugin;
REGISTER_ML_PLUGIN(::plugin);


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



// Makes for example "folder1/folder2/data/a.ipl" turn into "data/a.ipl"
inline std::string find_gta_path(std::string path)
{
    static const auto data = MakeSureStringIsDirectory(NormalizePath("data/"));
    return GetProperlyPath(std::move(path), data.c_str());
}


/*
 *  DataPlugin::OnStartup
 *      Startups the plugin
 */
bool DataPlugin::OnStartup()
{
    if(gvm.IsSA())
    {
        // Initialise the caching
        if(!cache.Startup())
            return false;

        // Initialises all the merges and overrides (see 'data_traits/' directory for those)
        for(auto& p : initializer::list())
            p->initialise(this);

        // Installs the hooks in any case, so we have the log always logging the loading of data files
        for(auto& pair : this->ovmap)
            pair.second.InstallHook();

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
    //
    //  Note:
    //      We Put a counter on the behaviour of mergeable data files so we can receive many of them.
    //      Notice the count starts from 1 for those because 0 is reserved for non mergeable data files.
    //

    static const bool has_ipl_behv = FindBehv(ipl_merger_name) != nullptr;
    static const bool has_ide_behv = FindBehv(ide_merger_name) != nullptr;

    // TODO more reliable behaviour for different files
    // NOTES FOR THIS TODO: 
    //      Each merger should have a unique id
    //      Each file at the SAME PATH (not SAME FILENAME) should have the same behaviour as before
    //

    if(file.is_ext("txt"))
    {
        // TODO
    }
    else if(file.is_ext("ide"))
    {
        if(has_ide_behv)
        {
            static uint32_t count = 0;// TODO NOT RELIABLE
            file.behaviour = SetCounter(SetType(file.hash, Type::ObjTypes), ++count);
            return MODLOADER_BEHAVIOUR_YES;
        }
    }
    else if(file.is_ext("ipl") || file.is_ext("zon"))
    {
        if(has_ipl_behv)
        {
            static uint32_t count = 0;
            file.behaviour = SetCounter(SetType(file.hash, Type::Scene), ++count);
            return MODLOADER_BEHAVIOUR_YES;
        }
    }
    else if(auto item = FindBehv(file))
    {
        file.behaviour = SetCounter(SetType(modloader::hash(file.filepath())/*file.hash*/, Type::Data), (item->canmerge? /*++item->count*/1 : 0));
         return MODLOADER_BEHAVIOUR_YES;
    }
    return MODLOADER_BEHAVIOUR_NO;
}

/*
 *  DataPlugin::InstallFile
 *      Installs a file using this plugin
 */
bool DataPlugin::InstallFile(const modloader::file& file)
{
    auto type = GetType(file.behaviour);
    if(type == Type::Data)
    {
        return this->InstallFile(file, file.hash, file.filename(), file.filepath());
    }
    else if(type == Type::Scene || type == Type::ObjTypes)
    {
        auto hash = (type == Type::Scene? ipl_merger_hash : ide_merger_hash);
        return this->InstallFile(file, hash, find_gta_path(file.filedir()), file.filepath());
    }
    return false;
}


/*
 *  DataPlugin::ReinstallFile
 *      Reinstall a file previosly installed that has been updated
 */
bool DataPlugin::ReinstallFile(const modloader::file& file)
{
    auto type = GetType(file.behaviour);
    if(type == Type::Data)
    {
        return this->ReinstallFile(file, file.hash);
    }
    else if(type == Type::Scene || type == Type::ObjTypes)
    {
        auto hash = (type == Type::Scene? ipl_merger_hash : ide_merger_hash);
        return this->ReinstallFile(file, hash);
    }
    return true; // Avoid catastrophical failure
}

/*
 *  DataPlugin::UninstallFile
 *      Uninstall a previosly installed file
 */
bool DataPlugin::UninstallFile(const modloader::file& file)
{
    auto type = GetType(file.behaviour);
    if(type == Type::Data)
    {
        return this->UninstallFile(file, file.hash, file.filename());
    }
    else if(type == Type::Scene || type == Type::ObjTypes)
    {
        auto hash = (type == Type::Scene? ipl_merger_hash : ide_merger_hash);
        return this->UninstallFile(file, hash, find_gta_path(file.filedir()));
    }
    return false;
}

/*
 *  DataPlugin::Update
 *      Updates the state of this plugin after a serie of install/uninstalls
 */
void DataPlugin::Update()
{
    // Refresh every overriden of multiple files right here
    // Note: Don't worry about this being called before the game evens boot up, the ov->Refresh() method takes care of it
    for(auto& ov : this->ovrefresh)
    {
        if(!ov->Refresh())
            plugin_ptr->Log("Warning: Failed to refresh some data file");   // very useful warning indeed
    }
    ovrefresh.clear();
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

