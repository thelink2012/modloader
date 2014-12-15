/*
* Copyright (C) 2013-2014 LINK/2012 <dma_2012@hotmail.com>
* Licensed under GNU GPL v3, see LICENSE at top level directory.
*
*/
#include <stdinc.hpp>
#include <tuple>
using namespace modloader;

struct files_behv_t
{
    bool        canmix;
    size_t      hash;
    uint32_t    count;
};

static files_behv_t files_behv[] = {
        { false, modloader::hash("timecyc.dat"),    0 },
        { true,  modloader::hash("plants.dat"),     0 }
};

static files_behv_t* find_behv(const modloader::file& f)
{
    for(auto& item : files_behv)
    {
        if(item.hash == f.hash)
            return &item;
    }
    return nullptr;
}



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




struct timecyc_traits
{
    struct dtraits : modloader::dtraits::OpenFile    // detour traits
    {
        static const char* what() { return "time cycle properties"; }
    };
};


/*
 *  DataPlugin::OnStartup
 *      Startups the plugin
 */
bool DataPlugin::OnStartup()
{
    if(gvm.IsSA())
    {
        if(!cache.Startup("gta3.std.data"))
            return false;

        bool isSAMP = !!GetModuleHandleA("samp");

        // File overrider params
        const auto reinstall_since_start = file_overrider<>::params(true, true, true, true);
        const auto reinstall_since_load  = file_overrider<>::params(true, true, false, true);
        const auto no_reinstall          = file_overrider<>::params(nullptr);

        auto ReloadPlantsDat = []
        {
            if(!injector::cstd<char()>::call<0x5DD780>()) // CPlantMgr::ReloadConfig
                plugin_ptr->Log("Failed to refresh plant manager");
        };

        auto ReloadTimeCycle = []
        {
            injector::cstd<void()>::call<0x05BBAC0>();   // CTimeCycle::Initialise
            // the are some vars at the end of the function body which should not be reseted while the game runs
            // doesn't cause serious issues but well... shall we take care of them?
        };

        AddMerger<plants_store>("plants.dat", true, reinstall_since_load, gdir_refresh(ReloadPlantsDat));

        using OpenTimecycDetour = OpenFileDetour<0x5BBADE, timecyc_traits::dtraits>;
        auto& timecyc_ov = AddDetour("timecyc.dat", reinstall_since_start, OpenTimecycDetour(), gdir_refresh(ReloadTimeCycle));
        auto& timecyc_detour = timecyc_ov.GetInjection().cast<OpenTimecycDetour>();

        if(isSAMP)
        {
            timecyc_detour.OnPosTransform([this](std::string file) -> std::string
            {
                if(file.size())
                {
                    auto path = cache.GetPathForData("timecyc.samp", false, true);

                    if(!CopyFileA(
                        std::string(loader->gamepath).append(file).c_str(),
                        std::string(loader->gamepath).append(path).c_str(),
                        FALSE))
                        plugin_ptr->Log("Warning: Failed to make timecyc for SAMP.");
                    else
                        file = std::move(path);
                        
                }
                return file;
            });
        }

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
    cache.Shutdown(false);  // destroys the cache depending upon if the cache failed at some point
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
    else if(auto item = find_behv(file))
    {
        file.behaviour = SetType(file.hash, Type::Data);
            
        // Does this data file can get mixed? If yeah, put a counter on the behaviour, so we can receive many
        // data files of this same type on Install events. Notice the count starts from 1 because 0 is reserved for canmix=false
        if(item->canmix)
            file.behaviour = SetCounter(file.behaviour, ++item->count);

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
        if(find_behv(file)->canmix)
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
        if(find_behv(file)->canmix)
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
        if(find_behv(file)->canmix)
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
