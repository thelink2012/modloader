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

#ifndef TINYMPL_REMOVE_HPP
#define TINYMPL_REMOVE_HPP

#include <tinympl/variadic/remove.hpp>
#include <tinympl/as_sequence.hpp>
#include <tinympl/sequence.hpp>

namespace tinympl {

/**
 * \ingroup SeqModAlgs
 * \class remove
 * \brief Remove all the elements equal to T from the input sequence
 * \param Sequence The input sequence
 * \param T The element to be removed
 * \param Out The type of the output sequence - defaults to the same kind of the
input sequence
 * \return `remove<...>::type` is a type templated from `Out` which contains the
new sequence
 * \note The comparison is done with \ref tinympl::equal_to - it can be
specialized
 * \sa variadic::remove
 */
template< class Sequence,
        typename T,
        template<class ... > class Out = as_sequence<Sequence>::template rebind>
struct remove : remove<as_sequence_t<Sequence>, T, Out> {};

template< typename T, template<class ... > class Out, class ... Args>
struct remove<sequence<Args...>, T, Out> : variadic::remove<T, Out, Args...> {};

} // namespace tinympl

#endif // TINYMPL_REMOVE_HPP
