/*
 *  Copyright (C) 2014 Denilson das Mercês Amorim (aka LINK/2012)
 *  Licensed under the Boost Software License v1.0 (http://opensource.org/licenses/BSL-1.0)
 *
 */
#pragma once
#include <datalib/data_info.hpp>
#include <type_wrapper/type_wrapper.hpp>

namespace datalib {

template<class T>
struct tag_hex_t : type_wrapper_base<T> // don't alias!! must be a different type
{};

// When a type is wrappend on this tag, it'll print it in hex base
template<class T>
using hex = type_wrapper<T, tag_hex_t<T>>;


/*
 *  data_info<> specialization for 'hex<T>'
 */
template<typename T>
struct data_info<hex<T>> : data_info<T>
{
    // Performs cheap precomparision
    static bool precompare(const hex<T>& a, const hex<T>& b)
    {
        return datalib::precompare(a.get_(), b.get_());
    }
};


} // namespace datalib

