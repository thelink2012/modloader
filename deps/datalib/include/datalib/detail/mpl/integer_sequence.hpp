/*
 *  Copyright (C) 2014 Denilson das Mercês Amorim (aka LINK/2012)
 *  Licensed under the Boost Software License v1.0 (http://opensource.org/licenses/BSL-1.0)
 *
 */
#pragma once
#include <tinympl/sequence.hpp>
#include <tinympl/int.hpp>
#include <tinympl/join.hpp>

namespace datalib {

// CXX14

/*
 *  Non-compliant implementation of C++14's std::integer_sequence (see http://en.cppreference.com/w/cpp/utility/integer_sequence)
 *
 *  integer_sequence
 *      The class template std::integer_sequence represents a compile-time sequence of integers. When used as an argument to a function template,
 *      the parameter pack Ints can be deduced and used in pack expansion. 
 *
 */
template<class T, T... Ints>
using integer_sequence = tinympl::sequence<std::integral_constant<T, Ints>...>; // must be a tinympl::sequence (see make_index_sequence)


/*
 * A helper alias template std::make_integer_sequence is defined to simplify creation of std::integer_sequence and
 * std::index_sequence types with 0, 1, 2, ..., N-1 as Ints:
 */
template<int N>
struct make_index_sequence
{
    using type = typename tinympl::join<
        typename make_index_sequence<N-1>::type,
        tinympl::sequence<tinympl::int_<N-1>>
    >::type;
};

template<>
struct make_index_sequence<1>
{
    using type = tinympl::sequence<tinympl::int_<0>>;
};

template<>
struct make_index_sequence<0>
{
    using type = tinympl::sequence<>;
};

} // namespace datalib
