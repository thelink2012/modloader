/*
 * Copyright (C) 2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#include <stdinc.hpp>
#include "../data.hpp"
using namespace modloader;
using std::string;

//
struct surfaud_traits : public data_traits
{
    static const bool can_cache         = true;     // Can this store get cached?
    static const bool is_reversed_kv    = false;    // Does the key contains the data instead of the value in the key-value pair?
    static const bool has_sections      = false;    // Does this data file contains sections?
    static const bool per_line_section  = false;    // Is the sections of this data file different on each line?

    // Detouring traits
    struct dtraits : modloader::dtraits::OpenFile
    {
        static const char* what() { return "surface audio infos"; }
    };
    
    // Detouring type
    using detour_type = modloader::OpenFileDetour<0x55F2C1, dtraits>;

    // Dominance Flags
    using domflags_fn = datalib::domflags_fn<flag_RemoveIfNotExistInOneCustomButInDefault>;

    using key_type      = size_t;
    using value_type    = data_slice<string, bool, bool, bool, bool, bool, bool, bool, bool, bool>;

    key_type key_from_value(const value_type& value)
    {
        return modloader::hash(get<0>(value));
    }
};

//
using surfaud_store = gta3::data_store<surfaud_traits, std::map<
                        surfaud_traits::key_type, surfaud_traits::value_type
                        >>;


//
static auto xinit = initializer([](DataPlugin* plugin_ptr)
{
    auto ReloadSurfaceInfo = std::bind(injector::thiscall<void(void*)>::call<0x55F420>, mem_ptr(0xB79538).get<void>());
    plugin_ptr->AddMerger<surfaud_store>("surfaud.dat", true, false, false, reinstall_since_start, gdir_refresh(ReloadSurfaceInfo));
});
