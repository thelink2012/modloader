/*
 * Copyright (C) 2015  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under the MIT License, see LICENSE at top level directory.
 * 
 */
#include <stdinc.hpp>
#include "../data_traits.hpp"
using namespace modloader;
using std::string;

struct streamini_traits : public data_traits
{
    static const bool has_sections      = false;
    static const bool per_line_section  = false;

    struct dtraits : modloader::dtraits::OpenFile
    {
        static const char* what() { return "stream config"; }
    };
    
    using detour_type = modloader::OpenFileDetour<0x5BCCDE, dtraits>;

    using key_type      = size_t;
    using value_type    = data_slice<insen<string>, either<int, real_t>>;

    key_type key_from_value(const value_type& value)
    {
        return hash_model(get<0>(value));   // not a model but insensitive hash
    }
};

using streamini_store = gta3::data_store<streamini_traits, std::map<
                        streamini_traits::key_type, streamini_traits::value_type
                        >>;


static auto xinit = initializer([](DataPlugin* plugin_ptr)
{
    if(gvm.IsSA())
    {
        plugin_ptr->AddMerger<streamini_store>("stream.ini", true, false, false, no_reinstall);
    }
});

