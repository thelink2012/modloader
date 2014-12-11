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

#ifndef TINYMPL_COUNT_IF_HPP
#define TINYMPL_COUNT_IF_HPP

#include <tinympl/variadic/count_if.hpp>
#include <tinympl/as_sequence.hpp>
#include <tinympl/sequence.hpp>

namespace tinympl {

/**
 * \ingroup SeqNonModAlgs
 * \class count_if
 * \brief Counts the number of elements which satisfy a given predicate
 * \param Sequence The input sequence
 * \param F The predicate - `F<T>::type::value` shall be convertible to bool
 * \return `count_if<...>::type` is `std::integral_constant<std::size_t,V>`
where `V` is the number of elements in the sequence which satisfy the predicate
`F`.
 * \sa variadic::count_if
 */
template<class Sequence, template<class ... T> class F>
struct count_if : count_if<as_sequence_t<Sequence>, F > {};

template<template<class ... T> class F, class ... Args>
struct count_if<sequence<Args...>, F > : variadic::count_if<F, Args...> {};

} // namespace tinympl

#endif // TINYMPL_COUNT_IF_HPP
