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

#ifndef TINYMPL_VARIADIC_GENERATE_N_HPP
#define TINYMPL_VARIADIC_GENERATE_N_HPP

#include <type_traits>
#include <cstddef>

namespace tinympl {
namespace variadic {

/**
 * \ingroup VarModAlgs
 * \class generate_n
 * \brief Generate n elements using a given generator metafunction
 * \param n The number of elements to generate
 * \param Gen The generator. `Gen< std::integral_constant<int,i> >::type` must
be a valid expression.
 * \param Out the output sequence type
 * \return `generate_n<...>::type` is a type templated from `Out` constructed
with n elements generated with `Gen< int_<0> >, Gen< int_<1> >, ... Gen<
int_<n-1> >`
 * \sa tinympl::generate_n
 */
template<std::size_t n,
    template<class ...> class Gen,
    template<class ...> class Out> struct generate_n;

template< std::size_t n, template<class ...> class Gen, template<class ...>
class Out>
struct generate_n {
private:
    template<int i, class ... Ts>
    struct impl {
        typedef typename Gen< std::integral_constant<int, i> >::type new_type;
        typedef typename generate_n < n - 1, Gen, Out >::
            template impl < i + 1, Ts..., new_type >::type type;
    };

    template<std::size_t, template<class ...> class, template<class ...> class>
    friend struct generate_n;

public:
    typedef typename impl<0>::type type;
};

template< template<class ...> class Gen, template<class ...> class Out>
struct generate_n<0, Gen, Out> {
private:
    template<int i, class ... Ts>
    struct impl {
        typedef Out<Ts...> type;
    };

    template<std::size_t, template<class ...> class, template<class ...> class>
    friend struct generate_n;

public:
    typedef typename impl<0>::type type;
};

} // namespace variadic
} // namespace tinympl

#endif // TINYMPL_VARIADIC_GENERATE_N_HPP
