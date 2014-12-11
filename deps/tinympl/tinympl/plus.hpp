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

#ifndef TINYMPL_PLUS_HPP
#define TINYMPL_PLUS_HPP

#include <type_traits>

namespace tinympl {

/**
 * \ingroup Arithmetic
 * \class plus
 * \brief Sums its arguments
 * \return `plus<Args...>::type` is a `std::integral_constant<T,v>` where `T` is the common type between the `Args::type...` and `v` is the sum of `Args::value ... `
 */
template<class ... Args> struct plus;
template<class Head,class ... Tail> struct plus<Head,Tail...> : plus<Head, typename plus<Tail...>::type> {};

template<class A,class B> struct plus<A,B> :
	std::integral_constant<
		typename std::common_type<
			typename A::value_type,
			typename B::value_type
		>::type, A::value + B::value>
{};

template<class Head> struct plus<Head> : std::integral_constant<typename Head::value_type,Head::value> {};

} // namespace tinympl

#endif // TINYMPL_PLUS_HPP
