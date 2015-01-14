/*
 *  Copyright (C) 2014 Denilson das Mercês Amorim (aka LINK/2012)
 *  Licensed under the Boost Software License v1.0 (http://opensource.org/licenses/BSL-1.0)
 *
 */
#pragma once
#include <datalib/data_info.hpp>
#include <type_traits>

namespace datalib {

/*
 *  data_info<> specialization for 'std::integral_constant<T, Value>'
 */
template<typename T, T Value>
struct data_info<std::integral_constant<T, Value>> : data_info<T>
{
    static const int complexity = 0;
};

} // namespace datalib


namespace std {

    /*
     *  Integral constants are always equivalent to each other
     */

    template<typename T, T Value>
    bool operator==(const std::integral_constant<T, Value>&, const std::integral_constant<T, Value>&) 
    {
        return true;
    }

    template<typename T, T Value>
    bool operator<(const std::integral_constant<T, Value>&, const std::integral_constant<T, Value>&) 
    {
        return false;
    }

};

