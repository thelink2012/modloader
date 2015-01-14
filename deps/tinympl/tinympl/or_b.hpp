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

#ifndef TINYMPL_OR_B_HPP
#define TINYMPL_OR_B_HPP

#include <type_traits>

namespace tinympl {

template<bool ... Args> struct or_b;
template<bool Head,bool ... Tail> struct or_b<Head,Tail...> : std::integral_constant<bool, Head || or_b<Tail...>::value> {};
template<bool Head> struct or_b<Head> : std::integral_constant<bool,Head> {};

} // namespace tinympl

#endif // TINYMPL_OR_B_HPP
