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

#ifndef TINYMPL_VARIADIC_MAX_ELEMENT_HPP
#define TINYMPL_VARIADIC_MAX_ELEMENT_HPP

#include <tinympl/variadic/min_element.hpp>
#include <tinympl/bind.hpp>

namespace tinympl {
namespace variadic {

/**
 * \ingroup VarMaxMin
 * \class max_element
 * \brief Compute the index of the largest element in a sequence
 * \param Cmp the comparator function; `Cmp<A,B>::type::value` must be
convertible to bool. Defaults to \ref tinympl::less
 * \param Args... the input sequence
 * \return `max_element<...>::type` is an
`std::integral_constant<std::size_t,v>` where `v` is the 0-based index of the
maximum element
 * \sa tinympl::max_element
 */
template<template<class ... > class Cmp, class ... Args>
struct max_element :
    min_element< bind<Cmp, arg2, arg1 >::template eval_t, Args... > {};

} // namespace variadic
} // namespace tinympl

#endif // TINYMPL_VARIADIC_MAX_ELEMENT_HPP
