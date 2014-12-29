/*
 * Copyright (C) 2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#include <stdinc.hpp>
#include <tuple>
#include "data_traits.hpp"
using namespace modloader;

#if 1
#include "vfs.hpp"
#include "cache.hpp"
#include "data.hpp"
#endif

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
    if(gvm.IsSA())
    {
        // File overrider params
        const auto reinstall_since_start = file_overrider<>::params(true, true, true, true);
        const auto reinstall_since_load  = file_overrider<>::params(true, true, false, true);
        const auto no_reinstall          = file_overrider<>::params(nullptr);

        //
        bool isSAMP = !!GetModuleHandleA("samp");

        // Initialise the caching
        if(!cache.Startup())
            return false;

        ///////////////////////////////////////////////////////////////

        auto ReloadPlantsDat = []
        {
            if(!injector::cstd<char()>::call<0x5DD780>()) // CPlantMgr::ReloadConfig
                plugin_ptr->Log("Failed to refresh plant manager");
        };

        auto ReloadTimeCycle = []
        {
            injector::cstd<void()>::call<0x05BBAC0>();   // CTimeCycle::Initialise
            // there are some vars at the end of that function body which should not be reseted while the game runs...
            // doesn't cause serious issues but well... shall we take care of them?
        };


        //
        //  Mergers and Overriders
        //

        // Detouring for plants surface properties
        AddMerger<plants_store>("plants.dat", true, reinstall_since_load, gdir_refresh(ReloadPlantsDat));

        // Detouring for vehicle upgrades
        AddMerger<carmods_store>("carmods.dat", true, no_reinstall);

        // Detouring for time cycle properties
        auto& timecyc_ov = AddDetour("timecyc.dat", reinstall_since_start, OpenTimecycDetour(), gdir_refresh(ReloadTimeCycle));
        if(isSAMP)
        {
            //
            // SAMP changes the path from any file named timecyc.dat to a custom path. This hook takes place
            // at kernel32:CreateFileA, so the only way to fix it is telling SAMP to load the timecyc using a different filename.
            //
            // So, the approach used here to get around this issue is, when the game is running under SAMP copy the
            // custom timecyc (which is somewhere at the modloader directory) into the gta3.std.data cache with a different extension and voilá. 
            //
            auto& timecyc_detour = timecyc_ov.GetInjection().cast<OpenTimecycDetour>();
            timecyc_detour.OnPosTransform([this](std::string file) -> std::string
            {
                if(file.size())
                {
                    std::string path = get<1>(cache.AddCacheFile("timecyc.samp", true));

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

    if(file.is_ext("txt"))
    {
        // TODO
    }
    else if(file.is_ext("ide"))
    {
        static uint32_t count = 0;
        file.behaviour = SetCounter(SetType(file.hash, Type::ObjTypes), ++count);
        return MODLOADER_BEHAVIOUR_YES;
    }
    else if(file.is_ext("ipl") || file.is_ext("zon"))
    {
        static uint32_t count = 0;
        file.behaviour = SetCounter(SetType(file.hash, Type::Scene), ++count);
        return MODLOADER_BEHAVIOUR_YES;
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
