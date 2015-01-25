/*
 * Copyright (C) 2015  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#include <stdinc.hpp>
#include "../data.hpp"
using namespace modloader;
using std::vector;

// TODO make it merge in the same group

struct xxxgrp_traits : public data_traits
{
    static const bool can_cache         = true;     // Can this store get cached?
    static const bool is_reversed_kv    = false;    // Does the key contains the data instead of the value in the key-value pair?
    static const bool has_sections      = false;    // Does this data file contains sections?
    static const bool per_line_section  = false;    // Is the sections of this data file different on each line?

    // Dominance Flags
    using domflags_fn = datalib::domflags_fn<flag_RemoveIfNotExistInOneCustomButInDefault>;
                      
    //
    using key_type   = int;
    using value_type = data_slice<vector<std::string>>;

    key_type key_from_value(const value_type&)
    {
        return grpindex++;
    }

    public: // traits data

        int grpindex = 0;   // Line index we are going tho for groups

        template<class Archive>
        void serialize(Archive& ar)
        {
            ar(this->grpindex);
        }
};

struct cargrp_traits : public xxxgrp_traits
{
    // Detouring traits
    struct dtraits : modloader::dtraits::OpenFile
    {
        static const char* what()       { return "car groups"; }
        static const char* datafile()   { return "cargrp.dat"; }
    };
    
    // Detouring type
    using detour_type = modloader::OpenFileDetour<0x5BD1BB, dtraits>;
};

struct pedgrp_traits : public xxxgrp_traits
{
    // Detouring traits
    struct dtraits : modloader::dtraits::OpenFile
    {
        static const char* what()       { return "ped groups"; }
        static const char* datafile()   { return "pedgrp.dat"; }
    };
    
    // Detouring type
    using detour_type = modloader::OpenFileDetour<0x5BCFFB, dtraits>;
};


template<class Traits>
using xxxgrp_store = gta3::data_store<Traits, std::map<
                        typename Traits::key_type, typename Traits::value_type
                        >>;






template<class Traits>
static void initialise(DataPlugin* plugin_ptr, std::function<void()> refresher)
{
    using store_type = xxxgrp_store<Traits>;
    plugin_ptr->AddMerger<store_type>(Traits::dtraits::datafile(), true, false, false, reinstall_since_load, refresher);
}

static auto xinit = initializer([](DataPlugin* plugin_ptr)
{
    initialise<pedgrp_traits>(plugin_ptr, gdir_refresh(injector::cstd<void()>::call<0x5BCFE0>));
    initialise<cargrp_traits>(plugin_ptr, gdir_refresh(injector::cstd<void()>::call<0x5BD1A0>));
});
