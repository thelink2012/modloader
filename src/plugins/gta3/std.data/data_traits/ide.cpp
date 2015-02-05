/*
 * Copyright (C) 2015  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#include <stdinc.hpp>
#include "../data_traits.hpp"
#include <traits/gta3/sa.hpp>
using namespace modloader;
using std::tuple;
using std::string;
using ipair = std::pair<int, int>;

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
    static const bool has_sections      = true;     // Does this data file contains sections?
    static const bool per_line_section  = false;

    // Detouring traits
    struct dtraits : modloader::dtraits::OpenFile
    {
        static const char* what()       { return "object types"; }
        static const char* datafile()   { return ide_merger_name; }
    };
    
    // Detouring type
    using detour_type = modloader::OpenFileDetour<0x5B8428, dtraits>;

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

    // Specialize this one so we can filter out IDE readme entries for specific files
    template<class StoreType>
    static DataPlugin::readme_data_list<StoreType>
        query_readme_data(const std::string& filename)
    {
        DataPlugin::readme_data_list<StoreType> list;
        bool is_peds_ide     = (filename == "peds.ide");
        bool is_vehicles_ide = (filename == "vehicles.ide");
        bool is_vehmods_ide  = (filename == "veh_mods.ide");

        if(is_peds_ide || is_vehicles_ide || is_vehmods_ide)   // can only have readme entries for those
        {
            list = data_traits::query_readme_data<StoreType>(filename);
            auto section = gta3::section_info::by_name(sections(), is_peds_ide? "peds" :
                                                                   is_vehicles_ide? "cars" :
                                                                   is_vehmods_ide? "objs" : "");

            // Filter outs readme stores that aren't related to the section type related to this file
            for(auto it = list.begin(); it != list.end(); )
            {
                auto& container = it->second.second.get().container();
                if(std::any_of(container.begin(), container.end(), [&](const std::pair<const key_type, value_type>& kv) {
                    return (StoreType::section_by_kv(kv.first, kv.second) != section);
                }))
                    it = list.erase(it);
                else
                    ++it;
            }
        }

        return list;
    }
};

//
using ide_store = gta3::data_store<ide_traits, std::map<
                        ide_traits::key_type, ide_traits::value_type
                        >>;

REGISTER_RTTI_FOR_ANY(ide_store);


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

static std::function<void()> MakeIdeReloader()
{
    using namespace injector;
    static std::set<std::string> ide_files;
    static bool has_new_sys = false;

    using loadide_hook = function_hooker<0x5B9206, void(const char*)>;
    using emptyide_hook = function_hooker<0x5B8428, void*(const char*, const char*)>;

    make_static_hook<loadide_hook>([](loadide_hook::func_type LoadObjectTypes, const char* filename)
    {
        ide_files.emplace(filename);
        return LoadObjectTypes(filename);
    });


    return []
    {
        auto AddOrMatchModel = [](std::function<void*(int)> Add, int id)
        {
            auto p = TraitsSA::GetModelInfo(id);
            if(p != nullptr) return p;
            return Add(id);
        };

        auto emptyide_mf = make_function_hook<emptyide_hook>([](emptyide_hook::func_type fopen, const char* filename, const char* mode) {
            void* f = fopen(filename, mode);
            return f? f : fopen(modloader::szNullFile, mode);
        });

        auto obj0_mf = make_function_hook<function_hooker<0x5B3D8E, void*(int id)>>(AddOrMatchModel);
        auto obj1_mf = make_function_hook<function_hooker<0x5B3D9A, void*(int id)>>(AddOrMatchModel);
        auto tobj_mf = make_function_hook<function_hooker<0x5B3F32, void*(int id)>>(AddOrMatchModel);
        auto weap_mf = make_function_hook<function_hooker<0x5B3FE6, void*(int id)>>(AddOrMatchModel);
        auto hier_mf = make_function_hook<function_hooker<0x5B407E, void*(int id)>>(AddOrMatchModel);
        auto anim_mf = make_function_hook<function_hooker<0x5B413B, void*(int id)>>(AddOrMatchModel);
        auto cars_mf = make_function_hook<function_hooker<0x5B6FD1, void*(int id)>>(AddOrMatchModel);
        auto peds_mf = make_function_hook<function_hooker<0x5B74A7, void*(int id)>>(AddOrMatchModel);

        scoped_nop<5> nop_ide_2dfx(0x5B86B7, 5);

        void(*LoadObjectTypes)(const char*) = ReadRelativeOffset(0x5B9206 + 1).get();
        for(auto& ide : ide_files)
        {
            if(ide == "data\\maps\\generic\\vegepart.ide" /*|| ide == "data\\maps\\generic\\procobj.ide"*/)
                LoadObjectTypes(ide.c_str());
        }
    };
}


// Object Types Merger
static auto xinit = initializer([](DataPlugin* plugin_ptr)
{
    // IDE Merger
    plugin_ptr->AddMerger<ide_store>(ide_merger_name, false, false, true, reinstall_since_load, MakeIdeReloader());

    // Readme Reader for CARS entries (vehicles.ide)
    plugin_ptr->AddReader<ide_store>([](const std::string& line) -> maybe_readable<ide_store>
    {
        static auto regex_vehicles = make_fregex(
                            "^%d %s %s "
                            "%{car|mtruck|quad|heli|f_heli|plane|f_plane|boat|train|bike|bmx|trailer} "
                            "%s %s %s "
                            "%{normal|poorfamily|richfamily|executive|worker|big|taxi|moped|motorbike|leisureboat|workerboat|bicycle|ignore} "
                            "%d %d %x(?: %d)?(?: %f)?(?: %f)?(?: %d)?$");

        if(regex_match(line, regex_vehicles))
        {
            ide_store store;
            if(store.insert<ide_traits::cars_type>(line))
                return store;
        }
        return nothing;
    });

    // Readme Reader for tunning OBJS entries (veh_mods.ide)
    plugin_ptr->AddReader<ide_store>([](const std::string& line) -> maybe_readable<ide_store>
    {
        static auto regex_vehmods = make_fregex(
            "^%d "
            R"___(%{hydralics|stereo|wheel_\w+|nto_\w+|bnt_\w+|chss_\w+|exh_\w+|bntl_\w+|bntr_\w+|spl_\w+|wg_l_\w+|wg_r_\w+|fbb_\w+|bbb_\w+|lgt_\w+|rf_\w+|fbmp_\w+|rbmp_\w+|misc_a_\w+|misc_b_\w+|misc_c_\w+})___"
            " %s %d %d$");

        if(regex_match(line, regex_vehmods))
        {
            ide_store store;
            if(store.insert<ide_traits::objs_type>(line))
                return store;
        }
        return nothing;
    });

    // Readme Reader for tunning PEDS entries (peds.ide)
    plugin_ptr->AddReader<ide_store>([](const std::string& line) -> maybe_readable<ide_store>
    {
        static auto regex_vehmods = make_fregex(
            "^%d %s %s "
            "%{CIVMALE|CIVFEMALE|COP|GANG\\d+|PLAYER1|PLAYER2|PLAYER_NETWORK|PLAYER_UNUSED|DEALER|MEDIC|FIREMAN|CRIMINAL|BUM|PROSTITUTE|SPECIAL|MISSION\\d+} "
            "%{STAT_\\w+} %s %x %x %s %d %d %{PED_TYPE_\\w+} %{VOICE_\\w+} %{VOICE_\\w+}$");

        if(regex_match(line, regex_vehmods))
        {
            ide_store store;
            if(store.insert<ide_traits::peds_type>(line))
                return store;
        }
        return nothing;
    });
});
