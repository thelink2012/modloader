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

#ifndef TINYMPL_VARIADIC_COUNT_IF_HPP
#define TINYMPL_VARIADIC_COUNT_IF_HPP

#include <type_traits>
#include <cstddef>

namespace tinympl {
namespace variadic {

/**
 * \ingroup VarNonModAlgs
 * \class count_if
 * \brief Counts the number of elements which satisfy a given predicate
 * \param F The predicate - `F<T>::type::value` shall be convertible to bool
 * \param Args... the input sequence
 * \return `count_if<...>::type` is `std::integral_constant<std::size_t,V>`
where `V` is the number of elements in the sequence which satisfy the predicate
`F`.
 * \sa tinympl::count_if
 */
template<template<class ... T> class F, class ... Args> struct count_if;

template<template<class ... T> class F, class Head, class ... Tail>
struct count_if<F, Head, Tail...> :
        std::integral_constant < std::size_t,
        count_if<F, Tail...>::type::value +
( F<Head>::type::value ? 1 : 0 ) >
{};

template<template<class ... T> class F> struct count_if<F> :
        std::integral_constant<std::size_t, 0>
{};

} // namespace variadic
} // namespace tinympl

#endif // TINYMPL_VARIADIC_COUNT_IF_HPP
