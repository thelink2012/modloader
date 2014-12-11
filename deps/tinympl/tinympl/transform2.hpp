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

#ifndef TINYMPL_TRANSFORM2_HPP
#define TINYMPL_TRANSFORM2_HPP

#include <tinympl/as_sequence.hpp>
#include <tinympl/transform_many.hpp>

namespace tinympl {

/**
 * \ingroup SeqModAlgs
 * \class transform2
 * \brief Transform two input sequences using a function
 * \param Sequence1 the first input sequence
 * \param Sequence2 the second input sequence
 * \param F The transform function. `F<T,U>::type` must be a valid expression
 * \param Out The output sequence type, defaults to the same kind of the input
sequence
 * \return `transform2<...>::type` is a type templated from `Out` which contains
the transformed types
 */
template < class Sequence1,
            class Sequence2,
            template<class ...> class F,
            template<class ...> class Out =
                as_sequence<Sequence1>::template rebind >
struct transform2 : transform_many<F, Out, Sequence1, Sequence2> {};

} // namespace tinympl

#endif // TINYMPL_TRANSFORM2_HPP
