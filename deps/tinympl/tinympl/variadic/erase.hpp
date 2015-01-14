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

#ifndef TINYMPL_VARIADIC_ERASE_HPP
#define TINYMPL_VARIADIC_ERASE_HPP

#include <cstddef>

namespace tinympl {
namespace variadic {

/**
 * \ingroup VarBasics
 * \class erase
 * \brief Produce an output sequence from a variadic template by removin the elements in the given range
 * \param start The index of the first element to be removed
 * \param end One plus the index of the last element to be removed
 * \param Out The output sequence type
 * \param Args... the input variadic template
 */
template<std::size_t start,std::size_t end,template<class ...> class Out,class ... Args> struct erase;

template<std::size_t start,std::size_t end,template<class ...> class Out,class ... Args> using erase_t = typename erase<start,end,Out,Args...>::type;

template<std::size_t start,std::size_t end,template<class ...> class Out,class Head,class ... Args> struct erase<start,end,Out,Head,Args...>
{
private:
	template<class ... CopiedElements>
	struct impl
	{
		typedef typename erase<start-1,end-1,Out,Args...>::template impl<CopiedElements...,Head>::type type;
	};

	template<std::size_t,std::size_t,template<class ...> class,class ...> friend struct erase;

public:
	static_assert(start <= end,"Start > end!");

	typedef typename impl<>::type type;
};

template<std::size_t end,template<class ...> class Out,class Head,class ... Args> struct erase<0,end,Out,Head,Args...>
{
private:
	template<class ... CopiedElements>
	struct impl
	{
		typedef typename erase<0,end-1,Out,Args...>::template impl<CopiedElements...>::type type;
	};

	template<std::size_t,std::size_t,template<class ...> class,class ...> friend struct erase;

public:
	typedef typename impl<>::type type;
};

template<template<class ...> class Out,class Head,class ... Args> struct erase<0,0,Out,Head,Args...>
{
private:
	template<class ... CopiedElements>
	struct impl
	{
		typedef Out<CopiedElements..., Head,Args...> type;
	};

	template<std::size_t,std::size_t,template<class ...> class,class ...> friend struct erase;

public:
	typedef typename impl<>::type type;
};

template<template<class ...> class Out> struct erase<0,0,Out>
{
private:
	template<class ... CopiedElements>
	struct impl
	{
		typedef Out<CopiedElements...> type;
	};

	template<std::size_t,std::size_t,template<class ...> class,class ...> friend struct erase;

public:
	typedef typename impl<>::type type;
};

} // namespace variadic
} // namespace tinympl

#endif // TINYMPL_VARIADIC_ERASE_HPP
