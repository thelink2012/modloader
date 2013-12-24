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
#ifndef MODLOADER_UTIL_CONTAINER_HPP
#define	MODLOADER_UTIL_CONTAINER_HPP

#include <string>
#include <cstring>
#include <algorithm>
#include <functional>
#include <cctype>

/*
 *  Containers and Strings utility functions
 */

namespace modloader
{
    /*
     *  String comparision functions, with case sensitive option
     */
    
    using std::strcmp;
    
    inline int strcmp(const char* str1, const char* str2, bool csensitive)
    {
        return (csensitive? ::strcmp(str1, str2) : ::_stricmp(str1, str2));
    }
    
    inline int strcmp(const char* str1, const char* str2, size_t num, bool csensitive)
    {
        return (csensitive? ::strncmp(str1, str2, num) : ::_strnicmp(str1, str2, num));
    }
    
    inline int compare(const std::string& str1, const std::string& str2, bool case_sensitive)
    {
        return strcmp(str1.c_str(), str2.c_str(), case_sensitive);
    }
    
    inline int compare(const std::string& str1, const std::string& str2, size_t num, bool case_sensitive)
    {
        return strcmp(str1.c_str(), str2.c_str(), num, case_sensitive);
    }
    
    inline int compare(const char* str1, const char* str2, bool case_sensitive)
    {
        return strcmp(str1, str2, case_sensitive);
    }
    
    inline int compare(const char* str1, const char* str2, size_t num, bool case_sensitive)
    {
        return strcmp(str1, str2, num, case_sensitive);
    }
    
    /*
     * pop_last_if
     *      Removes the last element from the @container if it is equal to @e
     */
    template<class C, class T>
    inline C& pop_last_if(C& container, const T& e)
    {
        if(container.back() == e) container.pop_back();
        return container;
    }
    
    /*
     *  Makes string @str lower case
     */
    inline std::string& tolower(std::string& str)
    {
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
        return str;
    }
    
    /*
     *  Makes string @str upper case
     */
    inline std::string& toupper(std::string& str)
    {
        std::transform(str.begin(), str.end(), str.begin(), ::toupper);
        return str;
    }
    
    /*
     *  Makes string @str lower case
     */
    inline char* tolower(char* str)
    {
        for(char* p = str; *p; ++p) *p = ::tolower(*p);
        return str;
    }
    
    /*
     *  Makes string @str upper case
     */
    inline char* toupper(char* str)
    {
        for(char* p = str; *p; ++p) *p = ::toupper(*p);
        return str;
    }
    
    /*
     * Remove space characters from the string @s. Removes left spaces if @trimLeft and removes right spaces if @trimRight.
     */
    inline std::string& trim(std::string& s, bool trimLeft = true, bool trimRight = true)
    {
        if(s.size())
        {
            if(trimLeft)
                s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
            if(trimRight)
                s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ref(::isspace))).base(), s.end());
        }
        return s;
    }
    
    /*
     *  Converts a string into a boolean
     */
    inline bool to_bool(const std::string& s)
    {
        return !!compare(s, "false", false);
    }
    
}

#endif	/* MODLOADER_UTIL_CONTAINER_HPP */

