/*
 *  Copyright (C) 2014 Denilson das Mercês Amorim (aka LINK/2012)
 *  Licensed under the Boost Software License v1.0 (http://opensource.org/licenses/BSL-1.0)
 *
 */
#pragma once
#include <type_traits>

//
//  This header contains:
//      [*] default data_info<> objects, including data_info_base
//      [*] delimopt type (delimiter for optional params in data_slice<>)
//
//

namespace datalib {

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
                                        
    // precompare should be a class/struct containing a [static equal_to(const T&, const T&)] method
    // This method should do very cheap comparision which will run before comparing any other type, it's used to avoid going further in comparisions
    // when a cheaper comparision is available before.
    // In this base data_info it is defined as a non-class type (void) so it doesn't perform any pre-comparision 
    using precompare = void;
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
};




namespace detail
{
    /*
     *  data_info_precomparer
     *      Helper to work with precomparision (data_info::precompare object)
     *      By using this you can detect if precompare is defined (i.e. is a class type not a fundamental type) and execute it
     *      This is a functor too by the way, which executes the pre-comparision (note: it returns true if precompare is not defined)
     */
    template<typename TVoid = void>
    struct data_info_precomparer
    {
        public:
            template<class T>
            struct precompare
            {
                using has_precompare_t = std::is_class<typename data_info<T>::precompare>;  // Has T::precompare as a class/struct object?
                static const bool has_precompare = has_precompare_t::value;                 // boolean constant for integral_constant above

                // Runs when has_precompare is true
                static bool exec(std::true_type, const T& a, const T& b)
                {
                    return data_info<T>::precompare::equal_to(a, b);
                }
    
                // Runs when has_precompare is false
                static bool exec(std::false_type, const T&, const T&)
                {
                    return true;
                }
    
                // Forwards the call to the exec(true, ...) or exec(false, ...) depending on wether T::precompare is defined properly
                static bool exec(const T& a, const T& b)
                {
                    return exec(has_precompare_t(), a, b);
                }
            };

        public:
            // Executes the precomparision, returns true if T::precompare is not defined as a class type
            template<class T>
            bool operator()(const T& a, const T& b) const
            {
                return precompare<T>::exec(a, b);
            }
    };
}


} // namespace datalib
