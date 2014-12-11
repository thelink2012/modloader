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

#ifndef TINYMPL_NONE_OF_HPP
#define TINYMPL_NONE_OF_HPP

#include <tinympl/variadic/none_of.hpp>
#include <tinympl/as_sequence.hpp>
#include <tinympl/sequence.hpp>

namespace tinympl {

/**
 * \ingroup SeqNonModAlgs
 * \class none_of
 * \brief Determines whether none of the elements in the sequence satisfy the
given predicate
 * \param Sequence the input sequence
 * \param F the predicate, `F<T>::type::value` must be convertible to bool
 * \return `none_of<...>::type` is a `std::integral_constant<bool,v>` where `v`
is true iff none of the elements in the sequence satisfy the predicate `F`
 * \sa variadic::none_of
 */
template< class Sequence, template<class ...> class F>
struct none_of : none_of<as_sequence_t<Sequence>, F> {};

template< template<class ...> class F, class ... Args>
struct none_of<sequence<Args...>, F > : variadic::none_of<F, Args...> {};

} // namespace tinympl

#endif // TINYMPL_NONE_OF_HPP
