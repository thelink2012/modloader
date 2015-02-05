/*
 * Copyright (C) 2015  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 *
 */
#include "stdinc.hpp"

enum class Type
{
    Ogg             = 0,
    StreamPak       = 1,
    StreamLookUp    = 2,
    PakFiles        = 3,
    Max             = 7,    // Max 3 bits
};


// Basical maskes
static const uint64_t hash_mask_base  = 0xFFFFFFFF;
static const uint64_t type_mask_base  = 0x0007;                 // Mask for type without any shifting
static const uint64_t bank_mask_base  = 0xFFFF;                 // Mask for bank without any shifting
static const uint64_t sound_mask_base = 0x03FF;                 // Mask for sound without any shifting
static const uint32_t type_mask_shf  = 32;                      // Takes 3 bits, starts from 33th bit because first 32th bits is a hash
static const uint32_t bank_mask_shf  = type_mask_shf + 3;       // Takes 16 bits
static const uint32_t sound_mask_shf = bank_mask_shf + 16;      // Takes 10 bits

// Sets the initial value for a behaviour, by using an filename hash and file type
inline uint64_t SetType(uint32_t hash, Type type)
{
    return modloader::file::set_mask(uint64_t(hash), type_mask_base, type_mask_shf, type);
}

// Sets the behaviour sound id bitfield 
inline uint64_t SetSound(uint64_t mask, uint16_t sound)
{
    return modloader::file::set_mask(mask, sound_mask_base, sound_mask_shf, sound);
}

// Sets the behaviour bank id bitfield
inline uint64_t SetBank(uint64_t mask, uint16_t bank)
{
    return modloader::file::set_mask(mask, bank_mask_base, bank_mask_shf, bank);
}

// Gets the behaviour sfx pak filename hash
inline uint32_t GetPakHash(uint64_t mask)
{
    return modloader::file::get_mask<uint32_t>(mask, hash_mask_base, 0);
}

// Gets the behaviour file type
inline Type GetType(uint64_t mask)
{
    return modloader::file::get_mask<Type>(mask, type_mask_base, type_mask_shf);
}

// Gets the behaviour sound id
inline uint16_t GetSound(uint64_t mask)
{
    return modloader::file::get_mask<uint16_t>(mask, sound_mask_base, sound_mask_shf);
}

// Gets the behaviour bank id
inline uint16_t GetBank(uint64_t mask)
{
    return modloader::file::get_mask<uint16_t>(mask, bank_mask_base, bank_mask_shf);
}


static void RedirectToThunk(injector::memory_pointer_tr call_at, injector::memory_pointer_tr thunk_at)
{
    using namespace injector;
    if(ReadMemory<uint8_t>(call_at+0, true) == 0xFF && ReadMemory<uint8_t>(call_at+1, true) == 0x15)
    {
        MakeNOP(call_at, 6, true);
        MakeCALL(call_at, thunk_at.get(), true);
    }
}


/*
 *  The plugin object
 */
class ThePlugin : public modloader::basic_plugin
{
    private:
        modloader::file_overrider ov_strmpaks;    // StrmPaks.dat overrider
        modloader::file_overrider ov_traklkup;    // TrakLkUp.dat overrider

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

using namespace modloader;

/*
 *  ThePlugin::GetInfo
 *      Returns information about this plugin 
 */
const ThePlugin::info& ThePlugin::GetInfo()
{
    static const char* extable[] = { "", "ogg", "ini", "dat", 0 };
    static const info xinfo      = { "std.tracks", get_version_by_date(), "LINK/2012", -1, extable };
    return xinfo;
}




/*
 *  ThePlugin::OnStartup
 *      Startups the plugin
 */
bool ThePlugin::OnStartup()
{
    using namespace modloader;
    if(gvm.IsSA())
    {
        RedirectToThunk(0x4E0A09, raw_ptr(CreateFileA));
        RedirectToThunk(0x4E0989, raw_ptr(CreateFileA));

        auto no_reinstall = file_overrider::params(nullptr);
        ov_traklkup.SetParams(no_reinstall);
        ov_strmpaks.SetParams(no_reinstall);
        ov_traklkup.SetFileDetour(WinCreateFileA<0x4E0A09>());
        ov_strmpaks.SetFileDetour(WinCreateFileA<0x4E0989>());

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
    if(!file.is_dir())
    {
        if(file.is_ext(""))
        {
            // Accept any file without extension in a STREAM/ folder as a STREAM
            // Also accept default STREAMS outside that folder....
            if((modloader::IsFileInsideFolder(file.filepath(), true, "STREAMS"))
            ||(!_stricmp(file.filename(), "AA"))       || (!_stricmp(file.filename(), "TK"))
            ||(!_stricmp(file.filename(), "ADVERTS"))  || (!_stricmp(file.filename(), "AMBIENCE"))
            ||(!_stricmp(file.filename(), "CUTSCENE")) || (!_stricmp(file.filename(), "BEATS"))
            ||(!_stricmp(file.filename(), "CH"))       || (!_stricmp(file.filename(), "CO"))
            ||(!_stricmp(file.filename(), "CR"))       || (!_stricmp(file.filename(), "DS"))
            ||(!_stricmp(file.filename(), "HC"))       || (!_stricmp(file.filename(), "MH"))
            ||(!_stricmp(file.filename(), "MR"))       || (!_stricmp(file.filename(), "NJ"))
            ||(!_stricmp(file.filename(), "RE"))       || (!_stricmp(file.filename(), "RG")))
            {
                file.behaviour = SetType(file.hash, Type::StreamPak);
                return MODLOADER_BEHAVIOUR_YES;
            }
        }
        else if(file.is_ext("dat"))
        {
            // Data files hashes, must be lower case (normalized), to check against file hash
            static const auto traklkup_dat = modloader::hash("traklkup.dat");
            static const auto strmpaks_dat = modloader::hash("strmpaks.dat");

            // Check if this is an bank loader data file by checking it's filename hash
            if(file.hash == traklkup_dat)
                file.behaviour = SetType(file.hash, Type::StreamLookUp);
            else if(file.hash == strmpaks_dat)
                file.behaviour = SetType(file.hash, Type::PakFiles);
            else
                return MODLOADER_BEHAVIOUR_NO;

            return MODLOADER_BEHAVIOUR_YES;
        }
        else if(file.is_ext("ogg"))
        {
            // TODO
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
    switch(GetType(file.behaviour))
    {
        case Type::Ogg:             return false; // TODO
        case Type::StreamPak:       return false; // TODO
        case Type::StreamLookUp:    return ov_traklkup.InstallFile(file);
        case Type::PakFiles:        return ov_strmpaks.InstallFile(file);
    }
    return false;
}

/*
 *  ThePlugin::ReinstallFile
 *      Reinstall a file previosly installed that has been updated
 */
bool ThePlugin::ReinstallFile(const modloader::file& file)
{
    switch(GetType(file.behaviour))
    {
        case Type::Ogg:             return false; // TODO
        case Type::StreamPak:       return false; // TODO
        case Type::StreamLookUp:    return ov_traklkup.ReinstallFile();
        case Type::PakFiles:        return ov_strmpaks.ReinstallFile();
    }
    return false;
}

/*
 *  ThePlugin::UninstallFile
 *      Uninstall a previosly installed file
 */
bool ThePlugin::UninstallFile(const modloader::file& file)
{
    switch(GetType(file.behaviour))
    {
        case Type::Ogg:             return false; // TODO
        case Type::StreamPak:       return false; // TODO
        case Type::StreamLookUp:    return ov_traklkup.UninstallFile();
        case Type::PakFiles:        return ov_strmpaks.UninstallFile();
    }
    return false;
}

/*
 *  ThePlugin::Update
 *      Updates the state of this plugin after a serie of install/uninstalls
 */
void ThePlugin::Update()
{
}
