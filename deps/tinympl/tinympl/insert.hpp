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

#ifndef TINYMPL_INSERT_HPP
#define TINYMPL_INSERT_HPP

#include <tinympl/variadic/erase.hpp>
#include <tinympl/as_sequence.hpp>
#include <tinympl/sequence.hpp>
#include <tinympl/join.hpp>

namespace tinympl {

/**
 * \ingroup SeqAlgsIntr
 * \class insert
 * \brief Insert a subsequence into a given sequence at a given position
 * \param Pos The insertion position in the main sequence
 * \param SubSeq The subsequence
 * \param Seq The main sequence
 * \param Out The output sequence type
 */
template < std::size_t Pos,
        class SubSeq,
        class Seq,
        template<class ...> class Out = as_sequence<Seq>::template rebind>
struct insert : insert<Pos, as_sequence_t<SubSeq>, as_sequence_t<Seq>, Out> {};

template< std::size_t Pos, class ... SubSeqArgs, class ... SeqArgs,
template<class...> class Out>
class insert<Pos, sequence<SubSeqArgs...>, sequence<SeqArgs...>, Out> {
    template<class ... Us>
    using head_seq = sequence<Us ..., SubSeqArgs ... >;

    typedef typename variadic::erase<Pos, sizeof ...( SeqArgs ), head_seq,
SeqArgs ... >::type head;
    typedef typename variadic::erase<0, Pos, sequence, SeqArgs ... >::type tail;

public:
    typedef typename join<Out<>, head, tail>::type type;
};

} // namespace tinympl

#endif // TINYMPL_INSERT_HPP
