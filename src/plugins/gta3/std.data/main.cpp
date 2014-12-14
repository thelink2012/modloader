/*
* Copyright (C) 2013-2014 LINK/2012 <dma_2012@hotmail.com>
* Licensed under GNU GPL v3, see LICENSE at top level directory.
*
*/
#include <stdinc.hpp>
#include <tuple>
using namespace modloader;

static const size_t timecyc_dat  = modloader::hash("timecyc.dat");
static const size_t popcycle_dat = modloader::hash("popcycle.dat");
static const size_t fonts_dat    = modloader::hash("fonts.dat");
static const size_t plants_dat   = modloader::hash("plants.dat");

static std::tuple<bool, size_t, uint32_t> files_behv[] = { // CanGetMixed, Hash, Counter
        std::make_tuple(false,      timecyc_dat,    0u),
        std::make_tuple(true,       plants_dat,     0u),
};
static const int canmix_elem    = 0;
static const int hash_elem      = 1;
static const int count_elem     = 2;



DataPlugin plugin;
REGISTER_ML_PLUGIN(::plugin);

/*
 *  DataPlugin::GetInfo
 *      Returns information about this plugin 
 */
const DataPlugin::info& DataPlugin::GetInfo()
{
    static const char* extable[] = { "dat", "cfg", "ide", "ipl", "zon", 0 };
    static const info xinfo      = { "std.data", get_version_by_date(), "LINK/2012", -1, extable };
    return xinfo;
}






/*
 *  DataPlugin::OnStartup
 *      Startups the plugin
 */
bool DataPlugin::OnStartup()
{
    {
        // File overrider params
        const auto reinstall_since_start = file_overrider<>::params(true, true, true, true);
        const auto reinstall_since_load  = file_overrider<>::params(true, true, false, true);
        const auto no_reinstall          = file_overrider<>::params(nullptr);

        auto ReloadPlantsDat = []
        {
            if(!injector::cstd<char()>::call<0x5DD780>()) // CPlantMgr::ReloadConfig
                plugin_ptr->Log("Failed to refresh plant manager");
        };

        AddMerger<plants_store>("plants.dat", reinstall_since_load, gdir_refresh(ReloadPlantsDat));

    }

    // TODO create cache
    return true;
}

/*
 *  DataPlugin::OnShutdown
 *      Shutdowns the plugin
 */
bool DataPlugin::OnShutdown()
{
    // TODO remove cache
    return true;
}


/*
 *  DataPlugin::GetBehaviour
 *      Gets the relationship between this plugin and the file
 */
int DataPlugin::GetBehaviour(modloader::file& file)
{
    if(file.is_ext("txt"))
    {
        // TODO
    }
    else if(file.is_ext("ide"))
    {
        static uint32_t count = 0;
        file.behaviour = SetType(file.hash, Type::ObjTypes);
        file.behaviour = SetCounter(file.behaviour, ++count);
        return MODLOADER_BEHAVIOUR_YES;
    }
    else if(file.is_ext("ipl") || file.is_ext("zon"))
    {
        static uint32_t count = 0;
        file.behaviour = SetType(file.hash, Type::Scene);
        file.behaviour = SetCounter(file.behaviour, ++count);
        return MODLOADER_BEHAVIOUR_YES;
    }
    else for(auto& item : files_behv)
    {
        // Iterate on this list of handleable data files and try to detect one
        // Check the filename hash with this item
        if(std::get<hash_elem>(item) == file.hash)
        {
            // Yeah, we can handle it, setup the behaviour....
            file.behaviour = SetType(file.hash, Type::Data);
            
            // Does this data file can get mixed? If yeah, put a counter on the behaviour, so we can receive many
            // data files of this same type on Install events
            if(std::get<canmix_elem>(item))
                file.behaviour = SetCounter(file.behaviour, ++std::get<count_elem>(item));

            return MODLOADER_BEHAVIOUR_YES;
        }
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
        auto m = FindMerger(file.filename());
        if(m->CanInstall())
        {
            fs.add_file(file.filename(), file.filepath(), &file);
            ovrefresh.emplace(FindMerger(file.filename()));
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
        auto m = FindMerger(file.filename());
        if(m->CanInstall())
        {
            ovrefresh.emplace(m);
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

    // TODO
    return false;
}

/*
 *  DataPlugin::Update
 *      Updates the state of this plugin after a serie of install/uninstalls
 */
void DataPlugin::Update()
{
    for(auto& ov : this->ovrefresh)
    {
        if(!ov->Refresh())
            plugin_ptr->Log("Warning: Failed to refresh some data file");
    }

    ovrefresh.clear();
}
