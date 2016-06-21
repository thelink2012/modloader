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

struct fistfite_traits : public data_traits
{
    static const bool has_sections      = false;
    static const bool per_line_section  = false;

    static const bool has_eof_string = true;
    static const char* eof_string() { return "ENDWEAPONDATA"; }

    struct dtraits : modloader::dtraits::SaOpenOr3VcLoadFileDetour
    {
        static const char* what()       { return "fight data"; }
        static const char* datafile()   { return "fistfite.cfg"; }
    };
    
    using detour_type = modloader::SaOpenOr3VcLoadFileDetour<xVc(0x527590), dtraits>;

    using key_type      = int;
    using value_type    = data_slice<string, pack<real_t, 4>, VCOnly<real_t>, char, string, int, int>;

    key_type key_from_value(const value_type&)
    {
        return fistfite_line++;
    }

public:
    bool eof = false;
    int fistfite_line = 0;

    template<class Archive>
    void serialize(Archive& archive)
    { archive(this->eof, this->fistfite_line); }
};

using fistfite_store = gta3::data_store<fistfite_traits, std::map<
                        fistfite_traits::key_type, fistfite_traits::value_type
                        >>;


static auto xinit = initializer([](DataPlugin* plugin_ptr)
{
    if(gvm.IsIII() || gvm.IsVC())
    {
        auto ReloadFightData = injector::cstd<void()>::call<xVc(0x527570)>;
        plugin_ptr->AddMerger<fistfite_store>("fistfite.dat", true, false, false, reinstall_since_load, gdir_refresh(ReloadFightData));
    }
});

