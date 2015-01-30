/*
 *  Copyright (C) 2014 Denilson das Mercês Amorim (aka LINK/2012)
 *  Licensed under the Boost Software License v1.0 (http://opensource.org/licenses/BSL-1.0)
 *
 */
#pragma once
#include <boost/variant.hpp>
#include <boost/functional/hash.hpp>
#include <boost/variant/detail/hash_variant.hpp>
#include <type_traits>

// What's the difference between 'boost::variant' and this 'either' alias?
// It's because the either type can have a 'empty' state (and it is the default state) whereas boost's variant cannot (...)

namespace datalib {

// Emptyness state type
using either_blank = boost::blank;

// Visitor for the either object
template<class T>
using either_static_visitor = boost::static_visitor<T>;

// An either object can have either a empty state, or a state containing any of 'Types...' types
template<typename... Types>
using either = boost::variant<either_blank, Types...>;

// Checks if the either object is empty
template<typename... Types>
inline bool empty(const either<Types...>& e)
{
    return !(e.which() > 0);    // index 0 is boost::blank, which means it's empty                               
}

// Clears the either object (setting it's state to empty)
template<typename... Types>
inline void clear(either<Types...>& e)
{
    e = either_blank();
}

template<typename T, typename... Types>
inline bool is_typed_as(const either<Types...>& e)
{
    return get<T>(&e) != nullptr;
}

using boost::apply_visitor;





// Internal stuff
namespace detail
{
    /*
     *  Counts the number of actual types in the either object
     *  That's iterates on it and stops when it finds boost's void type
     */

    template<int N, typename ...Types>
    struct either_count_types;

    template<int N, typename Head, typename ...Types>
    struct either_count_types<N, Head, Types...> :
        std::conditional<std::is_same<Head, boost::detail::variant::void_>::value,  // Is Head boost's void?
                            std::integral_constant<int, N>,                         // -> Yes, so stop iteration here
                            either_count_types<N+1, Types...>>::type                // -> Nope, continue iteration
    {};

    template<int N>
    struct either_count_types<N>            // All types readen (or none to read)
        : std::integral_constant<int, N>
    {};
}



/*
 *  Like std::tuple_size but for either objects
 *  Doesn't work in std::tuple_size or tinympl::size because of boost's void/dummy trailing types
 */

template<class either>
struct either_size;

template<typename ...Types>
struct either_size<either<Types...>> : detail::either_count_types<0, Types...>
{};


} // namespace datalib


/*
 *  std::hash specialization for either objects
 */
namespace std
{
    template<typename ...Types>
    struct hash<datalib::either<Types...>>
    {
        std::size_t operator()(const datalib::either<Types...>& e)
        {
            return boost::hash<datalib::either<Types...>>()(e);
        }
    };
}
