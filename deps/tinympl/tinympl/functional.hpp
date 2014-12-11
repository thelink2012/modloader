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

#ifndef TINYMPL_FUNCTIONAL_HPP
#define TINYMPL_FUNCTIONAL_HPP

#include <tinympl/int.hpp>
#include <tinympl/short.hpp>
#include <tinympl/char.hpp>
#include <tinympl/bool.hpp>
#include <tinympl/long.hpp>
#include <tinympl/plus.hpp>
#include <tinympl/multiplies.hpp>
#include <tinympl/minus.hpp>
#include <tinympl/divides.hpp>
#include <tinympl/modulus.hpp>
#include <tinympl/negate.hpp>
#include <tinympl/equal_to.hpp>
#include <tinympl/not_equal_to.hpp>
#include <tinympl/less.hpp>
#include <tinympl/greater.hpp>
#include <tinympl/less_equal.hpp>
#include <tinympl/greater_equal.hpp>
#include <tinympl/and_b.hpp>
#include <tinympl/or_b.hpp>
#include <tinympl/not_b.hpp>
#include <tinympl/logical_and.hpp>
#include <tinympl/logical_or.hpp>
#include <tinympl/logical_not.hpp>
#include <tinympl/identity.hpp>
#include <tinympl/inherit.hpp>
#include <tinympl/sizeof.hpp>
#include <tinympl/if.hpp>
#include <tinympl/apply.hpp>

namespace tinympl {

/**
 * \defgroup NewTypes Type wrappers
 * Templates which wrap a value into a type.
 */

/**
 * \defgroup Functional Metafunctions
 * Class templates which implement metafunctions
 * @{
 */

  /**
   * \defgroup Arithmetic Arithmetic operations
   * Metafunctions which perform arithmetic operations on `std::integral_constant` or equivalent types
   */

  /**
   * \defgroup Comparisons Comparisons
   * Metafunctions which perform comparisons operations on `std::integral_constant` or equivalent types
   */

  /**
   * \defgroup Logical Logical operations
   * Metafunctions which perform logical operations on `std::integral_constant` or equivalent types
   */

/** @} */

}

#endif // TINYMPL_FUNCTIONAL_HPP
