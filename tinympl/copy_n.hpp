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

#ifndef TINYMPL_COPY_N_HPP
#define TINYMPL_COPY_N_HPP

#include <tinympl/variadic/copy_n.hpp>
#include <tinympl/as_sequence.hpp>
#include <tinympl/sequence.hpp>

namespace tinympl {

/**
 * \ingroup SeqModAlgs
 * \class copy_n
 * \brief Copy the first N elements from the input sequence.
 * \param SequenceIn the input sequence
 * \param N The number of elements to be copied
 * \param Out The output sequence type, defaults to the same kind of the input
sequence
 * \return `copy_n<...>::type` is a type templated from `Out` which is
constructed with the first N types of the input sequence
 * \sa variadic::copy_n
 */
template< typename SequenceIn,
          std::size_t N,
      template<class ... > class Out = as_sequence<SequenceIn>::template rebind>
struct copy_n : copy_n<as_sequence_t<SequenceIn>, N, Out> {};

template<std::size_t N, template<class ...> class Out, class ... Args>
struct copy_n<sequence<Args...>, N, Out> : variadic::copy_n<N, Out, Args...> {};

} // namespace tinympl

#endif // TINYMPL_COPY_N_HPP
