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

#ifndef TINYMPL_VARIADIC_AT_HPP
#define TINYMPL_VARIADIC_AT_HPP

#include <cstddef>

namespace tinympl {
namespace variadic {

/**
 * \ingroup VarBasics
 * \class at
 * \brief Extract the i-th element of a variadic template
 * \param i The index to extract
 */
template<int i,class ... Args> struct at;

template<int i,class ... Args> using at_t = typename at<i,Args...>::type;

template<int i,class Head,class ... Tail> struct at<i,Head,Tail...>
{
	static_assert(i < sizeof ... (Tail) + 1,"Index out of range");
	typedef typename at<i-1,Tail...>::type type;
};

template<class Head,class ... Tail> struct at<0,Head,Tail...>
{
	typedef Head type;
};

} // namespace variadic
} // namespace tinympl

#endif // TINYMPL_VARIADIC_AT_HPP
