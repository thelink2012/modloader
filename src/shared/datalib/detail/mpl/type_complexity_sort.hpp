/*
 *  Copyright (C) 2014 Denilson das Mercês Amorim (aka LINK/2012)
 *  Licensed under the Boost Software License v1.0 (http://opensource.org/licenses/BSL-1.0)
 *
 */
#pragma once
#include <tinympl/sort.hpp>
#include <tinympl/at.hpp>
#include <tinympl/size.hpp>
#include <datalib/detail/mpl/integer_sequence.hpp>
#include <datalib/data_info.hpp>

namespace datalib {

/*
 *  type_complexity_sort
 *      Takes a typelist (std::tuple, std::pair, tinympl::sequence, etc) as input and outputs a integer_sequence which contains indices
 *      for the types in the typelist sorted by complexity order (least complexies first, most complexies later)
 */
template<class TypeList>
struct type_complexity_sort
{
    private:
        // Returns whether type 'T' is less complex than type 'U'
        template<class T, class U>
        using cmp_type_complexity = std::integral_constant<bool, (data_info<T>::complexity < data_info<U>::complexity)>;

        // Predicate for the sorting meta-function, determines which index ('Idx1' or 'Idx2') in the typelist is less complex
        template<class Idx1, class Idx2>
        using cmp_by_complexity = cmp_type_complexity<
            typename tinympl::at<Idx1::value, TypeList>::type,
            typename tinympl::at<Idx2::value, TypeList>::type>;

        // Perform the meta-sort
        using basic_sequence    = typename make_index_sequence<tinympl::size<TypeList>::value>::type;   // <- Unsorted sequence of [(0)-(N-1)] integers
        using sorted_sequence   = typename tinympl::sort<                                               // <- Sorted sequence by complexity
                                            basic_sequence,
                                            tinympl::as_sequence<basic_sequence>::template rebind,
                                            cmp_by_complexity
                                       >::type;

    public:
        // integer_sequence which contains the type indices sorted by complexity (least complexies first)
        using type = sorted_sequence;
};


} // namespace datalib
