/*
 * Copyright (C) 2015  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under the MIT License, see LICENSE at top level directory.
 * 
 */
#include <stdinc.hpp>
#include "../data_traits.hpp"
using namespace modloader;
using std::string;

struct pedstats_traits : public data_traits
{
    static const bool has_sections      = false;
    static const bool per_line_section  = false;

    struct dtraits : modloader::dtraits::OpenFile
    {
        static const char* what() { return "ped stats"; }
    };
    
    using detour_type = modloader::OpenFileDetour<0x5BB89F, dtraits>;

    using key_type      = int;
    using value_type    = data_slice<string, real_t, real_t, pack<int16_t, 4>, real_t, real_t, int16_t, int16_t>;

    key_type key_from_value(const value_type&)
    {
        return stat_line++;
    }

public:
    int stat_line = 0;

    template<class Archive>
    void serialize(Archive& archive)
    { archive(this->stat_line); }
};

using pedstats_store = gta3::data_store<pedstats_traits, std::map<
                        pedstats_traits::key_type, pedstats_traits::value_type
                        >>;


static auto xinit = initializer([](DataPlugin* plugin_ptr)
{
    if(gvm.IsSA())
    {
        auto ReloadPedStats = injector::cstd<void()>::call<0x5BB890>;
        plugin_ptr->AddMerger<pedstats_store>("pedstats.dat", true, false, false, reinstall_since_start, gdir_refresh(ReloadPedStats));
    }
});

