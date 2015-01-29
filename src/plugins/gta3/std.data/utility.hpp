/*
 * Copyright (C) 2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#include "datalib.hpp"
#include <modloader/util/hash.hpp>
#include <modloader/util/container.hpp>
// shall not include data.hpp or cache.hpp because we are included from stdinc

//
//  Case insensitive string type
//
struct tag_insen_t {};

template<class T>
using insen = tagged_type<T, tag_insen_t>;

template<class T>
bool operator==(const insen<T>& a, const insen<T>& b)
{ return (modloader::compare(get(a), get(b), false)) == 0; }

template<class T>
bool operator<(const insen<T>& a, const insen<T>& b)
{ return (modloader::compare(get(a), get(b), false)) < 0; }

template<class... Args>
inline insen<std::string> make_insen_string(Args&&... args)
{
    return make_tagged_type<insen<std::string>>(std::forward<Args>(args)...);
}

//
// Useful types to use on our gta3 data processing
//
using dummy_value = datalib::delimopt;
using real_t = basic_floating_point<float, floating_point_comparer::relative_epsilon<float>>;
template<std::size_t N>
using vecn = std::array<real_t, N>;
using vec2 = vecn<2>;
using vec3 = vecn<3>;
using vec4 = vecn<4>;
using quat = vec4;
using bbox = std::array<vec3, 2>;
using bsphere = std::tuple<vec3, real_t>;
using rgb  = std::array<uint16_t, 3>;   // cannot be uint8 because it is actually char
using rgba = std::array<uint16_t, 4>;   // 
using modelname = insen<std::string>;
using animname = insen<std::string>;
using texname = insen<std::string>;
using labelname = insen<std::string>;

struct dummy_string_traits
{ static std::string output() { return "_";  } };
using dummy_string = datalib::ignore<std::string, dummy_string_traits>;


// Hashes a string in a case insensitive manner
inline size_t hash_model(const char* model)
{
    return modloader::hash(model, ::tolower);
}

// Hashes a string in a case insensitive manner
inline size_t hash_model(const std::string& model)
{
    return modloader::hash(model, ::tolower);
}

// Hashes a string in a case insensitive manner
template<class T>
size_t hash_model(const insen<T>& model)
{
    return hash_model(get(model));
}

//
//  UData of shared pointers
//
namespace datalib
{
    template<typename T>
    struct data_info<udata<std::shared_ptr<T>>> : data_info_base
    {
        static const int complexity = 1;    // comparing a pointer should be as quick as a fundamental type
    };
}
namespace std
{
    /*
     *  shared_ptr should fail to be readen by datalib
     */
    template<class CharT, class Traits, class T> inline
    datalib::basic_icheckstream<CharT, Traits>& operator>>(datalib::basic_icheckstream<CharT, Traits>& is, const std::shared_ptr<T>& ptr)
    {
        is.setstate(std::ios::failbit);
        return is;
    }
    template<class CharT, class Traits, class T> inline
    std::basic_istream<CharT, Traits>& operator>>(std::basic_istream<CharT, Traits>& is, const std::shared_ptr<T>& ptr)
    {
        is.setstate(std::ios::failbit);
        return is;
    }
}


// Other udata specializations
namespace datalib
{
    template<>
    struct data_info<udata<int>> : data_info<int>
    {};
}


//
//  Additional Serializers
//

// Serializer for type_wrapper<> i.e. real_t
template<class Archive, class T, class Base>
inline void serialize(Archive& ar, type_wrapper<T, Base>& tw)
{
    ar(tw.get_());
}



template<size_t N, class T, class... Types>
struct find_that_type;

template<size_t N, class T, class ThisType, class... Types>
struct find_that_type<N, T, ThisType, Types...> :
    std::conditional<std::is_same<T, ThisType>::value,
                std::integral_constant<size_t, N>,
                find_that_type<N+1, T, Types...>>::type
{};

template<size_t N, class T>
struct find_that_type<N, T>
{ static_assert(std::is_same<T, void>::value, "Type not found"); };


// Stores a list of a specific data (normally shared pointers)
template<class... Types>
struct data_list
{
    using tuple_type = std::tuple<std::list<Types>...> ;
    static const size_t data_size = std::tuple_size<tuple_type>::value;

    // Serializes our stored data
    // Notice: The serialization of a data_list should come before the serialization
    // of any data slice!!! That's because of the way cereal deals with shared pointers serialization
    // as there should be the first pointer, that will have the data stored...
    // Since other slices (and thus pointers) can be skipped by further cache reads, we should store this data before the slices!
    template<class Archive>
    void serialize(Archive& ar)
    {
        ar(data);
    }

    // Clears the content of this store
    void clear()
    {
        clear(std::integral_constant<size_t, 0>());
    }

    // Clears any data that's only being held by us
    void clear_uniques()
    {
        clear_uniques(std::integral_constant<size_t, 0>());
    }

    // Gets the list that stores the specified Type
    template<class Type>
    std::list<Type>& get()
    {
        static const size_t index = find_that_type<0, Type, Types...>::value;
        return std::get<index>(data);
    }


private:
    
    tuple_type data;

    void clear(std::integral_constant<size_t, data_size>)
    {
    }

    template<size_t N>
    void clear(std::integral_constant<size_t, N>)
    {
        std::get<N>(data).clear();
        return clear(std::integral_constant<size_t, N+1>());
    }


    // Clears any pointer that's only being held by the specified list of pointers
    template<class T>
    void clear_uniques(std::list<T>& list)
    {
        for(auto it = list.begin(); it != list.end(); )
        {
            if(it->unique())
                it = list.erase(it);
            else
                ++it;
        }
    }

    void clear_uniques(std::integral_constant<size_t, data_size>)
    {
    }

    template<size_t N>
    void clear_uniques(std::integral_constant<size_t, N>)
    {
        clear_uniques(std::get<N>(data));
        return clear_uniques(std::integral_constant<size_t, N+1>());
    }
};

//
// Enum to string and vice-versa using a mapper
//
template<typename T>
struct enum_map
{ /* specialize a [static map<string, T>& map() {}] method */ };

namespace datalib
{
    template<class T> inline
    typename std::enable_if<std::is_enum<T>::value, T&>::type
    /* T& */ from_string(const std::string& str, T& value)
    {
        auto& map = enum_map<T>::map();
        auto it = map.find(str);
        if(it != map.end()) return (value = it->second);
        throw std::invalid_argument("Invalid conversion from string to enum");
    }

    template<class T> inline
    typename std::enable_if<std::is_enum<T>::value, const std::string&>::type
    /* const std::string& */ to_string(T value)
    {
        for(auto& x : enum_map<T>::map())
        {
            if(x.second == value)
                return x.first;
        }
        throw std::invalid_argument("Invalid conversion from enum to string");
    }
}
