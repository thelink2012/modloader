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

#ifndef TINYMPL_APPLY_HPP
#define TINYMPL_APPLY_HPP

namespace tinympl {

/**
 * \ingroup Functional
 * \class apply
 * \brief Return the result type of the metafunction class F called with arguments Args.,,
 */
template<class F,class ... Args> struct apply
{
	typedef typename F::template eval<Args...>::type type;
};

template<class F,class ... Args>
using apply_t = typename apply<F,Args...>::type;

} // namespace tinympl

#endif // TINYMPL_APPLY_HPP
