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

#ifndef TINYMPL_ALGORITHM_VARIADIC_HPP
#define TINYMPL_ALGORITHM_VARIADIC_HPP

#include <tinympl/variadic/accumulate.hpp>
#include <tinympl/variadic/all_of.hpp>
#include <tinympl/variadic/any_of.hpp>
#include <tinympl/variadic/copy.hpp>
#include <tinympl/variadic/copy_if.hpp>
#include <tinympl/variadic/copy_n.hpp>
#include <tinympl/variadic/count.hpp>
#include <tinympl/variadic/count_if.hpp>
#include <tinympl/variadic/fill_n.hpp>
#include <tinympl/variadic/find.hpp>
#include <tinympl/variadic/find_if.hpp>
#include <tinympl/variadic/generate_n.hpp>
#include <tinympl/variadic/is_unique.hpp>
#include <tinympl/variadic/left_fold.hpp>
#include <tinympl/variadic/max_element.hpp>
#include <tinympl/variadic/min_element.hpp>
#include <tinympl/variadic/none_of.hpp>
#include <tinympl/variadic/remove.hpp>
#include <tinympl/variadic/remove_if.hpp>
#include <tinympl/variadic/replace.hpp>
#include <tinympl/variadic/replace_if.hpp>
#include <tinympl/variadic/reverse.hpp>
#include <tinympl/variadic/right_fold.hpp>
#include <tinympl/variadic/sort.hpp>
#include <tinympl/variadic/transform.hpp>
#include <tinympl/variadic/unique.hpp>

// For backward compatibility
#include <tinympl/bind.hpp>
#include <tinympl/functional.hpp>
#include <tinympl/variadic.hpp>

namespace tinympl {
namespace variadic {

/**
 * \defgroup VarAlgs Variadic algorithms
 * Algorithms which operate on variadic templates
 * @{
 */

  /**
   * \defgroup VarNonModAlgs Non-modifying sequence operations
   * Algorithms which analyze a sequence without producing an output sequence
   */

  /**
   * \defgroup VarModAlgs Modifying sequence operations
   * Algorithms which produce an output sequence
   */

  /**
   * \defgroup VarMaxMin Minimum/maximum operations
   * Algorithms which compute the minimum/maximum of a sequence
   */

  /**
   * \defgroup VarSort Sorting operations
   * Algorithms to sort a sequence.
   */

  /**
   * \defgroup VarSet Set operations (on unsorted sequences)
   * Algorithms which perform set operations.
   * \note Unlike the `std` counterparts, these algorithms do not require an
  ordering of the elements.
   */

/**
 * \defgroup VarFold Folding operations
 * Algorithms which perform reduction operations on a sequence.
 */

/** @} */


}
}

#endif // TINYMPL_ALGORITHM_VARIADIC_HPP
