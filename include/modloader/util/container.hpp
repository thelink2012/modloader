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
#ifndef MODLOADER_UTIL_CONTAINER_HPP
#define	MODLOADER_UTIL_CONTAINER_HPP

#include <string>
#include <cstring>
#include <algorithm>
#include <functional>
#include <cctype>
#include <vector>
#include <deque>

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
        if(str1.length() == str2.length())
            return strcmp(str1.c_str(), str2.c_str(), case_sensitive);
        return (str1.length() < str2.length()? -1 : 1);
    }
    
    inline int compare(const std::string& str1, const std::string& str2, size_t num, bool case_sensitive)
    {
        if(str1.length() == str2.length())
            return strcmp(str1.c_str(), str2.c_str(), num, case_sensitive);
        return (str1.length() < str2.length()? -1 : 1);
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
                equal = (::tolower(*str++) == ::tolower(*prefix++));
            
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
            auto isspace = [](int c) -> int
            {
                return (c == 0x20) || (c >= 0x09 && c <= 0x0D);
            };

            // Ignore UTF-8 BOM
            while(s.size() >= 3 && s[0] == (char)(0xEF) && s[1] == (char)(0xBB) && s[2] == (char)(0xBF))
                s.erase(s.begin(), s.begin() + 3);

            if(trimLeft)
                s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::function<int(int)>(isspace))));
            if(trimRight)
                s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::function<int(int)>(isspace))).base(), s.end());
        }
        return s;
    }
    
    /*
     *  Splits the string delimited by tokens
     *  Not optimal, too lazy to do one
     */
    template<class T, class Container = std::vector<T>>
    inline auto split(T s, typename T::value_type c) -> Container
    {
        Container vector;
        size_t pos;
        s.erase(0, s.find_first_not_of(c));
        while(s.size())
        {
            pos = s.find_first_of(c);
            vector.emplace_back(s.substr(0, pos));
            s.erase(0, s.find_first_not_of(c, pos));
        }
        return vector;
    }

    /*
     *  Copies the elements in the range, defined by [first, last], to another range beginning at d_first while the unary predicate is true.
     */
    template<class InputIterator, class OutputIterator, class UnaryPredicate >
    inline OutputIterator copy_while(InputIterator first, InputIterator last, OutputIterator d_first, UnaryPredicate pred)
    {
        while(first != last && pred(*first))
            *d_first++ = *first++;
        return d_first;
    }

    /*
     *  Copies all the Args cstrings into the dest cstring (where destmax determines the range of dest)
     *  Always inserts a null terminator (except when dest==destmax)
     */

    template<class OutputIterator, class... Args>
    inline OutputIterator copy_cstr(OutputIterator dest, OutputIterator destmax)
    {
        if(dest != destmax) *dest = 0;
        return dest;
    }

    template<class InputIterator, class OutputIterator, class... Args>
    inline OutputIterator copy_cstr(OutputIterator dest, OutputIterator destmax, InputIterator arg, Args&&... args)
    {
        auto diff = destmax - dest;
        if(diff <= 1) return copy_cstr(dest, destmax);  // needs 1 space for null terminator

        OutputIterator newdest = copy_while(arg, arg + diff - 1, dest,
                    [](char c) { return c != 0; }); // needs auto on this lambda

        return copy_cstr(newdest, destmax, std::forward<Args>(args)...);
    }





    /*
     *  Converts a string into a boolean
     */
    inline bool to_bool(const std::string& s)
    {
        if(s.size() == 1) return s.front() != '0';
        return !!compare(s, "false", false);
    }

    inline std::string to_string(bool boolean)
    {
        return boolean? "true" : "false";
    }



    template<class MapT>
    inline void erase_from_map(MapT& map, const typename MapT::mapped_type& value)
    {
        for(auto it = map.begin(); it != map.end(); ++it)
        {
            if(it->second == value)
            {
                map.erase(it);
                break;
            }
        }
    }

}

#endif	/* MODLOADER_UTIL_CONTAINER_HPP */

