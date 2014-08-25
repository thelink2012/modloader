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
#ifndef MODLOADER_UTIL_VERSION_BY_DATE_HPP
#define	MODLOADER_UTIL_VERSION_BY_DATE_HPP
#pragma once

#include <cstdio>

namespace modloader
{
    /*
     *  get_version_by_date
     *      Returns version_number + compilation date + compilation time.
     *      The operation of finding the string is performed only once, later calls just performs a return.
     */
    inline const char* get_version_by_date(const char* version_number)
    {
        struct version_by_date
        {
            char buf[128];
            version_by_date(const char* version_number)
            {
                if(version_number) sprintf(buf, "%s %s %s", version_number, __DATE__, __TIME__);
                else sprintf(buf, "%s %s", __DATE__, __TIME__);
            }
        };

        static version_by_date version(version_number);
        return version.buf;
    }

    /*
     *  get_version_by_date
     *      Returns compilation date + compilation time.
     *      The operation of finding the string is performed only once, later calls just performs a return.
     */
    inline const char* get_version_by_date()
    {
        return get_version_by_date(nullptr);
    }
}


#endif