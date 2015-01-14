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

#ifndef TINYMPL_LOGICAL_OR_HPP
#define TINYMPL_LOGICAL_OR_HPP

#include <tinympl/or_b.hpp>

namespace tinympl {

/**
 * \ingroup Logical
 * \class logical_or
 * \brief Computes the logical or of all its arguments
 */
template<class ... Args> struct logical_or : or_b<Args::value ...> {};
template<class ... Args> using or_ = logical_or<Args...>;

} // namespace tinympl

#endif // TINYMPL_LOGICAL_OR_HPP
