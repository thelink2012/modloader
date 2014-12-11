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

#ifndef TINYMPL_FUSED_MAP_HPP
#define TINYMPL_FUSED_MAP_HPP

#include <tinympl/variadic/all_of.hpp>
#include <tinympl/variadic/is_unique.hpp>
#include <tinympl/map.hpp>
#include <algorithm>

namespace tinympl {

template<class ... KeyValuePairs>
struct fused_map : std::tuple<typename KeyValuePairs::second_type ... >
{
	static_assert( variadic::all_of< is_pair, KeyValuePairs...>::type::value, "All the arguments of a map must be key/value pairs");
	static_assert( variadic::is_unique<typename KeyValuePairs::first_type ...>::type::value,"Duplicate keys in the map");

	typedef std::tuple<typename KeyValuePairs::second_type ... > base_type;
	typedef map<KeyValuePairs...> map_type;

	using base_type::base_type;

	template<class Key>
	typename map_type::template at<Key>::type & at()
	{
		return std::get< map_type::template at<Key>::index >(*this);
	}

	template<class Key>
	typename map_type::template at<Key>::type const & at() const
	{
		return std::get< map_type::template at<Key>::index >(*this);
	}

	enum {size = map_type::size};
	enum {empty = map_type::empty};

	template<class Key>
	using count = typename map_type::template count<Key>;
};

}

#endif // TINYMPL_FUSED_MAP_HPP
