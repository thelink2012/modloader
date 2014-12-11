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

#ifndef TINYMPL_VARIADIC_REVERSE_HPP
#define TINYMPL_VARIADIC_REVERSE_HPP

namespace tinympl {
namespace variadic {

/**
 * \ingroup VarModAlgs
 * \class reverse
 * \brief Reverse the input sequence
 * \param Out the output sequence type, defaults to the same kind of the input
sequence type
 * \param Args... the input sequence
 * \return `reverse<...>::type` is a type templated from `Out` which contains
the reversed sequence
 * \sa tinympl::reverse
 */
template<template<class ...> class Out, class ... Args> struct reverse;

template<template<class ...> class Out, class Head, class ... Tail>
struct reverse<Out, Head, Tail...> {
private:
    template<class ... ReversedTail>
    struct impl {
        typedef typename reverse<Out, Tail...>::template
            impl<Head, ReversedTail...>::type type;
    };

    template<template<class ...> class, class ...> friend struct reverse;

public:
    typedef typename impl<>::type type;
};

template<template<class ...> class Out> struct reverse<Out> {
private:
    template<class ... ReversedTail>
    struct impl {
        typedef Out<ReversedTail...> type;
    };

    template<template<class ...> class, class ...> friend struct reverse;

public:
    typedef Out<> type;
};

} // namespace variadic
} // namespace tinympl

#endif // TINYMPL_VARIADIC_REVERSE_HPP
