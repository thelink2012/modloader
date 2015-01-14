/*
 * Copyright (C) 2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#pragma once

/*
 *  So, currently, as of 01/10/2015 the standard library implementation of regex in MSVC (and I heard in GCC and clang too) are
 *  very very slow, I mean VERY slow, I did a simple comparision and something boost does in 1 second the std implementation does in almost 20 seconds!
 *  So for now let's stick with the boost implementation of regex (Boost.Xpressive not Boost.Regex since header-only)
 */

#if REGEX_USE_STDCXX
#include <regex>
using sregex = std::regex;
using std::smatch;
using std::ssub_match;
using std::regex_match;
using std::sregex_iterator;
using std::sregex_token_iterator;
#else
#include <boost/xpressive/xpressive_dynamic.hpp>
using namespace boost::xpressive;
#endif

inline sregex make_regex(const char* begin, sregex::flag_type flags = sregex::ECMAScript|sregex::optimize)
{
#if REGEX_USE_STDCXX
    return sregex(begin, flags);
#else
    return sregex::compile(begin, flags);
#endif
}

inline sregex make_regex(const std::string& begin, sregex::flag_type flags = sregex::ECMAScript|sregex::optimize)
{
    return make_regex(begin.c_str(), flags);
}


