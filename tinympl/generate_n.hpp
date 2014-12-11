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

#ifndef TINYMPL_GENERATE_N_HPP
#define TINYMPL_GENERATE_N_HPP

#include <tinympl/variadic/generate_n.hpp>

namespace tinympl {

/**
 * \ingroup SeqModAlgs
 * \class generate_n
 * \brief Generate N elements using a given generator metafunction
 * \param N The number of elements to generate
 * \param Gen The generator. `Gen< std::integral_constant<int,i> >::type` must
be a valid expression.
 * \param Out the output sequence type
 * \return `generate_n<...>::type` is a type templated from `Out` constructed
with N elements generated with `Gen< int_<0> >, Gen< int_<1> >, ... Gen<
int_<N-1> >`
 * \sa variadic::generate_n
 */
template<std::size_t N,
        template<class ...> class Gen,
        template<class ...> class Out>
struct generate_n : variadic::generate_n<N, Gen, Out> {};

} // namespace tinympl

#endif // TINYMPL_GENERATE_N_HPP
