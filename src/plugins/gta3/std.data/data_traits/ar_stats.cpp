/*
 * Copyright (C) 2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#include <stdinc.hpp>
#include "../data.hpp"
using namespace modloader;

//
struct ar_stats_traits : public data_traits
{
    static const bool can_cache         = true;     // Can this store get cached?
    static const bool is_reversed_kv    = false;    // Does the key contains the data instead of the value in the key-value pair?
    static const bool has_sections      = false;    // Does this data file contains sections?
    static const bool per_line_section  = false;    // Is the sections of this data file different on each line?

    // Detouring traits
    struct dtraits : modloader::dtraits::OpenFile
    {
        static const char* what() { return "action reaction stats"; }
    };
    
    // Detouring type
    using detour_type = modloader::OpenFileDetour<0x5599D8, dtraits>;

    // Dominance Flags
    using domflags_fn = datalib::domflags_fn<flag_RemoveIfNotExistInOneCustomButInDefault>;

    // ar_stats data
    using key_type      = int;
    using value_type    = data_slice<int, dummy_string, real_t>;

    key_type key_from_value(const value_type& value)
    {
        return get<0>(value);
    }
};

//
using ar_stats_store = gta3::data_store<ar_stats_traits, std::map<
                        ar_stats_traits::key_type, ar_stats_traits::value_type
                        >>;


// ar_stats Surface Properties Merger
static auto xinit = initializer([](DataPlugin* plugin_ptr)
{
    auto ReloadArStats = injector::cstd<void()>::call<0x5599B0>;
    plugin_ptr->AddMerger<ar_stats_store>("ar_stats.dat", true, false, false, reinstall_since_load, gdir_refresh(ReloadArStats));
});

