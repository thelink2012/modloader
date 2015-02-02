/*
 *  Copyright (C) 2014 Denilson das Mercês Amorim (aka LINK/2012)
 *  Licensed under the Boost Software License v1.0 (http://opensource.org/licenses/BSL-1.0)
 *
 */
#pragma once
#include <tuple>
#include <tinympl/sequence.hpp>
#include <tinympl/as_sequence.hpp>
#include <tinympl/at.hpp>

namespace datalib {

//
// Internal details
//
namespace detail
{
    template<class T>
    struct /* anonymous */ type_wrapper
    {
        using type = T;
    };

    template<int N, class Functor, typename Type, typename... Types>
    inline void foreach_type_asfunc(Functor& functor)
    {
        if(functor(std::integral_constant<int, N>(), type_wrapper<Type>()))
            return foreach_type_asfunc<N+1, Functor, Types...>(functor);
    }

    template<int N, class Functor>
    inline void foreach_type_asfunc(Functor& functor)
    {
    }

    template<int N, class Functor, class Tuple, typename Type, typename... Types>
    inline void foreach_in_tuple(Tuple& tuple, Functor& functor)
    {
        using std::get;
        if(functor(std::integral_constant<int, N>(), type_wrapper<Type>(), get<N>(tuple)))
            return foreach_in_tuple<N+1, Functor, Tuple, Types...>(tuple, functor);
    }

    template<int N, class Functor, class Tuple>
    inline void foreach_in_tuple(Tuple& tuple, Functor& functor)
    {
    }

    template<size_t Max, class Functor, size_t N>
    inline void foreach_index(std::integral_constant<size_t, Max>, std::integral_constant<size_t, N>, Functor& functor)
    {
        if(functor(std::integral_constant<size_t, N>()))
            return foreach_index<Max, Functor>(std::integral_constant<size_t, Max>(), std::integral_constant<size_t, N+1>(), functor);
    }

    template<size_t Max, class Functor>
    inline void foreach_index(std::integral_constant<size_t, Max>, std::integral_constant<size_t, Max>, Functor& functor)
    {
    }
}




// Calls the 'functor' foreach type in 'Types'
// The functor must have the prototype [(std::integral_constant<>, TypeWrapper)], where TypeWrapper constains a ::type typedef speciying the type
template<class Functor, typename... Types>
inline void foreach_type_asfunc(Functor& functor)
{
    return detail::foreach_type_asfunc<0, Functor, Types...>(functor);
}

// Calls the 'functor' foreach value/type in the 'tuple'
// The functor must have the prototype [(std::integral_constant<>, TypeWrapper, const TypeWrapper::type&)], where TypeWrapper constains a ::type typedef speciying the type
template<class Functor, typename... Types>
inline void foreach_in_tuple(std::tuple<Types...>& tuple, Functor& functor)
{
    return detail::foreach_in_tuple<0, Functor, std::tuple<Types...>, Types...>(tuple, functor);
}

// Calls the 'functor' N times
// The functor must have the prototype [(std::integral_constant<>)]
template<size_t N, class Functor>
inline void foreach_index(Functor& functor)
{
    return detail::foreach_index<N, Functor>(std::integral_constant<size_t, N>(), std::integral_constant<size_t, 0>(), functor);
}

/*
 *  foreach_type_variadic
 *      Functor which calls the other 'functor' foreach type in 'Types'
 */
template<typename... Types>
struct foreach_type_variadic
{
    protected:
        template<class Functor>
        void exec(Functor& functor) const
        {
            return foreach_type_asfunc<Functor, Types...>(functor);
        }

    public:
        template<class Functor>
        void operator()(Functor& functor) const
        {
            return this->exec(functor);
        }
};

/*
 *  foreach_type
 *      Specialization for any kind of typelist (std::tuple, std::pair, etc)
 */
template<typename TypeList>
struct foreach_type : foreach_type<typename tinympl::as_sequence<TypeList>::type>
{
    template<class Functor>
    void operator()(Functor& functor) const
    {
        return this->exec(functor);
    }
};

/*
 *  foreach_type
 *      Specialization for tinympl::sequence typelist
 */
template<typename... Types>
struct foreach_type<tinympl::sequence<Types...>> : foreach_type_variadic<Types...>
{
    template<class Functor>
    void operator()(Functor& functor) const
    {
        return this->exec(functor);
    }
};



} // namespace datalib
