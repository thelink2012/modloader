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

#ifndef TINYMPL_INHERIT_HPP
#define TINYMPL_INHERIT_HPP

namespace tinympl {

/**
 * \ingroup Functional
 * \class inherit
 * \brief Construct a type inherited from the arguments
 */
template<class ... Args> struct inherit
{
	struct inherit_t : Args... {};
	typedef inherit_t type;
};


} // namespace tinympl

#endif // TINYMPL_INHERIT_HPP
