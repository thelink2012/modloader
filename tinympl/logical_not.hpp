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

#ifndef TINYMPL_LOGICAL_NOT_HPP
#define TINYMPL_LOGICAL_NOT_HPP

#include <tinympl/not_b.hpp>

namespace tinympl {

/**
 * \ingroup Logical
 * \class logical_not
 * \brief Negate the argument
 */
template<class T> struct logical_not : not_b<T::value> {};
template<class T> using not_ = logical_not<T>;

} // namespace tinympl

#endif // TINYMPL_LOGICAL_NOT_HPP
