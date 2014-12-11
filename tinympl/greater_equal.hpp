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

#ifndef TINYMPL_GREATER_EQUAL_HPP
#define TINYMPL_GREATER_EQUAL_HPP

#include <tinympl/negate.hpp>
#include <tinympl/less.hpp>

namespace tinympl {

/**
 * \ingroup Comparisons
 * \class greater_equal
 * \brief Determines whether `A` is greater than or equal to `B`
 * \return `greater_equal<A,B>::type` is a `std::integral_constant<bool,v>` where `v` is `A::value >= B::value`
 * \note The default behaviour is to forward the call to \ref less.
 */
template<class A,class B> struct greater_equal : negate< typename less<A,B>::type > {};

} // namespace tinympl

#endif // TINYMPL_GREATER_EQUAL_HPP
