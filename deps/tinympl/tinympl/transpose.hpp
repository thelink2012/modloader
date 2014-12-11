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

#ifndef TINYMPL_TRANSPOSE_HPP
#define TINYMPL_TRANSPOSE_HPP

#include <tinympl/variadic/all_of.hpp>
#include <tinympl/variadic/at.hpp>
#include <tinympl/as_sequence.hpp>
#include <tinympl/is_sequence.hpp>
#include <tinympl/sequence.hpp>
#include <tinympl/variadic/all_of.hpp>
#include <tinympl/size.hpp>
#include <tinympl/bind.hpp>
#include <tinympl/equal_to.hpp>
#include <tinympl/int.hpp>
#include <tinympl/at.hpp>

namespace tinympl {

/**
 * \ingroup SeqModAlgs
 * \class transpose
 * \brief Transpose a sequence of sequences
 * \param SequenceOfSequences The sequence of sequences which shall be
transposed
 * \param OutOuter The type of the outer output sequence
 * \param OutInner The type of the inner output sequence
 */
template < class SequenceOfSequences,
           template<class ...> class OutOuter =
                as_sequence<SequenceOfSequences>::template rebind,
           template<class ...> class OutInner =
                as_sequence<typename at<0,SequenceOfSequences>::type>::
                    template rebind >
struct transpose :
    transpose< as_sequence_t<SequenceOfSequences>, OutOuter, OutInner > {};

template< class ... Sequences,
          template<class ...> class OutOuter,
          template<class ...> class OutInner>
class transpose< sequence<Sequences...>, OutOuter, OutInner> {
    static_assert( variadic::all_of<is_sequence, Sequences...>::value,
        "transpose: not all the elements of the main sequence are sequences" );

    static_assert( sizeof...( Sequences ) > 0,
        "transpose is undefined on empty sequences" );

    enum {size = tinympl::size<
            typename variadic::at<0,Sequences...>::type>::value};

    static_assert( variadic::all_of <
                   bind < equal_to, int_<size>,
                   bind<tinympl::size, arg1> >::
                       template eval, Sequences...>::value,
                   "transpose: all the sequences must have the same size" );

    template<std::size_t i, class ... Bound>
    struct impl {
        typedef OutInner< typename at<i-1, Sequences>::type ... > cur_t;
        typedef typename impl < i - 1, cur_t, Bound...>::type type;
    };

    template<class ... Bound>
    struct impl<0, Bound...> {
        typedef OutOuter<Bound...> type;
    };

public:
    typedef typename impl<size>::type type;
};

} // namespace tinympl

#endif // TINYMPL_TRANSPOSE_HPP
