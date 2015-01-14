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

#ifndef TINYMPL_CHAR_HPP
#define TINYMPL_CHAR_HPP

#include <type_traits>

namespace tinympl {

//! \ingroup NewTypes
//! Compile time char
template<char i> using char_ = std::integral_constant<char,i>;

} // namespace tinympl

#endif // TINYMPL_CHAR_HPP
