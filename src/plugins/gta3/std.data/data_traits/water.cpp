/*
 * Copyright (C) 2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#include <stdinc.hpp>
#include "../data.hpp"
using namespace modloader;
using namespace std::placeholders;
using std::tuple;

// A water point defines a vertice and some water properties related to this vertice
// Notice we are using 'float' type on the XY instead of 'real_t', that's because water.dat XY floats are actually integers
// So, since XY is rounded, there's no need for taking time to do a epsilon comparision nor have precision
using water_point = std::tuple<float, float, real_t, real_t, real_t, real_t, real_t>;
using xyz         = std::tuple<int, int, real_t>;

static xyz make_xyz(const water_point& wp)
{
    return xyz((int)(get<0>(wp)), (int)(get<1>(wp)), get<2>(wp));
}

static xyz make_xyz(const optional<water_point>& wp)
{
    return make_xyz(wp.get());
}

template<size_t N, class value_type>
static void set_xyz_detail(std::integral_constant<size_t, N>, std::array<xyz, N>& pos, const value_type& data)
{
}

template<size_t I, size_t N, class value_type>
static void set_xyz_detail(std::integral_constant<size_t, I>, std::array<xyz, N>& pos, const value_type& data)
{
    pos[I] = make_xyz(const_cast<value_type&>(data).get<I>());
    return set_xyz_detail(std::integral_constant<size_t, I+1>(), pos, data);
}

template<size_t N, class value_type>
static std::array<xyz, N> make_xyz_array(const value_type& data)
{
    std::array<xyz, N> arr;
    set_xyz_detail(std::integral_constant<size_t, 0>(), arr, data);
    return arr;
}




//
struct water_traits : public data_traits
{
    static const bool can_cache         = true;     // Can this store get cached?
    static const bool is_reversed_kv    = false;    // Does the key contains the data instead of the value in the key-value pair?
    static const bool has_sections      = false;    // Does this data file contains sections?
    static const bool per_line_section  = false;    // Is the sections of this data file different on each line?

    // Detouring traits
    struct dtraits : modloader::dtraits::OpenFile
    {
        static const char* what() { return "water level"; }
    };
    
    // Detouring type
    using detour_type = modloader::OpenFileDetour<0x6EAF4D, dtraits>;

    // Dominance Flags
    using domflags_fn = datalib::domflags_fn<flag_RemoveIfNotExistInOneCustomButInDefault>;

    // water data
    using key_type      = either<std::array<xyz, 3>, std::array<xyz, 4>>;
    using value_type    = data_slice<water_point, water_point, water_point, optional<water_point>, delimopt, int>;

    key_type key_from_value(const value_type& data)
    {
        if(get<3>(data))   // is quad?
            return make_xyz_array<4>(data);
        else
            return make_xyz_array<3>(data);
    }

    template<class StoreType>
    static bool setbyline(StoreType& store, value_type& data, const gta3::section_info* section, const std::string& line)
    {
        if(line[0] != '*' && line[0] != 'p')    // the water.dat parser from the game ignores such cases, so we do.
            return gta3::data_traits::setbyline(store, data, section, line);
        return false;
    }
};

//
using water_store = gta3::data_store<water_traits, std::map<
                        water_traits::key_type, water_traits::value_type
                        >>;


// Water Level Merger
static auto xinit = initializer([](DataPlugin* plugin_ptr)
{
    auto ReloadWater = injector::cstd<void()>::call<0x6EAE80>;
    plugin_ptr->AddMerger<water_store>("water.dat", true, false, false, reinstall_since_load, gdir_refresh(ReloadWater));
});


