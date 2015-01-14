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

#ifndef TINYMPL_SIZEOF_HPP
#define TINYMPL_SIZEOF_HPP

#include <type_traits>

namespace tinympl {

/**
 * \ingroup Functional
 * \class sizeof_
 * \brief Returns an `std::integral_constant<std::size_t,V>` where `V` is the compile time size of the input type
 */
template<class T> struct sizeof_ : std::integral_constant<std::size_t, sizeof(T)> {};

} // namespace tinympl

#endif // TINYMPL_SIZEOF_HPP
