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
    /* Handler for SectionParser() */
    template<class ContainerType>
    struct SectionHandler
    {
        typedef std::function<bool(const char* line, ContainerType& map)>   onReadType;
        typedef std::function<bool(ContainerType& map)>                     onBeginType;
        typedef std::function<bool(ContainerType& map)>                     onEndType;
        
        const char* section;                   
        onReadType  onRead;
        onBeginType onBegin;
        onEndType   onEnd;
        
        SectionHandler() : section(0)
        {}
        
        SectionHandler(const SectionHandler& rhs)
            : section(rhs.section), onRead(rhs.onRead), onBegin(rhs.onBegin), onEnd(rhs.onEnd)
        {}
        
        SectionHandler& Section(const char* section)
        { this->section = section; return *this; }
        
        SectionHandler& OnRead(const onReadType& cb)
        { this->onRead = cb; return *this; }
        
        SectionHandler& OnBegin(const onBeginType& cb)
        { this->onBegin = cb; return *this; }
        
        SectionHandler& OnEnd(const onEndType& cb)
        { this->onEnd = cb; return *this; }
     };
    
     
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
    inline int PrintConfigLine(FILE* f, const char* format, ...)
    {
        char fmt[256];
        va_list va; va_start(va, format);
        bool bResult = vfprintf(f, FixFormatString(format, fmt), va) >= 0;
        va_end(va);
        return bResult;
    }
    


     /*
      * SectionParser
      *     Parses file @filepath as an GTA config file with sections calling @handlers for each specific section line.
      */
    template<class ContainerType, class SectionHandlerIt>
    inline bool SectionParser(const char* filepath, ContainerType& map, SectionHandlerIt begin, SectionHandlerIt end)
    {
        bool bHasHandler = false;
        SectionHandlerIt h;
        
        char *line, linebuf[512];
        FILE* cfg = fopen(filepath, "r");

        if(cfg)
        {
            /* Read each config line */
            while(line = ParseConfigLine(fgets(linebuf, sizeof(linebuf), cfg)))
            {
                if(*line == 0) continue;

                /* No section bein handled? */
                if(!bHasHandler)
                {
                    /* Find handler based on this line identification (e.g. "inst", "objs", etc) */
                    for(SectionHandlerIt ph = begin; !bHasHandler && ph != end; ++ph)
                    {
                        for(const char *pl = line, *pc = ph->section;   ; ++pc, ++pl)
                        {
                            if(*pc == 0)
                            {
                                h = ph; /* This is the section handler */
                                bHasHandler = true;
                                if(h->onBegin) h->onBegin(map);
                                break;
                            }
                            else if(*pl != *pc)
                                break;  /* This isn't the section handler */
                        }
                    }
                }
                else
                {
                    /* End of section? */
                    if(line[0] == 'e' && line[1] == 'n' && line[2] == 'd')
                    {
                        if(h->onEnd) h->onEnd(map);
                        bHasHandler = false;
                    }
                    else if(h->onRead)
                        h->onRead(line, map);  /* Handle this line */
                }
            }
            return true;
        }

        return false;
    }
    
}

#endif	/* MODLOADER_UTIL_FILE_HPP */

