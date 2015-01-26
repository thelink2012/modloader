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
struct object_traits : public data_traits
{
    static const bool can_cache         = true;     // Can this store get cached?
    static const bool is_reversed_kv    = false;    // Does the key contains the data instead of the value in the key-value pair?
    static const bool has_sections      = false;    // Does this data file contains sections?
    static const bool per_line_section  = false;    // Is the sections of this data file different on each line?

    // Detouring traits
    struct dtraits : modloader::dtraits::OpenFile
    {
        static const char* what()       { return "object data"; }
        static const char* datafile()   { return "object.dat"; }
    };
    
    // Detouring type
    using detour_type = modloader::OpenFileDetour<0x5B5444, dtraits>;

    // Dominance Flags
    using domflags_fn = datalib::domflags_fn<flag_RemoveIfNotExistInOneCustomButInDefault>;

    // Object Data
    using key_type      = std::size_t;
    using value_type    = data_slice<modelname,
                                real_t, real_t, real_t, real_t, real_t, real_t, real_t, int, int, int, int, int, delimopt,
                                real_t, real_t, real_t, insen<string>, real_t, real_t, real_t, real_t, real_t, int, int>;

    key_type key_from_value(const value_type& value)
    {
        return hash_model(get<0>(value));
    }


public:
    bool eof = false;
    static const char* eof_string() { return "*"; }

    // Whenever a '*' appears in the front of the line, all the following lines should be ignored.
    template<class StoreType>
    static bool setbyline(StoreType& store, value_type& data, const gta3::section_info* section, const std::string& line)
    {
        return setbyline_check_eof(store, data, section, line);
    }

    template<class Archive>
    void serialize(Archive& archive)
    {
        archive(this->eof);
    }
};

//
using object_store = gta3::data_store<object_traits, std::map<
                        object_traits::key_type, object_traits::value_type
                        >>;


// Objects Data Merger
static auto xinit = initializer([](DataPlugin* plugin_ptr)
{
    // XXX a perfect refresh needs to set all the CBaseModelInfo::m_wObjectInfoIndex to -1 before reloading the data file
    auto ReloadObjectData = std::bind(injector::cstd<void(const char*, char)>::call<0x5B5360>, "data/object.dat", 0);
    plugin_ptr->AddMerger<object_store>("object.dat", true, false, false, reinstall_since_load, gdir_refresh(ReloadObjectData));
});

