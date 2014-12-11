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

#ifndef TINYMPL_SET_INTERSECTION_HPP
#define TINYMPL_SET_INTERSECTION_HPP

#include <tinympl/variadic/count.hpp>
#include <tinympl/variadic/copy_if.hpp>
#include <tinympl/as_sequence.hpp>
#include <tinympl/sequence.hpp>
#include <tinympl/bind.hpp>

namespace tinympl {

/**
 * \ingroup SeqSet
 * \class set_intersection
 * \brief Computes the intersection between two sets
 * \param SequenceA The sequence which represents the first set - duplicates are
ignored
 * \param SequenceB The sequence which represents the second set - duplicates
are ignored
 * \param Out the output sequence type - defaults to the same kind of
`SequenceA`
 * \return A type templated from `Out` which contains the resulting sequence
 * \note The ordering of the first sequence is preserved
 */
template<class SequenceA,
        class SequenceB,
        template<class ...> class Out = as_sequence<SequenceA>::template rebind>
struct set_intersection :
    set_intersection< as_sequence_t<SequenceA>, as_sequence_t<SequenceB>, Out>
{};

template<class ... Ts, class ... Us, template<class ...> class Out>
struct set_intersection<sequence<Ts...>, sequence<Us...>, Out> :
        variadic::copy_if<
            bind<variadic::count, arg1, Us...>::template eval_t,
            Out,
            Ts...> {};

} // namespace tinympl

#endif // TINYMPL_SET_INTERSECTION_HPP
