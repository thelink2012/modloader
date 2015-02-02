/*
 *  Copyright (C) 2014 Denilson das Mercês Amorim (aka LINK/2012)
 *  Licensed under the Boost Software License v1.0 (http://opensource.org/licenses/BSL-1.0)
 *
 */
#pragma once
#include <tinympl/accumulate.hpp>
#include <tinympl/greater.hpp>
#include <tinympl/divides.hpp>
#include <tinympl/plus.hpp>
#include <tinympl/int.hpp>
#include <datalib/detail/mpl/integer_sequence.hpp>
#include <datalib/data_info.hpp>
#include <datalib/detail/either.hpp>

namespace datalib {

/*
 *  data_info<> specialization for 'boost::detail::variant::void_'
 *  This type should be ignored, it's used as unused types in 'boost::variadic' (used to implement our 'either' object)
 */
template<>
struct data_info<boost::detail::variant::void_> : data_info_base
{
    static const bool ignore = true;
    static const int  complexity = 0;
};


/*
 *  data_info<> specialization for 'either<Types...>'
 */
template<typename ...Types>
struct data_info<either<Types...>> : data_info_base
{
    private:    // Find the average of the types
        using either_type = either<Types...>;

        // The sum of all 'Types' complexity
        using sum = typename tinympl::accumulate<
                                        integer_sequence<int, data_info<Types>::complexity...>,
                                        tinympl::plus
                                    >::type;

        // The average of all types (sum / either_size)
        using average = typename tinympl::divides<sum, either_size<either_type>>::type;

    public:

        // The complexity of comparing this type is the average of the types, since the object assumes the state of one of the types (or none/empty),
        // it's basically the same as comparing ONE of them, but which one? we don't know, so the average should be ok
        static const int complexity = (average::value <= 1? 1 : 
                                        average::value - 1);

        // Performs cheap precomparision
        static bool precompare(const either_type& a, const either_type& b)
        {
            return a.which() == b.which();
        }
};



} // namespace datalib
