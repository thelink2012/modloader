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
using std::string;
using ipair = std::pair<int, int>;

// TODO refresh?

// Aliases of the possible endings of the OBJS and TOBJ lines
using objs0e = tuple<real_t, int>;
using objs1e = tuple<int, real_t, int>;
using objs2e = tuple<int, real_t, real_t, int>;
using objs3e = tuple<int, real_t, real_t, real_t, int>;
using tobj0e = tuple<objs0e, int16_t, int16_t>; // use int16 instead of int8 for hours, int8 is readen as character
using tobj1e = tuple<objs1e, int16_t, int16_t>;
using tobj2e = tuple<objs2e, int16_t, int16_t>;
using tobj3e = tuple<objs3e, int16_t, int16_t>;

// Aliases of possible endings of the 2DFX lines
// fx<gtasaexe_switch_id>_<read_order_inversed>e    (read order inversed is the order at either<...> inversed)
using fx8_2e = tuple<int>;
using fx1_1e = tuple<string>;
using fx9_3e = tuple<real_t, real_t, int>;
using fx10_5e = tuple<real_t, real_t, real_t, real_t, real_t, real_t, real_t, real_t, real_t, int>;
using fx6_6e = tuple<real_t, real_t, real_t, real_t, real_t, real_t, real_t, int, int, string, int>;
using fx3_7e = tuple<int, real_t, real_t, real_t, real_t, real_t, real_t, real_t, real_t, real_t, int, int, string>;
using fx0_8e = tuple<int, int, int, int, string, string, real_t, real_t, real_t, real_t, int, int, int, int, int, int, int, int, int>;
using fx7_4e = tuple<real_t, real_t, real_t, real_t, real_t, int, optional<string>, optional<string>, optional<string>, optional<string>>;
using fx5_9e = tuple<int, real_t, real_t, real_t, real_t, ipair,  ipair, ipair, ipair, ipair, ipair, ipair, ipair, ipair, ipair, ipair, ipair, ipair, ipair>;


//
struct ide_traits : public data_traits
{
    static const bool can_cache         = true;     // Can this store get cached?
    static const bool is_reversed_kv    = false;    // Does the key contains the data instead of the value in the key-value pair?
    static const bool has_sections      = true;     // Does this data file contains sections?
    static const bool per_line_section  = false;    // Is the sections of this data file different on each line?

    // Detouring traits
    struct dtraits : modloader::dtraits::OpenFile
    {
        static const char* what() { return "object types"; }
    };
    
    // Detouring type
    using detour_type = modloader::OpenFileDetour<0x5B8428, dtraits>;

    // Dominance Flags
    using domflags_fn = datalib::domflags_fn<flag_RemoveIfNotExistInOneCustomButInDefault>;

    // Section slices
    using path_type = data_slice<>;
    using txdp_type = data_slice<texname, texname>;
    using hier_type = data_slice<int, modelname, texname, delimopt, animname, real_t>;  /* (opt args not readen by the game but used in ides) */
    using anim_type = data_slice<int, modelname, texname, animname, real_t, int>;
    using weap_type = data_slice<int, modelname, texname, animname, int, real_t, delimopt, int>;  /* (opt args not readen by the game but used in ides) */
    using objs_type = data_slice<int, modelname, texname, either<objs3e, objs2e, objs1e, objs0e>>;
    using tobj_type = data_slice<int, modelname, texname, either<tobj3e, tobj2e, tobj1e, tobj0e>>;
    using _2dfx_type= data_slice<int, vec3, int, either<fx5_9e, fx0_8e, fx3_7e, fx6_6e, fx10_5e, fx7_4e, fx9_3e, fx8_2e, fx1_1e>>;
    using peds_type = data_slice<int, modelname, texname, string, string, string, hex<uint32_t>, hex<uint32_t>, animname, int, int, string, string, string>;
    using cars_type = data_slice<int, modelname, texname, string, string, labelname, animname, string, int, int, hex<uint32_t>, delimopt, int, real_t, real_t, int>;

    // Data
    using key_type   = either<int, std::size_t, std::tuple<int, vec3, int>>;   // <int> for most sections, <size_t> for txdp, <int, vec3> for 2dfx
    using value_type = gta3::data_section<objs_type, tobj_type, hier_type, anim_type, weap_type, cars_type, peds_type, txdp_type, _2dfx_type>;

    static const gta3::section_info* sections()
    {
        // Note: must be in the same order as declared in value_type
        static auto sections = gta3::make_section_info("objs", "tobj", "hier", "anim", "weap", "cars", "peds", "txdp", "2dfx");
        static_assert(std::tuple_size<decltype(sections)>::value == 1 + value_type::num_sections, "incompatible sizes");
        return sections.data();
    }

    // key_from_value
    struct key_from_value_visitor : gta3::data_section_visitor<key_type>
    {
        // path section is not supported, so just output a dummy key
        key_type operator()(const path_type& slice) const
        { return key_type(/* empty */); }

        // txdp section key should be the child model, since the game sets {child->parent = parent;}
        key_type operator()(const txdp_type& slice) const
        { return hash_model(get<0>(slice)); }

        // 2dfx section associates existing models (more than once too) to a 2dfx effect type at a position
        // so the key should be: first the model id to associate, second the effect position and third the effect type
        key_type operator()(const _2dfx_type& slice) const
        { return std::make_tuple(get<0>(slice), get<1>(slice), get<2>(slice)); }

        // all the other section types have a id in the elem0, just pick it as the key
        template<class T>
        key_type operator()(const T& slice) const
        { return int(get<0>(slice)); }

        // and of course the following should never happen
        key_type operator()(const either_blank&) const
        { throw std::invalid_argument("blank type"); }
    };

    key_type key_from_value(const value_type& value)
    {
        return value.apply_visitor(key_from_value_visitor());
    }
};

//
using ide_store = gta3::data_store<ide_traits, std::map<
                        ide_traits::key_type, ide_traits::value_type
                        >>;


// sections function specialization
namespace datalib {
    namespace gta3
    {
        inline const section_info* sections(const ide_traits::value_type&)
        {
            return ide_traits::sections();
        }
    }
}

// Vehicle Upgrades Merger
static auto xinit = initializer(std::bind(&DataPlugin::AddMerger<ide_store>, _1, ide_merger_name, false, false, true, no_reinstall));

