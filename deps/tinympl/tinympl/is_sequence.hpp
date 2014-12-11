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

#ifndef TINYMPL_IS_SEQUENCE_HPP
#define TINYMPL_IS_SEQUENCE_HPP

#include <type_traits>
#include <tinympl/as_sequence.hpp>

namespace tinympl {

/**
 * \ingroup SeqSupport
 * \class is_sequence
 * \brief Metafunction to determine if a given type is a sequence
 */
template<class T,class = void> struct is_sequence : std::false_type {};
template<class T> struct is_sequence<T,
	typename std::conditional<true, void, as_sequence_t<T> >::type> : std::true_type {};

}

#endif // TINYMPL_IS_SEQUENCE_HPP
