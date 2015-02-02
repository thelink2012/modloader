/*
 *  Copyright (C) 2014 Denilson das Mercês Amorim (aka LINK/2012)
 *  Licensed under the Boost Software License v1.0 (http://opensource.org/licenses/BSL-1.0)
 *
 */
#pragma once
#define BOOST_OPTIONAL_NO_IOFWD
#include <datalib/data_info/optional.hpp>
#include <datalib/detail/stream/fwd.hpp>

namespace datalib {

/*
 *  Input Checker
 */
template<class CharT, class Traits, class T> inline
datalib::basic_icheckstream<CharT, Traits>& operator>>(datalib::basic_icheckstream<CharT, Traits>& is, const optional<T>& opt)
{
    datalib::basic_icheckstream<CharT, Traits>::reposer xrepos(is);
    datalib::basic_icheckstream<CharT, Traits>::sentry xsentry(is);
    if(xsentry)
    {
        // skip optional stuff if necessary
        auto& dummy_ref = *((const T*)(nullptr));
        if(is >> dummy_ref)
        {
            xrepos.norepos();               // avoid repos on xrepos destruction, we are fine
            return is;
        }
    }
    is.clear(); // clear failure flags, optional object
    // expects reposition on xrepos destruction
    return is;
}

/*
 *  Input
 */
template<class CharT, class Traits, class T> inline
std::basic_istream<CharT, Traits>& operator>>(std::basic_istream<CharT, Traits>& is, optional<T>& opt)
{
    auto tell = is.tellg(); // before xsentry constructor runs!
    std::basic_istream<CharT, Traits>::sentry xsentry(is);
    if(xsentry)
    {
        T obj;
        if(is >> obj)
        {
            opt.emplace(std::move(obj));
            return is;
        }
    }
    opt = none;
    is.clear();         // clear failure flags, optional object (note seekg fails with eof flag active)
    is.seekg(tell);     // seek back to old pos, before we have read input
    is.clear();
    return is;
}

/*
 *  Output
 */
template<class CharT, class Traits, class T> inline
std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const optional<T>& opt)
{
    std::basic_ostream<CharT, Traits>::sentry xsentry(os);
    if(xsentry)
    {
        if(opt) 
        {
            ((os << opt.get()) && print_separator<T>(os));
        }
    }
    return os;
}

}
