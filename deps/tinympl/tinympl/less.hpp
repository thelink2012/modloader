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

#ifndef TINYMPL_LESS_HPP
#define TINYMPL_LESS_HPP

#include <type_traits>

namespace tinympl {

/**
 * \ingroup Comparisons
 * \class less
 * \brief Determines whether `A` is less than `B`
 * \return `less<A,B>::type` is a `std::integral_constant<bool,v>` where `v` is `A::value < B::value`
 * \note Users are allowed to specialize this metafunction for user-defined types
 *
 */
template<class A,class B> struct less : std::integral_constant<bool, (A::value < B::value)> {};

} // namespace tinympl

#endif // TINYMPL_LESS_HPP
