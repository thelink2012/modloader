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

#ifndef TINYMPL_VARIADIC_FIND_HPP
#define TINYMPL_VARIADIC_FIND_HPP

#include <tinympl/variadic/find_if.hpp>
#include <tinympl/bind.hpp>
#include <tinympl/equal_to.hpp>

namespace tinympl {
namespace variadic {

/**
 * \ingroup VarNonModAlgs
 * \class find
 * \brief Compute the index of the first element in the sequence which is equal
to the given type T
 * \param T The type to be tested
 * \param Args... the input sequence
 * \return `find<...>::type` is `std::integral_constant<std::size_t,v>` where
`v` is the 0-based index of the first element which is equal to `T`. If no such
element exists, `v` is `size<Sequence>::value`.
 * \note The comparison is done with \ref tinympl::equal_to - it can be
specialized
 * \sa tinympl::find
 */
template<typename T, class ... Args>
struct find : find_if< bind<equal_to, arg1, T>::template eval_t, Args ... > {};

} // namespace variadic
} // namespace tinympl

#endif // TINYMPL_VARIADIC_FIND_HPP
