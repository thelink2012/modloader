/*
 *  Copyright (C) 2014 Denilson das Mercês Amorim (aka LINK/2012)
 *  Licensed under the Boost Software License v1.0 (http://opensource.org/licenses/BSL-1.0)
 *
 */
#pragma once
#include <tuple>
#include <tinympl/accumulate.hpp>
#include <tinympl/plus.hpp>
#include <datalib/data_info.hpp>
#include <datalib/detail/mpl/integer_sequence.hpp>

namespace datalib {

/*
 *  data_info<> specialization for 'std::tuple<Types...>'
 */
template<typename ...Types>
struct data_info<std::tuple<Types...>> : data_info_base
{
    static const char separator = 0; // manual separator

    // The complexity of such a tuple is the sum of the complexity of all it's types
    static const int complexity = tinympl::accumulate<
                                        integer_sequence<int, data_info<Types>::complexity...>,
                                        tinympl::plus
                                    >::type::value;

    // Performs cheap precomparision
    static bool precompare(const std::tuple<Types...>& a, const std::tuple<Types...>& b)
    {
        return precompare(std::integral_constant<size_t, 0>(), a, b);
    }

private:
    static bool precompare(std::integral_constant<size_t, std::tuple_size<std::tuple<Types...>>::value>,
        const std::tuple<Types...>& a, const std::tuple<Types...>& b)
    {
        return true;
    }

    template<size_t I>
    static bool precompare(std::integral_constant<size_t, I>,
        const std::tuple<Types...>& a, const std::tuple<Types...>& b)
    {
        if(!datalib::precompare(std::get<I>(a), std::get<I>(b))) return false;
        return precompare(std::integral_constant<size_t, I+1>(), a, b);
    }
};

} // namespace datalib
