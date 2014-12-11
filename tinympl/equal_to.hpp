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

#ifndef TINYMPL_EQUAL_TO_HPP
#define TINYMPL_EQUAL_TO_HPP

#include <type_traits>

namespace tinympl {

/**
 * \ingroup Comparisons
 * \class equal_to
 * \brief Determines whether the types `A` and `B` are equal
 * \return `equal_to<A,B>::type` is a `std::integral_constant<bool,v>` where `v` is true iff `A` and `B` are equal
 * \note The default behaviour is to forward the call to std::is_same. Users are allowed to specialize this metafunction for user-defined types
 */
template<class A,class B> struct equal_to : std::is_same<A,B> {};
template<class T,class U,T t,U u> struct equal_to<
	std::integral_constant<T,t>,
	std::integral_constant<U,u> > : std::integral_constant<bool,t ==u> {};


} // namespace tinympl

#endif // TINYMPL_EQUAL_TO_HPP
