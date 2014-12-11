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

#ifndef TINYMPL_VARIADIC_ACCUMULATE_HPP
#define TINYMPL_VARIADIC_ACCUMULATE_HPP

#include <tinympl/variadic/left_fold.hpp>

namespace tinympl {
namespace variadic {

/**
 * \ingroup VarFold
 * \brief Alias for left_fold.
 * \see left_fold
 * \sa tinympl::accumulate
 */
template<template<class ...> class Op, class ... Args>
using accumulate = left_fold<Op, Args...>;

} // namespace variadic
} // namespace tinympl

#endif // TINYMPL_VARIADIC_ACCUMULATE_HPP
