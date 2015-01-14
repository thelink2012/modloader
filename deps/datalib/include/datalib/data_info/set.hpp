/*
 *  Copyright (C) 2014 Denilson das Mercês Amorim (aka LINK/2012)
 *  Licensed under the Boost Software License v1.0 (http://opensource.org/licenses/BSL-1.0)
 *
 */
#pragma once
#include <set>
#include <datalib/data_info/dyncontainer.hpp>

namespace datalib {

template<typename... Args>
struct is_dyncontainer<std::set<Args...>> : std::true_type
{};

template<typename... Args>
struct data_info<std::set<Args...>> :
    data_info_dyncontainer<std::set<Args...>>
{};

} // namespace datalib
