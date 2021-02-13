/*
 * Copyright (C) 2013-2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under the MIT License, see LICENSE at top level directory.
 * 
 *  std.movies -- Standard Movies Plugin for Mod Loader
 *
 */
#include <stdinc.hpp>
using namespace modloader;


class Test : public injector::scoped_base
{
public:
    //using func_type = std::function<Ret(Args...)>;
    using func_type = std::function<void(int, const char*)>;
    //using functor_type = std::function<Ret(func_type, Args&...)>;
    using functor_type = std::function<void(func_type, int&, const char*&)>;

    functor_type functor;

    Test() = default;
    Test(const Test&) = delete;
    Test(Test&& rhs) : functor(std::move(rhs.functor)) {}
    Test& operator=(const Test&) = delete;
    Test& operator=(Test&& rhs) { functor = std::move(rhs.functor); }

    virtual ~Test()
    {
        plugin_ptr->loader->Log(">>>>>>>>>>>>>dtor");
        restore();
    }

    void make_call(functor_type functor)
    {
        plugin_ptr->loader->Log(">>>>>>>>>>>>>make_call");
        this->functor = std::move(functor);
    }

    bool has_hooked() const
    {
        plugin_ptr->loader->Log(">>>>>>>>>>>>>has_hooked");
        return !!functor;
    }

    void restore() override
    {
        this->functor = nullptr;
        plugin_ptr->loader->Log(">>>>>>>>>>>>>restore");
    }
};

/*
 *  The plugin object
 */
class MediaPlugin : public modloader::basic_plugin
{
    private:
        uint32_t logo;              // Hash for logo.mpg
        uint32_t GTAtitles;         // Hash for GTAtitles.mpg
        uint32_t GTAtitlesGER;      // Hash for GTAtitlesGER.mpg

        file_overrider logo_detour;
        file_overrider titles_detour;

        modloader_re3_t* modloader_re3{};

        static void RE3Detour_PlayMovieInWindow_Logo(int, const char*);
        static void RE3Detour_PlayMovieInWindow_GTAtitles(int, const char*);

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


using CreateVideoPlayerDetourRE3 = modloader::basic_file_detour<dtraits::CreateVideoPlayer,
    Test,
    void, int, const char*>;

void MediaPlugin::RE3Detour_PlayMovieInWindow_Logo(int cmdshow, const char* filename)
{
    const auto& modloader_re3 = *mpg_plugin.modloader_re3;
    const auto PlayMovieInWindow = modloader_re3.re3_addr_table->PlayMovieInWindow;

    auto& logo_detour = mpg_plugin.logo_detour;
    if(logo_detour.NumInjections() == 1)
    {
        const auto& test = static_cast<Test&>(logo_detour.GetInjection(0));
        if(test.has_hooked())
            return test.functor(PlayMovieInWindow, cmdshow, filename);
    }

    return PlayMovieInWindow(cmdshow, filename);
}

void MediaPlugin::RE3Detour_PlayMovieInWindow_GTAtitles(int cmdshow, const char* filename)
{
    const auto& modloader_re3 = *mpg_plugin.modloader_re3;
    const auto PlayMovieInWindow = modloader_re3.re3_addr_table->PlayMovieInWindow;
    
    auto& titles_detour = mpg_plugin.titles_detour;
    if(titles_detour.NumInjections() == 1)
    {
        const auto& test = static_cast<Test&>(titles_detour.GetInjection(0));
        if(test.has_hooked())
            return test.functor(PlayMovieInWindow, cmdshow, filename);
    }

    return PlayMovieInWindow(cmdshow, filename);
}


/*
 *  MediaPlugin::OnStartup
 *      Startups the plugin
 */
bool MediaPlugin::OnStartup()
{
    if(gvm.IsIII() || gvm.IsVC() || gvm.IsSA() || loader->game_id == MODLOADER_GAME_RE3)
    {
        this->logo          = modloader::hash("logo.mpg");
        this->GTAtitles     = modloader::hash("gtatitles.mpg");
        this->GTAtitlesGER  = modloader::hash("gtatitlesger.mpg");

        auto params = file_overrider::params(true, true, false, false);
        logo_detour.SetParams(params);
        titles_detour.SetParams(params);
        
        if(loader->game_id == MODLOADER_GAME_RE3)
        {
            this->modloader_re3 = (modloader_re3_t*) loader->FindSharedData("MODLOADER_RE3")->p;

            modloader_re3->callback_table->PlayMovieInWindow_Logo = RE3Detour_PlayMovieInWindow_Logo;
            logo_detour.SetFileDetour(CreateVideoPlayerDetourRE3());

            modloader_re3->callback_table->PlayMovieInWindow_GTAtitles = RE3Detour_PlayMovieInWindow_GTAtitles;
            titles_detour.SetFileDetour(CreateVideoPlayerDetourRE3());
        }
        else
        {
            logo_detour.SetFileDetour(CreateVideoPlayerDetour<0x748B00>());
            titles_detour.SetFileDetour(CreateVideoPlayerDetour<0x748BF9>());
        }

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
