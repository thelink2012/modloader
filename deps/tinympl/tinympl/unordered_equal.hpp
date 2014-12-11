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

#ifndef TINYMPL_UNORDERED_EQUAL_HPP
#define TINYMPL_UNORDERED_EQUAL_HPP

#include <tinympl/variadic/count.hpp>
#include <tinympl/variadic/all_of.hpp>
#include <tinympl/as_sequence.hpp>
#include <tinympl/sequence.hpp>
#include <tinympl/logical_and.hpp>
#include <tinympl/variadic/count.hpp>
#include <tinympl/variadic/all_of.hpp>
#include <type_traits>

namespace tinympl {

/**
 * \ingroup SeqSet
 * \class unordered_equal
 * \brief Determines whether it is possible to reorder the sequence `A` to match
exactly the sequence `B`
 * \param SequenceA The first sequence
 * \param SequenceB The second sequence
 * \return `unordered_equal<A,B>::type` is a `std::integral_constant<bool,v>`
where `v` is true only if the two sequences are equal (except for the ordering
of the elements)
 * \note Compile time complexity is O(N^2)
 */
template<class SequenceA, class SequenceB>
struct unordered_equal :
    unordered_equal< as_sequence_t<SequenceA>, as_sequence_t<SequenceB> > {};

namespace detail {
template<class SequenceA, class SequenceB> struct unordered_equal_impl;

template<class ... As, class ... Bs>
struct unordered_equal_impl<sequence<As...>, sequence<Bs...> > {
private:
    template<class T>
    struct check_t {
        typedef std::integral_constant < bool,
                variadic::count<T, As...>::type::value ==
                variadic::count<T, Bs...>::type::value > type;
    };

public:
    typedef typename logical_and <
        typename variadic::all_of< check_t, As...>::type,
        typename variadic::all_of< check_t, Bs...>::type >::type type;
};
}

template<class ... As, class ... Bs>
struct unordered_equal<sequence<As...>, sequence<Bs...> > :
    detail::unordered_equal_impl< sequence<As...>, sequence<Bs...> >::type
{};

} // namespace tinympl

#endif // TINYMPL_UNORDERED_EQUAL_HPP
