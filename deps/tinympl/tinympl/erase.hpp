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

#ifndef TINYMPL_ERASE_HPP
#define TINYMPL_ERASE_HPP

#include <tinympl/variadic/erase.hpp>
#include <tinympl/as_sequence.hpp>
#include <tinympl/sequence.hpp>

namespace tinympl {

/**
 * \ingroup SeqAlgsIntr
 * \class erase
 * \brief Remove a range in a given sequence
 * \param First The first element to be removed
 * \param Last The first element which is not removed
 * \param Seq The input sequence
 * \param Out The output sequence type
 */
template<std::size_t First,
        std::size_t Last,
        class Seq,
        template<class...> class Out = as_sequence<Seq>::template rebind>
struct erase : erase<First, Last, as_sequence_t<Seq>, Out> {};

template<std::size_t First,
        std::size_t Last,
        class ... Args,
        template<class...> class Out>
struct erase<First, Last, sequence<Args...>, Out> :
    variadic::erase<First, Last, Out, Args...> {};

} // namespace tinympl

#endif // TINYMPL_ERASE_HPP
