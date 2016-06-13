/*
 * Copyright (C) 2015  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under the MIT License, see LICENSE at top level directory.
 * 
 */
#include <stdinc.hpp>
#include "../data_traits.hpp"
using namespace modloader;
using std::array;
template<class T>
using comma = custom_sep<T, ','>;

//
//  Tag for the first line in each decision file, that should be ignored.
//
struct tag_decision_fl : dummy_value {};

namespace datalib
{
    template<>
    struct data_info<udata<tag_decision_fl>> : data_info_base
    {};
};

namespace std
{
    template<class CharT, class Traits>
    static std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const udata<tag_decision_fl>&)
    { return (os << "data values2:"); }
};

//
//
struct decision_traits : public data_traits
{
    static const bool has_sections      = false;
    static const bool per_line_section  = false;

    struct dtraits : modloader::dtraits::OpenFile
    {
        static const char* what()       { return "decision maker"; }
        static const char* datafile()   { return decision_merger_name; }
    };
    
    using detour_type = modloader::OpenFileDetour<0x6076CE, dtraits>;

    //
    using decision_type   = std::tuple<comma<int>, array<comma<real_t>, 4>, array<comma<bool>, 2>, array<comma<real_t>, 6>>;
    using first_line_type = udata<tag_decision_fl>;    // there's a "data values2:" string on the top of every decision file
    using data_type       = std::tuple<comma<int>, comma<int>, array<decision_type, 6>>;
    
    //
    using key_type   = int;
    using value_type = data_slice<either<data_type, first_line_type>>;

    // skips the first line
    template<class StoreType>
    static bool setbyline(StoreType& store, value_type& data, const gta3::section_info* section, const std::string& line)
    {
        auto& traits = store.traits();
        if(!traits.has_first_line)
        {
            traits.has_first_line = true;
            data = value_type(make_udata<tag_decision_fl>());
            return true;
        }
        return data_traits::setbyline(store, data, section, line);
    }

    key_type key_from_value(const value_type& value)
    {
        auto& either = get<0>(value);
        if(is_typed_as<first_line_type>(either))
            return std::numeric_limits<int>::min();     // should come before anything at the first line!!11!!
        else
            return get(get<0>(get<data_type>(either))); // event type
    }

public:
    bool has_first_line = false;

    template<class Archive>
    void serialize(Archive& archive)
    { archive(this->has_first_line); }
};

using decision_store = gta3::data_store<decision_traits, linear_map<    // linear_map reduces bug chances here because of PedEvent.txt missing events
                        decision_traits::key_type, decision_traits::value_type
                        >>;

static auto xinit = initializer([](DataPlugin* plugin_ptr)
{
    if(!gvm.IsSA())
        return;

    // Refreshing decision makes is allowed only out of mission so we don't break any mission custom decision maker!
    auto ReloadDecisionMakers = [plugin_ptr]
    {
        if(injector::cstd<char()>::call(0x464D50))  // CTheScripts::IsPlayerOnAMission
            plugin_ptr->Log("Warning: Failed to refresh decision makers because player is in a mission!");
        else
            injector::cstd<void()>::call<0x5BF400>();
    };

    // Use reinstall_since_load instead of reinstall_since_start because of our call to IsPlayerOnAMission
    plugin_ptr->AddMerger<decision_store>(decision_merger_name, true, false, true, reinstall_since_load, ReloadDecisionMakers);
});
