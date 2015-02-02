/*
 * Copyright (C) 2015  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#include <stdinc.hpp>
#include "../data_traits.hpp"
using namespace modloader;
using std::string;

//
//  animgrp section base data
//
using animgrp_type = std::tuple<insen<string>, insen<string>, insen<string>, int>;  // not sure about sensitivenss of those strings
using animgrp_ptr  = std::shared_ptr<animgrp_type>;

static bool operator<(const animgrp_ptr& a, const animgrp_ptr& b)
{ return (*a < *b); }

static bool operator==(const animgrp_ptr& a, const animgrp_ptr& b)
{ return (*a == *b); }

namespace datalib
{
    template<>  // The udata<animgrp_ptr> should be ignored during the data_slice scan/print
    struct data_info<udata<animgrp_ptr>> : data_info_base
    {
        static const bool ignore = true;
    };
}

namespace std
{
    // Output for a animgrp section base pointer
    template<class CharT, class Traits>
    static std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const animgrp_ptr& ptr)
    {
        return (os << *ptr);
    }
}




//
struct animgrp_traits : public data_traits
{
    static const bool has_sections      = false;    // we're going by manual handling of sections on this one
    static const bool per_line_section  = false;

    struct dtraits : modloader::dtraits::OpenFile
    {
        static const char* what() { return "anim association"; }
    };
    
    using detour_type = modloader::OpenFileDetour<0x5BC92B, dtraits>;

    using key_type      = std::pair<animgrp_ptr, int>;  // .first = related section, .second = id in the section (min=begin, max=end)
    using value_type    = data_slice<either<animgrp_ptr, EndString, insen<string>>, udata<animgrp_ptr>>;

    // key_from_value id (.second)
    struct key_from_value_visitor : gta3::data_section_visitor<int>
    {
        animgrp_traits& traits;
        key_from_value_visitor(animgrp_traits& traits) : traits(traits) {}

        // For a string in the section get a unique id
        int operator()(const insen<string>&) const
        { return ++traits.grps[traits.current_grp]; }

        // For the beggining of a section, use <int>::min
        int operator()(const animgrp_ptr&) const
        { return std::numeric_limits<int>::min(); }

        // for the end of a section, use <int>::max
        int operator()(const EndString&) const
        { return std::numeric_limits<int>::max(); }

        int operator()(const either_blank&) const
        { throw std::invalid_argument("blank type"); }
    };

    //
    key_type key_from_value(const value_type& value)
    {
        return key_type(
            get(get<1>(value)),
            get<0>(value).apply_visitor(key_from_value_visitor(*this)));
    }

    // Does the manual section handling
    template<class StoreType>
    static bool setbyline(StoreType& store, value_type& data, const gta3::section_info* section, const std::string& line)
    {
        auto& traits = store.traits();
        if(traits.current_grp == nullptr)   // not working in a section
        {
            data_slice<animgrp_type> grpdata;
            if(grpdata.check(line) && grpdata.set(line))
            {
                traits.current_grp = traits.add_grp(grpdata.get<0>());
                data = value_type(traits.current_grp, make_udata<animgrp_ptr>(traits.current_grp));
                return true;
            }
        }
        else // then is in a section
        {
            if(data_traits::setbyline(store, data, section, line))
            {
                data.set<1>(make_udata<animgrp_ptr>(traits.current_grp));
                if(is_typed_as<EndString>(data.get<0>()))
                    traits.current_grp = nullptr;
                return true;
            }
        }
        return fail(line);
    }

public:
    animgrp_ptr current_grp;                // Working section
    std::map<animgrp_ptr, int> grps;        // List of sections

    // Adds a new section
    animgrp_ptr add_grp(const animgrp_type& animgrp)
    {
        return grps.emplace(std::make_shared<animgrp_type>(animgrp), 1).first->first;
    }

    template<class Archive>
    void serialize(Archive& archive)
    {
        archive(this->grps);
    }

};

using animgrp_store = gta3::data_store<animgrp_traits, std::map<
                        animgrp_traits::key_type, animgrp_traits::value_type
                        >>;

static auto xinit = initializer([](DataPlugin* plugin_ptr)
{
    plugin_ptr->AddMerger<animgrp_store>("animgrp.dat", true, false, false, no_reinstall);
});

