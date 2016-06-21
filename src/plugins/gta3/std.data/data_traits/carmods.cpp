/*
 * Copyright (C) 2015  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under the MIT License, see LICENSE at top level directory.
 * 
 */
#include <stdinc.hpp>
#include "../data_traits.hpp"
using namespace modloader;
using namespace injector;
using std::vector;

//
struct carmods_traits : public data_traits
{
    static const bool has_sections      = true;     // Does this data file contains sections?
    static const bool per_line_section  = false;

    // Detouring traits
    struct dtraits : modloader::dtraits::OpenFile
    {
        static const char* what()       { return "vehicle upgrades"; }
        static const char* datafile()   { return "carmods.dat"; }
    };
    
    // Detouring type
    using detour_type = modloader::OpenFileDetour<0x5B65BE, dtraits>;

    // Wheel Section Slice
    using wheel_type = data_slice<int, vector<modelname>>;

    // Mods Section Slice
    using mods_type = data_slice<modelname, vector<modelname>>;

    // Link Section Slice
    using link_type = data_slice<modelname, modelname>;
                                        
    //
    using value_type = gta3::data_section<wheel_type, mods_type, link_type>;

    static const gta3::section_info* sections()
    {
        // Note: must be in the same order as declared in value_type
        static auto sections = gta3::make_section_info("wheel", "mods", "link");
        static_assert(std::tuple_size<decltype(sections)>::value == 1 + value_type::num_sections, "incompatible sizes");
        return sections.data();
    }

    //
    struct key_type
    {
        using link_type   = std::pair<std::size_t, std::size_t>;    // Hash of two models
        using mods_type   = std::size_t;                            // Hash of a model
        using wheel_type = int;                                     // Index of a model
        either<link_type, mods_type, wheel_type> e;

        struct check_leq : either_static_visitor<bool>
        {
            template<class T, class U>
            bool operator()(const T& a, const U& b) const
            { throw std::invalid_argument("check_leq<T,U>"); }

            // Link needs a special less than operation...
            // If the links are swapped, then it's the same key, otherwise assume the first model as the key
            bool operator()(const link_type& a, const link_type& b) const
            { return (a.first == b.second? false : a.first < b.first); }

            template<class T>
            bool operator()(const T& a, const T& b) const
            { return a < b; }
        };

        key_type() = default;
        key_type(int id) : e(id) {}
        key_type(std::size_t hash) : e(hash) {}
        key_type(std::size_t l1, std::size_t l2) : e(link_type(l1, l2)) {}
        key_type(const key_type&) = default;

        bool operator<(const key_type& rhs) const
        {
            if(this->e.which() == rhs.e.which())
                return apply_visitor(check_leq(), this->e, rhs.e);
            return (this->e.which() < rhs.e.which());
        }

        template<class Archive>
        void serialize(Archive& ar)
        {
            ar(this->e);
        }
    };

    // key_from_value
    struct key_from_value_visitor : gta3::data_section_visitor<key_type>
    {
        key_type operator()(const wheel_type& slice) const
        { return key_type(get<0>(slice)); }

        key_type operator()(const mods_type& slice) const
        { return key_type(hash_model(get<0>(slice))); }

        key_type operator()(const link_type& slice) const
        { return key_type(hash_model(get<0>(slice)), hash_model(get<1>(slice))); }

        key_type operator()(const either_blank& slice) const
        { throw std::invalid_argument("blank type"); }
    };

    key_type key_from_value(const value_type& value)
    {
        return value.apply_visitor(key_from_value_visitor());
    }
};

//
using carmods_store = gta3::data_store<carmods_traits, std::map<
                        carmods_traits::key_type, carmods_traits::value_type
                        >>;

REGISTER_RTTI_FOR_ANY(carmods_store);


// sections function specialization
namespace datalib {
    namespace gta3
    {
        inline const section_info* sections(const carmods_traits::value_type&)
        {
            return carmods_traits::sections();
        }
    }
}

// Vehicle Upgrades Merger
static auto xinit = initializer([](DataPlugin* plugin_ptr)
{
    if(!gvm.IsSA())
        return;

    auto ReloadUpgrades = []
    {
        // Restore the amount of linked upgrades so carmods.dat can fill it again
        char* linkedUpgradeList = mem_ptr(0xB4E6D8).get();
        *(int*)(linkedUpgradeList + 0x78) = 0;

        // Restore the '-1' at the CVehicleModelInfo::m_wUpgrades so it can be filled by carmods.dat again
        uintptr_t pVehStore = ReadMemory<uintptr_t>(0x4C6770+1, true);
        for(size_t i = 0; i < *(size_t*)(pVehStore); ++i)
        {
            uintptr_t pVeh = (pVehStore + 4) + (0x308 * i);
            auto* pUpgrades = (unsigned short*)(pVeh + 0x2D6);
            std::fill(&pUpgrades[0], &pUpgrades[18], 0xFFFF);
        }

        injector::cstd<void()>::call<0x5B65A0>();
    };

    // Data File merger
    plugin_ptr->AddMerger<carmods_store>("carmods.dat", true, false, false, reinstall_since_load, gdir_refresh(ReloadUpgrades));

    // Readme reader
    plugin_ptr->AddReader<carmods_store>([](const std::string& line) -> maybe_readable<carmods_store>
    {
        // Matches an upgrade (except for wheels)
        static auto regex_mods = make_regex(R"___(^(\w+)(?:\s+(?:hydralics|stereo|nto_\w+|bnt_\w+|chss_\w+|exh_\w+|bntl_\w+|bntr_\w+|spl_\w+|wg_l_\w+|wg_r_\w+|fbb_\w+|bbb_\w+|lgt_\w+|rf_\w+|fbmp_\w+|rbmp_\w+|misc_a_\w+|misc_b_\w+|misc_c_\w+))+\s*$)___");

        smatch match;
        if(regex_match(line, match, regex_mods))
        {
            // Oh, the pattern matches with this line!
            if(HasModelInfo())
            {
                if(match.size() == 2 && MatchModelString(match[1])) // matches vehicle model?
                {
                    carmods_store store;
                    store.insert<carmods_traits::mods_type>(line);
                    return store;
                }
            }
            else
                return maybe<carmods_store>();
        }

        return nothing;
    });
});
