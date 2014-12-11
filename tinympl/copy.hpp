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

#ifndef TINYMPL_COPY_HPP
#define TINYMPL_COPY_HPP

#include <tinympl/variadic/copy.hpp>
#include <tinympl/as_sequence.hpp>
#include <tinympl/sequence.hpp>

namespace tinympl {

/**
 * \ingroup SeqModAlgs
 * \class copy
 * \brief Copy the elements from the input sequence to the output sequence
 * \param SequenceIn The input sequence
 * \param Out The output sequence type - defaults to the same sequence kind of
the input sequence
 * \return `copy<...>::type` is a type templated from `Out` which is constructed
with the elements of SequenceIn.
 * \sa variadic::copy
 */
template< class SequenceIn,
    template<class ...> class Out = as_sequence<SequenceIn>::template rebind>
struct copy : copy<as_sequence_t<SequenceIn>, Out> {};

template<template<class ...> class Out, class ... Args>
struct copy<sequence<Args...>, Out>  : variadic::copy<Out, Args...> {};

} // namespace tinympl

#endif // TINYMPL_COPY_HPP
