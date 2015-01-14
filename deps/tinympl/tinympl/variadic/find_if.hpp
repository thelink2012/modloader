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

#ifndef TINYMPL_VARIADIC_FIND_IF_HPP
#define TINYMPL_VARIADIC_FIND_IF_HPP

#include <type_traits>
#include <cstddef>

namespace tinympl {
namespace variadic {

/**
 * \ingroup VarNonModAlgs
 * \class find_if Compute the index of a the first element in the sequence which
satisfies a given predicate
 * \param F The test predicate - `F<T>::type::value` shall be convertible to
bool
 * \param Args... the input sequence
 * \return `find_if<...>::type` is `std::integral_constant<std::size_t,v>` where
`v` is the 0-based index of the first element which satisfy `F`. If no such
element exists, `v` is `size<Sequence>::value`.
 * \sa tinympl::find_if
 */
template<template<class ... T> class F, class ... Args> struct find_if;

template<template<class ...T> class F, class Head, class ... Tail>
struct find_if<F, Head, Tail...> :
        std::conditional < F<Head>::type::value,
        std::integral_constant<std::size_t, 0>,
        std::integral_constant < std::size_t, 1 +
        find_if<F, Tail...>::type::value >
        >::type
{};

template<template<class ...T> class F> struct find_if<F> :
        std::integral_constant<std::size_t, 0>
{};

} // namespace variadic
} // namespace tinympl

#endif // TINYMPL_VARIADIC_FIND_IF_HPP
