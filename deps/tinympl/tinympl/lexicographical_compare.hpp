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

#ifndef TINYMPL_LEXICOGRAPHICAL_COMPARE_HPP
#define TINYMPL_LEXICOGRAPHICAL_COMPARE_HPP

#include <tinympl/as_sequence.hpp>
#include <tinympl/sequence.hpp>
#include <tinympl/less.hpp>
#include <type_traits>

namespace tinympl {

/**
 * \ingroup SeqMaxMin
 * \class lexicographical_compare
 * \brief Compares two sequences using the given comparator
 * \param SequenceA the First sequence
 * \param SequenceB the second sequence
 * \param Comparator the comparator (default less)
 * \return An `std::integral_constant<int,v>` where `v` is -1 if the first
sequence is lexicographically smaller than the second, 1 if the first is greater
than the second and 0 if the two sequences are equal.
 */
template<class SequenceA,
        class SequenceB,
        template<class ...> class Comparator = less>
struct lexicographical_compare :
    lexicographical_compare<as_sequence_t<SequenceA>,
                            as_sequence_t<SequenceB>,
                            Comparator> {};

template<class AHead,
        class ... ATail,
        class BHead,
        class ... BTail,
        template<class ...> class Comparator>
struct lexicographical_compare<
    sequence<AHead, ATail...>,
    sequence<BHead, BTail...>,
    Comparator> :
        std::integral_constant < int,
        ( Comparator<AHead, BHead>::type::value ?
              -1 :
              Comparator<BHead, AHead>::type::value ?
              1 :
              lexicographical_compare <
                  sequence<ATail...>,
                  sequence<BTail...>,
                  Comparator >::value
        ) > {};

template<class ... As, template<class ...> class Comparator>
struct lexicographical_compare< sequence<As...>, sequence<>, Comparator> :
        std::integral_constant<int, 1> {};

template<class ... Bs, template<class ...> class Comparator>
struct lexicographical_compare< sequence<>, sequence<Bs...>, Comparator> :
        std::integral_constant < int, -1 > {};

template<template<class ...> class Comparator>
struct lexicographical_compare< sequence<>, sequence<>, Comparator> :
        std::integral_constant<int, 0> {};

} // namespace tinympl

#endif // TINYMPL_LEXICOGRAPHICAL_COMPARE_HPP
