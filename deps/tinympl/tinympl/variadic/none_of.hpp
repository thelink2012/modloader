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

#ifndef TINYMPL_VARIADIC_NONE_OF_HPP
#define TINYMPL_VARIADIC_NONE_OF_HPP

#include <type_traits>

namespace tinympl {
namespace variadic {

/**
 * \ingroup VarNonModAlgs
 * \class none_of
 * \brief Determines whether none of the elements in the sequence satisfy the
given predicate
 * \param F the predicate, `F<T>::type::value` must be convertible to bool
 * \param Args... the input sequence
 * \return `none_of<...>::type` is a `std::integral_constant<bool,v>` where `v`
is true iff none of the elements in the sequence satisfy the predicate `F`
 * \sa tinympl::none_of
 */
template< template<class ... T> class F, class ... Args> struct none_of;

template< template<class ... T> class F, class Head, class ... Args>
struct none_of<F, Head, Args...> :
        std::conditional <
        F<Head>::type::value,
        std::integral_constant<bool, false>,
        typename none_of<F, Args...>::type >::type
{};

template< template<class ... T> class F> struct none_of<F> :
        std::integral_constant<bool, true>
{};


} // namespace variadic
} // namespace tinympl

#endif // TINYMPL_VARIADIC_NONE_OF_HPP
