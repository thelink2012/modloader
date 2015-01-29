/*
 *  Copyright (C) 2014 Denilson das Mercês Amorim (aka LINK/2012)
 *  Licensed under the Boost Software License v1.0 (http://opensource.org/licenses/BSL-1.0)
 *
 */
#pragma once
#include <iosfwd>

namespace datalib {

// from memorybuf.hpp
class memorybuf;

// from memstream.hpp
template<class CharT, class Traits = std::char_traits<CharT>>
class basic_imemstream;

// from kstream.hpp
template<typename CharT, typename Traits = std::char_traits<CharT>>
class basic_icheckstream;


// Alias the icheckstream object
using icheckstream = basic_icheckstream<char, std::char_traits<char>>;

// Alias the imemstream object
using imemstream = basic_imemstream<char, std::char_traits<char>>;



} // namespace datalib
