/*
 *  Copyright (C) 2014 Denilson das Mercês Amorim (aka LINK/2012)
 *  Licensed under the Boost Software License v1.0 (http://opensource.org/licenses/BSL-1.0)
 *
 */
#pragma once
#include <type_wrapper/floating_point.hpp>
#include <datalib/data_info.hpp>

namespace datalib
{
    /*
     *  data_info<> specialization for 'std::basic_floating_point<T, Comparer>'
     */
    template<typename T, class Comparer>
    struct data_info<basic_floating_point<T, Comparer>> : data_info<T>
    {
        static const int complexity = 2 * data_info<T>::complexity;
    };
}
