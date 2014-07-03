/*
 * Copyright (C) 2013-2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 *  std.stream
 *      Loads all compatible files with imgs (on game request),
 *      it loads directly from disk not by creating a cache or virtual img.
 * 
 */
#include "streaming.hpp"
#include <modloader/util/file.hpp>
#include <modloader/util/injector.hpp>
using namespace modloader;


// TODO .img folder
// TODO .img files
// TODO ped.ifp


/*
 *  The plugin object
 */
class ThePlugin : public modloader::basic_plugin
{
    public:
        const info& GetInfo();
        bool OnStartup();
        bool OnShutdown();
        int GetBehaviour(modloader::file&);
        bool InstallFile(const modloader::file&);
        bool ReinstallFile(const modloader::file&);
        bool UninstallFile(const modloader::file&);
        
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
        streaming.Patch();
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
    if(file.IsExtension("img"))
    {
        file.behaviour = file.hash | (file.IsDirectory()? is_img_dir_mask : is_img_file_mask);
        return MODLOADER_BEHAVIOUR_YES;
    }
    else if(!file.IsDirectory())
    {
        FileType type = GetFileTypeFromExtension(file.FileExt());
        switch(type)
        {
            case FileType::None:
            {
                // None of the supported types by this plugin
                return MODLOADER_BEHAVIOUR_NO;
            }

            case FileType::Nodes:
            {
                // Disabled, may conflict with nodes from data folder, use .img folder instead
                return MODLOADER_BEHAVIOUR_NO;
            }

            case FileType::StreamedScene:
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

            case FileType::VehRecording:
            {
                int dummy;
                if(sscanf(file.FileName(), "carrec%d", &dummy) != 1)
                    return MODLOADER_BEHAVIOUR_NO;
                break;
            }
        }

        SetFileType(file, type);
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
    return false;
}

/*
 *  ThePlugin::ReinstallFile
 *      Reinstall a file previosly installed that has been updated
 */
bool ThePlugin::ReinstallFile(const modloader::file& file)
{
    if(file.behaviour & is_item_mask) return streaming.ReinstallFile(file);
    return false;
}

/*
 *  ThePlugin::UninstallFile
 *      Uninstall a previosly installed file
 */
bool ThePlugin::UninstallFile(const modloader::file& file)
{
    if(file.behaviour & is_item_mask) return streaming.UninstallFile(file);
    return false;
}

