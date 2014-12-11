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

#ifndef TINYMPL_VARIADIC_ANY_OF_HPP
#define TINYMPL_VARIADIC_ANY_OF_HPP

#include <type_traits>

namespace tinympl {
namespace variadic {

/**
 * \ingroup VarNonModAlgs
 * \class any_of
 * \brief Determines whether any of the elements in the sequence satisfy the
given predicate
 * \param F the predicate, `F<T>::type::value` must be convertible to `bool`
 * \param Args... the input sequence
 * \return `any_of<...>::type` is a `std::integral_constant<bool,v>` where `v`
is true iff at least one element in the sequence satisfy the predicate `F`
 * \sa tinympl::any_of
 */
template< template<class ... T> class F, class ... Args> struct any_of;

template< template<class ... T> class F, class Head, class ... Args>
struct any_of<F, Head, Args...> :
        std::conditional <
        F<Head>::type::value,
        std::integral_constant<bool, true>,
        typename any_of<F, Args...>::type >::type
{};

template< template<class ... T> class F> struct any_of<F> :
        std::integral_constant<bool, false>
{};


} // namespace variadic
} // namespace tinympl

#endif // TINYMPL_VARIADIC_ANY_OF_HPP
