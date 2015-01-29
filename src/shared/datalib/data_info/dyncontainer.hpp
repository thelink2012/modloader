/*
 *  Copyright (C) 2014 Denilson das Mercês Amorim (aka LINK/2012)
 *  Licensed under the Boost Software License v1.0 (http://opensource.org/licenses/BSL-1.0)
 *
 */
#pragma once
#include <datalib/data_info.hpp>
#include <datalib/detail/stream/fwd.hpp>

namespace datalib {

// Specialize this for dynamic containers
template<typename T>
struct is_dyncontainer : std::false_type
{};


// Dynamic containers data infos should derive from this
// ValueType is the type of value the container holds and EstimatedSize is the estimated number of elements the container will hold
template<class ContainerType, std::size_t EstimatedSize = 9>
struct data_info_dyncontainer : data_info_base
{
    using container_type = ContainerType;
    static const int estimated_elements = EstimatedSize;
    static const int complexity = estimated_elements * data_info<typename container_type::value_type>::complexity;

    // Performs precomparision (that's beforing comparing anything else, perform this cheap comparision)
    struct precompare
    {
        template<class Container>
        static bool equal_to(const Container& a, const Container& b)
        {
            return a.size() == b.size();
        }
    };
};


} // namespace datalib
