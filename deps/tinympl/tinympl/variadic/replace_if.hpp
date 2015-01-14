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

#ifndef TINYMPL_VARIADIC_REPLACE_IF_HPP
#define TINYMPL_VARIADIC_REPLACE_IF_HPP

#include <type_traits>

namespace tinympl {
namespace variadic {

/**
 * \ingroup VarModAlgs
 * \class replace_if
 * \brief Replace all the elements in the input sequence which satisfy a given
predicate with a given type T
 * \param F The predicate, `F<T>::type::value` must be convertible to bool
 * \param T The type used to replace the types
 * \param Out The type of the output sequence, defaults to the same kind of the
input sequence
 * \param Args... The input sequence
 * \return `replace_if<...>::type` is a type templated from `Out`
 * \sa tinympl::replace_if
 */
template<template<class ... T> class F,
        class T,
        template<class ...> class Out,
        class ... Args> struct replace_if;

template<template<class ... T> class F,
        class T,
        template<class ...> class Out,
        class Head,
        class ... Tail>
struct replace_if<F, T, Out, Head, Tail...> {
private:
    template<class ... CopiedElements>
    struct impl {
        typedef typename replace_if<F, T, Out, Tail...>::template
            impl <CopiedElements...,
                typename std::conditional <
                    F<Head>::type::value,
                    T,
                    Head >::type
                >::type type;
    };

    template<
        template<class ...> class,
        typename,
        template<class...> class,
        class ...> friend struct replace_if;

public:
    typedef typename impl<>::type type;
};

template<template<class ... T> class F, class T, template<class ...> class Out>
struct replace_if<F, T, Out> {
private:
    template<class ... CopiedElements>
    struct impl {
        typedef Out<CopiedElements...> type;
    };

    template<template<class ...> class,
            typename,
            template<class...> class,
            class ...> friend struct replace_if;

public:
    typedef typename impl<>::type type;
};

} // namespace variadic
} // namespace tinympl

#endif // TINYMPL_VARIADIC_REPLACE_IF_HPP
