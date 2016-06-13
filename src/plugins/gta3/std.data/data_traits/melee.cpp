/*
 * Copyright (C) 2015  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under the MIT License, see LICENSE at top level directory.
 * 
 */
#include <stdinc.hpp>
#include "../data_traits.hpp"
using namespace modloader;
using std::string;
using std::tuple;

enum class MeleeSection : uint8_t {
    None, Levels, Combo
};

enum class EndComboString : uint8_t {
    EndCombo,
};

template<>
struct enum_map<MeleeSection>
{
    static std::map<string, MeleeSection>& map()
    {
        static std::map<string, MeleeSection> xmap = {
            { "START_LEVELS",   MeleeSection::Levels }, 
            { "START_COMBO",    MeleeSection::Combo  }, 
        };
        return xmap;
    }
};

template<>
struct enum_map<EndComboString>
{
    static std::map<string, EndComboString>& map()
    {
        static std::map<string, EndComboString> xmap = {
            { "END_COMBO",   EndComboString::EndCombo }, 
        };
        return xmap;
    }
};

namespace datalib
{
    template<>  // The udata<MeleeSection> should be ignored during the data_slice scan/print
    struct data_info<udata<MeleeSection>> : data_info_base
    {
        static const bool ignore = true;
    };
}



//
struct melee_traits : public data_traits
{
    static const bool has_sections      = false;    // we're going by manual handling of sections on this one
    static const bool per_line_section  = false;

    static const bool has_eof_string = true;
    static const char* eof_string() { return "END_MELEE_DATA"; }

    struct dtraits : modloader::dtraits::OpenFile
    {
        static const char* what() { return "fighting data"; }
    };
    
    using detour_type = modloader::OpenFileDetour<0x5BEF47, dtraits>;

    // Slices as Tuples
    using combo_1_type  = tuple<dummy_string, string>;
    using combo_2_type  = tuple<dummy_string, real_t>;
    using combo_37_type = tuple<dummy_string, real_t, real_t, real_t, string, int, int, int, optional<real_t>>;
    using combo_8_type  = tuple<dummy_string, real_t, real_t>;
    using combo_9_type  = tuple<dummy_string, hex<int>>;
    using level_type    = tuple<dummy_string, vec3>;
    using data_type     = either<combo_37_type, level_type, combo_8_type, combo_2_type, combo_9_type, combo_1_type>;

    // Key & Value
    using key_type      = std::pair<int, int>;  // .first = related section id (-1=levels), .second = line in the section (min=begin, max=end)
    using value_type    = data_slice<either<EndComboString, data_type, MeleeSection>>;

    // key_from_value id (.second)
    struct key_from_value_visitor : gta3::data_section_visitor<int>
    {
        melee_traits& traits;
        key_from_value_visitor(melee_traits& traits) : traits(traits) {}

        // For level_type, combo_type and others get the line unique id of this section
        template<class T>
        int operator()(const T&) const
        { return ++traits.current_section_line; }

        // For the beggining of a section, use <int>::min
        int operator()(const MeleeSection&) const
        { return std::numeric_limits<int>::min(); }

        // for the end of a section, use <int>::max
        int operator()(const EndComboString&) const
        { return std::numeric_limits<int>::max(); }

        int operator()(const either_blank&) const
        { throw std::invalid_argument("blank type"); }
    };

    //
    key_type key_from_value(const value_type& value)
    {
        return key_type(
            this->current_section_id,
            get<0>(value).apply_visitor(key_from_value_visitor(*this)));
    }


    // Does the manual section handling
    template<class StoreType>
    static bool setbyline(StoreType& store, value_type& data, const gta3::section_info* section, const std::string& line)
    {
        melee_traits& traits = store.traits();

        if(has_reached_eof(traits, line))   // END_MELEE_DATA
            return false;

        if(traits.current_section == MeleeSection::None)    // Not yet in a section
        { 
            data_slice<MeleeSection> section;
            if(section.set(line))
            {
                MeleeSection sectype = section.get<0>();
                traits.current_section = sectype;
                traits.current_section_id = (sectype == MeleeSection::Levels? -1 : ++traits.current_section_id);
                traits.current_section_line = 0;
                data = value_type(sectype);
                return true;
            }
        }
        else // Then already in a section
        {
            if(data_traits::setbyline(store, data, section, line))
            {
                if(is_typed_as<EndComboString>(data.get<0>()))
                    traits.current_section = MeleeSection::None;
                return true;
            }
        }
        return fail(line);
    }

public:
    bool eof = false;
    MeleeSection current_section = MeleeSection::None;
    int current_section_id       = 0;                   // -1 is reserved for levels
    int current_section_line     = 0;

    template<class Archive>
    void serialize(Archive& archive)
    {
        archive(this->eof, this->current_section, this->current_section_id, this->current_section_line);
    }

};

using melee_store = gta3::data_store<melee_traits, std::map<
                        melee_traits::key_type, melee_traits::value_type
                        >>;

static auto xinit = initializer([](DataPlugin* plugin_ptr)
{
    if(gvm.IsSA())
    {
        auto ReloadMelee = injector::cstd<void()>::call<0x5BEDC0>;
        plugin_ptr->AddMerger<melee_store>("melee.dat", true, false, false, reinstall_since_load, ReloadMelee);
    }
});

