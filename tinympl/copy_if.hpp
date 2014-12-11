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

#ifndef TINYMPL_COPY_IF_HPP
#define TINYMPL_COPY_IF_HPP

#include <tinympl/variadic/copy_if.hpp>
#include <tinympl/as_sequence.hpp>
#include <tinympl/sequence.hpp>

namespace tinympl {

/**
 * \ingroup SeqModAlgs
 * \class copy_if
 * \brief Copy the elements of a given input sequence which satisfy a given
predicate - the ordering is preserved.
 * \param SequenceIn The input sequence
 * \param F The test predicate - F<T>::type::value shall be convertible to bool
 * \param Out The output sequence type - defaults to the same sequence kind of
the input sequence
 * \return `copy_if<...>::type` is a type templated from `Out` which is
constructed with the elements of SequenceIn which satisfy the predicate `F`.
 * \sa variadic::copy_if
 */
template<class SequenceIn, template<class ... T> class F, template<class ...>
class Out = as_sequence<SequenceIn>::template rebind>
struct copy_if : copy_if<as_sequence_t<SequenceIn>, F, Out> {};

template<template<class ... T> class F, template<class ...> class Out, class ...
Args>
struct copy_if<sequence<Args...>, F, Out>  : variadic::copy_if<F, Out, Args...>
{};

} // namespace tinympl

#endif // TINYMPL_COPY_IF_HPP
