/*
 *  Copyright (C) 2014 Denilson das Mercês Amorim (aka LINK/2012)
 *  Licensed under the Boost Software License v1.0 (http://opensource.org/licenses/BSL-1.0)
 *
 */
#pragma once
#include <datalib/data_info/either.hpp>
#include <datalib/detail/stream/fwd.hpp>
#include <datalib/detail/stream/kstream.hpp>
#include <datalib/detail/mpl/seqeach.hpp>

namespace datalib {

/*
 * Note:
 *  Order of arguments matters in the either object !!!
 *  e.g.:
 *      <int, float> ---> will detect int, then float (everything fine)
 *      <float, int> ---> will detect float but not int, since a pure int can also be a float (i.e. '10' is a float too))
 */



// CXX14 HELP-ME

/*
 *  Ugly looping ahead, awating for C++14's generic lambdas
 *  For now, let's manually write the generic functors
 */
namespace detail
{
    template<class Either>
    struct lambda_either_read_val
    {
        Either&     either;
        imemstream& stream;
        int         index;      // index of the type that should be readen ( this is not the same as which() !!!)
        bool&       result;     // outputs the result of the operation (=true) but on failure doesn't change the variable

        // Local-Scope Captures
        lambda_either_read_val(imemstream& stream, Either& either, int index, bool& result) :
            stream(stream), either(either), index(index), result(result)
        {}

        // The Functor
        template<class Integral, typename TypeWr>
        typename std::enable_if<!std::is_same<typename TypeWr::type, boost::detail::variant::void_>::value, bool>::type
        /* bool */ operator()(Integral, TypeWr)
        {
            if(index == Integral::value)    // only execute the read when it's the specified index
            {
                // Try to perform the read
                typename TypeWr::type value;
                if(stream >> value)
                {
                    // Successful, tell about the success and move the value to the either object
                    either = std::move(value);
                    this->result = true;
                }
                return false;   // stop iteration
            }
            return true;
        }

        // Avoid boost's voidness type
        template<class Integral, typename TypeWr>
        typename std::enable_if<std::is_same<typename TypeWr::type, boost::detail::variant::void_>::value, bool>::type
        /* bool */ operator()(Integral, TypeWr)
        {
            return false;
        }
    };

    template<class Either>
    struct lambda_either_find_index
    {
        const Either&           either;
        icheckstream&           stream;
        icheckstream::reposer   xrepos; // Used to reposition back on I/O failure
        int&                    result; // The output index ( this is not the same as which() !!! )
        
        // Local-Scope Captures
        lambda_either_find_index(icheckstream& stream, const Either& either, int& result) :
            stream(stream), either(either), result(result), xrepos(stream)
        {}

        // The Functor
        template<class Integral, typename TypeWr>
        typename std::enable_if<!std::is_same<typename TypeWr::type, boost::detail::variant::void_>::value, bool>::type
        /* bool */ operator()(Integral, TypeWr)
        {
            auto& is = stream;

            // Since basic_icheckstream doesn't write anything in '>>' use a null reference,
            // we won't take the time to construct a proper object
            auto& dummy_ref = *((const typename TypeWr::type*)(nullptr));

            // Check if it's possible to take the text on the stream pointer to the specified type
            if(is >> dummy_ref)
            {
                // Yep! Stop repositioning, set return index and stop iteration
                xrepos(!!is);
                this->result = Integral::value;
                return false; // stop iteration
            }
            else
            {
                // Hm, it's not this index, clear error flags and reposition the stream back
                is.clear();
                xrepos.repos();
            }
            return true;
        }

        // Avoid boost's voidness type
        template<class Integral, typename TypeWr>
        typename std::enable_if<std::is_same<typename TypeWr::type, boost::detail::variant::void_>::value, bool>::type
        /* bool */ operator()(Integral, TypeWr)
        {
            return false;
        }

    };
}




/*
 *  Input Checker
 */
template<class CharT, class Traits, class ...Args> inline
basic_icheckstream<CharT, Traits>& operator>>(basic_icheckstream<CharT, Traits>& is, const either<Args...>& either)
{
    icheckstream::sentry xsentry(is);
    if(xsentry)
    {
        // Find index to associate the input to the either object
        int result = -1;
        detail::lambda_either_find_index<std::decay<decltype(either)>::type> fun(is, either, result);
        foreach_type_variadic<Args...>()(fun);

        // If no index could be associated with the input, it indicates failure
        if(result == -1) is.setstate(std::ios::failbit);
    }
    return is;
}


/*
 *  Input
 */
template<class CharT, class Traits, class ...Args> inline
basic_imemstream<CharT, Traits>& operator>>(basic_imemstream<CharT, Traits>& is, either<Args...>& either)
{
    basic_imemstream<CharT, Traits>::sentry xsentry(is);
    if(xsentry)
    {
        int type_index = -1;

        // Make a stream checker with the current stream position (because we need to know the type we need to read in the first place)
        auto tell = std::streamoff(is.rdbuf()->pubseekoff(0, std::ios::cur, std::ios::in));
        basic_icheckstream<CharT, Traits>  icheck((char*)(is.rdbuf()->buffer()) + tell, size_t(is.rdbuf()->size() - tell));
        // Find the index of the type we need to read (this index isn't the which()!)
        detail::lambda_either_find_index<std::decay<decltype(either)>::type> index_fun(icheck, either, type_index);
        foreach_type_variadic<Args...>()(index_fun);

        if(type_index != -1)
        {
            // Read from the stream the specified type into the either object
            bool reading_result = false;
            detail::lambda_either_read_val<std::decay<decltype(either)>::type> xreader_fun(is, either, type_index, reading_result);
            foreach_type_variadic<Args...>()(xreader_fun);
            if(!reading_result) is.setstate(std::ios::failbit); // :(
        }
        else
        {
            is.setstate(std::ios::failbit);
        }
    }
    return is;
}

}

namespace std {

/*
 *  Input
 */
template<class CharT, class Traits, class ...Args> inline
std::basic_istream<CharT, Traits>& operator>>(std::basic_istream<CharT, Traits>& is_, datalib::either<Args...>& either)
{
    auto& is = static_cast<datalib::imemstream&>(is_);   // unsafe
    is >> either;
    return is;
}

} // namespace datalib


/*
 *  Output is implemented by the either class itself
 */

