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

#ifndef TINYMPL_REMOVE_IF_HPP
#define TINYMPL_REMOVE_IF_HPP

#include <tinympl/variadic/remove_if.hpp>
#include <tinympl/as_sequence.hpp>
#include <tinympl/sequence.hpp>

namespace tinympl {

/**
 * \ingroup SeqModAlgs
 * \class remove_if
 * \brief Remove the elements from the input sequence if they satisfy a given
predicate
 * \param Sequence The input sequence
 * \param F The predicate, `F<T>::type::value` must be convertible to bool
 * \param Out The output sequence type, defaults to the same kind of the input
sequence
 * \return `remove_if<...>::type` is a type templated from `Out` which contains
the new sequence
 * \sa variadic::remove_if
 */
template<class Sequence,
        template<class ... T> class F,
        template<class ...> class Out = as_sequence<Sequence>::template rebind>
struct remove_if : remove_if<as_sequence_t<Sequence>, F, Out> {};

template<template<class ... T> class F,
        template<class ...> class Out,
        class ... Args>
struct remove_if<sequence<Args...>, F, Out> :
    variadic::remove_if<F, Out, Args...> {};

} // namespace tinympl

#endif // TINYMPL_REMOVE_IF_HPP
