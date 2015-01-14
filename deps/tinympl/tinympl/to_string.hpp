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

#ifndef TINYMPL_TO_STRING_HPP
#define TINYMPL_TO_STRING_HPP

#include <tinympl/string.hpp>

namespace tinympl
{
namespace detail
{

//Handle generic numbers
template<class T,T value,T base = 10,class = void> struct to_string_impl
{
	typedef typename to_string_impl<T,value / base,base>::type head;
	typedef typename to_string_impl<T,value % base,base>::type tail;
	typedef typename head::template append<tail>::type type;
};

//Handle negative numbers
template<class T,T value,T base> struct to_string_impl<T,value,base,
	typename std::enable_if<(value < 0)>::type>
{
	typedef typename to_string_impl<T,-value,base>::type tail;
	typedef typename tail::template insert_c<0,'-'>::type type;
};

//Handle one digit numbers
template<class T,T value,T base> struct to_string_impl<T,value,base,
	typename std::enable_if<(value >= 0 && value < base)>::type>
{
	static_assert( value >= 0 && value < 16,"Base > 16 not supported");

	typedef basic_string<char,
		(value < 10 ?
			'0' + value :
			'a' + value - 10)> type;
};

}

/**
 * \ingroup String
 * @{
 */

//! Construct a string from a given integral value of type `T`.
template<class T,T value> using to_string = detail::to_string_impl<T,value>;
template<class T,T value> using to_string_t = typename to_string<T,value>::type;

//! Construct a string from the integer `value`
template<int value> using to_string_i = detail::to_string_impl<int,value>;
template<int value> using to_string_i_t = typename to_string_i<value>::type;

//! Construct a string from the long integer `value`
template<long value> using to_string_l = detail::to_string_impl<long,value>;
template<long value> using to_string_l_t = typename to_string_l<value>::type;

//! Construct a string from the unsigned integer `value`
template<unsigned value> using to_string_u = detail::to_string_impl<unsigned,value>;
template<unsigned value> using to_string_u_t = typename to_string_u<value>::type;

//! Construct a string from the long long integer `value`
template<long long value> using to_string_ll = detail::to_string_impl<long long,value>;
template<long long value> using to_string_ll_t = typename to_string_ll<value>::type;

/** @} */

}

#endif // TINYMPL_TO_STRING_HPP
