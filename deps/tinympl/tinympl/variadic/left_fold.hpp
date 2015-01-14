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

#ifndef TINYMPL_VARIADIC_LEFT_FOLD_HPP
#define TINYMPL_VARIADIC_LEFT_FOLD_HPP

namespace tinympl {
namespace variadic {

/**
 * \ingroup VarFold
 * \class left_fold
 * \brief Collapses a sequence starting from left using a functor.
 * \param F The functor; `F<T,U>` must be a valid expression
 * \param Args... The input sequence
 * \return A type which is the result of `F( ... F(F(A1,A2),A3) .. )`
 * \sa tinympl::left_fold
 */
template<template<class ...> class Op, class ... Args> struct left_fold;

template<template<class ...> class Op, class Head, class Next, class ... Tail>
struct left_fold<Op, Head, Next, Tail...> {
    typedef typename left_fold <
        Op,
        typename Op<Head, Next>::type,
        Tail... >::type type;
};

template<template<class ...> class Op, typename T>
struct left_fold<Op, T> {
    typedef T type;
};

} // namespace variadic
} // namespace tinympl

#endif // TINYMPL_VARIADIC_LEFT_FOLD_HPP
