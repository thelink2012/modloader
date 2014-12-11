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

#ifndef TINYMPL_RIGHT_FOLD_HPP
#define TINYMPL_RIGHT_FOLD_HPP

#include <tinympl/variadic/right_fold.hpp>
#include <tinympl/as_sequence.hpp>
#include <tinympl/sequence.hpp>

namespace tinympl {

/**
 * \ingroup SeqFold
 * \class right_fold
 * \brief Collapses a sequence starting from right using a functor.
 * \param Sequence The input sequence
 * \param F The functor; `F<T,U>` must be a valid expression
 * \return A type which is the result of `F(A1, F(A2,F(A3, ... ) ) )`
 * \sa variadic::right_fold
 */
template<class Sequence, template<class ...> class F>
struct right_fold : right_fold< as_sequence_t<Sequence>, F> {};

template<class ... Ts, template<class ...> class F>
struct right_fold<sequence<Ts...>, F> : variadic::right_fold<F, Ts...> {};

} // namespace tinympl

#endif // TINYMPL_RIGHT_FOLD_HPP
