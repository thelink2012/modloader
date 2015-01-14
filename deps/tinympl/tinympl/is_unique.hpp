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

#ifndef TINYMPL_IS_UNIQUE_HPP
#define TINYMPL_IS_UNIQUE_HPP

#include <tinympl/sequence.hpp>
#include <tinympl/as_sequence.hpp>
#include <tinympl/variadic/is_unique.hpp>

namespace tinympl {

/**
 * \ingroup SeqSet
 * \class is_unique
 * \brief Determines whether the input sequence contains only unique elements
 * \param Sequence the input sequence
 * \return `is_unique<...>::type` is a `std::integral_constant<bool,v>` where
`v` is true iff the input sequence contains no duplicates
 * \note Unlike `std::sort`, the input sequence is not required to be sorted,
but the compile time complexity is O(N^2)
 * \note The comparison is done with \ref tinympl::equal_to - it can be
specialized
 * \sa variadic::is_unique
 */
template<class Sequence> struct is_unique :
    is_unique<as_sequence_t<Sequence> > {};

template<class ... Args> struct is_unique<sequence<Args...> > :
    variadic::is_unique<Args...> {};

} // namespace tinympl

#endif // TINYMPL_IS_UNIQUE_HPP
