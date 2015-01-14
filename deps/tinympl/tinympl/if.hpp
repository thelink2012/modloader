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

#ifndef TINYMPL_IF_HPP
#define TINYMPL_IF_HPP

#include <type_traits>

namespace tinympl {

/**
 * \ingroup Functional
 * \class if_
 * \brief Returns `A` if `C::value` is true, otherwise `B`.
 */
template<class C,class A,class B> struct if_
{
	typedef typename std::conditional<C::value,A,B>::type type;
};

} // namespace tinympl

#endif // TINYMPL_IF_HPP
