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

#ifndef TINYMPL_VARIADIC_MIN_ELEMENT_HPP
#define TINYMPL_VARIADIC_MIN_ELEMENT_HPP

#include <tinympl/variadic/at.hpp>
#include <type_traits>
#include <cstddef>

namespace tinympl {
namespace variadic {

/**
 * \ingroup VarMaxMin
 * \class min_element
 * \brief Compute the index of the smallest element in a sequence
 * \param Cmp the comparator function; `Cmp<A,B>::type::value` must be
convertible to bool. Defaults to \ref tinympl::less
 * \param Args... the input sequence
 * \return `min_element<...>::type` is an
`std::integral_constant<std::size_t,v>` where `v` is the 0-based index of the
minimum element
 * \sa tinympl::min_element
 */
template<template<class ... > class Cmp, class ... Args> struct min_element;

namespace detail {

template<template<class ...> class Comp, class ... > struct min_element_impl;
template<template<class ...> class Comp, class Head, class ... Tail> struct
min_element_impl<Comp, Head, Tail...> {
private:
    enum {
        next_min = min_element_impl<Comp, Tail...>::type::value
    };

    enum {
        this_min = ! Comp<at_t<next_min, Tail...>, Head>::type::value
    };

public:
    typedef std::integral_constant < std::size_t,
            ( this_min ?
              0 :
              next_min + 1 ) > type;
};

template<template<class ... > class Comp, class Head> struct
min_element_impl<Comp, Head> {
    typedef std::integral_constant<std::size_t, 0> type;
};

} // namespace detail

template<template<class ...> class Comp, class ... Args>
struct min_element :
        detail::min_element_impl<Comp, Args...>::type
{};

} // namespace variadic
} // namespace tinympl

#endif // TINYMPL_VARIADIC_MIN_ELEMENT_HPP
