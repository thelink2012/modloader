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
// ValueType is the type of value the container holds
// AllPreComp is whether during precompare we should precompare all elements in the container (not recommended on unordered_ containers)
// EstimatedSize is the estimated number of elements the container will hold
template<class ContainerType, bool AllPreComp = false, std::size_t EstimatedSize = 9>
struct data_info_dyncontainer : data_info_base
{
    using container_type = ContainerType;
    static const int estimated_elements = EstimatedSize;
    static const int complexity = estimated_elements * data_info<typename container_type::value_type>::complexity;

    static bool precompare(const ContainerType& a, const ContainerType& b)
    {
        return dyncontainer_precompare(std::integral_constant<bool, AllPreComp>(), a, b);
    }

private:
    static bool dyncontainer_precompare(std::false_type, const ContainerType& a, const ContainerType& b)    // !AllPreComp
    {
        return (a.size() == b.size());
    }

    static bool dyncontainer_precompare(std::true_type, const ContainerType& a, const ContainerType& b) // AllPreComp
    {
        if(a.size() == b.size())
        {
            for(auto ita = a.begin(), itb = b.begin(); ita != a.end(); ++ita, ++itb)
            {
                if(!datalib::precompare(*ita, *itb))
                    return false;
            }
            return true;
        }
        return false;
    }
};



} // namespace datalib
