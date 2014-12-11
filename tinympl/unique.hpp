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

#ifndef TINYMPL_UNIQUE_HPP
#define TINYMPL_UNIQUE_HPP

#include <tinympl/variadic/unique.hpp>
#include <tinympl/as_sequence.hpp>
#include <tinympl/sequence.hpp>

namespace tinympl {

/**
 * \ingroup SeqModAlgs
 * \class unique
 * \brief Produces a sequence of unique elements from the input sequence,
preserving the ordering.
 * \param Sequence The input sequence.
 * \param Out the output sequence type - defaults to the same kind of the input
sequence.
 * \return `unique<...>::type` is a type templated from `Out` which contains the
resulting sequence.
 * \note Only the first (leftmost) duplicate is mantained in the output
sequence.
 * \sa variadic::unique
 */
template<class Sequence,
        template<class ...> class Out = as_sequence<Sequence>::template rebind>
struct unique : unique<as_sequence_t<Sequence>, Out> {};

template<template<class ...> class Out, class ... Args>
struct unique< sequence<Args...>, Out> : variadic::unique<Out, Args...> {};

} // namespace tinympl

#endif // TINYMPL_UNIQUE_HPP
