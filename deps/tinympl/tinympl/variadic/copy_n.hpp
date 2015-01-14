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

#ifndef TINYMPL_VARIADIC_COPY_N_HPP
#define TINYMPL_VARIADIC_COPY_N_HPP

#include <cstddef>

namespace tinympl {
namespace variadic {

/**
 * \ingroup VarModAlgs
 * \class copy_n
 * \brief Copy the first n elements from the input sequence.
 * \param n The number of elements to be copied
 * \param Out The output sequence type, defaults to the same kind of the input
sequence
 * \param Args... the input sequence
 * \return `copy_n<...>::type` is a type templated from `Out` which is
constructed with the first n types of the input sequence
 * \sa tinympl::copy_n
 */
template<std::size_t n, template<class ...> class Out, class ... Args>
struct copy_n;

template<std::size_t n,
        template<class ...> class Out,
        class Head, class ... Tail>
struct copy_n<n, Out, Head, Tail...> {
private:
    template<class ... CopiedElements> struct impl {
        typedef typename copy_n < n - 1, Out, Tail... >::template
            impl<CopiedElements..., Head>::type type;
    };

    template<std::size_t, template<class ...> class, class ...>
    friend struct copy_n;

public:
    static_assert( n <= 1 + sizeof...( Tail ), "n overflow" );
    typedef typename impl<>::type type;
};

template<template<class ...> class Out, class Head, class ... Tail>
struct copy_n<0, Out, Head, Tail...> {
private:
    template<class ... CopiedElements> struct impl {
        typedef Out<CopiedElements...> type;
    };

    template<std::size_t, template<class ...> class, class ...>
    friend struct copy_n;

public:
    typedef typename impl<>::type type;
};

template<template<class ...> class Out> struct copy_n<0, Out> {
private:
    template<class ... CopiedElements> struct impl {
        typedef Out<CopiedElements...> type;
    };

    template<std::size_t, template<class ...> class, class ...>
    friend struct copy_n;

public:
    typedef typename impl<>::type type;
};


} // namespace variadic
} // namespace tinympl

#endif // TINYMPL_VARIADIC_COPY_N_HPP
