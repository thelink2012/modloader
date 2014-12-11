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

#ifndef TINYMPL_LEFT_FOLD_HPP
#define TINYMPL_LEFT_FOLD_HPP

#include <tinympl/variadic/left_fold.hpp>
#include <tinympl/as_sequence.hpp>
#include <tinympl/sequence.hpp>

namespace tinympl {

/**
 * \ingroup SeqFold
 * \class left_fold
 * \brief Collapses a sequence starting from left using a functor.
 * \param Sequence The input sequence
 * \param F The functor; `F<T,U>` must be a valid expression
 * \return A type which is the result of `F( ... F(F(A1,A2),A3) .. )`
 * \sa variadic::left_fold
 */
template<class Sequence, template<class ...> class F>
struct left_fold : left_fold< as_sequence_t<Sequence>, F> {};

template<class ... Ts, template<class ...> class F>
struct left_fold<sequence<Ts...>, F> : variadic::left_fold<F, Ts...> {};

} // namespace tinympl

#endif // TINYMPL_LEFT_FOLD_HPP
