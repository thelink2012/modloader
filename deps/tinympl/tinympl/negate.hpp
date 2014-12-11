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

#ifndef TINYMPL_NEGATE_HPP
#define TINYMPL_NEGATE_HPP

#include <type_traits>

namespace tinympl {

/**
 * \ingroup Arithmetic
 * \class negate
 * \brief Returns the opposite of the compile time constant A
 * \return `negate<A>::type` is `std::integral_constant<A::value_type,-A::value>`
 */
template<class A> struct negate : std::integral_constant<typename A::value_type, - A::value> {};

} // namespace tinympl

#endif // TINYMPL_NEGATE_HPP
