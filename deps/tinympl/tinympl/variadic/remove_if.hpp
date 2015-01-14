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

#ifndef TINYMPL_VARIADIC_REMOVE_IF_HPP
#define TINYMPL_VARIADIC_REMOVE_IF_HPP

#include <tinympl/variadic/copy_if.hpp>
#include <tinympl/bind.hpp>
#include <tinympl/logical_not.hpp>

namespace tinympl {
namespace variadic {

/**
 * \ingroup VarModAlgs
 * \class remove_if
 * \brief Remove the elements from the input sequence if they satisfy a given
predicate
 * \param F The predicate, `F<T>::type::value` must be convertible to bool
 * \param Out The output sequence type, defaults to the same kind of the input
sequence
 * \param Args... The input sequence
 * \return `remove_if<...>::type` is a type templated from `Out` which contains
the new sequence
 * \sa tinympl::remove
 */
template<template<class ... T> class F,
        template<class ...> class Out,
        class ... Args>
struct remove_if :
    copy_if< bind<logical_not, bind<F, arg1> >::template eval, Out, Args...> {};

} // namespace variadic
} // namespace tinympl

#endif // TINYMPL_VARIADIC_REMOVE_IF_HPP
