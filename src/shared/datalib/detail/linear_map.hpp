/*
 *  Copyright (C) 2014 Denilson das Mercês Amorim (aka LINK/2012)
 *  Licensed under the Boost Software License v1.0 (http://opensource.org/licenses/BSL-1.0)
 *
 */
#pragma once
#include <datalib/detail/basic_linear_map.hpp>
#include <list>

namespace datalib {

// default linear map (using list)
template<typename Key, typename Value, class KeyComp = std::equal_to<Key>, class Allocator = std::allocator<std::pair<Key, Value>>>
using linear_map = basic_linear_map<KeyComp, std::list<std::pair<Key, Value>, Allocator>>;

}
