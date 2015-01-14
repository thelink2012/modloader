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

#ifndef TINYMPL_SHORT_HPP
#define TINYMPL_SHORT_HPP

#include <type_traits>

namespace tinympl {

//! \ingroup NewTypes
//! Compile time short
template<short i> using short_ = std::integral_constant<short,i>;

} // namespace tinympl

#endif // TINYMPL_SHORT_HPP
