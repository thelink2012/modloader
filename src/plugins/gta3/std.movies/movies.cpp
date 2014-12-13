/*
 * Copyright (C) 2013-2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 *  std.movies -- Standard Movies Plugin for Mod Loader
 *
 */
#include <stdinc.hpp>
using namespace modloader;

/*
 *  The plugin object
 */
class MediaPlugin : public modloader::basic_plugin
{
    private:
        uint32_t logo;              // Hash for logo.mpg
        uint32_t GTAtitles;         // Hash for GTAtitles.mpg
        uint32_t GTAtitlesGER;      // Hash for GTAtitlesGER.mpg

        file_overrider<> logo_detour;
        file_overrider<> titles_detour;

    public:
        const info& GetInfo();
        bool OnStartup();
        bool OnShutdown();
        int GetBehaviour(modloader::file&);
        bool InstallFile(const modloader::file&);
        bool ReinstallFile(const modloader::file&);
        bool UninstallFile(const modloader::file&);
        
} mpg_plugin;

REGISTER_ML_PLUGIN(::mpg_plugin);

/*
 *  MediaPlugin::GetInfo
 *      Returns information about this plugin 
 */
const MediaPlugin::info& MediaPlugin::GetInfo()
{
    static const char* extable[] = { "mpg", 0 };
    static const info xinfo      = { "std.movies", get_version_by_date(), "LINK/2012", -1, extable };
    return xinfo;
}





/*
 *  MediaPlugin::OnStartup
 *      Startups the plugin
 */
bool MediaPlugin::OnStartup()
{
    if(gvm.IsSA())
    {
        this->logo          = modloader::hash("logo.mpg");
        this->GTAtitles     = modloader::hash("gtatitles.mpg");
        this->GTAtitlesGER  = modloader::hash("gtatitlesger.mpg");

        auto params = file_overrider<>::params(true, true, false, false);
        logo_detour.SetParams(params).SetFileDetour(CreateVideoPlayerDetour<0x748B00>());
        titles_detour.SetParams(params).SetFileDetour(CreateVideoPlayerDetour<0x748BF9>());
        return true;
    }
    return false;
}

/*
 *  MediaPlugin::OnShutdown
 *      Shutdowns the plugin
 */
bool MediaPlugin::OnShutdown()
{
    return true;
}


/*
 *  MediaPlugin::GetBehaviour
 *      Gets the relationship between this plugin and the file
 */
int MediaPlugin::GetBehaviour(modloader::file& file)
{
    if(!file.is_dir())
    {
        if(file.hash == logo)
        {
            file.behaviour = logo;
            return MODLOADER_BEHAVIOUR_YES;
        }
        else if(file.hash == GTAtitles || file.hash == GTAtitlesGER)
        {
            file.behaviour = GTAtitles;
            return MODLOADER_BEHAVIOUR_YES;
        }
    }
    return MODLOADER_BEHAVIOUR_NO;
}

/*
 *  MediaPlugin::InstallFile
 *      Installs a file using this plugin
 */
bool MediaPlugin::InstallFile(const modloader::file& file)
{
    if(file.behaviour == logo)      return logo_detour.InstallFile(file);
    if(file.behaviour == GTAtitles) return titles_detour.InstallFile(file);
    return false;
}

/*
 *  MediaPlugin::ReinstallFile
 *      Reinstall a file previosly installed that has been updated
 */
bool MediaPlugin::ReinstallFile(const modloader::file& file)
{
    if(file.behaviour == logo)      return logo_detour.ReinstallFile();
    if(file.behaviour == GTAtitles) return titles_detour.ReinstallFile();
    return false;
}

/*
 *  MediaPlugin::UninstallFile
 *      Uninstall a previosly installed file
 */
bool MediaPlugin::UninstallFile(const modloader::file& file)
{
    if(file.behaviour == logo)      return logo_detour.UninstallFile();
    if(file.behaviour == GTAtitles) return titles_detour.UninstallFile();
    return false;
}
