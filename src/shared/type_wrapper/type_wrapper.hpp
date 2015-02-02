/*
 *  Copyright (C) 2014 Denilson das Mercês Amorim (aka LINK/2012)
 *  Licensed under the Boost Software License v1.0 (http://opensource.org/licenses/BSL-1.0)
 *
 */
#pragma once
#include <limits>       // for std::numeric_limits
#include <functional>   // for std::hash
#include <type_traits>

/*
 *  Base Traits for a type_wrapper
 *  Implements fundamental operations for the type
 *  You might want to derive from it for your own base traits.
 */
template<typename T>
class type_wrapper_base
{
    protected:
        // Comparision
        bool equal_to(const T& a1, const T& a2)
        { return a1 == a2; }
        bool not_equal_to(const T& a1, const T& a2)
        { return a1 != a2; }
        bool less(const T& a1, const T& a2)
        { return a1 < a2; }
        bool greater(const T& a1, const T& a2)
        { return a1 > a2; }
        bool less_equal(const T& a1, const T& a2)
        { return a1 <= a2; }
        bool greater_equal(const T& a1, const T& a2)
        { return a1 >= a2; }
};

/*
 *  type_wrapper
 *      Wraps up a fundamental type 'T' into a class object
 *      You cannot inherit from this class, instead send a 'Base' in the 2nd template argument
 *      Based from http://coliru.stacked-crooked.com/view?id=f5566f15c11c52e2db7189d602cc601a-f674c1a6d04c632b71a62362c0ccfc51
 */
template<typename T, class Base = type_wrapper_base<T>>
class type_wrapper : public Base
{
    private:
        static_assert(std::is_fundamental<T>::value, "Unsupported type wrapping");  // Only fundamental types are allowed
        T value;

    public:
        // Alias for T
        using value_type = T;

        // Constructors
        type_wrapper()                  : value() {}
        type_wrapper(value_type value)  : value(value) {}

        // This should convert to T implicitly
        operator T() const      { return value; }
        // To get a reference to the value this method is needed
        T& get_()               { return value; }
        const T& get_() const   { return value; }

        // Fundamental Operators
        type_wrapper& operator=(T v)    { value = v; return *this; }
        type_wrapper& operator+=(T v)   { value += v; return *this; }
        type_wrapper& operator-=(T v)   { value -= v; return *this; }
        type_wrapper& operator*=(T v)   { value *= v; return *this; }
        type_wrapper& operator/=(T v)   { value /= v; return *this; }
        type_wrapper& operator%=(T v)   { value %= v; return *this; }
        type_wrapper& operator++()      { ++value; return *this; }
        type_wrapper& operator--()      { --value; return *this; }
        type_wrapper operator++(int)    { return type_wrapper(value++); }
        type_wrapper operator--(int)    { return type_wrapper(value--); }
        type_wrapper& operator&=(T v)   { value &= v; return *this; }
        type_wrapper& operator|=(T v)   { value |= v; return *this; }
        type_wrapper& operator^=(T v)   { value ^= v; return *this; }
        type_wrapper& operator<<=(T v)  { value <<= v; return *this; }
        type_wrapper& operator>>=(T v)  { value >>= v; return *this; }
        type_wrapper operator+() const  { return type_wrapper(+value); }
        type_wrapper operator-() const  { return type_wrapper(-value); }
        type_wrapper operator!() const  { return type_wrapper(!value); }
        type_wrapper operator~() const  { return type_wrapper(~value); }

        // Fundamental Comparision Operators
        bool operator==(const T& rhs)   { return Base::equal_to(value, rhs); }
        bool operator!=(const T& rhs)   { return Base::not_equal_to(value, rhs); }
        bool operator<(const T& rhs)    { return Base::less(value, rhs); }
        bool operator>(const T& rhs)    { return Base::greater(value, rhs); }
        bool operator<=(const T& rhs)   { return Base::less_equal(value, rhs); }
        bool operator>=(const T& rhs)   { return Base::greater_equal(value, rhs); }

        // Uses the fundamental operators to execute other operations
        friend type_wrapper operator+(type_wrapper iw, type_wrapper v)
        { return iw+=v; }
        friend type_wrapper operator+(type_wrapper iw, T v)
        { return iw+=v; }
        friend type_wrapper operator+(T v, type_wrapper iw)
        { return type_wrapper(v)+=iw; }
        friend type_wrapper operator-(type_wrapper iw, type_wrapper v)
        { return iw-=v; }
        friend type_wrapper operator-(type_wrapper iw, T v)
        { return iw-=v; }
        friend type_wrapper operator-(T v, type_wrapper iw)
        { return type_wrapper(v)-=iw; }
        friend type_wrapper operator*(type_wrapper iw, type_wrapper v)
        { return iw*=v; }
        friend type_wrapper operator*(type_wrapper iw, T v)
        { return iw*=v; }
        friend type_wrapper operator*(T v, type_wrapper iw)
        { return type_wrapper(v)*=iw; }
        friend type_wrapper operator/(type_wrapper iw, type_wrapper v)
        { return iw/=v; }
        friend type_wrapper operator/(type_wrapper iw, T v)
        { return iw/=v; }
        friend type_wrapper operator/(T v, type_wrapper iw)
        { return type_wrapper(v)/=iw; }
        friend type_wrapper operator%(type_wrapper iw, type_wrapper v)
        { return iw%=v; }
        friend type_wrapper operator%(type_wrapper iw, T v)
        { return iw%=v;}
        friend type_wrapper operator%(T v, type_wrapper iw)
        { return type_wrapper(v)%=iw; }
        friend type_wrapper operator&(type_wrapper iw, type_wrapper v)
        { return iw&=v; }
        friend type_wrapper operator&(type_wrapper iw, T v)
        { return iw&=v; }
        friend type_wrapper operator&(T v, type_wrapper iw)
        { return type_wrapper(v)&=iw; }
        friend type_wrapper operator|(type_wrapper iw, type_wrapper v)
        { return iw|=v; }
        friend type_wrapper operator|(type_wrapper iw, T v)
        { return iw|=v; }
        friend type_wrapper operator|(T v, type_wrapper iw)
        { return type_wrapper(v)|=iw; }
        friend type_wrapper operator^(type_wrapper iw, type_wrapper v)
        { return iw^=v; }
        friend type_wrapper operator^(type_wrapper iw, T v)
        { return iw^=v; }
        friend type_wrapper operator^(T v, type_wrapper iw)
        { return type_wrapper(v)^=iw; }
        friend type_wrapper operator<<(type_wrapper iw, type_wrapper v)
        { return iw<<=v; }
        friend type_wrapper operator<<(type_wrapper iw, T v)
        { return iw<<=v; }
        friend type_wrapper operator<<(T v, type_wrapper iw)
        { return type_wrapper(v)<<=iw; }
        friend type_wrapper operator>>(type_wrapper iw, type_wrapper v)
        { return iw>>=v; }
        friend type_wrapper operator>>(type_wrapper iw, T v)
        { return iw>>=v; }
        friend type_wrapper operator>>(T v, type_wrapper iw)
        { return type_wrapper(v)>>=iw; }
};


// Specializations for type_wrapper
namespace std
{
    // Specializations of std::hash for user-defined types must satisfy Hash requirements. 
    template<typename T>
    struct hash<type_wrapper<T>>
    {
        std::size_t operator()(const type_wrapper& value) const
        { return std::hash<T>()(value); }
    };

    // Specializations of std::numeric_limits must define all members declared static constexpr in the primary template,
    // in such a way that they are usable as integral constant expressions. 
    template<typename T>
    struct numeric_limits<type_wrapper<T>> : std::numeric_limits<T>
    {};

    //  Specializations of std::atomic must have a deleted copy constructor, a deleted copy assignment operator, and a constexpr value constructor. 
    // -----------> should we implement possibily lock-free std::atomic just like the fundamental compiler types?

    // Other Specialization Notes:
    // [*] None of the type traits defined in <type_traits> may be specialized for a user-defined type, except for std::common_type.
    // [*] Specializing the template std::complex for any type other than float, double, and long double is unspecified. 
};
