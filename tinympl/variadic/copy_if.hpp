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

#ifndef TINYMPL_VARIADIC_COPY_IF_HPP
#define TINYMPL_VARIADIC_COPY_IF_HPP

#include <type_traits>

namespace tinympl {
namespace variadic {

/**
 * \ingroup VarModAlgs
 * \class copy_if
 * \brief Copy the elements of a given input sequence which satisfy a given
predicate - the ordering is preserved.
 * \param F The test predicate - F<T>::type::value shall be convertible to bool
 * \param Out The output sequence type - defaults to the same sequence kind of
the input sequence
 * \param Args... The input sequence
 * \return `copy_if<...>::type` is a type templated from `Out` which is
constructed with the elements of SequenceIn which satisfy the predicate `F`.
 * \sa tinympl::copy_if
 */
template< template<class ... T> class F,
        template<class ...> class Out,
        class ... Args> struct copy_if;

template<template<class ... T> class F,
        template<class ...> class Out,
        class Head,
        class ... Tail>
struct copy_if<F, Out, Head, Tail...> {
private:
    template<class ... CopiedElements>
    struct impl {
        template<class ... Args>
        using next = typename copy_if<F, Out, Tail...>::template
            impl<CopiedElements..., Args...>;

        typedef typename std::conditional < F<Head>::type::value,
                                            typename next<Head>::type,
                                            typename next<>::type
                                          >::type type;
    };

    template<template<class ... T> class,
            template<class ...> class,
            class ...> friend struct copy_if;

public:
    typedef typename impl<>::type type;
};

template<template<class ... T> class F, template<class ...> class Out>
struct copy_if<F, Out> {
private:
    template<class ... CopiedElements>
    struct impl {
        typedef Out<CopiedElements...> type;
    };

    template<template<class ... T> class, template<class ...> class, class ...>
    friend struct copy_if;

public:
    typedef typename impl<>::type type;
};

} // namespace variadic
} // namespace tinympl

#endif // TINYMPL_VARIADIC_COPY_IF_HPP
