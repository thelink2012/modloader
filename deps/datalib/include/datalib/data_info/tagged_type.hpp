/*
 *  Copyright (C) 2014 Denilson das Mercês Amorim (aka LINK/2012)
 *  Licensed under the Boost Software License v1.0 (http://opensource.org/licenses/BSL-1.0)
 *
 */
#pragma once
#include <datalib/data_info/default.hpp>    // cannot include from <datalib/data_info.hpp> because it includes me
#include <utility>

namespace datalib {

/*
 *  Tag object
 *      Intermediate object for tagging a pair
 */
template<class Tag>
struct tag_t
{
    using tag_type = Tag;
    bool operator==(const tag_t& rhs) const { return true; }     // Always
    bool operator<(const tag_t& rhs)  const { return false; }    // ...equal
};

// Aliasing for a tagged type
template<class T, class Tag>
using tagged_type = std::pair<T, tag_t<Tag>>;

// Gets the object wrapped in the tagged object
template<class T, class Tag>
inline T& get(tagged_type<T, Tag>& tt)
{
    return tt.first;
}

// Gets the object wrapped in the tagged object
template<class T, class Tag>
inline const T& get(const tagged_type<T, Tag>& tt)
{
    return tt.first;
}

// Makes a tagged type object in place
template<class TaggedType, class... Args>
inline const TaggedType make_tagged_type(Args&&... args)
{
    return TaggedType(std::piecewise_construct,
        std::forward_as_tuple(std::forward<Args>(args)...),
        std::forward_as_tuple());
}




/*
 *  tagged_type_precomp in case of HasPrecomp=true
 *      Auxiliar to precompare data infos that do have a precomparer
 */
template<bool HasPrecomp, class T, class Tag>
struct tagged_type_precomp
{
    // Performs precomparision (that's beforing comparing anything else, perform this cheap comparision)
    struct precompare
    {
        static bool equal_to(const tagged_type<T, Tag>& t1, const tagged_type<T, Tag>& t2)
        {
            return data_info<T>::precompare::equal_to(get(t1), get(t2));
        }
    };
};

/*
 *  tagged_type_precomp in case of HasPrecomp=false
 *      Auxiliar to precompare data infos that do not have a precomparer
 */
template<class T, class Tag>
struct tagged_type_precomp<false, T, Tag>
{
    using precompare = data_info_base::precompare;
};

/*
 *  data_info<> specialization for 'tagged_type<T, Tag>'
 */
template<typename T, class Tag>
struct data_info<tagged_type<T, Tag>> : data_info<T>
{
    using precompare = typename tagged_type_precomp<
                                    std::is_class<data_info<T>::precompare>::value, T, Tag>::precompare;
};

}
