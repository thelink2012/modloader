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

#ifndef TINYMPL_LAMBDA_HPP
#define TINYMPL_LAMBDA_HPP

#include <tinympl/bind.hpp>
#include <type_traits>

namespace tinympl {

/**
 * \ingroup BindAndLambda
 * @{
 */


template<class T> struct protect {typedef T type;};

template<class Expr>
struct lambda
{
	template<class ... Ts>
	struct eval
	{
		template<class T,class Enable = void> struct pick {typedef T type;};
		template<class T> struct pick<T, typename std::enable_if< (is_placeholder<T>::type::value > 0)>::type> {typedef variadic::at_t<is_placeholder<T>::value-1, Ts ... > type;};
		template<class T> struct pick<T, typename std::enable_if< is_bind_expression<T>::type::value>::type> {typedef typename T::template eval<Ts...>::type type;};

		typedef typename pick<Expr>::type type;
	};

	template<class ... Ts> using eval_t = typename eval<Ts...>::type;
};

template< template<class ...> class F, class ... Args> struct lambda<F<Args...> >
{
	template<class ... Ts>
	struct eval
	{
		template<class T> using forward_t = typename T::template eval<Ts...>::type;

		typedef typename F< forward_t<lambda<Args> > ... >::type type;
	};

	template<class ... Ts> using eval_t = typename eval<Ts...>::type;
};

template< template<class ...> class F,class ... Args> struct lambda< protect<F<Args...> > >
{
	template<class ... Ts>
	struct eval
	{
		template<class T> using forward_t = typename T::template eval<Ts...>::type;

		typedef F< forward_t< lambda<protect<Args> > > ... > type;
	};

	template<class ... Ts> using eval_t = typename eval<Ts...>::type;
};

template<class Expr> struct is_bind_expression<lambda<Expr> > : std::true_type {};

/** @} */

}

#endif // TINYMPL_LAMBDA_HPP
