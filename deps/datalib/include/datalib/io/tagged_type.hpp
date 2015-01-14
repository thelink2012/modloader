/*
 *  Copyright (C) 2014 Denilson das Mercês Amorim (aka LINK/2012)
 *  Licensed under the Boost Software License v1.0 (http://opensource.org/licenses/BSL-1.0)
 *
 */
#pragma once
#include <datalib/data_info/tagged_type.hpp>
#include <datalib/detail/stream/fwd.hpp>

namespace datalib {

/*
 *  Input Checker
 */
template<class CharT, class Traits, class T, class Tag> inline
datalib::basic_icheckstream<CharT, Traits>& operator>>(datalib::basic_icheckstream<CharT, Traits>& is, const tagged_type<T, Tag>& tt)
{
    return (is >> get(tt));
}


/*
 *  Input
 */
template<class CharT, class Traits, class T, class Tag> inline
std::basic_istream<CharT, Traits>& operator>>(std::basic_istream<CharT, Traits>& is, tagged_type<T, Tag>& tt)
{
    return (is >> get(tt));
}

/*
 *  Output
 */
template<class CharT, class Traits, class T, class Tag> inline
std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const tagged_type<T, Tag>& tt)
{
    return (os << get(tt));
}

}
