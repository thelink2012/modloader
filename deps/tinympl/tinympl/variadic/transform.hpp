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

#ifndef TINYMPL_VARIADIC_TRANSFORM_HPP
#define TINYMPL_VARIADIC_TRANSFORM_HPP

namespace tinympl {
namespace variadic {

/**
 * \ingroup VarModAlgs
 * \class transform
 * \brief Transform an input sequence using a transform function
 * \param F The transform function. `F<T>::type` must be a valid expression
 * \param Out The output sequence type, defaults to the same kind of the input
sequence
 * \param Args... the input sequence
 * \return `transform<...>::type` is a type templated from `Out` which contains
the transformed types
 * \sa tinympl::transform
 */
template<template<class ... T> class F,
        template<class ... > class Out,
        class ... Args> struct transform;

template< template<class ... T> class F,
        template<class ... > class Out,
        class ... Args>
struct transform {
    typedef Out< typename F<Args>::type ... > type;
};

} // namespace variadic
} // namespace tinympl

#endif // TINYMPL_VARIADIC_TRANSFORM_HPP
