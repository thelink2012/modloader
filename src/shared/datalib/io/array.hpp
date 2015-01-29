/*
 *  Copyright (C) 2014 Denilson das Mercês Amorim (aka LINK/2012)
 *  Licensed under the Boost Software License v1.0 (http://opensource.org/licenses/BSL-1.0)
 *
 */
#pragma once
#include <array>
#include <datalib/data_info/array.hpp>
#include <datalib/detail/stream/fwd.hpp>

namespace datalib {

/*
 *  Input Checker
 */
template<class CharT, class Traits, class T, std::size_t N> inline
datalib::basic_icheckstream<CharT, Traits>& operator>>(datalib::basic_icheckstream<CharT, Traits>& is, const std::array<T, N>& array)
{
    datalib::basic_icheckstream<CharT, Traits>::reposer xrepos(is, true);
    datalib::basic_icheckstream<CharT, Traits>::sentry  xsentry(is);
    if(xsentry)
    {
        for(std::size_t i = 0; i < N; ++i)
            if((is >> array[i]).fail()) break;
    }
    return is;
}

}

namespace std {

/*
 *  Input
 */
template<class CharT, class Traits, class T, std::size_t N> inline
std::basic_istream<CharT, Traits>& operator>>(std::basic_istream<CharT, Traits>& is, std::array<T, N>& array)
{
    std::basic_istream<CharT, Traits>::sentry xsentry(is);
    if(xsentry)
    {
        for(std::size_t i = 0; i < N; ++i)
            if((is >> array[i]).fail()) break;
    }
    return is;
}

/*
 *  Output
 */
template<class CharT, class Traits, class T, std::size_t N> inline
std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const std::array<T, N>& array)
{
    std::basic_ostream<CharT, Traits>::sentry xsentry(os);
    if(xsentry)
    {
        for(std::size_t i = 0; i < N; ++i)
        {
            if(os << array[i])
            {
                if(i+1 != N)    // all but the last element should have a ' ' after them
                {
                    if((os << ' ').fail())
                        break;
                }
            }
            else break;
        }
    }
    return os;
}

}
