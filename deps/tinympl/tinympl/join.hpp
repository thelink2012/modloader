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

#ifndef TINYMPL_JOIN_HPP
#define TINYMPL_JOIN_HPP

#include <tinympl/sequence.hpp>
#include <tinympl/as_sequence.hpp>

namespace tinympl {

/**
 * \ingroup SeqAlgsIntr
 * \class join
 * \brief Merge two sequences
 * \param Args The sequences
 * \return A sequence constructed by joining all the passed sequences with the
type of the first one
 */
template<class ... Args> struct join;

template<class Head, class Next, class ... Tail>
struct join<Head, Next, Tail...> {
    typedef
        typename join < typename join<Head, Next>::type, Tail... >::type type;
};

template<class Head, class Last> struct join<Head, Last> {
private:
    template<class S1, class S2, template<class ...> class Out> struct do_join;

    template<class ... S1, class ... S2, template<class ...> class Out>
    struct do_join< sequence<S1...>, sequence<S2...>, Out > {
        typedef Out<S1..., S2...> type;
    };

public:
    typedef typename do_join <
    as_sequence_t<Head>,
                  as_sequence_t<Last>,
                  as_sequence<Head>::template rebind >::type type;
};

template<class Head> struct join<Head> {
    typedef Head type;
};

} // namespace tinympl

#endif // TINYMPL_JOIN_HPP
