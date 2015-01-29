/*
 *  Copyright (C) 2014 Denilson das Mercês Amorim (aka LINK/2012)
 *  Licensed under the Boost Software License v1.0 (http://opensource.org/licenses/BSL-1.0)
 *
 */
#pragma once
#include <datalib/data_info/pair.hpp>
#include <datalib/detail/stream/fwd.hpp>

namespace datalib {

/*
 *  Input Checker
 */
template<class CharT, class Traits, class T1, class T2> inline
datalib::basic_icheckstream<CharT, Traits>& operator>>(datalib::basic_icheckstream<CharT, Traits>& is, const std::pair<T1, T2>& pair)
{
    datalib::basic_icheckstream<CharT, Traits>::reposer xrepos(is, true);
    datalib::basic_icheckstream<CharT, Traits>::sentry  xsentry(is);
    if(xsentry)
    {
        ((is >> pair.first) && (is >> pair.second));
    }
    return is;
}

}


namespace std {

/*
 *  Input
 */
template<class CharT, class Traits, class T1, class T2> inline
std::basic_istream<CharT, Traits>& operator>>(std::basic_istream<CharT, Traits>& is, std::pair<T1, T2>& pair)
{
    std::basic_istream<CharT, Traits>::sentry xsentry(is);
    if(xsentry)
    {
        ((is >> pair.first) && (is >> pair.second));
    }
    return is;
}

/*
 *  Output
 */
template<class CharT, class Traits, class T1, class T2> inline
std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const std::pair<T1, T2>& pair)
{
    std::basic_ostream<CharT, Traits>::sentry xsentry(os);
    if(xsentry)
    {
        ((os << pair.first) && (os << ' ') && (os << pair.second));
    }
    return os;
}

}
