/*
 *  Copyright (C) 2014 Denilson das Mercês Amorim (aka LINK/2012)
 *  Licensed under the Boost Software License v1.0 (http://opensource.org/licenses/BSL-1.0)
 *
 */
#pragma once
#include <datalib/data_info/hex.hpp>
#include <datalib/detail/stream/fwd.hpp>
#include <iomanip>

namespace datalib
{

/*
 *  Input Checker
 */
template<class CharT, class Traits, class T> inline
basic_icheckstream<CharT, Traits>& operator>>(datalib::basic_icheckstream<CharT, Traits>& is, const hex<T>& h)
{
    auto f = is.setf(std::ios::hex , std::ios::basefield);
    is >> h.get_();
    is.setf(f, std::ios::basefield);
    return is;
}

/*
 *  Input
 */
template<class CharT, class Traits, class T> inline
std::basic_istream<CharT, Traits>& operator>>(std::basic_istream<CharT, Traits>& is, hex<T>& h)
{
    auto f = is.setf(std::ios::hex , std::ios::basefield);
    is >> h.get_();
    is.setf(f, std::ios::basefield);
    return is;
}

/*
 *  Output
 */
template<class CharT, class Traits, class T> inline
std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const hex<T>& h)
{
    auto f = os.setf(std::ios::hex , std::ios::basefield);
    os << h.get_();
    os.setf(f, std::ios::basefield);
    return os;
}

}
