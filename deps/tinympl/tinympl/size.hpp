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

#ifndef TINYMPL_SIZE_HPP
#define TINYMPL_SIZE_HPP

#include <tinympl/variadic/size.hpp>
#include <tinympl/as_sequence.hpp>

namespace tinympl {

/**
 * \ingroup SeqAlgsIntr
 * \class size
 * \brief Get the number of elements of a sequence
 * \param Seq the sequence
 */
template<class Seq> struct size : size< as_sequence_t<Seq> > {};
template<class ... Args>
struct size<sequence<Args...> > : variadic::size<Args...> {};

} // namespace tinympl

#endif // TINYMPL_SIZE_HPP
