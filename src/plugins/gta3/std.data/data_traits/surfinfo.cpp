/*
 * Copyright (C) 2015  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under the MIT License, see LICENSE at top level directory.
 * 
 */
#include <stdinc.hpp>
#include "../data_traits.hpp"
using namespace modloader;
using std::string;

struct surfinfo_traits : public data_traits
{
    static const bool has_sections      = false;
    static const bool per_line_section  = false;

    struct dtraits : modloader::dtraits::OpenFile
    {
        static const char* what() { return "surface infos"; }
    };
    
    using detour_type = modloader::OpenFileDetour<0x55EBA4, dtraits>;

    using key_type      = size_t;
    using value_type    = data_slice<string, string, real_t, real_t, string, string, pack<char, 29>, string>;

    key_type key_from_value(const value_type& value)
    {
        return modloader::hash(get<0>(value));
    }
};

using surfinfo_store = gta3::data_store<surfinfo_traits, std::map<
                        surfinfo_traits::key_type, surfinfo_traits::value_type
                        >>;


static auto xinit = initializer([](DataPlugin* plugin_ptr)
{
    if(gvm.IsSA())
    {
        auto ReloadSurfaceInfo = std::bind(injector::thiscall<void(void*)>::call<0x55F420>, mem_ptr(0xB79538).get<void>());
        plugin_ptr->AddMerger<surfinfo_store>("surfinfo.dat", true, false, false, reinstall_since_start, gdir_refresh(ReloadSurfaceInfo));
    }
});
