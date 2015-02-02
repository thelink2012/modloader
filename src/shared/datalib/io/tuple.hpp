/*
 *  Copyright (C) 2014 Denilson das Mercês Amorim (aka LINK/2012)
 *  Licensed under the Boost Software License v1.0 (http://opensource.org/licenses/BSL-1.0)
 *
 */
#pragma once
#include <datalib/data_info/tuple.hpp>
#include <datalib/detail/stream/fwd.hpp>
#include <datalib/detail/stream/kstream.hpp>
#include <datalib/detail/mpl/seqeach.hpp>

namespace datalib {

// CXX14 HELP-ME

/*
 *  Ugly looping ahead, awating for C++14's generic lambdas
 *  For now, let's manually write the generic functors
 */
namespace detail
{
    template<class Tuple>
    struct lambda_tuple_write_val
    {
        Tuple&          tuple;
        std::ostream&   stream;

        // Local-Scope Captures
        lambda_tuple_write_val(std::ostream& stream, Tuple& tuple) :
            stream(stream), tuple(tuple) {}

        // The functor
        template<typename Integral, typename TypeWr>
        bool operator()(Integral, TypeWr, typename TypeWr::type& item)
        {
            if(stream << item)
            {
                print_separator<typename TypeWr::type>(stream);
            }
            return !!stream;
        }
    };

    template<class Tuple>
    struct lambda_tuple_read_val
    {
        Tuple&      tuple;
        std::istream& stream;

        // Local-Scope Captures
        lambda_tuple_read_val(std::istream& stream, Tuple& tuple) :
            stream(stream), tuple(tuple) {}

        // The functor
        template<typename Integral, typename TypeWr>
        bool operator()(Integral, TypeWr, typename TypeWr::type& item)
        {
            stream >> item;
            return !!stream;
        }
    };

    template<class Tuple>
    struct lambda_tuple_check_val
    {
        const Tuple&            tuple;
        icheckstream&           stream;
        icheckstream::reposer&  xrepos;

        // Local-Scope Captures
        lambda_tuple_check_val(icheckstream& stream, const Tuple& tuple, icheckstream::reposer& xrepos) :
            stream(stream), tuple(tuple), xrepos(xrepos) {}

        // The functor
        template<typename Integral, typename TypeWr>
        bool operator()(Integral, TypeWr, typename TypeWr::type& item)
        {
            stream >> item;
            return !!stream;
        }
    };
}


} // namespace datalib

namespace datalib {

/*
 *  Input Checker
 */
template<class CharT, class Traits, class ...Args> inline
datalib::basic_icheckstream<CharT, Traits>& operator>>(datalib::basic_icheckstream<CharT, Traits>& is, const std::tuple<Args...>& tuple)
{
    datalib::basic_icheckstream<CharT, Traits>::reposer xrepos(is, true);
    datalib::basic_icheckstream<CharT, Traits>::sentry  xsentry(is);
    if(xsentry)
    {
        datalib::detail::lambda_tuple_check_val<std::decay<decltype(tuple)>::type> fun(is, tuple, xrepos);
        datalib::foreach_in_tuple(const_cast<std::tuple<Args...>&>(tuple), fun);
        xrepos(!!is);
    }
    return is;
}

}

namespace std {

/*
 *  Input
 */
template<class CharT, class Traits, class ...Args> inline
std::basic_istream<CharT, Traits>& operator>>(std::basic_istream<CharT, Traits>& is, std::tuple<Args...>& tuple)
{
    std::basic_istream<CharT, Traits>::sentry xsentry(is);
    if(xsentry)
    {
        datalib::detail::lambda_tuple_read_val<std::decay<decltype(tuple)>::type> fun(is, tuple);
        datalib::foreach_in_tuple(tuple, fun);
    }
    return is;
}

/*
 *  Output
 */
template<class CharT, class Traits, class ...Args> inline
std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const std::tuple<Args...>& tuple)
{
    using basic_ostream = std::basic_ostream<CharT, Traits>;
    basic_ostream::sentry xsentry(os);
    if(xsentry)
    {
        auto& ncv_tuple = const_cast<std::tuple<Args...>&>(tuple);
        datalib::detail::lambda_tuple_write_val<std::decay<decltype(tuple)>::type> fun(os, ncv_tuple);
        datalib::foreach_in_tuple(ncv_tuple, fun);
    }
    return os;
}


}
