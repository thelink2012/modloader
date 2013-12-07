/* 
 * San Andreas Mod Loader Utilities Headers
 * Created by LINK/2012 <dma_2012@hotmail.com>
 * 
 *  This file provides helpful functions for plugins creators.
 * 
 *  This source code is offered for use in the public domain. You may
 *  use, modify or distribute it freely.
 *
 *  This code is distributed in the hope that it will be useful but
 *  WITHOUT ANY WARRANTY. ALL WARRANTIES, EXPRESS OR IMPLIED ARE HEREBY
 *  DISCLAIMED. This includes but is not limited to warranties of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */
#ifndef MODLOADER_UTIL_HASH_HPP
#define	MODLOADER_UTIL_HASH_HPP

#include <string>
#include <cstring>

namespace modloader
{
    /*
     *  We're going to use FNV1a as our hashing algorithm because it is fast, easy to implement and has pretty good randomness.
     */
    
    /* fnv1a hashing algorithm functor */
    template<size_t nbits> struct fnv1a;
    
    /* FNV1a 32 bits specialization */
    template<> struct fnv1a<32>
    {
        typedef uint32_t hash_type;
        
        /* Performs the hash algorithm initialization */
        hash_type init()
        {
            return (2166136261U);
        }

        /* Performs an interation in the hash algorithm */
        hash_type transform(hash_type fvn, char c)
        {
            return ((((fvn) ^ (uint32_t)( uint8_t(c) )) * 16777619));
        }

        /* Performs the hash algorithm finalization */
        hash_type final(hash_type fnv)
        {
            return (fnv);
        }
        
        
        /* Hashes data */
        template<class TransformerFunctor, class ConditionFunctor>
        hash_type operator()(const void* data, size_t size, TransformerFunctor tr, ConditionFunctor cond)
        {
            const char* bytes = (const char*)(data);
            
            hash_type fnv = init();
            for( ; cond(*bytes, size); ++bytes)
                fnv = transform(fnv, tr(*bytes));
            return final(fnv);
        }
    };
    
    /* Default functors arguments for fnv1a functor */
    struct fnv_fun
    {
        /* ------ Binary functors ------ */
        
        struct transformer_binary
        {
            char operator()(char c)
            { return c; }
        };

        struct condition_binary
        {
            bool operator()(char c, size_t& size)
            { return (--size != 0); }
        };
        
        /* ------ ASCII functors ------ */
        
        typedef transformer_binary transformer_ascii;
        
        struct condition_ascii
        {
            bool operator()(char c, size_t size)
            { return (c != 0); } 
        };
    };
    
    
    /*
     *  Hash abstraction functions
     *  Just use those functions and don't worry how we're getting the hash, you must trust us!
     */
    
    /* Hashes binary data */
    inline size_t hash(const void* data, size_t len)
    {
        return fnv1a<32>()(data, len, fnv_fun::transformer_binary(), fnv_fun::condition_binary());
    }
    
    /* Hashes strings */
    inline size_t hash(const std::string& string)
    {
        return fnv1a<32>()(string.c_str(), -1, fnv_fun::transformer_ascii(), fnv_fun::condition_ascii());
    }
    
    /* Hashes C strings */
    inline size_t hash(const char* string)
    {
        return fnv1a<32>()(string, -1, fnv_fun::transformer_ascii(), fnv_fun::condition_ascii());
    }
    
    /* Hashes string with transformation */
    template<class TransformerFunctor>
    inline size_t hash(const std::string& string, TransformerFunctor tr)
    {
        return fnv1a<32>()(string.c_str(), -1, tr, fnv_fun::condition_ascii());
    }
    
    /* Hashes C strings with transformation */
    template<class TransformerFunctor>
    inline size_t hash(const char* string, TransformerFunctor tr)
    {
        return fnv1a<32>()(string, -1, tr, fnv_fun::condition_ascii());
    }   
    
}
    
#endif	/* MODLOADER_UTIL_HASH_HPP */

