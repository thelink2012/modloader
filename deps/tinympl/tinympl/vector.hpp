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

#ifndef TINYMPL_VECTOR_HPP
#define TINYMPL_VECTOR_HPP

#include <tinympl/algorithm.hpp>
#include <tinympl/sequence.hpp>

namespace tinympl {

/**
 * \defgroup Containers Containers
 * Full and half compile time containers of types and values.
 * @{
 */

/**
 * \class vector
 * \brief A compile time vector of types
 * Vector is the simplest tinympl sequence type. It provides standard modifiers and random access
 * to its elements.
 */
template<class ... Args>
struct vector
{
	enum
	{
		size = sizeof ... (Args) //!< The size of the vector
	};

	enum
	{
		empty = (size == 0) //!< Determine whether the vector is empty
	};

	//! Access the i-th element
	template<std::size_t i>
	struct at
	{
		static_assert(i < size,"Index i is out of range");
		typedef typename variadic::at<i,Args...>::type type;
	};

	//! Return a new vector constructed by inserting `T` on the back of the current vector
	template<class T>
	struct push_back
	{
		typedef vector<Args...,T> type;
	};

	//! Return a new vector constructed by inserting `T` on the front of the current vector
	template<class T>
	struct push_front
	{
		typedef vector<T,Args...> type;
	};

	//! Return a new vector constructed by removing the last element of the current vector
	struct pop_back
	{
		typedef typename variadic::erase<size-1,size,tinympl::vector,Args...>::type type;
	};

	//! Return a new vector constructed by removing the first element of the current vector
	struct pop_front
	{
		typedef typename variadic::erase<0,1,tinympl::vector,Args...>::type type;
	};

	//! Return a new vector constructed by erasing the elements in the range [first,last) of the current vector
	template<std::size_t first,std::size_t last>
	struct erase : tinympl::erase<first,last,vector<Args...>,tinympl::vector> {};

	//! Return a new vector constructed by inserting the elements `Ts...` in the current vector starting at the index `i`
	template<std::size_t i,class ... Ts>
	struct insert : tinympl::insert<i,
		sequence<Ts...>,
		vector<Args...>,
		tinympl::vector> {};

	//! Return the first element of the vector
	struct front
	{
		typedef typename variadic::at<0,Args...>::type type;
	};

	//! Return the last element of the vector
	struct back
	{
		typedef typename variadic::at<size-1,Args...>::type type;
	};
};

/** @} */

/**
 * \ingroup SeqCustom
 * \brief Customization point to allow `vector` to work as a tinympl sequence
 */
template<class ... Args> struct as_sequence<vector<Args...> >
{
	typedef sequence<Args...> type;
	template<class ... Ts> using rebind = vector<Ts...>;
};

}

#endif // TINYMPL_VECTOR_HPP
