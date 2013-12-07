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

namespace modloader
{
    /* Handler for SectionParser() */
    template<class ContainerType>
    struct SSectionHandler
    {
        const char* section;
        bool (*handler)(const char* line, ContainerType& map);
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
     *  ScanConfigLine
     *      Scans the GTA configuration line @str with format @fmt
     *      Retruns true if the scan sucessfully acquieres @count arguments, returns false otherwise
     */
    inline bool ScanConfigLine(int count, const char* str, const char* fmt, ...)
    {
        va_list va; va_start(va, fmt);
        int result = vsscanf(str, fmt, va);
        va_end(va);
        return (result == count);
    }
    


     /*
      * SectionParser
      *     Parses file @filepath as an GTA config file with sections calling @handlers for each specific section line.
      */
    template<class ContainerType, class SectionHandler>
    inline bool SectionParser(const char* filepath, ContainerType& map, SectionHandler* handlers)
    {
        SectionHandler* h = 0;
        char *line, linebuf[512];
        FILE* cfg = fopen(filepath, "r");

        if(cfg)
        {
            /* Read each config line */
            while(line = ParseConfigLine(fgets(linebuf, sizeof(linebuf), cfg)))
            {
                if(*line == 0) continue;

                /* No section bein handled? */
                if(h == 0)
                {
                    /* Find handler based on this line identification (e.g. "inst", "objs", etc) */
                    for(SectionHandler* ph = handlers; h == 0 && ph->section; ++ph)
                    {
                        for(const char *pl = line, *pc = ph->section;   ; ++pc, ++pl)
                        {
                            if(*pc == 0)
                                h = ph; /* This is the section handler */
                            else if(*pl != *pc)
                                break;  /* This isn't the section handler */
                        }
                    }
                }
                else
                {
                    /* End of section? */
                    if(line[0] == 'e' && line[1] == 'n' && line[2] == 'd')
                        h = 0;                  /* Yeah, no section being handled now */
                    else if(h->handler)
                        h->handler(line, map);  /* Handle this line */
                }
            }
            return true;
        }

        return false;
    }
    
}

#endif	/* MODLOADER_UTIL_FILE_HPP */

