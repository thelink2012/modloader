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

#ifndef TINYMPL_COUNT_HPP
#define TINYMPL_COUNT_HPP

#include <tinympl/as_sequence.hpp>
#include <tinympl/sequence.hpp>
#include <tinympl/variadic/count.hpp>

namespace tinympl {

/**
 * \ingroup SeqNonModAlgs
 * \class count
 * \brief Counts the number of elements in a sequence equal to a given one
 * \param Sequence The input sequence
 * \param T the type to be tested.
 * \return `count<...>::type` is `std::integral_constant<std::size_t,V>` where
`V` is the number of elements in the sequence equal to `T`
 * \note The comparison is done with \ref tinympl::equal_to - it can be
specialized.
 * \sa variadic::count
 */
template<class Sequence, typename T>
struct count : count<as_sequence_t<Sequence>, T > {};

template<typename T, class ... Args>
struct count<sequence<Args...>, T > : variadic::count<T, Args...> {};

} // namespace tinympl

#endif // TINYMPL_COUNT_HPP
