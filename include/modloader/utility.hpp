/* 
 * Mod Loader Plugin Related Utilities
 * Created by LINK/2012 <dma_2012@hotmail.com>
 * 
 *  This source code is offered for use in the public domain. You may
 *  use, modify or distribute it freely.
 *
 *  This code is distributed in the hope that it will be useful but
 *  WITHOUT ANY WARRANTY. ALL WARRANTIES, EXPRESS OR IMPLIED ARE HEREBY
 *  DISCLAIMED. This includes but is not limited to warranties of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE
 * 
 */
#pragma once
#ifndef MODLOADER_UTIL_HPP
#define	MODLOADER_UTIL_HPP
#include <modloader/modloader.hpp>
#include <modloader/util/path.hpp>
#include <functional>

namespace modloader
{
    // Scoped chdir relative to gamedir
    struct scoped_gdir : public modloader::scoped_chdir
    {
        scoped_gdir(const char* newdir) : scoped_chdir(
            (!newdir[0]? (plugin_ptr->loader->gamepath) : ((std::string(plugin_ptr->loader->gamepath) + newdir).data()))
         )
        { }
    };

    template<class FuncT>
    std::function<void()> gdir_refresh(FuncT& func)
    {
        return [func]
        {
            scoped_gdir xdir("");
            return func();
        };
    }

}

#endif
