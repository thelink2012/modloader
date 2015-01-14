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

#ifndef TINYMPL_REPLACE_HPP
#define TINYMPL_REPLACE_HPP

#include <tinympl/variadic/replace.hpp>
#include <tinympl/as_sequence.hpp>
#include <tinympl/sequence.hpp>

namespace tinympl {

/**
 * \ingroup SeqModAlgs
 * \class replace
 * \brief Replace all the elements in the input sequence equal to *Old* with
*New*
 * \param SequenceIn The input sequence
 * \param Old The type to be replaced
 * \param New The new type
 * \param Out The type of the output sequence, defaults to the same kind of the
input sequence
 * \return `replace<...>::type` is a type templated from `Out`
 * \note The comparison is done with \ref tinympl::equal_to - it can be
specialized
 * \sa variadic::replace
 */
template<class SequenceIn,
        class Old,
        class New,
        template<class ...> class Out =
            as_sequence<SequenceIn>::template rebind>
struct replace : replace<as_sequence_t<SequenceIn>, Old, New, Out> {};

template<class Old, class New, template<class ...> class Out, class ... Args>
struct replace<sequence<Args...>, Old, New, Out> :
    variadic::replace<Old, New, Out, Args...> {};

} // namespace tinympl

#endif // TINYMPL_REPLACE_HPP
