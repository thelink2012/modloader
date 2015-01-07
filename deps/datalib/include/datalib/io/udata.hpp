/*
 *  Copyright (C) 2014 Denilson das Mercês Amorim (aka LINK/2012)
 *  Licensed under the Boost Software License v1.0 (http://opensource.org/licenses/BSL-1.0)
 *
 */
#pragma once
#include <datalib/data_info/udata.hpp>
#include <datalib/detail/stream/fwd.hpp>

namespace datalib {

//
//  udata is marked as ignored in the data_info<> but just in case anyone try I/O with it, fail!
//

/*
 *  Input Checker
 */
template<class CharT, class Traits, class T> inline
datalib::basic_icheckstream<CharT, Traits>& operator>>(datalib::basic_icheckstream<CharT, Traits>& is, const udata<T>&)
{
    is.setstate(std::ios::failbit);
    return is;
}


/*
 *  Input
 */
template<class CharT, class Traits, class T> inline
std::basic_istream<CharT, Traits>& operator>>(std::basic_istream<CharT, Traits>& is, udata<T>&)
{
    is.setstate(std::ios::failbit);
    return is;
}

/*
 *  Output
 */
template<class CharT, class Traits, class T> inline
std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const udata<T>&)
{
    os.setstate(std::ios::failbit);
    return os;
}

}
