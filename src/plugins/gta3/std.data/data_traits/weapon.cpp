/*
 * Copyright (C) 2015  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under the MIT License, see LICENSE at top level directory.
 * 
 */
#include <stdinc.hpp>
#include "../data_traits.hpp"
using namespace modloader;
using std::string;

//
struct weapon_traits_3vc : public data_traits
{
    static const bool has_sections      = false;
    static const bool per_line_section  = false;

    static const bool has_eof_string = true;
    static const char* eof_string() { return "ENDWEAPONDATA"; }

    struct dtraits : modloader::dtraits::SaOpenOr3VcLoadFileDetour
    {
        static const char* what()       { return "weapon data"; }
        static const char* datafile()   { return "weapon.dat"; }
    };
    
    using detour_type = modloader::SaOpenOr3VcLoadFileDetour<0x5BE68A, dtraits>;

    using key_type      = size_t;
    using value_type    = data_slice<string, string, real_t, int, int, int, int, real_t, real_t, real_t, real_t, real_t, real_t, real_t, string, IIIOnly<string>, VCOnly<pack<real_t, 3>>, real_t, real_t, real_t, real_t, int, int, VCOnly<hex<int>>, VCOnly<int>>;

    key_type key_from_value(const value_type& value)
    {
        return modloader::hash(get<0>(value));
    }

public: // eof_string related
    bool eof = false;

    template<class Archive>
    void serialize(Archive& archive)
    { archive(this->eof); }
};

//
struct weapon_traits_sa : public data_traits
{
    static const bool has_sections      = true;     // Does this data file contains sections?
    static const bool per_line_section  = true;     // Is the sections of this data file different on each line?

    // End of file string
    static const bool has_eof_string = true;
    static const char* eof_string() { return "ENDWEAPONDATA"; }

    // Detouring traits
    struct dtraits : modloader::dtraits::SaOpenOr3VcLoadFileDetour
    {
        static const char* what()       { return "weapon data"; }
        static const char* datafile()   { return "weapon.dat"; }
    };
    
    // Detouring type
    using detour_type = modloader::SaOpenOr3VcLoadFileDetour<0x5BE68A, dtraits>;

    // Section slices
    using melee_type = data_slice<char, insen<string>, string, real_t, real_t, int, int, int, string, int, hex<int>, string>;
    using gun_type   = data_slice<char, insen<string>, string, real_t, real_t, int, int, int, string, int, int, vec3, int, int, real_t, real_t, int, int, int, int, int, int, int, hex<int>, delimopt, real_t, real_t, real_t, real_t>;
    using aim_type   = data_slice<char, string, real_t, real_t, real_t, real_t, int, int, int, int>;

    // Data
    using key_type   = std::tuple<bool, std::size_t, int>;    // <0> = is_aimsec; <1> = name hash; <2> = skill_type
    using value_type = gta3::data_section<melee_type, gun_type, aim_type>;

    // Possible sections
    static const gta3::section_info* sections()
    {
        // Note: must be in the same order as declared in value_type
        static auto sections = gta3::make_section_info("\xA3", "$", "%");
        static_assert(std::tuple_size<decltype(sections)>::value == 1 + value_type::num_sections, "incompatible sizes");
        return sections.data();
    }


    // key_from_value
    struct key_from_value_visitor : gta3::data_section_visitor<key_type>
    {
        key_type operator()(const melee_type& slice) const
        { return key_type(false, hash_model(get<1>(slice)), -1); }

        key_type operator()(const gun_type& slice) const
        { return key_type(false, hash_model(get<1>(slice)), get<12>(slice)); }

        key_type operator()(const aim_type& slice) const
        { return key_type(true, modloader::hash(get<1>(slice)), -1); }

        key_type operator()(const either_blank& slice) const
        { throw std::invalid_argument("blank type"); }
    };

    static key_type key_from_value(const value_type& value)
    {
        return value.apply_visitor(key_from_value_visitor());
    }

    // Returns the section pointer for the current line
    static const gta3::section_info* section_by_line(const gta3::section_info* sections, const std::string& line)
    {
        static auto meleesec = gta3::section_info::by_name(sections, "\xA3", -1);
        auto section = section_by_line_noeof(sections, line);

        // use some section just to continue into setbyline and check for eof_string
        if(section == nullptr) return line == eof_string()? meleesec : nullptr;
        return section;
    }

    // Returns the section pointer for the current line ignoring the case of eof_string
    static const gta3::section_info* section_by_line_noeof(const gta3::section_info* sections, const std::string& line)
    {
        static auto meleesec = gta3::section_info::by_name(sections, "\xA3", -1);
        static auto gunsec = gta3::section_info::by_name(sections, "$", -1);
        static auto aimsec = gta3::section_info::by_name(sections, "%", -1);
        switch(line[0])
        {
            case '\xA3': return meleesec;
            case '$':    return gunsec;
            case '%':    return aimsec;
            default:     return nullptr;
        }
    }

public: // eof_string related
    bool eof = false;

    template<class Archive>
    void serialize(Archive& archive)
    { archive(this->eof); }
};

//
template<typename Traits>
using weapon_store = gta3::data_store<Traits, std::map<
                        typename Traits::key_type, typename Traits::value_type
                        >>;

using weapon_store_3vc = weapon_store<weapon_traits_3vc>;
using weapon_store_sa  = weapon_store<weapon_traits_sa>;

REGISTER_RTTI_FOR_ANY(weapon_store_3vc);
REGISTER_RTTI_FOR_ANY(weapon_store_sa);


// sections function specialization
namespace datalib {
    namespace gta3
    {
        inline const section_info* sections(const weapon_traits_sa::value_type&)
        {
            return weapon_traits_sa::sections();
        }
    }
}


// Weapon Merger
static auto xinit = initializer([](DataPlugin* plugin_ptr)
{
    auto ReloadWeaponData = injector::cstd<void()>::call<0x5BF750>;

    if(gvm.IsSA())
    {
        // Weapon Merger
        plugin_ptr->AddMerger<weapon_store_sa>("weapon.dat", true, false, false, reinstall_since_load, gdir_refresh(ReloadWeaponData));

        // Readme Reader for weapon.dat lines
        plugin_ptr->AddReader<weapon_store_sa>([](const std::string& line) -> maybe_readable<weapon_store_sa>
        {
            static auto base_weap   = "^%c %s %{MELEE|INSTANT_HIT|PROJECTILE|AREA_EFFECT|CAMERA|USE} %f %f %d %d %d";   // continues...
            static auto regex_melee = make_fregex(string(base_weap).append(" %{UNARMED|BBALLBAT|KNIFE|GOLFCLUB|SWORD|CHAINSAW|DILDO|FLOWERS} %d %x %s$")); 
            static auto regex_gun   = make_fregex(string(base_weap).append(" %s %d %d %f %f %f %d %d %f %f %d %d %d %d %d %d %d %x(?: %f)?(?: %f)?(?: %f)?(?: %f)?$"));
            static auto regex_aim   = make_fregex("^%c %{[A-Za-z][A-Za-z0-9_]*} %f %f %f %f %d %d %d %d$");

            static auto meleesec = gta3::section_info::by_name(weapon_traits_sa::sections(), "\xA3", -1);
            static auto gunsec = gta3::section_info::by_name(weapon_traits_sa::sections(), "$", -1);
            static auto aimsec = gta3::section_info::by_name(weapon_traits_sa::sections(), "%", -1);

            if(auto section = weapon_traits_sa::section_by_line_noeof(weapon_traits_sa::sections(), line))
            {
                if((section == meleesec && regex_match(line, regex_melee))
                || (section == gunsec && regex_match(line, regex_gun))
                //|| (section == aimsec && regex_match(line, regex_aim))
                )
                {
                    weapon_store_sa store;
                    if(store.insert(section, line))
                        return store;
                }
            }

            return nothing;
        });
    }
    else
    {
        // Weapon Merger
        plugin_ptr->AddMerger<weapon_store_3vc>("weapon.dat", true, false, false, reinstall_since_load, gdir_refresh(ReloadWeaponData));

        plugin_ptr->AddReader<weapon_store_3vc>([](const std::string& line) -> maybe_readable<weapon_store_3vc>
        {
            static auto regex_weapx = make_fregex("^%s %{MELEE|INSTANT_HIT|PROJECTILE|AREA_EFFECT|CAMERA} %f %d %d %d %d %f %f %f %f %f %f %f %s"
                                                + std::string(gvm.IsIII()? " %s" : " %f %f %f")
                                                + " %f %f %f %f %d %d"
                                                + string(gvm.IsVC()? " %x %d" : "")
                                                + "$");
            smatch match;
            if(regex_match(line, match, regex_weapx))
            {
                weapon_store_3vc store;
                if(store.insert(nullptr, line))
                    return store;
            }
            return nothing;
        });
    }
});
