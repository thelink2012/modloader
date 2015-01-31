/*
 * Copyright (C) 2015  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#include <stdinc.hpp>
#include "../data_traits.hpp"
using namespace modloader;
using std::string;

enum class StatCondition
{
    LessThan, MoreThan,
};

template<>
struct enum_map<StatCondition>
{
    static std::map<string, StatCondition>& map()
    {
        static std::map<string, StatCondition> xmap = {
            { "lessthan", StatCondition::LessThan }, 
            { "morethan", StatCondition::MoreThan },
        };
        return xmap;
    }
};


struct statdisp_traits : public data_traits
{
    static const bool has_sections      = false;
    static const bool per_line_section  = false;

    struct dtraits : modloader::dtraits::OpenFile
    {
        static const char* what() { return "stat update conditions"; }
    };
    
    using detour_type = modloader::OpenFileDetour<0x55988F, dtraits>;

    using key_type      = int;
    using value_type    = data_slice<int, dummy_string, StatCondition, real_t, labelname>;

    key_type key_from_value(const value_type&)
    {
        return msg_line++;
    }

public:
    int msg_line = 0;

    template<class Archive>
    void serialize(Archive& archive)
    { archive(this->msg_line); }
};

using statdisp_store = gta3::data_store<statdisp_traits, std::map<
                        statdisp_traits::key_type, statdisp_traits::value_type
                        >>;

static auto xinit = initializer([](DataPlugin* plugin_ptr)
{
    auto ReloadStatMessages = []{}; // shouldn't refresh at all, would break save game
    plugin_ptr->AddMerger<statdisp_store>("statdisp.dat", true, false, false, reinstall_since_load, gdir_refresh(ReloadStatMessages));
});
