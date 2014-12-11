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

#ifndef TINYMPL_AT_HPP
#define TINYMPL_AT_HPP

#include <tinympl/variadic/at.hpp>
#include <tinympl/as_sequence.hpp>
#include <tinympl/sequence.hpp>

namespace tinympl {

/**
 * \ingroup SeqAlgsIntr
 * \class at
 * \brief Get the i-th element of a sequence
 * \param I The index of the desired element
 * \param Seq The input sequence
*/
template<std::size_t I, class Sequence>
struct at : at <I, as_sequence_t<Sequence> > {};

template<std::size_t I, class ... Args>
struct at<I, sequence<Args...> > : variadic::at<I, Args...> {};

} // namespace tinympl

#endif // TINYMPL_AT_HPP
