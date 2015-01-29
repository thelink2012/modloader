/*
 *  Copyright (C) 2014 Denilson das Mercês Amorim (aka LINK/2012)
 *  Licensed under the Boost Software License v1.0 (http://opensource.org/licenses/BSL-1.0)
 *
 */
#pragma once
#include <type_wrapper/datalib/data_info/floating_point.hpp>
#include <datalib/detail/stream/fwd.hpp>
#include <iomanip>
#include <limits>

/*
 *  Input Checker
 */
template<class CharT, class Traits, class T, class Comp> inline
datalib::basic_icheckstream<CharT, Traits>& operator>>(datalib::basic_icheckstream<CharT, Traits>& is, const basic_floating_point<T, Comp>& tw)
{
    return (is >> tw.get_());
}

/*
 *  Input
 */
template<class CharT, class Traits, class T, class Comp> inline
std::basic_istream<CharT, Traits>& operator>>(std::basic_istream<CharT, Traits>& is, basic_floating_point<T, Comp>& tw)
{
    return (is >> std::setprecision(std::numeric_limits<T>::digits10 + 2) >> tw.get_());
}

/*
 *  Output
 */
template<class CharT, class Traits, class T, class Comp> inline
std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const basic_floating_point<T, Comp>& tw)
{
    return (os << std::setprecision(std::numeric_limits<T>::digits10 + 2) << tw.get_());
}
