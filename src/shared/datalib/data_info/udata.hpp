/*
 *  Copyright (C) 2014 Denilson das Mercês Amorim (aka LINK/2012)
 *  Licensed under the Boost Software License v1.0 (http://opensource.org/licenses/BSL-1.0)
 *
 */
#pragma once
#include <datalib/data_info/tagged_type.hpp>

namespace datalib {

struct udata_tag {};

template<class T>
using udata = tagged_type<T, udata_tag>;

template<class T>
inline bool operator==(const udata<T>& a, const udata<T>& b)
{ return get(a) == get(b); }
template<class T>
inline bool operator<(const udata<T>& a, const udata<T>& b)
{ return get(a) < get(b); }

template<class T, class... Args>
inline udata<T> make_udata(Args&&... args)
{
    return udata<T>(std::piecewise_construct,
        std::forward_as_tuple(std::forward<Args>(args)...),
        std::forward_as_tuple());
}


/*
 *  data_info<> specialization for 'udata<T>'
 */
template<typename T>
struct data_info<udata<T>> : data_info_base
{
    static_assert(std::is_same<T, void>::value, /* <<--- should always evaluate to false */
            "Custom udata should have a data_info<> specialization.");
};


} // namespace datalib

