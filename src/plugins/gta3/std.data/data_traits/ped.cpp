/*
 * Copyright (C) 2015  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under the MIT License, see LICENSE at top level directory.
 * 
 */
#include <stdinc.hpp>
#include "../data_traits.hpp"
using namespace modloader;
using std::set;
using std::string;

enum class PedRelationship : uint8_t
{
    None, Hate, Dislike, Like, Respect
};

template<>
struct enum_map<PedRelationship>
{
    static std::map<string, PedRelationship>& map()
    {
        static std::map<string, PedRelationship> xmap = {
            { "Hate", PedRelationship::Hate }, 
            { "Like", PedRelationship::Like }, 
            { "Dislike", PedRelationship::Dislike }, 
            { "Respect", PedRelationship::Respect }, 
        };
        return xmap;
    }
};


//
struct ped_traits : public data_traits
{
    static const bool has_sections      = false;
    static const bool per_line_section  = false;

    struct dtraits : modloader::dtraits::OpenFile
    {
        static const char* what() { return "ped relationship data"; }
    };
    
    using detour_type = modloader::OpenFileDetour<0x608B45, dtraits>;

    using key_type      = std::pair<size_t, PedRelationship>;
    using value_type    = data_slice<either<std::tuple<PedRelationship, set<string>>, string>>;

    // key_from_value
    struct key_from_value_visitor : gta3::data_section_visitor<key_type>
    {
        ped_traits& traits;
        key_from_value_visitor(ped_traits& traits) : traits(traits) {}

        key_type operator()(const std::tuple<PedRelationship, set<string>>& tuple) const
        {
            return key_type(traits.pedtype, get<0>(tuple));
        }

        key_type operator()(const string& pedtype) const
        {
            traits.pedtype = modloader::hash(pedtype);
            return key_type(traits.pedtype, PedRelationship::None); 
        }

        key_type operator()(const either_blank& slice) const
        { throw std::invalid_argument("blank type"); }
    };

    key_type key_from_value(const value_type& value)
    {
        return get<0>(value).apply_visitor(key_from_value_visitor(*this));
    }

    public:

        size_t pedtype = 0;   // Working ped type hash

        template<class Archive>
        void serialize(Archive& archive)
        { archive(this->pedtype); }
};

using ped_store = gta3::data_store<ped_traits, std::map<
                        ped_traits::key_type, ped_traits::value_type
                        >>;

static auto xinit = initializer([](DataPlugin* plugin_ptr)
{
    auto ReloadPedRelationship = []{}; // refreshing ped relationship during gameplay might break save game, don't do it at all
    plugin_ptr->AddMerger<ped_store>("ped.dat", true, false, false, reinstall_since_load, gdir_refresh(ReloadPedRelationship));
});

