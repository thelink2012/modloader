/*
 *  Injectors - Function Calls Using Variadic Templates
 *	Header with helpful stuff for ASI memory hacking
 *
 *  (C) 2012-2014 LINK/2012 <dma_2012@hotmail.com>
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
#pragma once
#include "injector.hpp"
#include <utility>

namespace injector
{
    // Call function at @p returning @Ret with args @Args
    template<class Ret, class ...Args>
    inline Ret Call(memory_pointer_tr p, Args&&... a)
    {
        Ret(*fn)(Args...) = p.get();
        return fn(std::forward<Args>(a)...);
    }
    
    template<class Ret, class ...Args>
    inline Ret StdCall(memory_pointer_tr p, Args&&... a)
    {
        Ret(__stdcall *fn)(Args...) = p.get();
        return fn(std::forward<Args>(a)...);
    }
    
    template<class Ret, class ...Args>
    inline Ret ThisCall(memory_pointer_tr p, Args&&... a)
    {
        Ret(__thiscall *fn)(Args...) = p.get();
        return fn(std::forward<Args>(a)...);
    }

    template<class Ret, class ...Args>
    inline Ret FastCall(memory_pointer_tr p, Args&&... a)
    {
        Ret(__fastcall *fn)(Args...) = p.get();
        return fn(std::forward<Args>(a)...);
    }
    
} 
