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

#ifndef TINYMPL_TRANSFORM_HPP
#define TINYMPL_TRANSFORM_HPP

#include <tinympl/variadic/transform.hpp>
#include <tinympl/as_sequence.hpp>
#include <tinympl/sequence.hpp>

namespace tinympl {

/**
 * \ingroup SeqModAlgs
 * \class transform
 * \brief Transform an input sequence using a transform function
 * \param Sequence the input sequence
 * \param F The transform function. `F<T>::type` must be a valid expression
 * \param Out The output sequence type, defaults to the same kind of the input
sequence
 * \return `transform<...>::type` is a type templated from `Out` which contains
the transformed types
 * \sa variadic::transform
 */
template<class Sequence,
        template<class ... T> class F, template<class ... > class Out =
            as_sequence<Sequence>::template rebind>
struct transform : transform<as_sequence_t<Sequence>, F, Out> {};

template<template<class ... T> class F,
        template<class ... > class Out,
        class ... Args>
struct transform<sequence<Args...>, F, Out> :
    variadic::transform<F, Out, Args...> {};

} // namespace tinympl

#endif // TINYMPL_TRANSFORM_HPP
