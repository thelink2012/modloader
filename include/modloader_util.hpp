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
     */
    inline bool RegisterReplacementFile(CPlugin& plugin, const char* name,  std::string& buf, const char* path)
    {
        if(!buf.empty())
        {
            plugin.Log("warning: Failed to replace a file %s with \"%s\" because the file already has a replacement!\n"
                "\tReplacement: %s", name, path, buf.c_str());
            return false;
        }
        else
        {
            plugin.Log("Found replacement file for %s\n\tReplacement: %s", name, path);
            buf = path;
            return true;
        }
    }
}


#endif	/* MODLOADER_UTIL_HPP */

