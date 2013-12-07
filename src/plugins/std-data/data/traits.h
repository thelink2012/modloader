/*
 * Copyright (C) 2013  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 *
 */
#ifndef TRAITS_H
#define	TRAITS_H

#include <DataTraits.hpp>
using namespace DataTraitsNamespace;

#include <string>
#include <cstdint>

/* Base trait for our purposes... we need to have a path */
template<class T>
struct DataTraitsBase : public DataTraits<T>
{
    std::string path;
    
    // child must implement [bool LoadData()] loading 'path' file.
    // normally this implementation is like
    // bool LoadData() { return DataTraits::LoadData(path.c_str(), myParser); }
};

/* Our data structures will be at a data namespace */
namespace data
{
    /* Rules on this namespace:
     *      [*] Every object starting with S is a POD-type
     */

    /* We will need some wrapper */
    namespace util
    {
        /* This wrapper will provide a string buffer with a hash */
        template<size_t N>
        struct SString
        {
            //static const size_t n = N;
            char     buf[N];    /* this must be on the top */
            uint32_t hash;

            // TODO
            
            void Recalc()
            {
                hash = 0; // TODO
            }

            bool operator==(const SString& sz) const
            {
                return this->hash == sz.hash;
            }

            bool operator<(const SString& sz) const
            {
                return this->hash < sz.hash;
            }
        };

        /* This wrapper will provide a floating-point buffer.
         * The real purposes of this is to provide good comparision between them
         */
        template<class T>
        struct SComplex
        {
            typedef T value_type;

            value_type f;

            // TODO
            
            // Must make those comparisions better

            bool operator==(const SComplex& sz) const
            {
                const T epsilon = std::numeric_limits<T>::epsilon();
                return std::abs(this->f - sz.f) <= epsilon;
            }

            bool operator<(const SComplex& sz) const
            {
                const T epsilon = std::numeric_limits<T>::epsilon();
                return ( ((this->f - sz.f) < epsilon) && (fabs(this->f - sz.f) > epsilon) );
            }
        };
        template<size_t N>
        struct SVector
        {
            SComplex<float> c[N];
            
            template<class Functor>
            bool comp(const SVector& b) const
            {
                Functor comparer;
                for(size_t i = 0; i < N; ++i)
                {
                    if(!(comparer(c[i], b.c[i])))
                        return false;
                }
                return true;                
            }
            
            bool operator==(const SVector& b) const
            {
                return comp<std::equal_to<SComplex<float>>>(b);
            }
            
            bool operator<(const SVector& b) const
            {
                return comp<std::less<SComplex<float>>>(b);
            }
            
            SComplex<float>& operator[](size_t i)
            {
                return c[i];
            }
            
            const SComplex<float>& operator[](size_t i) const
            {
                return c[i];
            }
        };

        /* typedef the wrappers */
        typedef uint32_t        hash;
        typedef uint32_t        uhash;      /* upper case hash */
        typedef SString<32>     string32;
        typedef SComplex<float> complex;
        typedef SVector<2>      vec2;
        typedef SVector<3>      vec3;
        typedef SVector<4>      vec4;
        typedef int             integer;
        typedef unsigned int    flags;
        typedef unsigned int    obj_id;


    }
}

#define OPI(item, op)   (a.item op b.item)
#define EQ(item)        OPI(item, ==)
#define LE(item)        OPI(item, <)

/* Include data structures */
#include "ide.h"
#include "ipl.h"

#undef OPI
#undef EQ
#undef LE


#endif	/* TRAITS_H */

