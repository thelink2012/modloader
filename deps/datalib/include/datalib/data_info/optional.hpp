/*
 *  Copyright (C) 2014 Denilson das Mercês Amorim (aka LINK/2012)
 *  Licensed under the Boost Software License v1.0 (http://opensource.org/licenses/BSL-1.0)
 *
 */
#pragma once
#include <datalib/data_info.hpp>
#include <datalib/detail/optional.hpp>

#ifndef BOOST_OPTIONAL_NO_IOFWD
#error Please include optional<T> from datalib/io/optional.hpp
#endif

namespace datalib {


/*
 *  data_info<> specialization for 'optional<T>'
 */
template<typename T>
struct data_info<optional<T>> : data_info<T>
{
    static const char separator = '\0';
    static const char base_separator = data_info<T>::separator;

    // Performs precomparision (that's beforing comparing anything else, perform this cheap comparision)
    struct precompare
    {
        // Checks if both optional objects have a equivalent state
        static bool equal_to(const optional<T>& opt1, const optional<T>& opt2)
        {
            if((bool)(opt1) == (bool)(opt2))
                return true;
            return false;
        }
    };
};


} // namespace datalib

