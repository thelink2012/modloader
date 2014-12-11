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

#ifndef TINYMPL_SORT_HPP
#define TINYMPL_SORT_HPP

#include <tinympl/variadic/sort.hpp>
#include <tinympl/as_sequence.hpp>
#include <tinympl/sequence.hpp>
#include <tinympl/less.hpp>

namespace tinympl {

/**
 * \ingroup SeqSort
 * \class sort
 * \brief Sort the input sequence according to a given comparison function
 * \param Sequence the input sequence
 * \param Out the output sequence type, defaults to the same kind of the input
sequence type
 * \param Cmpl The comparison operator. `Cmp<A,B>::type::value` must be
convertible to bool. The comparator must produce total ordering between
elements. Defaults to \ref tinympl::less
 * \note The compile time complexity is O(N^2)
 * \return `sort<...>::type` is a type templated from `Out` which contains the
sorted sequence
 * \sa variadic::sort
 */
template<class Sequence,
        template<class ...> class Out = as_sequence<Sequence>::template rebind,
        template<class ... > class Cmp = less>
struct sort : sort<as_sequence_t<Sequence>, Out, Cmp> {};

template<template<class ... > class Cmp,
        template<class ...> class Out,
        class ... Args>
struct sort<sequence<Args...>, Out, Cmp> : variadic::sort<Cmp, Out, Args...> {};

} // namespace tinympl

#endif // TINYMPL_SORT_HPP
