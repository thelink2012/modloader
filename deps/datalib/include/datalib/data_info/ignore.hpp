/*
 *  Copyright (C) 2014 Denilson das Mercês Amorim (aka LINK/2012)
 *  Licensed under the Boost Software License v1.0 (http://opensource.org/licenses/BSL-1.0)
 *
 */
#pragma once
#include <datalib/data_info/default.hpp>    // cannot include from <datalib/data_info.hpp> because it includes me

namespace datalib {

template<class T>
struct ignore_traits;


/*
 *  Ignore object
 *  This objects takes mostly no space in memory and when doing I/O with it it ignores the input and gives a default output
 *  Traits help in finding out the default output and what to do with the input.
 */
template<typename T, class Traits = ignore_traits<T>>
struct ignore
{
    bool operator==(const ignore& rhs) const { return true; }     // Always
    bool operator<(const ignore& rhs)  const { return false; }    // ...equal
};

/*
 *  Default traits for the ignore class
 */
template<class T>
struct ignore_traits
{
    static T output() { return T();  }   // Called to know what should it output to the file
};




/*
 *  data_info<> specialization for 'ignore<T, Traits>'
 */
template<typename T, class Traits>
struct data_info<ignore<T, Traits>> : data_info_base
{
    static const int complexity = 0;
};


} // namespace datalib

