/*
 * Copyright (C) 2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#pragma once

#if _MSC_VER
#   pragma warning(disable : 4018)  // @deps\tinympl\tinympl\variadic\at.hpp(33): warning C4018: '<' : signed/unsigned mismatch
#endif

// datalib fundamentals
#include <datalib/data_slice.hpp>
#include <datalib/dominance.hpp>
#include <datalib/data_store.hpp>

// additional types
#include <datalib/detail/linear_map.hpp>
#include <type_wrapper/floating_point.hpp>
#include <type_wrapper/datalib/io/floating_point.hpp>

// datalib I/O with common types
#include <datalib/io/either.hpp>
#include <datalib/io/optional.hpp>
#include <datalib/io/tuple.hpp>
#include <datalib/io/pair.hpp>
#include <datalib/io/array.hpp>
#include <datalib/io/string.hpp>
#include <datalib/io/ignore.hpp>
#include <datalib/io/hex.hpp>
#include <datalib/io/tagged_type.hpp>
#include <datalib/io/udata.hpp>

// datalib gta3 stuff
#include <datalib/gta3/io.hpp>
#include <datalib/gta3/data_section.hpp>
#include <datalib/gta3/data_store.hpp>

using namespace datalib;

#include <modloader/modloader.hpp>

/*
 *  data_traits
 *      Extended traits from gta3::data_traits, adding stuff handled by std.data itself instead of datalib.
 *      
 *      Additional stuff:
 *
 *          static const bool can_cache         -> Can this store get cached?
 *          static const bool is_reversed_kv    -> Does the key contains the data instead of the value in the key-value pair?
 *          static const bool has_sections      -> Does this data file contains sections?
 *          static const bool per_line_section  -> Does the sections of this data file different on each line?
 *
 *          [optional] void static_serialize(Archive, IsSaving, Functor)
 *                                              -> Serializes the static data of a data_traits.
 *                                                 The Archive argument is a reference to the serializer (call it's operator()() to serialize something)
 *                                                 and the Functor is a function that serializes the content of a list of stores (should usually be done)
 *
 *          [optional] void after_merge(HasBeenSucessful)
 *                                              -> Called after a merge happened, this means all the data you used for the merging process
 *                                                 can be freed.
 *
 */
struct data_traits : public gta3::data_traits
{
    template<class Archive, class FuncT>
    static void static_serialize(Archive& archive, bool saving, FuncT serialize_store)
    {
        serialize_store();
    }


    static void after_merge(bool successful)
    {}

    // make setbyline output a error on failure
    template<class StoreType, typename TData>
    static bool setbyline(StoreType& store, TData& data, const gta3::section_info* section, const std::string& line, bool allowlog = true)
    {
        if(!gta3::data_traits::setbyline(store, data, section, line))
        {
            if(allowlog) plugin_ptr->Log("Warning: Failed to parse data line: %s", line.c_str());
            return false;
        }
        return true;
    }
};
