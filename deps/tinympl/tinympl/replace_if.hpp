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

#ifndef TINYMPL_REPLACE_IF_HPP
#define TINYMPL_REPLACE_IF_HPP

#include <tinympl/variadic/replace_if.hpp>
#include <tinympl/as_sequence.hpp>
#include <tinympl/sequence.hpp>

namespace tinympl {

/**
 * \ingroup SeqModAlgs
 * \class replace_if
 * \brief Replace all the elements in the input sequence which satisfy a given
predicate with a given type T
 * \param SequenceIn The input sequence
 * \param F The predicate, `F<T>::type::value` must be convertible to bool
 * \param T The type used to replace the types
 * \param Out The type of the output sequence, defaults to the same kind of the
input sequence
 * \return `replace_if<...>::type` is a type templated from `Out`
 * \sa variadic::replace_if
 */
template<class SequenceIn,
        template<class ... T> class F,
        class T,
        template<class ...> class Out =
            as_sequence<SequenceIn>::template rebind>
struct replace_if : replace_if<as_sequence_t<SequenceIn>, F, T, Out> {};

template<template<class ... T> class F,
        class T,
        template<class ...> class Out,
        class ... Args>
struct replace_if<sequence<Args...>, F, T, Out> :
    variadic::replace_if<F, T, Out, Args...> {};

} // namespace tinympl

#endif // TINYMPL_REPLACE_IF_HPP
