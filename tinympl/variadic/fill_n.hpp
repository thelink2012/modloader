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

#ifndef TINYMPL_VARIADIC_FILL_N_HPP
#define TINYMPL_VARIADIC_FILL_N_HPP

#include <cstddef>

namespace tinympl {
namespace variadic {

/**
 * \ingroup VarModAlgs
 * \class fill_n
 * \brief Fills an output sequence with n equal elements
 * \param n The number of elements
 * \param T The type of the elements
 * \param Out The output sequence type
 * \return `fill_n<...>::type` is a type templated from `Out` constructed with n
types equal to `T`
 * \sa tinympl::fill_n
 */
template<std::size_t n, class T, template<class ...> class Out> struct fill_n;


template<std::size_t n, class T, template<class ...> class Out> struct fill_n {
private:
    template<class ... Args> struct impl {
        typedef typename fill_n < n - 1, T, Out >::template
            impl<Args..., T>::type type;
    };

    template<std::size_t, class, template<class ...> class>
    friend struct fill_n;

public:
    typedef typename impl<>::type type;
};

template<class T, template<class...> class Out> struct fill_n<0, T, Out> {
private:
    template<class ... Args> struct impl {
        typedef Out<Args...> type;
    };

    template<std::size_t, class, template<class ...> class>
    friend struct fill_n;

public:
    typedef typename impl<>::type type;
};

} // namespace variadic
} // namespace tinympl

#endif // TINYMPL_VARIADIC_FILL_N_HPP
