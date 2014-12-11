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

#ifndef TINYMPL_FIND_IF_HPP
#define TINYMPL_FIND_IF_HPP

#include <tinympl/variadic/find_if.hpp>
#include <tinympl/as_sequence.hpp>
#include <tinympl/sequence.hpp>

namespace tinympl {

/**
 * \ingroup SeqNonModAlgs
 * \class find_if Compute the index of a the first element in the sequence which
satisfies a given predicate
 * \param Sequence The input sequence
 * \param F The test predicate - `F<T>::type::value` shall be convertible to
bool
 * \return `find_if<...>::type` is `std::integral_constant<std::size_t,v>` where
`v` is the 0-based index of the first element which satisfy `F`. If no such
element exists, `v` is `size<Sequence>::value`.
 * \sa variadic::find_if
 */
template<class Sequence, template<class ... T> class F>
struct find_if : find_if<as_sequence_t<Sequence>, F > {};

template<template<class ... T> class F, class ... Args>
struct find_if<sequence<Args...>, F> : variadic::find_if<F, Args...> {};

} // namespace tinympl

#endif // TINYMPL_FIND_IF_HPP
