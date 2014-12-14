#pragma once
#include <stdinc.hpp>

struct plants_traits : gta3::data_traits
{
    static const bool is_reversed_kv = false;
    static const bool has_sections = false;
    static const bool per_line_section = false;

    // "plants.dat" -> fs path
    
    using domflags_fn = datalib::domflags_fn<flag_RemoveIfNotExistInOneCustomButInDefault>;

    template<class TData>
    std::pair<std::size_t, int> key_from_value(TData& value)
    {
        return std::make_pair(std::hash<std::string>()(get<0>(value)), get<1>(value)); 
    }

    struct dtraits : modloader::dtraits::OpenFile    // detour traits
    {
        static const char* what() { return "plants surface properties"; }
    };

    using detour_type = modloader::OpenFileDetour<0x5DD3D1, dtraits>;
};

using plants_store = gta3::data_store<plants_traits, std::map<
                        std::pair<std::size_t, int>,
                        data_slice<std::string,
                                  int, int, int, int, int, int, int, int, int, int,
                                  real_t, real_t, real_t, real_t, real_t, real_t, real_t>
                        >>;

