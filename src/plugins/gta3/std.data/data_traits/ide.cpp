/*
 * Copyright (C) 2015  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under the MIT License, see LICENSE at top level directory.
 * 
 */
#include <stdinc.hpp>
#include "../data_traits.hpp"
#include <traits/gta3/sa.hpp>
#include <interfaces/gta3/std.stream.hpp>
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

static std::function<void()> MakeIdeReloader();

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


/*
 *  IDE Refresher
 *      This is called once to return a functor responssible for refreshing IDEs.
 */
static std::function<void()> MakeIdeReloader()
{
    using namespace injector;
    using namespace std::placeholders;
    static std::set<std::string> ide_files; // List of all registered ide files

    using loadide_hook = function_hooker<0x5B9206, void(const char*)>;
    make_static_hook<loadide_hook>([](loadide_hook::func_type LoadObjectTypes, const char* filename)
    {
        ide_files.emplace(filename);
        return LoadObjectTypes(filename);
    });


    // The following functor will be called whenever IDEs need refresh
    return []
    {
        using emptyide_hook = function_hooker<0x5B8428, void*(const char*, const char*)>;

        // Those are used on each IDE line that is parsed
        int mWorkingModelId     = -1;
        bool mHasAnyChange      = false;

        // We need a streaming refresher to perform this, so let's communicate with std.stream
        IStreamRefresher* refresher = StreamRefresherCreate(STREAMREFRESHER_CAPABILITY_MODEL, 0);
        if(!refresher)
        {
            plugin_ptr->Log("Warning: Cannot refresh IDE files because std.stream.dll is missing!");
            return;
        }

        // In case the IDE file doesn't exist just open a blank file
        auto emptyide_mf = make_function_hook<emptyide_hook>([](emptyide_hook::func_type fopen, const char* filename, const char* mode) {
            void* f = fopen(filename, mode);
            return f? f : fopen(modloader::szNullFile, mode);
        });

        // Tells the refresher that something changed in THIS IDE LINE
        // This should be called before ANY actual change to the model info!!!!!!
        auto BeforeChange = [&]()
        {
            if(!mHasAnyChange)
            {
                mHasAnyChange = true;
                plugin_ptr->Log("Updating model %d definition...", mWorkingModelId);
                refresher->Clear();
                refresher->RequestRefresh(mWorkingModelId);
                refresher->PrepareRefresh();
                refresher->DestroyEntities();
                refresher->RemoveModels();
            }
        };


        //
        // Change Checkers
        //

        // Replacement for AddModelInfo that gets the existing model info instead of adding one
        auto AddOrMatchModel = [&](std::function<void*(int)> AddModelInfo, int id, TraitsSA::ModelType type)
        {
            mWorkingModelId = id;
            if(auto p = TraitsSA::GetModelInfo(id))
            {
                if(TraitsSA::GetModelType(id) == type)
                    return p;
                else
                {
                    plugin_ptr->Log("Warning: Model type for %d changed during definition refresh from %d to %d",
                                    id, TraitsSA::GetModelType(id), type);
                    BeforeChange();
                    // continue to alloc another struct for the new type
                }
            }
            return AddModelInfo(id);
        };

        // Checks if this model name changed, if it did request a refresh
        auto CheckModelChange = [&](std::function<uint32_t(const char*)> GetUppercaseKey, const char* modelname)
        {
            auto newkey = GetUppercaseKey(modelname);
            if(newkey != TraitsSA::GetModelKey(mWorkingModelId))
                BeforeChange();
            return newkey;
        };

        // Checks if this tex dcitionary changed, if it did request a refresh
        auto CheckTexDictionaryChange = [&](std::function<void(void*, const char*)> SetTexDictionary, void* modelinfo, const char* txdname)
        {
            auto FindTxdSlot = injector::cstd<int(const char*)>::call<0x731850>;
            if(FindTxdSlot(txdname) != TraitsSA::GetModelTxdIndex(mWorkingModelId))
            {
                BeforeChange();
                SetTexDictionary(modelinfo, txdname);
                refresher->RebuildTxdAssociationMap();
            }
        };

        //
        auto DummyColSet = [](std::function<void(void*, void*, int)> SetColModel, void* modelinfo, void* col, int b)
        {
            return;
        };

        //
        //  IDE Line Parsers (to refresh the line)
        //

        // Use this line parser when a specific model type isn't refreshable
        auto DummyLineParse = [](std::function<int(const char*)>, const char*) -> int
        { return -1; };

        // Use this line parser when the model type is refreshable
        auto LineParseRefresh = [&](std::function<int(const char*)> parse, const char* line) -> int
        {
            mHasAnyChange = false;
            auto i = parse(line);
            if(mHasAnyChange)
            {
                refresher->RequestModels();
                refresher->RecreateEntities();
            }
            return i;
        };

        // Refreshers parsers
        auto objs_parse = make_function_hook<function_hooker<0x5B85DD, int(const char*)>>(LineParseRefresh);
        auto tobj_parse = make_function_hook<function_hooker<0x5B862C, int(const char*)>>(LineParseRefresh);
        auto weap_parse = make_function_hook<function_hooker<0x5B8634, int(const char*)>>(LineParseRefresh);
        auto hier_parse = make_function_hook<function_hooker<0x5B863C, int(const char*)>>(DummyLineParse);      // nope cuz generic models + col change
        auto anim_parse = make_function_hook<function_hooker<0x5B8644, int(const char*)>>(LineParseRefresh);
        auto cars_parse = make_function_hook<function_hooker<0x5B864C, int(const char*)>>(LineParseRefresh);
        auto peds_parse = make_function_hook<function_hooker<0x5B8654, int(const char*)>>(DummyLineParse);      // nope cuz generic models + col change
        auto txdp_parse = make_function_hook<function_hooker<0x5B86C5, int(const char*)>>(DummyLineParse);
        auto a2dfx_parse= make_function_hook<function_hooker<0x5B86B7, int(const char*)>>(DummyLineParse);

        // AddOrMatchModel checkers
        auto obj0_mf = make_function_hook<function_hooker<0x5B3D8E, void*(int id)>>(std::bind(AddOrMatchModel, _1, _2, TraitsSA::ModelType::DamageAtomic));
        auto obj1_mf = make_function_hook<function_hooker<0x5B3D9A, void*(int id)>>(std::bind(AddOrMatchModel, _1, _2, TraitsSA::ModelType::Atomic));
        auto tobj_mf = make_function_hook<function_hooker<0x5B3F32, void*(int id)>>(std::bind(AddOrMatchModel, _1, _2, TraitsSA::ModelType::Time));
        auto weap_mf = make_function_hook<function_hooker<0x5B3FE6, void*(int id)>>(std::bind(AddOrMatchModel, _1, _2, TraitsSA::ModelType::Weapon));
        auto hier_mf = make_function_hook<function_hooker<0x5B407E, void*(int id)>>(std::bind(AddOrMatchModel, _1, _2, TraitsSA::ModelType::Clump));
        auto anim_mf = make_function_hook<function_hooker<0x5B413B, void*(int id)>>(std::bind(AddOrMatchModel, _1, _2, TraitsSA::ModelType::Clump));
        auto cars_mf = make_function_hook<function_hooker<0x5B6FD1, void*(int id)>>(std::bind(AddOrMatchModel, _1, _2, TraitsSA::ModelType::Vehicle));
        auto peds_mf = make_function_hook<function_hooker<0x5B74A7, void*(int id)>>(std::bind(AddOrMatchModel, _1, _2, TraitsSA::ModelType::Ped));

        // Model change checkers
        auto objs_mdlch = make_function_hook<function_hooker<0x5B3DB0, uint32_t(const char*)>>(CheckModelChange);
        auto tobj_mdlch = make_function_hook<function_hooker<0x5B3F51, uint32_t(const char*)>>(CheckModelChange);
        auto weap_mdlch = make_function_hook<function_hooker<0x5B3FF2, uint32_t(const char*)>>(CheckModelChange);
        auto hier_mdlch = make_function_hook<function_hooker<0x5B408A, uint32_t(const char*)>>(CheckModelChange);
        auto anim_mdlch = make_function_hook<function_hooker<0x5B4147, uint32_t(const char*)>>(CheckModelChange);
        auto cars_mdlch = make_function_hook<function_hooker<0x5B6FE3, uint32_t(const char*)>>(CheckModelChange);
        auto peds_mdlch = make_function_hook<function_hooker<0x5B74B6, uint32_t(const char*)>>(CheckModelChange);
            
        // Texture change checkers
        auto objs_texch = make_function_hook<function_hooker_thiscall<0x5B3DC2, void(void*, const char*)>>(CheckTexDictionaryChange);
        auto tobj_texch = make_function_hook<function_hooker_thiscall<0x5B3F63, void(void*, const char*)>>(CheckTexDictionaryChange);
        auto weap_texch = make_function_hook<function_hooker_thiscall<0x5B400B, void(void*, const char*)>>(CheckTexDictionaryChange);
        auto hier_texch = make_function_hook<function_hooker_thiscall<0x5B409C, void(void*, const char*)>>(CheckTexDictionaryChange);
        auto anim_texch = make_function_hook<function_hooker_thiscall<0x5B4159, void(void*, const char*)>>(CheckTexDictionaryChange);
        auto cars_texch = make_function_hook<function_hooker_thiscall<0x5B6FF8, void(void*, const char*)>>(CheckTexDictionaryChange);
        auto peds_texch = make_function_hook<function_hooker_thiscall<0x5B74CB, void(void*, const char*)>>(CheckTexDictionaryChange);

        // Disallow change of modelinfo anim file
        static uint8_t asm_retn4[] = { 0xC2, 0x04, 0x00 };
        scoped_write<sizeof(asm_retn4)> clump_noanimch;
        scoped_write<sizeof(asm_retn4)> vehhh_noanimch;
        clump_noanimch.write(0x4C5200, asm_retn4, sizeof(asm_retn4), true);
        vehhh_noanimch.write(0x4C7670, asm_retn4, sizeof(asm_retn4), true);

        // Disallow col changes
        //auto peds_nocollch = make_function_hook<function_hooker_thiscall<0x5B74E5, void(void*, void*, int)>>(DummyColSet);
        //auto hier_nocollch = make_function_hook<function_hooker_thiscall<0x5B40AA, void(void*, void*, int)>>(DummyColSet);
        //auto weap_nocollch = make_function_hook<function_hooker_thiscall<0x5B4025, void(void*, void*, int)>>(DummyColSet);


        // Perform the refreshing
        void(*LoadObjectTypes)(const char*) = ReadRelativeOffset(0x5B9206 + 1).get();
        for(auto& ide : ide_files)
        {
            LoadObjectTypes(ide.c_str());
        }

        refresher->Release();
    };
}

