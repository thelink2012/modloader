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

#ifndef TINYMPL_FILL_N_HPP
#define TINYMPL_FILL_N_HPP

#include <tinympl/variadic/fill_n.hpp>

namespace tinympl {

/**
 * \ingroup SeqModAlgs
 * \class fill_n
 * \brief Fills an output sequence with N equal elements
 * \param N The number of elements
 * \param T The type of the elements
 * \param Out The output sequence type
 * \return `fill_n<...>::type` is a type templated from `Out` constructed with N
types equal to `T`
 * \sa variadic::fill_n
 */
template<std::size_t N, class T, template<class ...> class Out>
struct fill_n : variadic::fill_n<N, T, Out> {};

} // namespace tinympl

#endif // TINYMPL_FILL_N_HPP
