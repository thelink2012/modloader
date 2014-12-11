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

#ifndef TINYMPL_MAX_ELEMENT_HPP
#define TINYMPL_MAX_ELEMENT_HPP

#include <tinympl/variadic/max_element.hpp>
#include <tinympl/less.hpp>
#include <tinympl/as_sequence.hpp>
#include <tinympl/sequence.hpp>

namespace tinympl {

/**
 * \ingroup SeqMaxMin
 * \class max_element
 * \brief Compute the index of the largest element in a sequence
 * \param Sequence the input sequence
 * \param Cmp the comparator function; `Cmp<A,B>::type::value` must be
convertible to bool. Defaults to \ref tinympl::less
 * \return `max_element<...>::type` is an
`std::integral_constant<std::size_t,v>` where `v` is the 0-based index of the
maximum element
 * \sa variadic::max_element
 */
template<class Sequence, template<class ... > class Cmp = less>
struct max_element : max_element<as_sequence_t<Sequence>, Cmp > {};

template<template<class ... > class Cmp, class ... Args>
struct max_element<sequence<Args...>, Cmp > :
    variadic::max_element<Cmp, Args...> {};

} // namespace tinympl

#endif // TINYMPL_MAX_ELEMENT_HPP
