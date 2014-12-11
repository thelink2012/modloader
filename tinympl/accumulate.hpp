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

#ifndef TINYMPL_ACCUMULATE_HPP
#define TINYMPL_ACCUMULATE_HPP

#include <tinympl/left_fold.hpp>

namespace tinympl {

/**
 * \ingroup SeqFold
 * \class accumulate
 * \brief Alias for left_fold.
 * \see left_fold
 * \sa variadic::accumulate
 */
template<class Seq, template<class ... > class F>
struct accumulate : left_fold<Seq, F> {};

} // namespace tinympl

#endif // TINYMPL_ACCUMULATE_HPP
