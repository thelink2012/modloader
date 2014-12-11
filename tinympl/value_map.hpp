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

#ifndef TINYMPL_VALUE_MAP_HPP
#define TINYMPL_VALUE_MAP_HPP

#include <tinympl/algorithm.hpp>
#include <tinympl/map.hpp>

namespace tinympl {

/**
 * \ingroup Containers
 * \class value_map
 * \brief A compile time map from a compile-time value to another
 * This class represents a compile time mapping between values.
 * The first two parameters are respectively the type of the key and of the values.
 * The mapping is specified using `std::pair`, that is every parameter of `map` must be an
 * `std::pair< std::integral_constant<KeyType,key>,std::integral_constant<ValueType,value> >`
 *
 * `value_map` supports standard insertion/removal and element access.
 * \note \ref equal_to is specialized for `map`, in order to make the order in which the key/value pairs are specified irrelevant.
 */
template<class KeyType,class ValueType,class ... Args>
struct value_map
{
	/**
	 * \brief Determines whether `T` is a valid key type, i.e. `std::integral_constant<KeyType,KeyValue>`
	 */
	template<class T> struct is_valid_key_type : std::false_type {};
	template<KeyType k> struct is_valid_key_type<std::integral_constant<KeyType,k>> : std::true_type {};

	/**
	 * \brief Determines whether `T` is a valid value type, i.e. `std::integral_constant<ValueType,Value>`
	 */
	template<class T> struct is_valid_value_type : std::false_type {};
	template<ValueType k> struct is_valid_value_type<std::integral_constant<ValueType,k>> : std::true_type {};

	static_assert( variadic::all_of< is_pair, Args...>::type::value, "All the arguments of a map must be key/value pairs");
	static_assert( variadic::is_unique<typename Args::first_type ...>::type::value,"Duplicate keys in the map");
	static_assert( variadic::all_of< is_valid_key_type, typename Args::first_type...>::type::value, "Wrong type of key");
	static_assert( variadic::all_of< is_valid_value_type, typename Args::second_type...>::type::value, "Wrong type of value");

	/**
	 * \brief Convenience using-declaration to quickly define keys for this `value_map`
	 */
	template<KeyType k> using key = std::integral_constant<KeyType,k>;

	/**
	 * \brief Convenience using-declaration to quickly define values for this `value_map`
	 */
	template<ValueType v> using value = std::integral_constant<ValueType,v>;

	/**
	 * \brief Convenience using-declaration to quickly define a key/value pair for this `value_map`
	 */
	template<KeyType k,ValueType v>
	using pair = std::pair< key<k>,value<v> >;

	/**
	 * \class at
	 * \brief Return the value element with the given key
	 */
	template<KeyType k>
	struct at
	{
		enum {index = variadic::find<key<k>, typename Args::first_type ... >::type::value };

		static_assert(index < sizeof...(Args),"Key k not present in the map");

		typedef typename variadic::at<index,Args...>::type::second_type type;
		enum {value = type::value};
	};

	enum
	{
		size = sizeof ... (Args) //!< The number of elements contained in the map
	};

	enum
	{
		empty = (size == 0) //!< Determines whether the map is empty
	};

	/**
	 * \brief Count the number of elements in the map with a given key.
	 * \note Since this class is a `map` and not a `multimap`, the only possible results for this operation are 0 and 1.
	 */
	template<KeyType k>
	using count = std::integral_constant<
			std::size_t,
			(variadic::find<key<k>, typename Args::first_type ... >::type::value == size ? 0 : 1)>;

	/**
	 * \class insert
	 * \brief Returns a new map with the new Key/Value pair, or this map if the key is already present in the map
	 */
	template<KeyType k,ValueType v>
	struct insert
	{
		typedef typename std::conditional<
			count<k>::type::value == 0,
			value_map<KeyType,ValueType,Args..., pair<k,v> >,
			value_map<KeyType,ValueType,Args...> >::type type;
	};

	/**
	 * \class insert_many
	 * \brief Calls \ref insert many times to insert many Key/Value pairs
	 */
	template<class ... KeyValuePairs>
	struct insert_many
	{
		static_assert( variadic::all_of< is_pair, KeyValuePairs...>::type::value, "All the arguments of insert_many must be key/value pairs");
		static_assert( variadic::all_of< is_valid_key_type, typename Args::first_type...>::type::value, "Wrong type of key");
		static_assert( variadic::all_of< is_valid_value_type, typename Args::second_type...>::type::value, "Wrong type of value");

		template<class Map,class T> using insert_one_t = typename Map::template insert<T::first_type::value, T::second_type::value>;

		typedef typename variadic::left_fold<insert_one_t,value_map,KeyValuePairs...>::type type;
	};

	/**
	 * \class erase
	 * \brief Return a new map constructed by the current map removing the `k` key, if present, otherwise return the current map.
	 */
	template<KeyType k>
	class erase
	{
		template<class T> struct key_comparer : std::is_same<typename T::first_type,key<k> > {};

		template<class ... Ts>
		using rebind = value_map<KeyType,ValueType,Ts...>;

	public:
		typedef typename variadic::remove_if< key_comparer, rebind, Args...>::type type;
	};
};

template<class KeyType,class ValueType,class ... As,class ... Bs>
struct equal_to< value_map<KeyType,ValueType,As...>, value_map<KeyType,ValueType,Bs...> > : unordered_equal< sequence<As...>, sequence<Bs...> > {};

}

#endif // TINYMPL_VALUE_MAP_HPP
