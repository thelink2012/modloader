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

#ifndef TINYMPL_MULTIPLIES_HPP
#define TINYMPL_MULTIPLIES_HPP

#include <type_traits>

namespace tinympl {

/**
 * \ingroup Arithmetic
 * \class multiplies
 * \brief Multiplies its arguments
 * \return `multiplies<Args...>::type` is a `std::integral_constant<T,v>` where `T` is the common type between the `Args::type...` and `v` is the product of `Args::value ...`
 */
template<class ... Args> struct multiplies;

template<class Head,class ... Tail> struct multiplies<Head,Tail...> : multiplies<Head, typename multiplies<Tail...>::type> {};

template<class A,class B> struct multiplies<A,B> :
	std::integral_constant<
		typename std::common_type<
			typename A::value_type,
			typename B::value_type
		>::type, A::value * B::value>
{};

} // namespace tinympl

#endif // TINYMPL_MULTIPLIES_HPP
