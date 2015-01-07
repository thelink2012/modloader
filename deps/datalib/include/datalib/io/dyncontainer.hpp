/*
 *  Copyright (C) 2014 Denilson das Mercês Amorim (aka LINK/2012)
 *  Licensed under the Boost Software License v1.0 (http://opensource.org/licenses/BSL-1.0)
 *
 */
#pragma once
#include <datalib/data_info/dyncontainer.hpp>
#include <datalib/detail/stream/fwd.hpp>

//
// NOTICE:
//      By including just this IO header you won't be able to do IO with std::vector, std::list and such,
//      you need to include their respective data_info headers.
//

namespace datalib {

/*
 *  Input Checker
 */
template<class CharT, class Traits, class ContainerType, typename = std::enable_if<is_dyncontainer<ContainerType>::value>::type>
inline
datalib::basic_icheckstream<CharT, Traits>& operator>>(datalib::basic_icheckstream<CharT, Traits>& is, const ContainerType& cont)
{
    using reposer_t = datalib::basic_icheckstream<CharT, Traits>::reposer;
    datalib::basic_icheckstream<CharT, Traits>::reposer xrepos(is, true);
    datalib::basic_icheckstream<CharT, Traits>::sentry  xsentry(is);
    if(xsentry)
    {
        bool good = !!is;
        while(good)
        {
            reposer_t repos(is);
            is >> *(const ContainerType::value_type*)(nullptr);
            good = !!is;
            repos(good);    // put the stream pointer back to where it was if the stream failed to read the previous element
        }
        is.clear();
    }
    return is;
}

}

namespace std {

/*
 *  Input
 */
template<class CharT, class Traits, class ContainerType, typename = std::enable_if<is_dyncontainer<ContainerType>::value>::type>
inline
std::basic_istream<CharT, Traits>& operator>>(std::basic_istream<CharT, Traits>& is, ContainerType& cont)
{
    std::basic_istream<CharT, Traits>::sentry  xsentry(is);
    if(xsentry)
    {
        ContainerType::value_type value;
        bool good = !!is;
        cont.clear();
        while(good)
        {
            auto tell = is.tellg();
            if(is >> value)
                cont.emplace(cont.end(), std::move(value));
            
            good = !!is;
            if(!good) { is.clear(); is.seekg(tell); }   // seekg fails when flag's failed
        }
        is.clear(); // needs to clear again in case seekg has set the flag
    }
    return is;
}

/*
 *  Output
 */
template<class CharT, class Traits, class ContainerType, typename = std::enable_if<is_dyncontainer<ContainerType>::value>::type>
inline
std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const ContainerType& cont)
{
    std::basic_ostream<CharT, Traits>::sentry xsentry(os);
    if(xsentry)
    {
        for(auto it = cont.begin(); it != cont.end(); ++it)
        {
            if(os << *it)
            {
                if((os << ' ').fail())
                    break;
            }
            else break;
        }
    }
    return os;
}

}
