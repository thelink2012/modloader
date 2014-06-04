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
    // List of references for objects of type T
    template<class T>
    using ref_list = std::vector<std::reference_wrapper<T>>;
    
    /*
     *  Gets ref_list from containers 
     */
    
    template<class C>
    inline auto refs(C& container) -> ref_list<typename C::value_type>
    {
        ref_list<typename C::value_type> list;
        list.reserve(container.size());
        for(auto& ref : container) list.emplace_back(ref);
        return list;
    }

    template<class C>
    inline auto refs_mapped(C& container) -> ref_list<typename C::mapped_type>
    {
        ref_list<typename C::mapped_type> list;
        list.reserve(container.size());
        for(auto& pair : container) list.emplace_back(pair.second);
        return list;
    }
    
    
    
    
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
    
    inline bool starts_with(const char* str, const char* prefix, bool case_sensitive)
    {
        while(*prefix)
        {
            bool equal;
            if(case_sensitive)
                equal = (*str++ == *prefix++);
            else
                equal = (tolower(*str++) == tolower(*prefix++));
            
            if(!equal) return false;
        }
        return true;
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
                s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::function<int(int)>(::isspace))));
            if(trimRight)
                s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::function<int(int)>(::isspace))).base(), s.end());
                
            if(s.size() >= 3)
            {
                // Ignore UTF-8 BOM
                if(s[0] == 0xEF && s[1] == 0xBB && s[2] == 0xBF)
                    s.erase(s.begin(), s.begin() + 3); 
            }
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

