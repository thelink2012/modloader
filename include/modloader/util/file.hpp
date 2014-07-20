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
#ifndef MODLOADER_UTIL_FILE_HPP
#define	MODLOADER_UTIL_FILE_HPP

#include <cstdio>
#include <cstdarg>
#include <vector>
#include <functional>

namespace modloader
{
    static const size_t MaxLineSize = 2048;
    
    /*
     *  GetFileMagic
     *      Gets the first 4 bytes from the file @filename 
     */
    inline char* GetFileMagic(const char* filename, char* buf)
    {
        char* result = nullptr;
        if(FILE* f = fopen(filename, "rb"))
        {
            if(fread(buf, 1, 4, f) == 4) result = buf;
            fclose(f);
        }
        return result;
    }
    
    /*
     *  IsFileMagic
     *      Checks if the first 4 bytes from the file @filename is equal to @magic
     */
    inline bool IsFileMagic(const char* filename, const char* magic)
    {
        char buf[4];
        if(GetFileMagic(filename, buf))
            return !strncmp(buf, magic, 4);
        return false;
    }
    
    
    /*
     *  ReadEntireFile
     *      Reads a file into memory
     *      @filepath: File to be read
     *      @out: Vector that will receive the file data
     */
    inline bool ReadEntireFile(const char* filepath, std::vector<char>& out)
    {
        /* 64 bits file not supported */

        bool bResult = false;
        long fsize;
        
        if(FILE* f = fopen(filepath, "rb"))
        { 
            /* Get file size */
            if(!fseek(f, 0, SEEK_END) && ((fsize = ftell(f)) != 1L) && !fseek(f, 0, SEEK_SET))
            {
                if(true)
                { 
                    /* Read the file data into the vector */
                    out.resize(fsize);
                    if(fread(out.data(), 1, fsize, f) == fsize)
                        bResult = true; /* Success */
                    else
                        out.clear();    /* Failure, clear output */

                    bResult = true;
                }
            }

            fclose(f);
        }
        
        /* Bye function */
        return bResult;
    }
    
    /*
     *  ParseConfigLine
     *      Parses @str (modifying it's content) as if it is a Grand Theft Auto config line.
     *      Grand Theft Auto config lines use '#' and ';' as comments and they ignore ',' and spaces.
     *      So, if @str is "  a, b, c,   d, e ; cool " the output is "a, b, c,   d, e "
     */
    inline char* ParseConfigLine(char* str)
    {
        char* result = str;
        bool bTrim = true;
        
        if(result)
        {
            /* Iterate on string making apropiate changes */
            for(char* p = result; *p; ++p)
            {
                if(*p <= ' ' || *p == ',')
                {
                    *p = ' ';
                    if(bTrim) result = p+1;
                }
                else if(*p == '#' || *p == ';')
                {
                    /* Comments, end line here */
                    *p = 0;
                    break;
                }
                else
                    bTrim = false; /* First non space character, no more trim */
            }
        }
        return result;
    }

    template<class ForwardIterator, class Functor>
    void foreach_line(ForwardIterator begin, ForwardIterator end, Functor func)
    {
        ForwardIterator sol, eol, pos = begin;
        
        while(pos != end)
        {
            sol = pos;
            eol = end;
            
            for(; pos != end; ++pos)
            {
                if(*pos == '\n' || *pos == 0)
                {
                    eol = pos++;
                    break;
                }
            }
            
            func(sol, eol);
        }
    }
    
    
}

#endif	/* MODLOADER_UTIL_FILE_HPP */

