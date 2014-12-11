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

#ifndef TINYMPL_VARIADIC_SIZE_HPP
#define TINYMPL_VARIADIC_SIZE_HPP

#include <type_traits>
#include <cstddef>

namespace tinympl {
namespace variadic {

/**
 * \ingroup VariadicBasic
 * \class size
 * \brief Compute the size of a variadic template
 * \return `size<Args...>::value` is equivalent to `sizeof ... (Args)`
 */
template<class ... Args> struct size;

template<class ... Args> using size_t = typename size<Args...>::type;

template<class ... Args> struct size : std::integral_constant<std::size_t,sizeof...(Args)> {};

} // namespace variadic
} // namespace tinympl

#endif // TINYMPL_VARIADIC_SIZE_HPP
