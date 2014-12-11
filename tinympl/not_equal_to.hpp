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

#ifndef TINYMPL_NOT_EQUAL_TO_HPP
#define TINYMPL_NOT_EQUAL_TO_HPP

#include <type_traits>
#include <tinympl/equal_to.hpp>

namespace tinympl {

/**
 * \class not_equal_to
 * \brief Determines whether the types `A` and `B` are not equal
 * \return `not_equal_to<A,B>::type` is a `std::integral_constant<bool,v>` where `v` is `!equal_to<A,B>::value`
 */
template<class A,class B> struct not_equal_to : std::integral_constant<bool,! equal_to<A,B>::value > {};

} // namespace tinympl

#endif // TINYMPL_NOT_EQUAL_TO_HPP
