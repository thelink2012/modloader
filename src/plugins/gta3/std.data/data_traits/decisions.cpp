/*
 * Copyright (C) 2015  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#include <stdinc.hpp>
#include "../data.hpp"
using namespace modloader;

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
    using first_line_type = std::pair<std::string, std::string>;    // there's a "data values2:" string on the top of every decision file
    using decision_type   = std::tuple<int, int, int, real_t, real_t, real_t, real_t, int, int, real_t, real_t, real_t, real_t, real_t, real_t, int, real_t,
                                     real_t, real_t, real_t, int, int, real_t, real_t, real_t, real_t, real_t, real_t, int, real_t, real_t, real_t, real_t, int, int, real_t,
                                     real_t, real_t, real_t, real_t, real_t, int, real_t, real_t, real_t, real_t, int, int, real_t, real_t, real_t, real_t, real_t, real_t, int, real_t, real_t,
                                     real_t, real_t, int, int, real_t, real_t, real_t, real_t, real_t, real_t, int, real_t, real_t, real_t, real_t, int, int, real_t, real_t, real_t, real_t, real_t, real_t>;

    using key_type   = int;
    using value_type = data_slice<either<decision_type, first_line_type>>;

    key_type key_from_value(const value_type& value)
    {
        auto& either = get<0>(value);
        if(is_typed_as<first_line_type>(either))
            return std::numeric_limits<int>::min();     // should come before anything at the first line!!11!!
        else
            return get<0>(get<decision_type>(either));  // event type
    }
};

using decision_store = gta3::data_store<decision_traits, std::map<
                        decision_traits::key_type, decision_traits::value_type
                        >>;

static auto xinit = initializer([](DataPlugin* plugin_ptr)
{
    plugin_ptr->AddMerger<decision_store>(decision_merger_name, true, false, true, no_reinstall);
});
