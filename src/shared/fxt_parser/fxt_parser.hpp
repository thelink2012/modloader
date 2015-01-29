/*
 *  Injectors - Fake GTA Text Implementation
 *	Header with helpful stuff for ASI memory hacking
 *
 *  (C) 2014 LINK/2012 <dma_2012@hotmail.com>
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
#include <cstdio>
#include "fxt.hpp"

namespace injector
{
    /*
     *  Parses a FXT file @filename into the text manager @manager
     *  If unable to open the file, returns false, otherwise returns true
     */
    template<class TextManager>
    bool ParseFXT(TextManager& manager, const char* filename, typename TextManager::hash_type table = 0)
    {
        if(FILE* f = fopen(filename, "r"))
        {
            char buf[1024];
            
            // Alright, that format is pretty easy
            while(fgets(buf, sizeof(buf), f))
            {
                char *key = 0, *value = 0;

                // Parses the fxt line
                for(char* p = buf; *p; ++p)
                {
                    // # is used as comments...
                    if(*p == '#')
                    {
                        *p = 0;
                        break;
                    }

                    // If no key found yet...
                    if(key == 0)
                    {
                        // ...and the current iterating character is a space...
                        if(isspace(*p))
                        {
                            // ..we've just found that the key is there
                            *p = 0;
                            key = buf;
                        }
                    }
                    // But, if key has been found but no value found yet...
                    else if(value == 0)
                    {
                        // ...we are actually reading the first character from the value!
                        value = p;
                    }
                }

                // Adds into the text map only if found both key and value
                if(key && value) manager.add(key, value, table);
            }

            // Done
            fclose(f);
            return true;
        }
        
        return false;
    }

}
