/*
 * Copyright (C) 2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */

#ifndef ARGS_TRANSLATOR_HACKS_FINDCLEOSCRIPTS_HPP
#define	ARGS_TRANSLATOR_HACKS_FINDCLEOSCRIPTS_HPP

#include <windows.h>
#include "../xtranslator.hpp"

namespace hacks
{
    struct FindCleoScripts  // Not thread-safe
    {
        struct PseudoHandle
        {
            HANDLE hFind;
            
            PseudoHandle(HANDLE hFind) : hFind(hFind)
            {}
        };
        
        static std::list<PseudoHandle>& GetList()
        {
            static std::list<PseudoHandle> x;
            return x;
        }
        
        static PseudoHandle* FindHandle(HANDLE hFind)
        {
            for(auto& h : GetList()) if(hFind == &h) return &h;
            return nullptr;
        }
        
        static PseudoHandle* AddHandle(HANDLE hFind)
        {
            GetList().emplace_back(hFind);
            return &GetList().back();
        }
        
        static HANDLE RemoveHandle(HANDLE hFind)
        {
            HANDLE hResult = NULL;
            
            auto& list = GetList();
            for(auto it = list.begin(); it != list.end(); ++it)
            {
                if(&(*it) == hFind)
                {
                    hResult = it->hFind;
                    list.erase(it);
                    break;
                }
            }
            
            return hResult;
        }
        
    
        template<const char* Symbol, class Ret, class... Args>
        static bool Finder(Ret& result, Args&...)
        {
            return false;
        }
       
    };
  
        template<>
        bool FindCleoScripts::Finder<aFindFirstFileA>(HANDLE& result, LPCTSTR& lpFileName, LPWIN32_FIND_DATAA& lpFindFileData)
        {
            auto* p = strrchr(lpFileName, '.');
            if(p && p != lpFileName && *(p-1) == '*' && !strnicmp(p+1, "cs", 2))
            {
                HANDLE hStock = result = FindFirstFileA(lpFileName, lpFindFileData);
                if(result != INVALID_HANDLE_VALUE)
                {
                    result = AddHandle(result);
                }
                
                printf("First %p %p %d\n", result, hStock, result);
                return true;
            }
            return false;
        }

        template<>
        bool FindCleoScripts::Finder<aFindNextFileA>(BOOL& result, HANDLE& hFindFile, LPWIN32_FIND_DATAA& lpFindFileData)
        {
            if(PseudoHandle* h = FindHandle(hFindFile))
            {
                result = FindNextFileA(h->hFind, lpFindFileData);
                printf("Next %p %p %d\n", hFindFile, h->hFind, result);
                return true;
            }
            return false;
        }

        template<>
        bool FindCleoScripts::Finder<aFindClose>(BOOL& result, HANDLE& hFindFile)
        {
            if(HANDLE h = RemoveHandle(hFindFile))
            {
                result = FindClose(h);
                printf("Closing %p %p %d\n", hFindFile, h, result);
                return true;
            }
            return false;
        }
    
};


#endif
