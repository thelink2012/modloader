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
 *  data_info<> specialization for 'tagged_type<T, Tag>'
 */

template<class TaggedType>
struct data_info_tagged;

template<typename T, class Tag>
struct data_info_tagged<tagged_type<T, Tag>> : data_info<T>
{
    // Performs cheap precomparision
    static bool precompare(const tagged_type<T, Tag>& a, const tagged_type<T, Tag>& b)
    {
        return datalib::precompare(get(a), get(b));
    }
};

template<typename T, class Tag>
struct data_info<tagged_type<T, Tag>> : data_info_tagged<tagged_type<T, Tag>>
{
};

}
