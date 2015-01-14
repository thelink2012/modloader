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

#ifndef TINYMPL_VARIADIC_REMOVE_HPP
#define TINYMPL_VARIADIC_REMOVE_HPP

#include <tinympl/variadic/remove_if.hpp>
#include <tinympl/bind.hpp>
#include <tinympl/equal_to.hpp>

namespace tinympl {
namespace variadic {

/**
 * \ingroup VarModAlgs
 * \class remove
 * \brief Remove all the elements equal to T from the input sequence
 * \param T The element to be removed
 * \param Out The type of the output sequence - defaults to the same kind of the
input sequence
 * \param Args... The input sequence
 * \return `remove<...>::type` is a type templated from `Out` which contains the
new sequence
 * \note The comparison is done with \ref tinympl::equal_to - it can be
specialized
 * \sa tinympl::remove
 */
template< typename T, template<class ... > class Out, class ... Args>
struct remove :
    remove_if< bind<equal_to, arg1, T>::template eval_t, Out, Args...> {};



} // namespace variadic
} // namespace tinympl

#endif // TINYMPL_VARIADIC_REMOVE_HPP
