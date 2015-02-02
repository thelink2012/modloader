/*
 *  Copyright (C) 2014 Denilson das Mercês Amorim (aka LINK/2012)
 *  Licensed under the Boost Software License v1.0 (http://opensource.org/licenses/BSL-1.0)
 *
 */
#pragma once
#include <array>
#include <datalib/data_info.hpp>

namespace datalib {

/*
 *  data_info<> specialization for 'std::array<T, N>'
 */
template<typename T, std::size_t N>
struct data_info<std::array<T, N>> : data_info_base
{
    static const char separator = 0; // manual separator
    static const int complexity = N * data_info<T>::complexity; // N times the complexity of T

    static bool precompare(const std::array<T, N>& a, const std::array<T, N>& b)
    {
        for(size_t i = 0; i < N; ++i)
        {
            if(!datalib::precompare(a[i], b[i]))
                return false;
        }
        return true;
    }
};


} // namespace datalib

