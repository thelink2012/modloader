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

#ifndef TINYMPL_VARIADIC_REPLACE_HPP
#define TINYMPL_VARIADIC_REPLACE_HPP

#include <tinympl/variadic/replace_if.hpp>
#include <tinympl/bind.hpp>
#include <tinympl/equal_to.hpp>

namespace tinympl {
namespace variadic {

/**
 * \ingroup VarModAlgs
 * \class replace
 * \brief Replace all the elements in the input sequence equal to *Old* with
*New*
 * \param Old The type to be replaced
 * \param New The new type
 * \param Out The type of the output sequence, defaults to the same kind of the
input sequence
 * \param Args... The input sequence
 * \return `replace<...>::type` is a type templated from `Out`
 * \note The comparison is done with \ref tinympl::equal_to - it can be
specialized
 * \sa tinympl::replace
 */
template<class Old,
        class New,
        template<class...> class Out,
        class ... Args>
struct replace :
    replace_if<bind<equal_to, arg1, Old>::template eval_t, New, Out, Args...>
{};


} // namespace variadic
} // namespace tinympl

#endif // TINYMPL_VARIADIC_REPLACE_HPP
