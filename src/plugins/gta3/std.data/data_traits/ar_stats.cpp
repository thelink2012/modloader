/*
 * Copyright (C) 2015  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under the MIT License, see LICENSE at top level directory.
 * 
 */
#include <stdinc.hpp>
#include "../data_traits.hpp"
using namespace modloader;

struct ar_stats_traits : public data_traits
{
    static const bool has_sections      = false;
    static const bool per_line_section  = false;

    struct dtraits : modloader::dtraits::OpenFile
    {
        static const char* what() { return "action reaction stats"; }
    };
    
    using detour_type = modloader::OpenFileDetour<0x5599D8, dtraits>;

    using key_type      = int;
    using value_type    = data_slice<int, dummy_string, real_t>;

    key_type key_from_value(const value_type& value)
    {
        return get<0>(value);
    }
};

using ar_stats_store = gta3::data_store<ar_stats_traits, std::map<
                        ar_stats_traits::key_type, ar_stats_traits::value_type
                        >>;


static auto xinit = initializer([](DataPlugin* plugin_ptr)
{
    if(gvm.IsSA())
    {
        auto ReloadArStats = injector::cstd<void()>::call<0x5599B0>;
        plugin_ptr->AddMerger<ar_stats_store>("ar_stats.dat", true, false, false, reinstall_since_load, gdir_refresh(ReloadArStats));
    }
});

