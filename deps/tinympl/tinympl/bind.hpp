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

#ifndef TINYMPL_BIND_HPP
#define TINYMPL_BIND_HPP

#include <tinympl/variadic.hpp>

namespace tinympl {

/**
 * \defgroup BindAndLambda Bind and Lambda expressions.
 * Helpers to transform and define metafunction classes on the fly.
 * @{
 */

/**
 * \class bind
 * \brief Produce a template type by binding the given arguments on the passed template template.
 *
 * `tinympl::bind` is the compile time equivalent of `std::bind`. It produces a new template
 * type `bind<...>::template eval` which invokes the given one (`F`) with some of its arguments bound to `Args`.
 * Notice that in C++11 the effect of bind can be achieved with template aliases. In order to produce a cleaner
 * code, we recommend to use template aliases wherever is possible, and use `bind` only when necessary.
 *
 * `bind< std::is_same, arg1, U>::template eval` is equivalent to
 * `template<class T> using is_same1 = std::is_same<T,U>;`
 *
 * `bind` also automatically recognize bind expressions in its subarguments, so it is possible to nest multiple bind calls:
 *
 * `bind< std::is_same, bind<std::remove_reference,arg1>, U>::template eval` is equivalent to
 * `template<class T> using is_same1 = std::is_same< typename std::remove_reference<T>::type, U>;`
 */
template< template<class ... T> class F,class ... Args> struct bind;

template<std::size_t> struct arg;
typedef arg<1> arg1;
typedef arg<2> arg2;
typedef arg<3> arg3;
typedef arg<4> arg4;
typedef arg<5> arg5;
typedef arg<6> arg6;
typedef arg<7> arg7;
typedef arg<8> arg8;

/**
 * \brief Determine whether a type is a placeholder.
 * `is_placeholder<T>::value` is 0 if `T` is not a placeholder, otherwise is the index of the placeholder
 */
template<class T> struct is_placeholder : std::integral_constant<std::size_t, 0> {};
template<std::size_t i> struct is_placeholder< arg<i> > : std::integral_constant<std::size_t, i> 
{
	static_assert(i != 0, "Placeholder arg<0> is undefined");
};

/**
 * \brief Determine whether a type is a bind expression.
 */
template<class T> struct is_bind_expression : std::false_type {};
template<template<class ... T> class F,class ... Args> struct is_bind_expression< bind<F,Args...> > : std::true_type {};

/** @} */

template< template<class ... T> class F,class Head,class ... Tail> struct bind<F,Head,Tail...>
{
private:
	template<class ... Args>
	struct call
	{
		template<class ... BoundArgs>
		struct eval
		{
			template<class T,class Enable = void> struct pick {typedef T type;};
			template<class T> struct pick<T, typename std::enable_if< (is_placeholder<T>::value > 0) >::type> {typedef variadic::at_t<is_placeholder<T>::value-1, Args ... > type;};
			template<class T> struct pick<T, typename std::enable_if< is_bind_expression<T>::type::value>::type> {typedef typename T::template eval<Args...>::type type;};

			typedef typename pick<Head>::type argument_t;

			//Forward the call to bind
			typedef typename bind<F,Tail...>::template call<Args...>::template eval<BoundArgs..., argument_t>::type type;
		};
	};

	template< template<class ...> class,class ...> friend struct bind;

public:
	template<class ... Args>
	struct eval
	{
		using type = typename call<Args...>::template eval<>::type;
	};

	template<class ... Args>
	using eval_t = typename eval<Args...>::type;
};

template< template<class ... T> class F> struct bind<F>
{
private:
	template<class ... Args>
	struct call
	{
		template<class ... BoundArgs>
		struct eval
		{
			typedef typename F<BoundArgs...>::type type;
		};
	};

	template< template<class ...> class,class ...> friend struct bind;

public:
	template<class ... Args>
	struct eval
	{
		using type = typename call<Args...>::template eval<>::type;
	};

	template<class ... Args>
	using eval_t = typename eval<Args...>::type;
};

}

#endif // MPL_BIND_HPP
