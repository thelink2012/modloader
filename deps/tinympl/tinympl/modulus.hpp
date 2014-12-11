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

#ifndef TINYMPL_MODULUS_HPP
#define TINYMPL_MODULUS_HPP

#include <type_traits>

namespace tinympl {

/**
 * \ingroup Arithmetic
 * \class modulus
 * \brief Computes `A`%`B` where `A` and `B` are compile time constants
 * \return `modulus<A,B>::type` is a `std::integral_constant<T,v>` where `T` is the common type between `A` and `B`, and `v` is `A::value % B::value`
 */
template<class A,class B> struct modulus :
	std::integral_constant<
		typename std::common_type<
			typename A::value_type,
			typename B::value_type>::type,
		A::value % B::value>
{};

} // namespace tinympl

#endif // TINYMPL_MODULUS_HPP
