/*
 *  Copyright (C) 2014 Denilson das Mercês Amorim (aka LINK/2012)
 *  Licensed under the Boost Software License v1.0 (http://opensource.org/licenses/BSL-1.0)
 *
 */
#pragma once
#include <string>
#include <datalib/data_info/dyncontainer.hpp>

namespace datalib {

template<typename CharT, typename Traits, typename Allocator>
struct is_dyncontainer<std::basic_string<CharT, Traits, Allocator>> : std::true_type
{};

template<typename CharT, typename Traits, typename Allocator>
struct data_info<std::basic_string<CharT, Traits, Allocator>>
    : data_info_dyncontainer<std::basic_string<CharT, Traits, Allocator>>
{
    static const char separator = data_info_base::separator;
};

} // namespace datalib
