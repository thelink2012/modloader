/*
 * Copyright (C) 2013  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 *  File data traits base and utility types
 * 
 */
#ifndef TRAITS_H
#define	TRAITS_H

#include <DataTraits.hpp>
using namespace DataTraitsNamespace;

#include <string>
#include <cstdint>
#include <modloader_util_hash.hpp>

/* Base trait for our purposes... we need to have a path */
template<class ContainerType>
struct DataTraitsBase : public DataTraits<ContainerType>
{
    std::string path;
    
    /*
     *  The child must implement [bool LoadData()] loading 'path' file.
     *  Normally this implementation is like
     *      bool LoadData() { return DataTraits::LoadData(path.c_str(), MyParser);
     */
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
        /* String wrapper with a hash */
        template<size_t N> struct SString;
        
        /* SString specialization with 0 space for the string buffer, that's, it only deppends on the hash */
        template<>
        struct SString<0>
        {
            uint32_t hash;
            
            SString& operator=(const char* str)
            {
                this->hash = modloader::hash(str);
                return *this;
            }
            
            bool operator==(const SString& rhs) const
            {
                return (this->hash == rhs.hash);
            }
            
            bool operator<(const SString& rhs) const
            {
                return (this->hash < rhs.hash);
            }
            
            /* The following are dummies since we don't have the original buffer */
            
            SString& recalc()
            { return *this; }
            
            template<class TransformerFunctor>
            SString& recalc(TransformerFunctor tr)
            { return *this; }
        };
        
        /* This wrapper will provide a string buffer with a hash */
        template<size_t N>
        struct SString
        {
            /* BE SURE TO recalc THE HASH AFTER ASSIGNING SOMETHING TO buf */
            
            char     buf[N];    /* this must be on the top */
            size_t   hash;

            /* not copyable, you must copy into buf and then do recalc */
            
            bool operator==(const SString& rhs) const
            {
                return this->hash == rhs.hash;
            }

            bool operator<(const SString& rhs) const
            {
                return this->hash < rhs.hash;
            }
            
            /* Recalculates the hash from buf */
            SString& recalc()
            {
                this->hash = modloader::hash(buf);
                return *this;
            }
            
            /* Recalculates the hash from buf */
            template<class TransformerFunctor>
            SString& recalc(TransformerFunctor tr)
            {
                this->hash = modloader::hash(buf, tr);
                return *this;
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
        
        /*
         * This wrapper puts N containers to build a vector type
         * Useful for coordinates 
         */
        template<size_t N>
        struct SVector
        {
            SComplex<float> c[N];
            
            /* Main comparisions operators */
            
            bool operator==(const SVector& b) const
            {
                for(size_t i = 0; i < N; ++i)
                {
                    if(!(c[i] == b.c[i])) return false;
                }
                return true;      
            }
            
            bool operator<(const SVector& b) const
            {
                for(size_t i = 0; i < N; ++i)
                {
                    if(c[i] < b.c[i]) return true;
                }
                return false;     
            }
            
            /* Returns a SComplex by index */
            SComplex<float>& operator[](size_t i)
            {
                return c[i];
            }
            
            const SComplex<float>& operator[](size_t i) const
            {
                return c[i];
            }
        };

        /* Let's see if types size are alright */
        static_assert(sizeof(SComplex<float>) == sizeof(float), "Wrong size of SComplex");
        static_assert(sizeof(SVector<3>) == 3 * sizeof(float), "Wrong size of SVector");
        
        /* typedef the wrappers */
        typedef int             integer;
        typedef SString<0>      hash;
        typedef SString<8>      string8;
        typedef SString<16>     string16;
        typedef SString<24>     string24;
        typedef SString<32>     string32;
        typedef SString<64>     string64;
        typedef SString<128>    string128;
        typedef SComplex<float> complex;
        typedef SVector<2>      vec2;
        typedef SVector<3>      vec3;
        typedef SVector<4>      vec4;
        
        /* game objects typedef */
        typedef string32        model;
        typedef unsigned int    flags;
        typedef unsigned int    obj_id;
    }
}

#define OPI(item, op)   (a.item op b.item)
#define EQ(item)        OPI(item, ==)
#define LE(item)        OPI(item, <)


#endif	/* TRAITS_H */

