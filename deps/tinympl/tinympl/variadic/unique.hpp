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

#ifndef TINYMPL_VARIADIC_UNIQUE_HPP
#define TINYMPL_VARIADIC_UNIQUE_HPP

#include <tinympl/variadic/remove.hpp>

namespace tinympl {
namespace variadic {

/**
 * \ingroup VarModAlgs
 * \class unique
 * \brief Produces a sequence of unique elements from the input sequence,
preserving the ordering.
 * \param Out the output sequence type - defaults to the same kind of the input
sequence.
 * \param Args... The input sequence.
 * \return `unique<...>::type` is a type templated from `Out` which contains the
resulting sequence.
 * \note Only the first (leftmost) duplicate is mantained in the output
sequence.
 * \sa tinympl::unique
 */
template<template<class ...> class Out, class ... Args> struct unique;

template<template<class ...> class Out, class Head, class ... Tail>
struct unique<Out, Head, Tail...> {
private:
    template<class ... Ts>
    struct impl {
        template<class ... Us> using next = unique<Out, Us...>;
        typedef typename remove<Head, next, Tail...>::type::
            template impl<Ts..., Head>::type type;
    };

    template<template<class...> class, class...> friend struct unique;

public:
    typedef typename impl<>::type type;

};

template<template<class ...> class Out> struct unique<Out> {
private:
    template<class ... Ts>
    struct impl {
        typedef Out<Ts...> type;
    };

    template<template<class...> class, class...> friend struct unique;

public:
    typedef typename impl<>::type type;
};


} // namespace variadic
} // namespace tinympl

#endif // TINYMPL_VARIADIC_UNIQUE_HPP
