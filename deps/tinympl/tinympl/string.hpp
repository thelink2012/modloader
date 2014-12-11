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

#ifndef TINYMPL_STRING_HPP
#define TINYMPL_STRING_HPP

#include <tinympl/algorithm.hpp>
#include <tinympl/sequence.hpp>

namespace tinympl
{

/**
 * \defgroup String Compile-time string
 * Support for compile time strings
 */

/**
 * \ingroup Containers String
 * @{
 */

/**
 * \class basic_string
 * \brief A vector of values of type T
 * \param T The type of the characters
 * \param chars... The characters which compose the string
 */
template<class T,T ... chars>
struct basic_string
{
	enum
	{
		size = sizeof ... (chars) //!< The size of the vector
	};

	enum
	{
		empty = (size == 0) //!< Determine whether the vector is empty
	};

	typedef basic_string<T,chars...> type; //!< This type
	typedef T value_type; //!< The type of the string characters
	typedef const T * const_pointer;

	//! Get the i-th character
	/**
	 * \warning If this function is called in a non-constexpr context, no out of range protection is provided
	 */
	constexpr static value_type at(std::size_t i) {return c_str()[i];}

	//! Return a pointer to a null-terminated C-style string
	constexpr static const_pointer c_str() {return v_;}

	//! Return a copy of the first character
	constexpr static value_type front() {return at(0);}

	//! Return a copy of the last character
	constexpr static value_type back() {return at(size - 1);}

private:
	template<T t>
	using wrap = std::integral_constant<T,t>;

	template<class ... Us>
	using unwrap = basic_string<T, Us::value ... >;

public:

	//! Return a new string constructed by inserting the string `Str` at the position `pos`.
	/**
	 * \sa insert_c
	 */
	template<std::size_t pos,class Str>
	using insert = tinympl::insert<pos,
			Str,
			basic_string<T,chars...> >;

	//! Return a new string constructed by inserting the characters `NewChars...` at the position `pos`
	/**
	 * \sa insert
	 */
	template<std::size_t pos,T ... NewChars>
	using insert_c = insert<pos, basic_string<T,NewChars...> >;

	//! Return a new string constructed by removing `count` characters starting at position `pos`.
	/**
	 * \warning In order to mimic the behavior of `std::string`, this function
	 *  takes an index and a *size* of the range, **not** a [first,last) range.
	 */
	template<std::size_t pos,std::size_t count>
	using erase = tinympl::erase<pos,pos + count,
		basic_string<T,chars...> >;

	//! Return a new string constructed by appending the string `Str` at the end of this string.
	/**
	 * \sa append_c
	 */
	template<class Str>
	using append = insert<size,Str>;

	//! Return a new string constructed by appending the characters `NewChars...` at the end of this string.
	/**
	 * \sa append
	 */
	template<T ... NewChars>
	using append_c = insert_c<size,NewChars...>;

	//! Return a substring long `count` starting at position `pos`.
	template<std::size_t pos, std::size_t count>
	class substr
	{
		static_assert( pos <= size, "substr pos out of range ");
		static_assert( pos + count <= size, "substr count out of range");

	public:
		typedef typename tinympl::erase<0,pos,
			typename tinympl::erase<pos+count,size, basic_string >::type>::type type;
	};

	//! Return a new string constructed by replacing `count` characters starting at `pos` with the string `Str`.
	/**
	 * \sa replace_c
	 */
	template<std::size_t pos, std::size_t count, class Str>
	class replace
	{
		static_assert( pos <= size, "substr pos out of range ");
		static_assert( pos + count <= size, "substr count out of range");

		typedef typename erase<pos,count>::type str_cleared;

	public:
		typedef typename str_cleared::template insert<pos,Str>::type type;
	};

	//! Return a new string constructed by replacing `count` characters starting at `pos` with the characters `ts...`
	/**
	 * \sa replace
	 */
	template<std::size_t pos, std::size_t count, T ... ts>
	using replace_c = replace<pos,count, basic_string<T,ts...> >;

	//! Alias for \ref lexicographical_compare, mimic `std::string::compare`.
	/**
	 * \sa lexicographical_compare
	 */
	template<class OtherStr>
	using compare = lexicographical_compare<
		basic_string<T,chars...>,
		OtherStr>;

private:
	template<std::size_t i,std::size_t count>
	using unguarded_substr = substr<i, (i+count <= size ? count : size - i)>;

	template<class Str, int i, std::size_t s> struct find_impl :
		std::conditional<
			unguarded_substr<i,Str::size>::type::template
				compare<Str>::value == 0,
			std::integral_constant<int, i>,
			find_impl<Str,i+1,s> >::type {};

	template<class Str, int s> struct find_impl<Str, s, s> : std::integral_constant<std::size_t, s> {};

	template<class Str,int i> struct rfind_impl :
		std::conditional<
			unguarded_substr<i,Str::size>::type::template
				compare<Str>::value == 0,
			std::integral_constant<std::size_t, i>,
			rfind_impl<Str,i-1> >::type {};

	template<class Str, std::size_t s> struct find_impl< Str, -1, s> : std::integral_constant< std::size_t, s> {};

public:

	//! Return the index of the first character of the first occurrence of the substring `Str`, or `size` if `Str` is not a substring of this string.
	/**
	 * \sa rfind
	 * \sa find_c
	 */
	template<class Str>
	using find = find_impl<Str,0,size>;

	//! Return the index of the first character of the last occurrence of the substring `Str`, or `size` if `Str` is not a substring of this string.
	template<class Str>
	using rfind = rfind_impl<Str,size>;

	//! Return the index of the first character of the first occurrence of the substring `ts...`, or `size` if `ts...` is not a substring of this string.
	template<T ... ts> using find_c = find< basic_string<T,ts...> >;

	//! Return the index of the first character of the last occurrence of the substring `ts...`, or `size` if `ts...` is not a substring of this string.
	template<T ... ts> using rfind_c = rfind< basic_string<T,ts...> >;

private:
	static constexpr T v_[size + 1] = {chars ... , 0};
};

/** @} */

/**
 * \addtogroup String
 * @{
 */

template<class T,T ... chars>
constexpr T basic_string<T,chars...>::v_ [ size + 1];

/**
 * \class make_basic_string
 * \brief Construct a \ref basic_string from a constexpr pointer to a null-terminated string.
 */
template<class T, const T * ptr>
class make_basic_string
{
	template<T ... ts> struct extract
	{
		typedef typename std::conditional<
			ptr[ sizeof ... (ts) ] == 0,
			basic_string<T, ts...>,
			extract<ts..., ptr[sizeof ... (ts) ]>
		>::type::type type;
	};
public:
	using type = typename extract<>::type;
};

//! Alias for \ref make_basic_string of `char`.
/**
 * Construct a \ref `basic_string <char,chars...>` with `chars...` initialized from a constexpr pointer to a null-terminated string.
 * \param p A pointer to a constexpr null-terminated string.
 */
template<const char * p>
using string = typename make_basic_string<char,p>::type;

/** @} */

/**
 * \ingroup SeqCustom
 * \brief Customization point to allow `basic_string` to work as a tinympl sequence
 */
template<class T,T ... ts> struct as_sequence<basic_string<T,ts...> >
{
private:
	template<class ... Args>
	struct do_rebind
	{
		template<class U>
		using check_t = std::is_same<typename U::value_type,T>;

		static_assert(variadic::all_of<check_t,Args...>::value,
					  "basic_string: unable to rebind when the arguments are not of the same value_type" );

		typedef basic_string<T, Args::value ...> type;
	};

public:
	typedef sequence< std::integral_constant<T,ts> ...> type;
	template<class ... Ts> using rebind = typename do_rebind<Ts...>::type;
};

}

#endif // TINYMPL_STRING_HPP
