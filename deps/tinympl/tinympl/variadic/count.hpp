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

#ifndef TINYMPL_VARIADIC_COUNT_HPP
#define TINYMPL_VARIADIC_COUNT_HPP

#include <tinympl/variadic/count_if.hpp>
#include <tinympl/bind.hpp>
#include <tinympl/equal_to.hpp>

namespace tinympl {
namespace variadic {

/**
 * \ingroup VarNonModAlgs
 * \class count
 * \brief Counts the number of elements in a sequence equal to a given one
 * \param T the type to be tested.
 * \param Args... the input sequence
 * \return `count<...>::type` is `std::integral_constant<std::size_t,V>` where
`V` is the number of elements in the sequence equal to `T`
 * \note The comparison is done with \ref tinympl::equal_to - it can be
specialized.
 * \sa tinympl::count
 */
template<typename T, class ... Args>
struct count : count_if< bind<equal_to, arg1, T>::template eval_t, Args ... >
{};


} // namespace variadic
} // namespace tinympl

#endif // TINYMPL_VARIADIC_COUNT_HPP
