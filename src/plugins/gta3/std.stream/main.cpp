/*
 * Standard Streamer Plugin for Mod Loader
 * Copyright (C) 2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under the MIT License, see LICENSE at top level directory.
 *
 */
#include <stdinc.hpp>
#include "streaming.hpp"
//using namespace modloader;


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
    if(gvm.IsVC() || gvm.IsSA())
    {
        // Setup abstract streaming
        streaming = new CAbstractStreaming();
        streaming->Patch();
        streaming->InitRefreshInterface(); // TODO move to ctor?

        // Setup ped.ifp overrider
        ov_ped_ifp.SetParams(file_overrider::params(nullptr));
        ov_ped_ifp.SetFileDetour(RwStreamOpenDetour<0x4D565A>());

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
    if(!file.is_dir() && file.is_ext("img"))
    {
        // This is a image file
        file.behaviour = file.hash | is_img_file_mask;
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

                case ResType::AnimFile:
                {
                    // Make sure it isn't the special ifp file ped.ifp
                    static const auto ped_ifp = modloader::hash("ped.ifp");
                    if(file.hash == ped_ifp)
                    {
                        file.behaviour = file.hash | is_pedifp_mask;
                        return MODLOADER_BEHAVIOUR_YES;
                    }
                    break;
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
