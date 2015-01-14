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

#ifndef TINYMPL_IDENTITY_HPP
#define TINYMPL_IDENTITY_HPP

namespace tinympl {

/**
 * \ingroup Functional
 * \class identity
 * \brief Returns the argument passed
 */
template<class T> struct identity {typedef T type;};

} // namespace tinympl

#endif // TINYMPL_IDENTITY_HPP
