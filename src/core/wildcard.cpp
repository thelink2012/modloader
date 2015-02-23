/*
 * Copyright (C) 2015 LINK/2012 <dma_2012@hotmail.com>
 * Licensed under the MIT License, see LICENSE at top level directory.
 * 
 */
#include <stdinc.hpp>

/*
    Spec:
        * Matching any of [] are not supported
        * Matching is case insensitive
        * End of pattern/string can container a additional path slash
        * Path slashes are matched whenever '\' or '//' are in the string but on the wildcard only '/' is supported
        * If the wildcard contains a '*' to match and the wildcard contains no directory in it, subdirs are allowed in the string
          otherwise subdirs aren't allowed unless '**' is used.
        * '?' matches any character except for path slashes and end of string
        * '\' is used to escape the next character
*/

bool match_wildcard(const char* pattern, const char* string)
{
    bool allow_subdir = false;
    bool had_anydir_in_pattern = false;
    while(true)
    {
        switch(*pattern++)
        {
            case '\0':
                if(*string == '\0') return true;
                if((*string == '/' || *string == '\\') && (*(string+1) == '\0')) return true;
                return false;

            case '/':
            case '\\':
                if(*string != '/' && *string != '\\')
                {
                    if(*string == '\0' && *pattern == '\0')
                        break;
                    return false;
                }
                ++string;
                had_anydir_in_pattern = true;
                break;

            case '*':
                allow_subdir = (!had_anydir_in_pattern || (*pattern == '*'));
                while(*pattern == '*') ++pattern;

                if(*pattern == '\0')
                    return true;


                for(; *string; ++string)
                {
                    if(match_wildcard(string, pattern))
                        return true;
                    else if((*string == '/' || *string == '\\') && !allow_subdir)
                        return false;
                }

                return false;

            case '?':
                if((*string == '/' || *string == '\\') || *string == '\0')
                    return false;
                ++string;
                break;

            default:
                if(*string == '\0' || tolower(*string) != tolower(*(pattern-1)))
                    return false;
                ++string;
                break;
        }
    }
}
