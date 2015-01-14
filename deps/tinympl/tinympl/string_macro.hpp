// Copyright (C) 2013, Ennio Barbaro.
//
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://sbabbi.github.io/tinympl for documentation.
//
// You are welcome to contact the author at:
//  enniobarbaro@gmail.com
//

#ifndef TINYMPL_STRING_MACRO_HPP
#define TINYMPL_STRING_MACRO_HPP

#include <tinympl/string.hpp>

/**
 * \file string_macro.hpp Definition of macros to simplify the creation of a tinympl::string
 */

#define TINYMPL_STRING_JOIN2(arg1,arg2) TINYMPL_DO_STRING_JOIN2(arg1,arg2)
#define TINYMPL_DO_STRING_JOIN2(arg1,arg2) arg1 ## arg2

/**
 * \ingroup String
 */

/**
 * \def MAKE_TINYMPL_STRING(name,str)
 * Define a typedef called `name` to a `tinympl::basic_string` which contains the string `str`
*/
#define MAKE_TINYMPL_STRING(name,str) \
	constexpr const char TINYMPL_STRING_JOIN2(tinympl_string_temporary_, name) [] = str; \
	typedef tinympl::string<TINYMPL_STRING_JOIN2(tinympl_string_temporary_, name)> name

#endif // TINYMPL_STRING_MACRO_HPP
