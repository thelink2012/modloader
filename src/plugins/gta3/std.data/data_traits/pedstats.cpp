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
struct pedstats_traits : public data_traits
{
    static const bool can_cache         = true;     // Can this store get cached?
    static const bool is_reversed_kv    = false;    // Does the key contains the data instead of the value in the key-value pair?
    static const bool has_sections      = false;    // Does this data file contains sections?
    static const bool per_line_section  = false;    // Is the sections of this data file different on each line?

    // Detouring traits
    struct dtraits : modloader::dtraits::OpenFile
    {
        static const char* what() { return "ped stats"; }
    };
    
    // Detouring type
    using detour_type = modloader::OpenFileDetour<0x5BB89F, dtraits>;

    // Dominance Flags
    using domflags_fn = datalib::domflags_fn<flag_RemoveIfNotExistInOneCustomButInDefault>;

    // pedstats data
    using key_type      = int;
    using value_type    = data_slice<string, real_t, real_t, int, int, int, int, real_t, real_t, int, int>;

    key_type key_from_value(const value_type&)
    {
        return stat_line++;
    }

public:
    int stat_line = 0;

    template<class Archive>
    void serialize(Archive& archive)
    { archive(this->stat_line); }
};

//
using pedstats_store = gta3::data_store<pedstats_traits, std::map<
                        pedstats_traits::key_type, pedstats_traits::value_type
                        >>;


// Ped Stats Merger
static auto xinit = initializer([](DataPlugin* plugin_ptr)
{
    auto ReloadPedStats = injector::cstd<void()>::call<0x5BB890>;
    plugin_ptr->AddMerger<pedstats_store>("pedstats.dat", true, false, false, reinstall_since_start, gdir_refresh(ReloadPedStats));
});

