/*
 *  Copyright (C) 2014 Denilson das Mercês Amorim (aka LINK/2012)
 *  Licensed under the Boost Software License v1.0 (http://opensource.org/licenses/BSL-1.0)
 *
 */
#pragma once
#include "type_wrapper.hpp"
#include <functional>
#include <algorithm>
#include <cmath>

//
//
// floating point class object with special comparision using epsilons
//
// A good read on floating point comparision:
// http://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
// Check also the other related articles at the index ('Previously on this channel')
//
//




/*
 *  floating_point_comparer
 *      This is a helper for 'basic_floating_point'
 *      Special floating point comparers to be used in 'basic_floating_point' template
 *      Use either default_epsilon, relative_epsilon or absolute_epsilon
 *
 *      no_epsilon does not perform epsilon comparision
 *      relative_epsilon is a relative epsilon comparision, that's the epsilon depends on the number being compared (recommended)
 *      absolute_epsilon is a fixed epislon for all numbers
 *
 */
struct floating_point_comparer
{
    private:

        // Internal stuff
        struct detail
        {
            // Performs the comparision between 'a' and 'b' using their 'delta' by using absolute/fixed epsilon comparision
            // @Pred is the predicate to compare the delta with the epsilons
            // @Ep is the epsilon values
            template<class Pred, class Ep>
            struct absolute_epsilon_checker
            {
                template<class T>
                bool operator()(const T& delta, const T&, const T&) const
                {
                    Pred pred;
                    return pred(delta, Ep::absolute_epsilon()); // For numbers near zero
                }
            };
            
            // Performs the comparision between 'a' and 'b' using their 'delta' by using relative epsilon comparision
            // @Pred is the predicate to compare the delta with the epsilons
            // @Ep is the epsilon values
            template<class Pred, class Ep>
            struct relative_epsilon_checker
            {
                template<class T>
                bool operator()(const T& delta, const T& a, const T& b) const
                {
                    Pred pred;
                    return absolute_epsilon_checker<Pred, Ep>()(delta, a, b) ? // For numbers near zero
                        true : pred(delta, ((std::max)(std::abs(a), std::abs(b)) * Ep::relative_epsilon())); // For numbers farther from zero
                }
            };

            // Episiloner contains a bunch of predicates (less, greater, equal_to) which uses epsilon comparision to determine equality
            // @T is the type of floating-point
            // @Ep is the epsilon values
            // @EpsilonChecker is either 'relative_epsilon_checker' or 'absolute_epsilon_checker'
            template<typename T, class Ep, template<class, class> class EpsilonChecker>
            struct epsiloner
            {
                // Checks wether 'a' and 'b' are (almost) equal to each other
                // The 'delta' is the distance between them (absolute)
                static bool almost_equal(const T& delta, const T& a, const T& b)
                {
                    return EpsilonChecker<std::less_equal<T>, Ep>()(delta, a, b);  // When the delta is lesser or equal the epsilon, we're almost equal
                }

                // 'equal_to' predicate
                struct equal_to
                {
                    bool operator()(const T& a, const T& b) const
                    {
                        auto delta = std::abs(a - b);
                        return almost_equal(delta, a, b);
                    }
                };

                // 'less' predicate
                struct less
                {
                    bool operator()(const T& a, const T& b) const
                    {
                        auto diff = (a - b);
                        if(diff < 0)    // The difference between 'a' and 'b' is negative, so we're lesser
                        {
                            auto delta = std::abs(diff);
                            return !almost_equal(delta, a, b); // We shouldn't be almost equal, this is not 'less_equal'
                        }
                        return false;
                    }
                };

                // 'greater' predicate
                struct greater
                {
                    bool operator()(const T& a, const T& b) const
                    {
                        auto diff = (a - b);
                        if(diff > 0)    // The difference between 'a' and 'b' is positive, so we're greater
                        {
                            auto delta = std::abs(diff);
                            return !almost_equal(delta, a, b); // We shouldn't be almost equal, this is not 'greater_equal'
                        }
                        return false;
                    }
                };
            };
        };



    public:

        // Epsilon systems to be used in relative_epsilon and absolute_epsilon templates
        struct epsilon
        {
            // Default values epsilon
            template<typename T>
            struct default_epsilon
            {
                static T absolute_epsilon() { return std::numeric_limits<T>::epsilon(); }
                static T relative_epsilon() { return std::numeric_limits<T>::epsilon(); }
            };

            // Pre-defined epsilon by the user (Epsilon = Mantissa * 10^Exp)
            template<typename T, int Mantissa, int Exp>
            struct defined_epsilon
            {
                static T absolute_epsilon()
                {
                    static const T epsilon = T(Mantissa) * std::pow(T(10), T(Exp));
                    return epsilon;
                }
            };
        };

        // Doesn't perform epsilon comparision
        template<typename T>
        struct no_epsilon
        {
            using equal_to = std::equal_to<T>;
            using less     = std::less<T>;
            using greater  = std::greater<T>;
        };

        // Performs relative epsilon comparision (depends on the number) using the 'Ep' epsilon values
        template<typename T, class Ep = epsilon::default_epsilon<T>>
        using relative_epsilon = detail::epsiloner<T, Ep, detail::relative_epsilon_checker>;

        // Performs absolute epsilon comparision (fixed) using the 'Ep' epsilon values
        template<typename T, class Ep = epsilon::default_epsilon<T>>
        using absolute_epsilon = detail::epsiloner<T, Ep, detail::absolute_epsilon_checker>;
};


/*
 *  basic_floating_point_traits
 *      Base for basic_floating_point. This perform comparisions using the FpComparer object (see floating_point_comparer::*)
 */
template<typename T, class FpComparer>
struct basic_floating_point_traits : public type_wrapper_base<T>
{
    protected:

        bool equal_to(const T& a1, const T& a2)
        { return FpComparer::equal_to()(a1, a2); }
        bool less(const T& a1, const T& a2)
        { return FpComparer::less()(a1, a2); }
        bool greater(const T& a1, const T& a2)
        { return FpComparer::greater()(a1, a2); }

        bool not_equal_to(const T& a1, const T& a2)
        { return !this->equal_to(a1, a2); }
        bool less_equal(const T& a1, const T& a2)
        { return !this->greater(a1, a2); }
        bool greater_equal(const T& a1, const T& a2)
        { return !this->less(a1, a2); }
};

/*
 *  basic_floating_point
 *      Wrapper around a floating point type in a class, which performs comparision by epsilon
 *      No epsilon comparision is performed by default (Comparer = floating_point_comparer::no_epsilon)
 */
template<typename T, class Comparer = floating_point_comparer::no_epsilon<T>>
using basic_floating_point = type_wrapper<T, basic_floating_point_traits<T, Comparer>>;


