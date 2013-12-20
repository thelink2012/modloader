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
#ifndef MODLOADER_UTIL_HPP
#define	MODLOADER_UTIL_HPP
#pragma once

#include <string>

#include "modloader.hpp"
#include "modloader_util_container.hpp"
#include "modloader_util_file.hpp"
#include "modloader_util_hash.hpp"
#include "modloader_util_path.hpp"

namespace modloader
{
    /*
     *  Registers @path into @buf if it is empty.
     *  Returns true if registration is sucessfull and false otherwise.
     *  Logging happens with @plugin ::Log and @name to specify a identifier for the path
     *  
     *  The behaviour of this function can change whenever we think it's necessary
     */
    inline bool RegisterReplacementFile(CPlugin& plugin, const char* name,  std::string& buf, const char* path, bool bLogOnSuccess = true)
    {
        // If buffer is not empty, there's a replacement present, log it
        if(!buf.empty())
        {
            plugin.Log("Warning: Overriding replacement for file %s, from \"%s\" with \"%s\"",
                       name, buf.c_str(), path);
        }
        
        // Do replacement
        buf = path;
        
        // Log successful replacement
        if(bLogOnSuccess)
        {
            plugin.Log("Found replacement for file %s which is \"%s\"", name, path);
        }
        
        return true;
    }
}


#endif	/* MODLOADER_UTIL_HPP */

