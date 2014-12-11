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

#ifndef TINYMPL_VARIADIC_IS_UNIQUE_HPP
#define TINYMPL_VARIADIC_IS_UNIQUE_HPP

#include <tinympl/variadic/find.hpp>
#include <type_traits>

namespace tinympl {
namespace variadic {

/**
 * \ingroup VarSet
 * \class is_unique
 * \brief Determines whether the input sequence contains only unique elements
 * \param Args... the input sequence
 * \return `is_unique<...>::type` is a `std::integral_constant<bool,v>` where
`v` is true iff the input sequence contains no duplicates
 * \note Unlike `std::sort`, the input sequence is not required to be sorted,
but the compile time complexity is O(N^2)
 * \note The comparison is done with \ref tinympl::equal_to - it can be
specialized
 * \sa tinympl::is_unique
 */
template<class ... Args> struct is_unique;

template<class Head, class ... Tail>
struct is_unique<Head, Tail...> : std::conditional <
    find<Head, Tail...>::type::value == sizeof ...( Tail ),
    typename is_unique<Tail...>::type,
    std::integral_constant<bool, false> >::type
{};

template<> struct is_unique<> :
        std::integral_constant<bool, true>
{};


} // namespace variadic
} // namespace tinympl

#endif // TINYMPL_VARIADIC_IS_UNIQUE_HPP
