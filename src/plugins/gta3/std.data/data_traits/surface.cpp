/*
 * Copyright (C) 2015  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under the MIT License, see LICENSE at top level directory.
 * 
 */
#include <stdinc.hpp>
#include "../data_traits.hpp"
using namespace modloader;

struct surface_traits : public data_traits
{
    static const bool has_sections      = false;
    static const bool per_line_section  = false;

    struct dtraits : modloader::dtraits::OpenFile
    {
        static const char* what() { return "surface adhesion limits"; }
    };
    
    using detour_type = modloader::OpenFileDetour<0x55D100, dtraits>;

    // The first string from the line doesn't matter, it's just a helper to the human reader.
    // The following values are real numbers, '---' like values or nothing
    using key_type      = int;
    using value_type    = data_slice<dummy_string, std::list<either<real_t, std::string>>>; // (XXX list because of a move issue with either)

    key_type key_from_value(const value_type&)
    {
        return adhesion_line++;
    }

public:
    int adhesion_line = 0;

    template<class Archive>
    void serialize(Archive& archive)
    { archive(this->adhesion_line); }
};

struct surface_traits_3vc : surface_traits
{
    struct dtraits : modloader::dtraits::LoadFile
    {
        static const char* what()
        {
            return "surface adhesion limits";
        }
    };

    using detour_type = modloader::LoadFileDetour<xVc(0x4CE8CC), dtraits>;
};

struct surface_traits_sa : surface_traits
{
};

template<typename Traits>
using surface_store = gta3::data_store<Traits, std::map<
                        typename Traits::key_type, typename Traits::value_type
                        >>;

using surface_store_3vc = surface_store<surface_traits_3vc>;
using surface_store_sa  = surface_store<surface_traits_sa>;


static auto xinit = initializer([](DataPlugin* plugin_ptr)
{
    if(gvm.IsSA())
    {
        auto ReloadSurfaceInfo = std::bind(injector::thiscall<void(void*)>::call<0x55F420>, mem_ptr(0xB79538).get<void>());
        plugin_ptr->AddMerger<surface_store_sa>("surface.dat", true, false, false, reinstall_since_start, gdir_refresh(ReloadSurfaceInfo));
    }
    else if(gvm.IsIII() || gvm.IsVC())
    {
        auto ReloadSurfaceTable = std::bind(injector::cstd<void(const char*)>::call<xVc(0x4CE8A0)>, "data/surface.dat");
        plugin_ptr->AddMerger<surface_store_3vc>("surface.dat", true, false, false, reinstall_since_start, gdir_refresh(ReloadSurfaceTable));
    }
});
