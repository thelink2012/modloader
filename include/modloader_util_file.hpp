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
#ifndef MODLOADER_UTIL_FILE_HPP
#define	MODLOADER_UTIL_FILE_HPP

#include <cstdio>
#include <cstdarg>
#include <vector>
#include <functional>

namespace modloader
{
    static const size_t MaxLineSize = 512;
    
    /*
     *  ReadEntireFile
     *      Reads a file into memory
     *      @filepath: File to be read
     *      @out: Vector that will receive the file data
     *      @sizeLimit: If file size is greater than this value, failure happens.
     */
    inline bool ReadEntireFile(const char* filepath, std::vector<char>& out, uint64_t sizeLimit = -1)
    {
        /* 64 bits file not supported */
        
        bool bResult = false;
        uint64_t fsize;
        
        if(FILE* f = fopen(filepath, "rb"))
        { 
            /* Get file size */
            if(!fseek(f, 0, SEEK_END) && ((fsize = ftell(f)) != 1L) && !fseek(f, 0, SEEK_SET))
            {
                /* Read the file data into the vector */
                out.resize(fsize);
                if(fread(out.data(), 1, fsize, f) == fsize)
                    bResult = true; /* Success */
                else
                    out.clear();    /* Failure, clear output */
                
                bResult = true;
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
        
        //printf("\n>>p> %s\n", str);
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
    
    
    /*
     *  FixFormatString
     *      Replaces %f with %g
     */
    inline char* FixFormatString(const char* format, char* out)
    {
        char* result = out;
        char lastlast = 0, last = 0;
        
        for(const char* p = format; *p; ++p, ++out)
        {
            if(lastlast != '%' && last == '%' && *p == 'f')
                *out = 'g';
            else
                *out = *p;
            
            lastlast = last;
            last = *p;
        }
        
        *out = 0;
        return result;
    }
    
    
    /*
     *  ScanConfigLine
     *      Scans the GTA configuration line @str with format @format
     *      Returns the number of items in the arglist successfully filled
     */
    inline int ScanConfigLine(const char* str, const char* format, ...)
    {
        char fmt[256];
        va_list va; va_start(va, format);
        int result = vsscanf(str, FixFormatString(format, fmt), va);
        va_end(va);
        return result;
    }
    
    /*
     *  ScanConfigLine
     *      Scans the GTA configuration line @str with format @format
     *      Retruns true if the scan sucessfully acquieres @count arguments, returns false otherwise
     */
    inline bool ScanConfigLine(const char* str, int count, const char* format, ...)
    {
        char fmt[256];
        va_list va; va_start(va, format);
        int result = vsscanf(str, FixFormatString(format, fmt), va);
        va_end(va);
        return (result >= count);
    }

    /*
     *  PrintConfigLine
     *      Writes the GTA configuration line with @format to string @str
     *      Returns true on success, false otherwise
     */
    inline bool PrintConfigLine(char* str, const char* format, ...)
    {
        char fmt[256];
        va_list va; va_start(va, format);
        bool bResult = vsprintf(str, FixFormatString(format, fmt), va) >= 0;
        va_end(va);
        return bResult;
    }
    
    /*
     *  PrintConfigLine
     *      Writes the GTA configuration line with @format to file @f
     *      Returns true on success, false otherwise
     */
    inline bool PrintConfigLine(FILE* f, const char* format, ...)
    {
        char fmt[256];
        va_list va; va_start(va, format);
        bool bResult = vfprintf(f, FixFormatString(format, fmt), va) >= 0;
        va_end(va);
        return bResult;
    }
    
}

#endif	/* MODLOADER_UTIL_FILE_HPP */

