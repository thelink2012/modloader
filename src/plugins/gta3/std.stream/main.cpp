/*
 * Copyright (C) 2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 *  std.stream
 *      Loads all compatible files with imgs (on game request),
 *      it loads directly from disk not by creating a cache or virtual img.
 * 
 */
#include "streaming.hpp"
#include <modloader/util/file.hpp>
#include <modloader/util/path.hpp>
#include <modloader/util/injector.hpp>
#include <modloader/gta3/gta3.hpp>
using namespace modloader;


/*
 *  The plugin object
 */
class ThePlugin : public modloader::basic_plugin
{
    private:
        file_overrider<> ov_ped_ifp;

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
    static const info xinfo      = { "std.img", "R0.1", "LINK/2012", 48, extable };
    return xinfo;
}





/*
 *  ThePlugin::OnStartup
 *      Startups the plugin
 */
bool ThePlugin::OnStartup()
{
    if(gvm.IsSA())
    {
        // Setup abstract streaming
        streaming.Patch();

        // Setup ped.ifp overrider
        ov_ped_ifp.SetParams(file_overrider<>::params(nullptr));
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
    return true;
}


/*
 *  ThePlugin::GetBehaviour
 *      Gets the relationship between this plugin and the file
 */
int ThePlugin::GetBehaviour(modloader::file& file)
{
    if(!file.IsDirectory() && file.IsExtension("img"))
    {
        // This is a image file
        file.behaviour = file.hash | is_img_file_mask;
        return MODLOADER_BEHAVIOUR_YES;
    }
    else if(!file.IsDirectory())
    {
        ResType type = GetResTypeFromExtension(file.FileExt());
        if(type == ResType::None)  // None of the supported types by this plugin
            return MODLOADER_BEHAVIOUR_NO;
        
        auto inFolder = GetPathComponentBack<char>(file.FilePath(), 2);             // the folder the file is inside
        bool isPlayer = gvm.IsSA() && (inFolder.find("player") != inFolder.npos);   // force clothing item (for player.img)?

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
                }

                case ResType::StreamedScene:
                {
                    std::string str;
                    // If the IPL file have '_stream' on it's name it's probably a stream IPL
                    if((str = file.FileName()).find("_stream") != str.npos)
                    {
                        // Make sure by reading the file magic and comparing with 'bnry'
                        if(IsFileMagic(file.FullPath().c_str(), "bnry"))
                            break;
                    }
                    return MODLOADER_BEHAVIOUR_NO;
                }

                case ResType::VehRecording:
                {
                    // Make sure it's file name is carrec formated
                    int dummy;
                    if(sscanf(file.FileName(), "carrec%d", &dummy) != 1)
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
    if(file.behaviour & is_item_mask) return streaming.InstallFile(file);
    if(file.behaviour & is_pedifp_mask) return ov_ped_ifp.InstallFile(file);
    if(file.behaviour & is_img_file_mask) return streaming.AddImgFile(file);
    return false;
}

/*
 *  ThePlugin::ReinstallFile
 *      Reinstall a file previosly installed that has been updated
 */
bool ThePlugin::ReinstallFile(const modloader::file& file)
{
    if(file.behaviour & is_item_mask) return streaming.ReinstallFile(file);
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
    if(file.behaviour & is_item_mask) return streaming.UninstallFile(file);
    if(file.behaviour & is_pedifp_mask) return ov_ped_ifp.UninstallFile();
    if(file.behaviour & is_img_file_mask) return streaming.RemImgFile(file);
    return false;
}

/*
 *  ThePlugin::Update
 *      Updates the plugin context after a serie of installs/uninstalls
 */
void ThePlugin::Update()
{
    streaming.Update();
}
