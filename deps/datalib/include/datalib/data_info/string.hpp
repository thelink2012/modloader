/*
 *  Copyright (C) 2014 Denilson das Mercês Amorim (aka LINK/2012)
 *  Licensed under the Boost Software License v1.0 (http://opensource.org/licenses/BSL-1.0)
 *
 */
#pragma once
#include <string>
#include <datalib/data_info.hpp>
#include <datalib/detail/stream/fwd.hpp>

namespace datalib {

/*
 *  data_info<> specialization for 'std::basic_string<CharT, Traits, Allocator>'
 */
template<typename CharT, typename Traits, typename Allocator>
struct data_info<std::basic_string<CharT, Traits, Allocator>> : data_info_base
{
    static const int complexity = 8;    // not possible to specify, depends on runtime's length
                                        // using 8 because it's lesser than the floating point complexity (10)

    // Performs precomparision (that's beforing comparing anything else, perform this cheap comparision)
    struct precompare
    {
        static bool equal_to(
            const std::basic_string<CharT, Traits, Allocator>& str1,
            const std::basic_string<CharT, Traits, Allocator>& str2)
        {
            return str1.length() == str2.length();
        }
    };
};


} // namespace datalib
