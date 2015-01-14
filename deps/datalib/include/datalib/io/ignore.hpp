/*
 *  Copyright (C) 2014 Denilson das Mercês Amorim (aka LINK/2012)
 *  Licensed under the Boost Software License v1.0 (http://opensource.org/licenses/BSL-1.0)
 *
 */
#pragma once
#include <datalib/data_info/ignore.hpp>
#include <datalib/detail/stream/fwd.hpp>
#include <utility>

namespace datalib {

/*
 *  Input Checker
 */
template<class CharT, class Traits, typename T, class IgTraits> inline
basic_icheckstream<CharT, Traits>& operator>>(basic_icheckstream<CharT, Traits>& is, const ignore<T, IgTraits>& ig)
{
    basic_icheckstream<CharT, Traits>::sentry  xsentry(is);
    if(xsentry)
    {
        auto& dummy = *((const T*)(nullptr));
        is >> dummy;
    }
    return is;
}

/*
 *  Input
 */
template<class CharT, class Traits, class T, class IgTraits> inline
std::basic_istream<CharT, Traits>& operator>>(std::basic_istream<CharT, Traits>& is, ignore<T, IgTraits>& ig)
{
    std::basic_istream<CharT, Traits>::sentry xsentry(is);
    if(xsentry)
    {
        T obj;
        is >> obj;
    }
    return is;
}

/*
 *  Output
 */
template<class CharT, class Traits, class T, class IgTraits> inline
std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const ignore<T, IgTraits>& ig)
{
    std::basic_ostream<CharT, Traits>::sentry xsentry(os);
    if(xsentry)
    {
        auto obj = IgTraits::output();
        os << obj;
    }
    return os;
}

}
