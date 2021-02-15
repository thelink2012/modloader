/*
 * Standard Streamer Plugin for Mod Loader
 * Copyright (C) 2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under the MIT License, see LICENSE at top level directory.
 *
 */
#include <stdinc.hpp>
#include "streaming.hpp"
//using namespace modloader;

static const size_t ped_ifp = modloader::hash("ped.ifp");

static bool ShouldIgnoreFile(const modloader::file&);

/*
 *  The plugin object
 */
class ThePlugin : public modloader::basic_plugin
{
    private:
        file_overrider ov_ped_ifp;

    public:
        const info& GetInfo();
        bool OnStartup();
        bool OnShutdown();
        int GetBehaviour(modloader::file&);
        bool InstallFile(const modloader::file&);
        bool ReinstallFile(const modloader::file&);
        bool UninstallFile(const modloader::file&);
        void Update();

} plugin;
REGISTER_ML_PLUGIN(::plugin);


/*
 *  ThePlugin::GetInfo
 *      Returns information about this plugin 
 */
const ThePlugin::info& ThePlugin::GetInfo()
{
    static const char* extable[] = { "img", "dff", "txd", "col", "ipl", "dat", "ifp", "rrr", "scm", 0 };
    static const info xinfo      = { "std.stream", get_version_by_date(), "LINK/2012", 52, extable };
    return xinfo;
}





/*
 *  ThePlugin::OnStartup
 *      Startups the plugin
 */
bool ThePlugin::OnStartup()
{
    if(gvm.IsIII() || gvm.IsVC() || gvm.IsSA() || loader->game_id == MODLOADER_GAME_RE3)
    {
        // Setup abstract streaming
        streaming = new CAbstractStreaming();
        streaming->Patch();
        streaming->InitRefreshInterface(); // TODO move to ctor?

        // Setup ped.ifp overrider
        if(gvm.IsIII())
        {
            ov_ped_ifp.SetParams(file_overrider::params(nullptr));
            ov_ped_ifp.SetFileDetour(Gta3LoadIfpDetour<xIII(0x4038FC)>());
        }
        else if(gvm.IsVC() || gvm.IsSA())
        {
            ov_ped_ifp.SetParams(file_overrider::params(nullptr));
            ov_ped_ifp.SetFileDetour(RwStreamOpenDetour<0x4D565A>());
        }

        return true;
    }
    return false;
}

/*
 *  ThePlugin::OnShutdown
 *      Shutdowns the plugin
 */
bool ThePlugin::OnShutdown()
{
    if(streaming)
    {
        streaming->ShutRefreshInterface(); // TODO move to dtor?
        delete streaming;
        streaming = nullptr;
    }
    return true;
}


/*
 *  ThePlugin::GetBehaviour
 *      Gets the relationship between this plugin and the file
 */
int ThePlugin::GetBehaviour(modloader::file& file)
{
    if(ShouldIgnoreFile(file))
    {
        plugin_ptr->Log("Ignoring file \"%s\" because it's a freaking Rockstar left over.", file.filedir());
        return MODLOADER_BEHAVIOUR_NO;
    }

    if(!file.is_dir() && file.is_ext("img"))
    {
        // This is a image file
        file.behaviour = file.hash | is_img_file_mask;
        return MODLOADER_BEHAVIOUR_YES;
    }
    else if(!file.is_dir() && file.hash == ped_ifp)
    {
        file.behaviour = file.hash | is_pedifp_mask;
        return MODLOADER_BEHAVIOUR_YES;
    }
    else if(!file.is_dir())
    {
        ResType type = GetResTypeFromExtension(file.filext());
        if(type == ResType::None)  // None of the supported types by this plugin
            return MODLOADER_BEHAVIOUR_NO;
        
        // Check for forced player clothing items
        auto inFolder = GetPathComponentBack<char>(file.filedir(), 2);             // the folder the file is inside
        bool isPlayer = false;
        if(gvm.IsSA() && inFolder.find("player") != inFolder.npos)
            isPlayer = (inFolder == "player.img" || inFolder == "player_img");

        // If inside a folder with img extension, ignore any checking, just accept the file
        // Oh yeah, and don't do this checking on clothing files for player.img
        if(isPlayer == false && GetFileExtension(inFolder) != "img")
        {
            switch(type)
            {
                case ResType::Nodes:
                {
                    // Disabled, may conflict with nodes from data folder, use .img folder instead
                    return MODLOADER_BEHAVIOUR_NO;
                }

                case ResType::StreamedScene:
                {
                    // Make sure this is a binary IPL by reading the file magic
                    char buf[4];
                    if(FILE* f = fopen(file.fullpath().c_str(), "rb"))
                    {
                        if(fread(buf, 4, 1, f) && !memcmp(buf, "bnry", 4))
                        {
                            fclose(f);
                            break;
                        }
                        fclose(f);
                    }
                    return MODLOADER_BEHAVIOUR_NO;
                }

                case ResType::VehRecording:
                {
                    // Make sure it's file name is carrec formated
                    int dummy;
                    if(sscanf(file.filename(), "carrec%d", &dummy) != 1)
                        return MODLOADER_BEHAVIOUR_NO;
                    break;
                }
            }
        }

        SetResType(file, type);
        if(isPlayer) file.behaviour |= is_fcloth_mask;  // Forced clothing item
        return MODLOADER_BEHAVIOUR_YES;
    }
    return MODLOADER_BEHAVIOUR_NO;
}

/*
 *  ThePlugin::InstallFile
 *      Installs a file using this plugin
 */
bool ThePlugin::InstallFile(const modloader::file& file)
{
    if(file.behaviour & is_item_mask) return streaming->InstallFile(file);
    if(file.behaviour & is_pedifp_mask) return ov_ped_ifp.InstallFile(file);
    if(file.behaviour & is_img_file_mask) return streaming->AddImgFile(file);
    return false;
}

/*
 *  ThePlugin::ReinstallFile
 *      Reinstall a file previosly installed that has been updated
 */
bool ThePlugin::ReinstallFile(const modloader::file& file)
{
    if(file.behaviour & is_item_mask) return streaming->ReinstallFile(file);
    if(file.behaviour & is_pedifp_mask) return ov_ped_ifp.ReinstallFile();
    if(file.behaviour & is_img_file_mask) return true;
    return false;
}

/*
 *  ThePlugin::UninstallFile
 *      Uninstall a previosly installed file
 */
bool ThePlugin::UninstallFile(const modloader::file& file)
{
    if(file.behaviour & is_item_mask) return streaming->UninstallFile(file);
    if(file.behaviour & is_pedifp_mask) return ov_ped_ifp.UninstallFile();
    if(file.behaviour & is_img_file_mask) return streaming->RemImgFile(file);
    return false;
}

/*
 *  ThePlugin::Update
 *      Updates the plugin context after a serie of installs/uninstalls
 */
void ThePlugin::Update()
{
    streaming->Update();
}



/*
*  ShouldIgnoreFile
*      So there's a few Rockstar left overs that are too hard to handle (or I don't want to think how to handle).
*      I'm just going to do a hard coded ignore then.
*/
bool ShouldIgnoreFile(const modloader::file& file)
{
    // All this bullshit happens only in VC.
    if(!gvm.IsVC())
        return false;

    if(file.is_dir())
        return false;

    static const size_t generic_txd = modloader::hash("generic.txd");

    static const size_t unused_cols[] = {
        modloader::hash("airport.col"),
        modloader::hash("airportn.col"),
        modloader::hash("bank.col"),
        modloader::hash("bridge.col"),
        modloader::hash("cisland.col"),
        modloader::hash("club.col"),
        modloader::hash("concerth.col"),
        modloader::hash("docks.col"),
        modloader::hash("downtown.col"),
        modloader::hash("downtows.col"),
        modloader::hash("golf.col"),
        modloader::hash("haiti.col"),
        modloader::hash("haitin.col"),
        modloader::hash("hotel.col"),
        modloader::hash("islandsf.col"),
        modloader::hash("lawyers.col"),
        modloader::hash("littleha.col"),
        modloader::hash("mall.col"),
        modloader::hash("mansion.col"),
        modloader::hash("nbeach.col"),
        modloader::hash("nbeachbt.col"),
        modloader::hash("nbeachw.col"),
        modloader::hash("oceandn.col"),
        modloader::hash("oceandrv.col"),
        modloader::hash("stadint.col"),
        modloader::hash("starisl.col"),
        modloader::hash("stripclb.col"),
        modloader::hash("washintn.col"),
        modloader::hash("washints.col"),
        modloader::hash("yacht.col"),
    };

    if(file.hash == generic_txd && IsFileInsideFolder(file.filedir(), true, "gta3.img"))
    {
        return true;
    }

    if(file.is_ext("col")
    && std::find(std::begin(unused_cols), std::end(unused_cols), file.hash) != std::end(unused_cols)
    && IsFileInsideFolder(file.filedir(), false, "maps"))
    {
        return true;
    }

    return false;
}