/*
 * Copyright (C) 2015  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 *
 */
#include <stdinc.hpp>

enum class Type
{
    Ogg             = 0,
    StreamPak       = 1,    // StrmPak.dat
    StreamLookUp    = 2,    // StrmLkUp.dat
    PakFiles        = 3,    // ...
    Max             = 7,    // Max 3 bits
};


// Basical maskes
static const uint64_t hash_mask_base  = 0xFFFFFFFF;
static const uint64_t type_mask_base  = 0x0007;                 // Mask for type without any shifting
static const uint32_t type_mask_shf  = 32;                      // Takes 3 bits, starts from 33th bit because first 32th bits is a hash

// Sets the initial value for a behaviour, by using an filename hash and file type
inline uint64_t SetType(uint32_t hash, Type type)
{
    return modloader::file::set_mask(uint64_t(hash), type_mask_base, type_mask_shf, type);
}

// Gets the behaviour file type
inline Type GetType(uint64_t mask)
{
    return modloader::file::get_mask<Type>(mask, type_mask_base, type_mask_shf);
}


/*
 *  The plugin object
 */
class ThePlugin : public modloader::basic_plugin
{
    private:
        modloader::file_overrider ov_strmpaks;                  // StrmPaks.dat overrider
        modloader::file_overrider ov_traklkup;                  // TrakLkUp.dat overrider
        std::map<std::string, const modloader::file*> streams;  // Stream paks

    public:
        const info& GetInfo();

        bool OnStartup();
        bool OnShutdown();
        int GetBehaviour(modloader::file&);
        bool InstallFile(const modloader::file&);
        bool ReinstallFile(const modloader::file&);
        bool UninstallFile(const modloader::file&);

    private: // Patching stuff
        static void RedirectToThunk(injector::memory_pointer_tr call_at, injector::memory_pointer_tr thunk_at);

        template<uintptr_t CatAt>
        static void PatchStreamCat();

        template<uintptr_t NewAt>
        static void AllocMaxPath();

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
        // Make the CreateFileA caller call a function not a function pointer
        RedirectToThunk(0x4E0A09, raw_ptr(CreateFileA));
        RedirectToThunk(0x4E0989, raw_ptr(CreateFileA));

        // Let the strcat buffer that holds the path to the stream files be large enought to hold a full path
        AllocMaxPath<0x4E0AF2>();
        AllocMaxPath<0x4E0C80>();
        AllocMaxPath<0x4E0DA2>();

        // Patch the lstrcatA calls to detour streams
        PatchStreamCat<0x4E0AF9>();
        PatchStreamCat<0x4E0C9D>();
        PatchStreamCat<0x4E0DA9>();

        // Overriders
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
        case Type::Ogg:             return false;
        case Type::StreamPak:       this->streams.emplace(file.filename(), &file); return true;
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
        case Type::Ogg:             return false;
        case Type::StreamPak:       return false;
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
        case Type::Ogg:             return false;
        case Type::StreamLookUp:    return ov_traklkup.UninstallFile();
        case Type::PakFiles:        return ov_strmpaks.UninstallFile();
        case Type::StreamPak:
                                    if(!this->loader->has_game_started) 
                                    {
                                        this->streams.erase(file.filename());
                                        return true;
                                    }
                                    return false;
    }
    return false;
}


/**************************************/

void ThePlugin::RedirectToThunk(injector::memory_pointer_tr call_at, injector::memory_pointer_tr thunk_at)
{
    using namespace injector;
    if(ReadMemory<uint8_t>(call_at+0, true) == 0xFF && ReadMemory<uint8_t>(call_at+1, true) == 0x15)
    {
        MakeNOP(call_at, 6, true);
        MakeCALL(call_at, thunk_at.get(), true);
    }
}

template<uintptr_t NewAt>
void ThePlugin::AllocMaxPath()
{
    using namespace injector;
    make_static_hook<function_hooker<NewAt, void*(size_t)>>([](std::function<void*(size_t)> new_, size_t size)
    {
        return size < MAX_PATH? new_(MAX_PATH) : new_(size);
    });
}

template<uintptr_t CatAt>
void ThePlugin::PatchStreamCat()
{
    using lstrcatA_type = LPTSTR(__stdcall*)(LPTSTR, LPTSTR);
    auto mycat = [](LPTSTR dest, LPTSTR cat) -> LPTSTR
    {
        auto& plugin = plugin_ptr->cast<ThePlugin>();
        auto it = plugin.streams.find(NormalizePath(cat));
        if(it != plugin.streams.end())
            return strcpy(dest, it->second->fullpath().c_str());
        else
            return strcat(dest, cat);
    };

    static lstrcatA_type ptr = (lstrcatA_type)(mycat);
    WriteMemory(CatAt, &ptr, true);
}
