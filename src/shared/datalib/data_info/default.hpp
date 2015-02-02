/*
 *  Copyright (C) 2014 Denilson das Mercês Amorim (aka LINK/2012)
 *  Licensed under the Boost Software License v1.0 (http://opensource.org/licenses/BSL-1.0)
 *
 */
#pragma once
#include <type_traits>
#include <iosfwd>

//
//  This header contains:
//      [*] default data_info<> objects, including data_info_base
//      [*] delimopt type (delimiter for optional params in data_slice<>)
//
//

namespace datalib {

#ifdef DATALIB_FAST_COMPILATION
#   ifndef DATALIB_DATAINFO_NOPRECOMP
#       define DATALIB_DATAINFO_NOPRECOMP
#   endif
#endif

/*
 *  delimopt
 *      Any type coming after this type is optional
 */
struct delimopt
{
    bool operator==(const delimopt& rhs) const { return true; }     // Always
    bool operator<(const delimopt& rhs)  const { return false; }    // ...equal
};


/*
 *  data_info_base
 *      Any data_info<> specialization should inherit from this base type
 *      This type defines default (common) values for the child data_info, you can then override a specific setting if needed
 *      Check the explanation for each property in the member's comments
 */
struct data_info_base
{
    // Should this type get ignored?
    static const bool ignore = false;

    // The character written after this type (could be '\0' for no separator)
    static const char separator = ' ';

    // The complexity to compare two objects of this type. Notice this is the complexity AFTER 'precompare' happened.
    // (0 means no complexity (no operation); 1 means fundamental complexity; Negative numbers are undefined behaviour)
    static const int  complexity = 1;

    // This should do a very cheap comparision which will run before comparing any other type, it's used to avoid going further in comparisions
    // when a cheaper comparision is available before.
    // NOTE: NO ONE (not even datalib) should call data_info<T>::precompare directly, use datalib::precompare instead!!!!!!
    template<class T>
    static bool precompare(const T& a, const T& b)
    {
        return true;
    }
};

/*
 *  data_info
 *      Default data_info<> specialization, used mostly by fundamental types
 */
template<typename T>
struct data_info : data_info_base
{
    static const int complexity = std::is_floating_point<T>::value?
                                        10 :                        // <- Operating on a float takes aproximately the same as operating on 10 ints
                                         1;                         // <- Fundamental complexity
};

/*
 *  data_info
 *      data_info<> specialization for void, which should be ignored and is a no-op
 */
template<>
struct data_info<void> : data_info_base
{
    static const bool ignore     = true;    // Should be ignored
    static const int  complexity = 0;       // No-op comparision
};

/*
 *  data_info
 *      data_info<> specialization for delimopt
 */
template<>
struct data_info<delimopt> : data_info_base
{
    static const bool ignore = true;        // The type should be ignored but it is handled internally to allow optional types after it
    static const int  complexity = 0;       // Unecessary since we won't touch it at all, only used by compile time compares, but yeah lets use
};


template<class T>
inline bool precompare(const T& a, const T& b)
{
#if !defined(DATALIB_DATAINFO_NOPRECOMP)
    return data_info<T>::precompare(a, b);
#else
    return true;
#endif
}

template<class T, class CharT, class Traits>
inline std::basic_ostream<CharT, Traits>& print_separator(std::basic_ostream<CharT, Traits>& os)
{
    auto separator = data_info<T>::separator;
    if(separator) os << separator;
    return os;
}

} // namespace datalib
