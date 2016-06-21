/*
 * Copyright (C) 2016  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under the MIT License, see LICENSE at top level directory.
 * 
 */
#include <stdinc.hpp>
#include "../data_traits.hpp"
using namespace modloader;
using std::string;
using std::tuple;

//%s %d %d %d %d %d %d %d %d %f %f %d %d %d %d %d %d %d %d %d %f %d %f %d %d %d %d %f %d %d %f %f %f %d %d %f %f %f %f %f %d

// FIREBALL integer token is broken
template<class T>
using fixtok = either<T, string>;

struct particle_traits : public data_traits
{
    static const bool has_sections      = false;
    static const bool per_line_section  = false;

    static const bool has_eof_string = true;
    static const char* eof_string() { return ";the end"; }

    struct dtraits : modloader::dtraits::SaOpenOr3VcLoadFileDetour
    {
        static const char* what()       { return "particle data"; }
        static const char* datafile()   { return "particle.cfg"; }
    };
    
    using detour_type = modloader::SaOpenOr3VcLoadFileDetour<xVc(0x565B2E), dtraits>;

    using key_type      = int;
    using value_type    = data_slice<string, pack<int, 8>, real_t, real_t, pack<int, 9>, real_t, int, real_t, pack<int, 4>, real_t, int, int, real_t, real_t, real_t, fixtok<int>, int, real_t, VCOnly<vec3>, real_t, int>;
    //using value_type    = data_slice<string, int, int, int, int, int, int, int, int, real_t, real_t, int, int, int, int, int, int, int, int, int, real_t, int, real_t, int, int, int, int, real_t, int, int, real_t, real_t, real_t, int, int, real_t, VCOnly<vec3>, real_t, int>;

    key_type key_from_value(const value_type&)
    {
        return particle_line++;
    }

public:
    bool eof = false;
    int particle_line = 0;

    template<class Archive>
    void serialize(Archive& archive)
    { archive(this->eof, this->particle_line); }
};

using particle_store = gta3::data_store<particle_traits, std::map<
                        particle_traits::key_type, particle_traits::value_type
                        >>;


static auto xinit = initializer([](DataPlugin* plugin_ptr)
{
    if(gvm.IsIII() || gvm.IsVC())
    {
        auto ReloadParticleData = injector::cstd<void()>::call<xVc(0x565940)>;
        plugin_ptr->AddMerger<particle_store>("particle.cfg", true, false, false, reinstall_since_load, gdir_refresh(ReloadParticleData));
    }
});

