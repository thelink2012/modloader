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

#ifndef TINYMPL_VARIADIC_RIGHT_FOLD_HPP
#define TINYMPL_VARIADIC_RIGHT_FOLD_HPP

namespace tinympl {
namespace variadic {

/**
 * \ingroup VarFold
 * \class right_fold
 * \brief Collapses a sequence starting from right using a functor.
 * \param F The functor; `F<T,U>` must be a valid expression
 * \param Args... The input sequence
 * \return A type which is the result of `F(A1, F(A2,F(A3, ... ) ) )`
 * \sa tinympl::right_fold
 */
template<template<class ...> class Op, class ... Args> struct right_fold;

template<template<class ...> class Op, class Head, class ... Tail>
struct right_fold<Op, Head, Tail...> {
    typedef typename Op < Head,
            typename right_fold<Op, Tail...>::type >::type type;
};

template<template<class ...> class Op, typename T> struct right_fold<Op, T> {
    typedef T type;
};

} // namespace variadic
} // namespace tinympl

#endif // TINYMPL_VARIADIC_RIGHT_FOLD_HPP
