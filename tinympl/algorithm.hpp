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

#ifndef TINYMPL_ALGORITHM_HPP
#define TINYMPL_ALGORITHM_HPP

#include <tinympl/accumulate.hpp>
#include <tinympl/all_of.hpp>
#include <tinympl/any_of.hpp>
#include <tinympl/at.hpp>
#include <tinympl/copy.hpp>
#include <tinympl/copy_if.hpp>
#include <tinympl/copy_n.hpp>
#include <tinympl/count.hpp>
#include <tinympl/count_if.hpp>
#include <tinympl/erase.hpp>
#include <tinympl/fill_n.hpp>
#include <tinympl/find.hpp>
#include <tinympl/find_if.hpp>
#include <tinympl/generate_n.hpp>
#include <tinympl/insert.hpp>
#include <tinympl/is_unique.hpp>
#include <tinympl/join.hpp>
#include <tinympl/left_fold.hpp>
#include <tinympl/lexicographical_compare.hpp>
#include <tinympl/max_element.hpp>
#include <tinympl/min_element.hpp>
#include <tinympl/none_of.hpp>
#include <tinympl/remove.hpp>
#include <tinympl/remove_if.hpp>
#include <tinympl/replace.hpp>
#include <tinympl/replace_if.hpp>
#include <tinympl/reverse.hpp>
#include <tinympl/right_fold.hpp>
#include <tinympl/set_difference.hpp>
#include <tinympl/set_intersection.hpp>
#include <tinympl/set_union.hpp>
#include <tinympl/size.hpp>
#include <tinympl/sort.hpp>
#include <tinympl/transform.hpp>
#include <tinympl/transform2.hpp>
#include <tinympl/transform_many.hpp>
#include <tinympl/transpose.hpp>
#include <tinympl/unique.hpp>
#include <tinympl/unordered_equal.hpp>
#include <tinympl/zip.hpp>

// For backward compatibility.
#include <tinympl/algorithm_variadic.hpp>
#include <tinympl/functional.hpp>
#include <tinympl/sequence.hpp>

namespace tinympl {

/**
 * \defgroup SeqAlgs Sequence algorithms
 * Algorithms which operate on sequences
 * @{
 */

  /**
   * \defgroup SeqAlgsIntr Sequence introspection operations
   * Algorithms which perform trivial operations on sequences
   */

  /**
   * \defgroup SeqNonModAlgs Non-modifying sequence operations
   * Algorithms which analyze a sequence without producing an output sequence
   */

  /**
   * \defgroup SeqModAlgs Modifying sequence operations
   * Algorithms which produce an output sequence
   */

  /**
   * \defgroup SeqMaxMin Minimum/maximum operations
   * Algorithms which compute the minimum/maximum of a sequence and
  lexicographical compare
   */

  /**
   * \defgroup SeqSort Sorting operations
   * Algorithms to sort a sequence.
   */

  /**
   * \defgroup SeqSet Set operations (on unsorted sequences)
   * Algorithms which perform set operations.
   * \note Unlike the `std` counterparts, these algorithms do not require an
  ordering of the elements.
   */

  /**
   * \defgroup SeqFold Folding operations
   * Algorithms which perform reduction operations on a sequence.
   */

/** @} */

}

#endif // TINYMPL_ALGORITHM_HPP
