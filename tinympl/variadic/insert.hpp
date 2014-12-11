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

#ifndef TINYMPL_VARIADIC_INSERT_HPP
#define TINYMPL_VARIADIC_INSERT_HPP

#include <cstddef>

namespace tinympl {
namespace variadic {

/**
 * \ingroup VarBasics
 * \class insert
 * \brief Produce an output sequence from a variadic template inserting a new element at a given position
 * \param pos The position of the new element
 * \param T the new element
 * \param Out the output sequence type
 * \param Args... the input variadic template
 */
template<std::size_t pos,class T,template<class ... > class Out,class ... Args> struct insert;

template<std::size_t pos,class T,template<class ... > class Out,class Head,class ... Args> struct insert<pos,T,Out,Head,Args...>
{
private:
	template<class ... CopiedElements>
	struct impl
	{
		typedef typename insert<pos-1,T,Out,Args...>::template impl<CopiedElements...,Head>::type type;
	};

	template<std::size_t,class,template<class ... > class,class ...> friend struct insert;

public:
	static_assert(pos <= sizeof ... (Args) + 1,"pos > sequence size!");

	typedef typename impl<>::type type;
};

template<class T,template<class ... > class Out,class Head,class ... Args> struct insert<0,T,Out,Head,Args...>
{
private:
	template<class ... CopiedElements>
	struct impl
	{
		typedef Out< CopiedElements ..., T, Head,Args ... > type;
	};

	template<std::size_t,class,template<class ... > class,class ...> friend struct insert;

public:
	typedef typename impl<>::type type;
};

template<class T,template<class ... > class Out> struct insert<0,T,Out>
{
private:
	template<class ... CopiedElements>
	struct impl
	{
		typedef Out< CopiedElements ..., T > type;
	};

	template<std::size_t,class,template<class ... > class,class ...> friend struct insert;

public:
	typedef typename impl<>::type type;
};

} // namespace variadic
} // namespace tinympl

#endif // TINYMPL_VARIADIC_INSERT_HPP
