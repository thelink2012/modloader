/*
 *  Copyright (C) 2014 Denilson das Mercês Amorim (aka LINK/2012)
 *  Licensed under the Boost Software License v1.0 (http://opensource.org/licenses/BSL-1.0)
 *
 */
#pragma once
#include <datalib/detail/stream/fwd.hpp>
#include <type_traits>
#include <stdexcept>

// Specialization to transform enum to string and vice-versa
namespace datalib
{
    // template<class T> std::string to_string(T evalue);
    // template<class T> T& from_string(const std::string& str, T& evalue);
}

namespace datalib
{

/*
 *  Input Checker
 */
template<class CharT, class Traits, class T> inline
typename std::enable_if<std::is_enum<T>::value, basic_icheckstream<CharT, Traits>&>::type
/* basic_icheckstream<CharT, Traits>& */ operator>>(datalib::basic_icheckstream<CharT, Traits>& is, const T&)
{
    using namespace datalib;
    try {
        std::string str; T value;
        if(is >> str) from_string<T>(str, value);
    } catch(const std::invalid_argument&) {
        // ignore this exception
    }
    return is;
}

}

namespace std
{

/*
 *  Input
 */
template<class CharT, class Traits, class T> inline
typename std::enable_if<std::is_enum<T>::value, std::basic_istream<CharT, Traits>&>::type
/* std::basic_istream<CharT, Traits>& */ operator>>(std::basic_istream<CharT, Traits>& is, T& value)
{
    using namespace datalib;
    try {
        std::string str;
        if(is >> str) from_string<T>(str, value);
    } catch(const std::invalid_argument&) {
        // ignore this exception
    }
    return is;
}

/*
 *  Output
 */
template<class CharT, class Traits, class T> inline
typename std::enable_if<std::is_enum<T>::value, std::basic_ostream<CharT, Traits>&>::type
/* std::basic_ostream<CharT, Traits>& */ operator<<(std::basic_ostream<CharT, Traits>& os, const T& value)
{
    using namespace datalib;
    os << datalib::to_string(value);
    return os;
}

}
