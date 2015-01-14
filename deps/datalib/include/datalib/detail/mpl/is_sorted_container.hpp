/*
 *  Copyright (C) 2014 Denilson das Mercês Amorim (aka LINK/2012)
 *  Licensed under the Boost Software License v1.0 (http://opensource.org/licenses/BSL-1.0)
 *
 */
#pragma once
#include <type_traits>
#include <map>

namespace datalib {

template<class Type>
struct is_sorted_container : std::false_type {};

template<class Key, class Value, class Compare, class Alloc>
struct is_sorted_container<std::map<Key, Value, Compare, Alloc>> : std::true_type {};

}
