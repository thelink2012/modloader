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

#ifndef TINYMPL_VARIADIC_SORT_HPP
#define TINYMPL_VARIADIC_SORT_HPP

#include <tinympl/variadic/min_element.hpp>
#include <tinympl/variadic/erase.hpp>
#include <tinympl/variadic/at.hpp>

namespace tinympl {
namespace variadic {

/**
 * \ingroup VarSort
 * \class sort
 * \brief Sort the input sequence according to a given comparison function
 * \param Out the output sequence type, defaults to the same kind of the input
sequence type
 * \param Cmpl The comparison operator. `Cmp<A,B>::type::value` must be
convertible to bool. The comparator must produce total ordering between
elements. Defaults to \ref tinympl::less
 * \param Args... the input sequence
 * \note The compile time complexity is O(N^2)
 * \return `sort<...>::type` is a type templated from `Out` which contains the
sorted sequence
 * \sa tinympl::sort
 */
template<template<class ... > class Cmp,
        template<class ...> class Out,
        class ... Args> struct sort;

template<template<class ... > class Comp,
        template<class ...> class Out,
        class ... Args>
struct sort {
private:
    template<class ... OtherArgs>
    using next_sort = sort<Comp, Out, OtherArgs...>;

    enum {this_min = min_element<Comp, Args...>::type::value };
    typedef typename erase < this_min, this_min + 1, next_sort, Args ... >::type
        next;

    template<class ... CopiedElements>
    struct impl {
        typedef typename next::template impl<CopiedElements...,
                                        typename at<this_min, Args...>::type
                                        >::type type;
    };

    template<template<class ... > class , template<class ...> class, class ...>
    friend struct sort;

public:
    typedef typename impl<>::type type;
};

template<template<class ... > class Comp, template<class ...> class Out>
struct sort<Comp, Out> {
private:
    template<class ... CopiedElements>
    struct impl {
        typedef Out<CopiedElements...> type;
    };

    template<template<class ... > class , template<class ...> class, class ...>
    friend struct sort;

public:
    typedef typename impl<>::type type;
};

} // namespace variadic
} // namespace tinympl

#endif // TINYMPL_VARIADIC_SORT_HPP
