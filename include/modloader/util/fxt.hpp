/* 
 * Mod Loader Utilities Headers
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
#ifndef MODLOADER_UTIL_FXT_HPP
#define	MODLOADER_UTIL_FXT_HPP

#include <modloader/util/injector.hpp>  // Include this first, otherwise bad things will happen
#include <modloader/util/hash.hpp>
#include <injector/game/fxt_parser.hpp>

#include <map>
#include <cctype>
#include <string>


namespace modloader
{
    // Hashing functor for the fxt manager
    struct fxt_hash_functor
    {
        size_t operator()(const char* key)
        {
            // FXT entries should contain max of 7 chars!
            return fnv1a<32>()(key, 7, ::toupper, fnv_fun::condition_ascii());
        }
    };

    // Proper fxt manager object
    typedef injector::basic_fxt_manager<std::map<size_t, std::string>, fxt_hash_functor>    fxt_manager;
}

#endif
