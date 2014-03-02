/*
 * Copyright (C) 2013-2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 *  File data traits base and utility types
 * 
 */
#ifndef TRAITS_H
#define	TRAITS_H

#include <algorithm>
#include <string>
#include <cstdint>
#include <modloader_util_hash.hpp>
#include <modloader_util_parser.hpp>
using modloader::SectionInfo;
using modloader::ScanConfigLine;
using modloader::PrintConfigLine;

#include <DataTraits.hpp>
using namespace DataTraitsNamespace;


/* Base trait for our purposes... we need to have a path */
template<class ContainerType>
struct DataTraitsBase : public DataTraits<ContainerType>
{
    typedef typename DataTraits<ContainerType>::container_type  container_type;
    typedef typename DataTraits<ContainerType>::pair_ref_type   pair_ref_type;
    
    std::string path;

    
    /*
     *  The child must implement the following:
     *      [optional] type handler_type= The handler for line set/get, Must contain Set(), Get() and Func()
     *      constexpr bool is_sorted    = whether the traits.map is sorted or not
     *      bool LoadData()             = loading @path file.
     *      auto domflags()             = Returns the domination flags (used in the dominance algorithm). This may be a int or a functor.
     *      const char* what()          = What does this trait handle? Used to describe in the log.
     * 
     */
};








/*****************************************************/
/* Helpers                                           */
/*****************************************************/

/*
*  Helper function to get dominance flags from a integer or a functor
*/
inline int GetDomFlags(int flags) { return flags; }
template<class Functor>
inline int GetDomFlags(Functor f) { return f(); /* Non-key overload */ }



/*
 *  Functors to call SectionParser 
 */
template<bool HasSection, class HandlerType>
struct SectionParserTemplate
{
    template<class C>
    bool operator()(const char* filename, C& map)
    {
        // HasSection = true
        return modloader::SectionParser<HandlerType>(filename, map);
    }
};

template<class HandlerType>
struct SectionParserTemplate<false, HandlerType>
{
    template<class C>
    bool operator()(const char* filename, C& map)
    {
        // HasSection = false
        return modloader::SectionParserNoSection<HandlerType>(filename, map);
    }
};

/*
 *  Functors to call SectionBuilder 
 */
template<bool HasSection, class HandlerType>
struct SectionBuilderTemplate
{
    template<class C>
    bool operator()(const char* filename, C& map)
    {
        // HasSection = true
        return modloader::SectionBuilder<HandlerType>(filename, map);
    }
};

template<class HandlerType>
struct SectionBuilderTemplate<false, HandlerType>
{
    template<class C>
    bool operator()(const char* filename, C& map)
    {
        // HasSection = false
        return modloader::SectionBuilderNoSection<HandlerType>(filename, map);
    }
};


/*
 *  Functor object that returns @DomFlags
 *  Intended to be used as parameter to the template DataTraitsImpl, for the argument DomFlags
 */
template<int iDomFlags>
struct DomFlags
{
    int operator()() { return iDomFlags; }
};

/* Functor that calls a.get(b) */
template<class T, class U>
struct call_get
{
    bool operator()(const T& a, U& b) { return a.get(b); }
};
        
/* Functor that calls a.set(b) */
template<class T, class U>
struct call_set
{
    bool operator()(T& a, const U& b) { return a.set(b); }
};
/*****************************************************/




/*
 *  The following template object implements the DataTraitsBase object has it's requiered to be implmented.
 *  You still need a base class that implements [static const char* what();]
 * 
 *  The implementation is fully based on the template arguments that are received:
 *      @ContainerType is the same as in DataTraits, the container to use for DataTraits::map
 *      @HasSection tells if the data is sectioned
 *      @HandlerType is the handler for the parser/builder function (that's, implement Set/Get/Find, see modloadere_util_parser.hpp include file)
 *      @IsSorted tells if the container is sorted
 *      @DomFlags is a functor object that returns an integer (the actual flags) or another functor object that receives the container key as parameter and returns the flags
 *      @WhatString tells what the container handles, used for logging
 *
 */
template<class ContainerType, bool HasSection, class HandlerType, bool IsSorted, class DomFlags>
struct DataTraitsImpl : public DataTraitsBase<ContainerType>
{
    public:
        typedef DataTraitsBase<ContainerType> super;
        typedef typename super::container_type container_type;
        typedef HandlerType                    handler_type;
        
        static const bool is_sorted = IsSorted;
        static const decltype(DomFlags()()) domflags() { return DomFlags()(); }
        
        
        static bool Parser(const char* filename, container_type& map)
        {
            return SectionParserTemplate<HasSection, HandlerType>()(filename, map);
        }
        
        static bool Build(const char* filename, const std::vector<typename super::pair_ref_type>& lines)
        {
            return SectionBuilderTemplate<HasSection, HandlerType>()(filename, lines);
        }
        
        bool LoadData()
        {
            return super::LoadData(super::path.c_str(), Parser);
        }
};

/*
 *  Same as DataTraitsImpl, except instead of 'class HandlerType' we have 'bool HasKeyValue' boolean
 *      @HasKeyValue: Teels wether the trait handles only keys (e.g. value is dummy) or both key and value.
 *                    Used to tell if uses the default HandlerType KeyValue or KeyOnly
 */
template<class ContainerType, bool HasSection, bool HasKeyValue, bool IsSorted, class DomFlags>
struct DataTraitsImplSimple
    : public DataTraitsImpl<
                ContainerType, HasSection,
                typename std::conditional<
                    HasKeyValue,
                    modloader::parser::KeyValue<DataTraits<ContainerType>>,
                    modloader::parser::KeyOnly<DataTraits<ContainerType>>
                >::type, IsSorted, DomFlags
            >
{
};


/*
 * 
 *  Data key/value from the traits must contain the following functions to pass throught the algorithms
 * 
 *  Value must implement at least:
 *          bool operator==(const type& b) const
 * 
 *          if HasKeyValue:
 *                  static SectionInfo* FindSectionByLine(const char* line)
 *                  static SectionInfo* FindSectionById(uint8_t section)
 *                  bool get(char* line) const
 *                  bool set(SectionInfo*, const char*)
 * 
 * 
 *  Key must implement at least:
 *          .....
 * 
 *          if HasKeyValue:
 *                  bool set(const value_type&) 
 *          else:
 *                  static SectionInfo* FindSectionByLine(const char* line)
 *                  static SectionInfo* FindSectionById(uint8_t section)
 *                  bool get(char* line) const
 *                  bool set(SectionInfo*, const char*)
 * 
 */




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
            
            
            static T epsilon()
            {
                // This is our epsilon, based on program outputs (like MEd) and the actual original file
                // Don't use std::numeric_limits epsilon, it's not for our purposes!
                return (T)(0.008);
            }
            
            bool operator==(const SComplex& sz) const
            {
                const float epsilon = this->epsilon();
                return std::abs(this->f - sz.f) <= epsilon;
            }

            bool operator<(const SComplex& sz) const
            {
                const float epsilon = this->epsilon();
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
        typedef SString<256>    string256;
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
#define EQ_ARRAY(item)  (std::equal(std::begin(a.item), std::end(a.item), std::begin(b.item)))


extern void Log(const char* msg, ...);


#endif	/* TRAITS_H */

