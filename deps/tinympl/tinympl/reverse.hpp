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

#ifndef TINYMPL_REVERSE_HPP
#define TINYMPL_REVERSE_HPP

#include <tinympl/variadic/reverse.hpp>
#include <tinympl/as_sequence.hpp>
#include <tinympl/sequence.hpp>

namespace tinympl {

/**
 * \ingroup SeqModAlgs
 * \class reverse
 * \brief Reverse the input sequence
 * \param Sequence the input sequence
 * \param Out the output sequence type, defaults to the same kind of the input
sequence type
 * \return `reverse<...>::type` is a type templated from `Out` which contains
the reversed sequence
 * \sa variadic::reverse
 */
template<class Sequence,
        template<class ...> class Out = as_sequence<Sequence>::template rebind>
struct reverse : reverse< as_sequence_t<Sequence>, Out> {};

template<template<class ...> class Out, class ... Args>
struct reverse<sequence<Args...>, Out> : variadic::reverse<Out, Args...> {};

} // namespace tinympl

#endif // TINYMPL_REVERSE_HPP
