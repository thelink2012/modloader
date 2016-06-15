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
    None,
    Hate, Dislike, Like, Respect,  // SA
    Threat, Avoid,                 // III/VC
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
            { "Threat", PedRelationship::Threat },
            { "Avoid", PedRelationship::Avoid },
        };
        return xmap;
    }
};


//
struct ped_traits : public data_traits
{
    static const bool has_sections      = false;
    static const bool per_line_section  = false;

    using key_type      = std::pair<size_t, PedRelationship>;
    using value_type    = data_slice<either< std::tuple<PedRelationship, set<string>>, std::tuple<string, VC3Only<pack<real_t, 5>>> >>;

    // key_from_value
    struct key_from_value_visitor : gta3::data_section_visitor<key_type>
    {
        ped_traits& traits;
        key_from_value_visitor(ped_traits& traits) : traits(traits) {}

        key_type operator()(const std::tuple<PedRelationship, set<string>>& tuple) const
        {
            return key_type(traits.pedtype, get<0>(tuple));
        }

        key_type operator()(const std::tuple<string, VC3Only<pack<real_t, 5>>>& tuple) const
        {
            traits.pedtype = modloader::hash(get<0>(tuple));
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

struct ped_traits_sa : public ped_traits
{
    struct dtraits : modloader::dtraits::OpenFile
    {
        static const char* what() { return "ped relationship data"; }
    };
    
    using detour_type = modloader::OpenFileDetour<0x608B45, dtraits>;
};

struct ped_traits_3vc : public ped_traits
{
    struct dtraits : modloader::dtraits::LoadFile
    {
        static const char* what() { return "ped relationship data"; }
    };
    
    using detour_type = modloader::LoadFileDetour<xVc(0x530BD7), dtraits>;
};


template<typename Traits>
using ped_store = gta3::data_store<Traits, std::map<
                        typename Traits::key_type, typename Traits::value_type
                        >>;

using ped_store_sa = ped_store<ped_traits_sa>;
using ped_store_3vc = ped_store<ped_traits_3vc>;

static auto xinit = initializer([](DataPlugin* plugin_ptr)
{
    auto ReloadPedRelationship = [] {}; // refreshing ped relationship during gameplay might break save game, don't do it at all

    if(gvm.IsSA())
        plugin_ptr->AddMerger<ped_store_sa>("ped.dat", true, false, false, reinstall_since_load, gdir_refresh(ReloadPedRelationship));
    else
        plugin_ptr->AddMerger<ped_store_3vc>("ped.dat", true, false, false, reinstall_since_load, gdir_refresh(ReloadPedRelationship));
});
