/*
* Copyright (C) 2013-2014 LINK/2012 <dma_2012@hotmail.com>
* Licensed under GNU GPL v3, see LICENSE at top level directory.
*
*/
#include <modloader/modloader.hpp>
#include <modloader/util/hash.hpp>
#include <tuple>



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

enum class Type
{
    Data        = 0,
    Scene       = 1,
    ObjTypes    = 2,

    Max         = 7,    // Max 3 bits
};

// Basical maskes
static const uint64_t hash_mask_base  = 0xFFFFFFFF;
static const uint64_t type_mask_base  = 0x0007;                 // Mask for type without any shifting
static const uint64_t count_mask_base = 0xFFFF;                 // Mask for count without any shifting
static const uint32_t type_mask_shf   = 32;                     // Takes 3 bits, starts from 33th bit because first 32th bits is a hash
static const uint32_t count_mask_shf  = type_mask_shf + 3;      // Takes 28 bits


// Sets the initial value for a behaviour, by using an filename hash and file type
inline uint64_t SetType(uint32_t hash, Type type)
{
    return modloader::file::set_mask(uint64_t(hash), hash_mask_base, type_mask_shf, type);
}

inline uint64_t SetCounter(uint64_t behaviour, uint32_t count)
{
    return modloader::file::set_mask(behaviour, count_mask_base, count_mask_shf, count);
}

inline Type GetType(uint64_t mask)
{
    return modloader::file::get_mask<Type>(mask, type_mask_base, type_mask_shf);
}




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
        void Update();
        
} plugin;

REGISTER_ML_PLUGIN(::plugin);

/*
 *  ThePlugin::GetInfo
 *      Returns information about this plugin 
 */
const ThePlugin::info& ThePlugin::GetInfo()
{
    static const char* extable[] = { "dat", "cfg", "ide", "ipl", "zon", 0 };
    static const info xinfo      = { "std.data", "R0.1", "LINK/2012", -1, extable };
    return xinfo;
}





/*
 *  ThePlugin::OnStartup
 *      Startups the plugin
 */
bool ThePlugin::OnStartup()
{
    // TODO create cache
    return true;
}

/*
 *  ThePlugin::OnShutdown
 *      Shutdowns the plugin
 */
bool ThePlugin::OnShutdown()
{
    // TODO remove cache
    return true;
}


/*
 *  ThePlugin::GetBehaviour
 *      Gets the relationship between this plugin and the file
 */
int ThePlugin::GetBehaviour(modloader::file& file)
{
    if(file.is_ext("txt"))
    {
        // TODO
    }
    else if(file.is_ext("ide"))
    {
        static uint32_t count = 0;
        // TODO
    }
    else if(file.is_ext("ipl") || file.is_ext("zon"))
    {
        static uint32_t count = 0;
        // TODO
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
            if(std::get<canmix_elem>(item)) SetCounter(file.behaviour, ++std::get<count_elem>(item));

            return MODLOADER_BEHAVIOUR_YES;
        }
    }
    return MODLOADER_BEHAVIOUR_NO;
}

/*
 *  ThePlugin::InstallFile
 *      Installs a file using this plugin
 */
bool ThePlugin::InstallFile(const modloader::file& file)
{
    auto type = GetType(file.behaviour);

    if(type == Type::Data)
    {
        if(file.hash == plants_dat)
            ;//TODO

        return true;
    }

    return false;
}

/*
 *  ThePlugin::ReinstallFile
 *      Reinstall a file previosly installed that has been updated
 */
bool ThePlugin::ReinstallFile(const modloader::file& file)
{
    return false;
}

/*
 *  ThePlugin::UninstallFile
 *      Uninstall a previosly installed file
 */
bool ThePlugin::UninstallFile(const modloader::file& file)
{
    return false;
}

/*
 *  ThePlugin::Update
 *      Updates the state of this plugin after a serie of install/uninstalls
 */
void ThePlugin::Update()
{
}
