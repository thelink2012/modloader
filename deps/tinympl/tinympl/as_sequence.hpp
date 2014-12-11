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

#ifndef TINYMPL_AS_SEQUENCE_HPP
#define TINYMPL_AS_SEQUENCE_HPP

#include <tinympl/sequence.hpp>

namespace tinympl {

/**
 * \defgroup SeqSupport Sequence support
 * Basic sequences support - provides user defined customization points to define sequences.
 * @{
 */

/**
 * \class as_sequence
 * \brief Provide a customization points by allowing the user to specialize this class
 */
template<class T> struct as_sequence;

/**
 * \defgroup SeqCustom Sequence customization points
 * Allows various sequence types to be used in the sequence algorithms.
 * @{
 */

/**
 * \brief Customization point to allow any variadic template type to work with tinympl
 */
template<class ... Args,template<class ...> class Seq> struct as_sequence< Seq<Args...> > {
	typedef sequence<Args...> type;
	template<class ... Ts> using rebind = Seq<Ts...>;
};

/** @} */

/**
 * \brief Convenience using declaration to convert a given sequence to a `tinympl::sequence` with the same content
 */
template<class T> using as_sequence_t = typename as_sequence<T>::type;

/** @} */
}

#endif // TINYMPL_AS_SEQUENCE_HPP
