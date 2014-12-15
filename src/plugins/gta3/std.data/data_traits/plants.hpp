/*
 * Copyright (C) 2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#pragma once
#include <stdinc.hpp>

//
struct plants_traits : gta3::data_traits
{
    static const bool is_reversed_kv    = false;    // Does the key contains the data instead of the value in the key-value pair?
    static const bool has_sections      = false;    // Does this data file contains sections?
    static const bool per_line_section  = false;    // Is the sections of this data file different on each line?

    // Dominance Flags
    using domflags_fn = datalib::domflags_fn<flag_RemoveIfNotExistInOneCustomButInDefault>;

    template<class TData>
    std::pair<std::size_t, int> key_from_value(TData& value)
    {
        return std::make_pair(std::hash<std::string>()(get<0>(value)), get<1>(value)); 
    }

    // Detouring traits
    struct dtraits : modloader::dtraits::OpenFile
    {
        static const char* what() { return "plants surface properties"; }
    };
    
    // Detouring type
    using detour_type = modloader::OpenFileDetour<0x5DD3D1, dtraits>;
};

//
using plants_store = gta3::data_store<plants_traits, std::map<
                        std::pair<std::size_t, int>,
                        data_slice<std::string,
                                  int, int, int, int, int, int, int, int, int, int,
                                  real_t, real_t, real_t, real_t, real_t, real_t, real_t>
                        >>;

