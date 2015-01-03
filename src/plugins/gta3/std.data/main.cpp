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

    if(file.is_ext("txt"))
    {
        // TODO
    }
    else if(file.is_ext("ide"))
    {
        if(has_ide_behv)
        {
            static uint32_t count = 0;
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
        file.behaviour = SetCounter(SetType(file.hash, Type::Data), (item->canmerge? ++item->count : 0));
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

    // delay in-game installs to go tho Update()

    if(type == Type::Data)
    {
        if(FindBehv(file)->canmerge)
        {
            auto m = FindMerger(file.hash);
            if(m->CanInstall())
            {
                fs.add_file(file.filename(), file.filepath(), &file);
                ovrefresh.emplace(FindMerger(file.filename()));
                return true;
            }
        }
        else
        {
            return FindMerger(file.hash)->InstallFile(file);
        }
    }
    else if(type == Type::Scene)
    {
        auto m = FindMerger(ipl_merger_name);
        if(m->CanInstall())
        {
            fs.add_file(find_gta_path(file.filedir()), file.filepath(), &file);
            return true;
        }
    }


    // TODO

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
        if(FindBehv(file)->canmerge)
        {
            auto m = FindMerger(file.filename());
            if(m->CanInstall())
            {
                ovrefresh.emplace(m);
                return true;
            }
        }
        else
        {
            return FindMerger(file.hash)->ReinstallFile();
        }
    }
    else if(type == Type::Scene)
    {
        auto m = FindMerger(ipl_merger_name);
        if(m->CanInstall())
        {
            fs.add_file(find_gta_path(file.filedir()), file.filepath(), &file);
            return true;
        }
    }

    // TODO

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
        if(FindBehv(file)->canmerge)
        {
            auto m = FindMerger(file.filename());
            if(m->CanUninstall())
            {
                if(fs.rem_file(file.filename(), &file))
                {
                    ovrefresh.emplace(m);
                    return true;
                }
            }
        }
        else
        {
            return FindMerger(file.hash)->UninstallFile();
        }
    }
    else if(type == Type::Scene)
    {
        auto m = FindMerger(ipl_merger_name);
        if(m->CanUninstall())
        {
            if(fs.rem_file(file.filename(), &file))
                return true;
        }
    }

    // TODO

    return false;
}

/*
 *  DataPlugin::Update
 *      Updates the state of this plugin after a serie of install/uninstalls
 */
void DataPlugin::Update()
{
    // Refresh every overriden of multiple files right here
    for(auto& ov : this->ovrefresh)
    {
        if(!ov->Refresh())
            plugin_ptr->Log("Warning: Failed to refresh some data file");
    }
    ovrefresh.clear();
}
