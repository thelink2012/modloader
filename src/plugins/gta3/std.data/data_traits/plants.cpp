/*
 * Copyright (C) 2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#include <stdinc.hpp>
#include "../data.hpp"
using namespace modloader;
using namespace std::placeholders;

//
struct plants_traits : gta3::data_traits
{
    static const bool can_cache         = true;     // Can this store get cached?
    static const bool is_reversed_kv    = false;    // Does the key contains the data instead of the value in the key-value pair?
    static const bool has_sections      = false;    // Does this data file contains sections?
    static const bool per_line_section  = false;    // Is the sections of this data file different on each line?

    // Detouring traits
    struct dtraits : modloader::dtraits::OpenFile
    {
        static const char* what() { return "plants surface properties"; }
    };
    
    // Detouring type
    using detour_type = modloader::OpenFileDetour<0x5DD3D1, dtraits>;

    // Dominance Flags
    using domflags_fn = datalib::domflags_fn<flag_RemoveIfNotExistInOneCustomButInDefault>;

    // Plants data
    using key_type      = std::pair<std::size_t, int>;
    using value_type    = data_slice<std::string,
                            int, int, int, int, int, int, int, int, int, int,
                            real_t, real_t, real_t, real_t, real_t, real_t, real_t>;

    key_type key_from_value(const value_type& value)
    {
        return key_type(std::hash<std::string>()(get<0>(value)), get<1>(value)); 
    }
};

//
using plants_store = gta3::data_store<plants_traits, std::map<
                        plants_traits::key_type, plants_traits::value_type
                        >>;


// Plants Surface Properties Merger
static auto xinit = initializer([](DataPlugin* plugin_ptr)
{
    auto ReloadPlantsDat = [plugin_ptr]
    {
        if(!injector::cstd<char()>::call<0x5DD780>()) // CPlantMgr::ReloadConfig
            plugin_ptr->Log("Failed to refresh plant manager");
    };

    plugin_ptr->AddMerger<plants_store>("plants.dat", true, reinstall_since_load, ReloadPlantsDat);
});

