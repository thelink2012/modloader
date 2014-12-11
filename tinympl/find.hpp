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

#ifndef TINYMPL_FIND_HPP
#define TINYMPL_FIND_HPP

#include <tinympl/variadic/find.hpp>
#include <tinympl/as_sequence.hpp>
#include <tinympl/sequence.hpp>

namespace tinympl {

/**
 * \ingroup SeqNonModAlgs
 * \class find
 * \brief Compute the index of the first element in the sequence which is equal
to the given type T
 * \param Sequence The input sequence
 * \param T The type to be tested
 * \return `find<...>::type` is `std::integral_constant<std::size_t,v>` where
`v` is the 0-based index of the first element which is equal to `T`. If no such
element exists, `v` is `size<Sequence>::value`.
 * \note The comparison is done with \ref tinympl::equal_to - it can be
specialized
 * \sa variadic::find
 */
template<class Sequence, typename T>
struct find : find<as_sequence_t<Sequence>, T > {};

template<typename T, class ... Args>
struct find<sequence<Args...>, T > : variadic::find<T, Args...> {};

} // namespace tinympl

#endif // TINYMPL_FIND_HPP
