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
{ return (modloader::compare(get(a), get(b), false)); }

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
