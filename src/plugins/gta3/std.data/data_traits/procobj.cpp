/*
 * Copyright (C) 2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#include <stdinc.hpp>
#include "../data.hpp"
using namespace modloader;
using std::string;

//
struct procobj_traits : public data_traits
{
    static const bool can_cache         = true;     // Can this store get cached?
    static const bool is_reversed_kv    = true;     // Does the key contains the data instead of the value in the key-value pair?
    static const bool has_sections      = false;    // Does this data file contains sections?
    static const bool per_line_section  = false;    // Is the sections of this data file different on each line?

    // Detouring traits
    struct dtraits : modloader::dtraits::OpenFile
    {
        static const char* what() { return "procedural objects"; }
    };
    
    // Detouring type
    using detour_type = modloader::OpenFileDetour<0x5A3154, dtraits>;

    // Dominance Flags
    using domflags_fn = datalib::domflags_fn<flag_RemoveIfNotExistInOneCustomButInDefault>;

    using key_type      = data_slice<insen<string>, modelname, real_t, real_t, int, int, real_t, real_t, real_t, real_t, real_t, real_t, int, int>;
    using value_type    = dummy_value;

    value_type value_from_key(const key_type&)
    {
        return dummy_value();
    }
};

//
using procobj_store = gta3::data_store<procobj_traits, linear_map<
                        procobj_traits::key_type, procobj_traits::value_type
                        >>;


//
static auto xinit = initializer([](DataPlugin* plugin_ptr)
{
    auto ReloadProcObjs = []{};//std::bind(injector::thiscall<void(void*)>::call<0x5A3EA0>, mem_ptr(0xBB7CB0).get<void>());
    plugin_ptr->AddMerger<procobj_store>("procobj.dat", true, false, false, reinstall_since_load, gdir_refresh(ReloadProcObjs));
});
