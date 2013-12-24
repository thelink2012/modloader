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
#ifndef MODLOADER_UTIL_PARSER_HPP
#define	MODLOADER_UTIL_PARSER_HPP

#include "modloader_util_file.hpp"
#include <functional>

namespace modloader
{
    
    struct SectionInfo
    {
            int         id;
            const char* name;
            
            /*
             * Finds the section info related to the text @line data looking in the array @sections
             * Note only the first chars of the text line matters, the rest are ignored
             */
            static SectionInfo* FindByLine(SectionInfo* sections, const char* line)
            {
                // It's fine to compare against ' ' instead of using isspace() here because the line string is fully processed!
                
                /* On each section on the array... */
                for(SectionInfo* p = sections; p->name; ++p)
                {
                    if(p->name[0] == 0) continue;   // Ignore if has no string in it
                    
                    /* ...Compare first chars from line with the section name */
                    for(const char *s1 = line, *s2 = p->name;  ; ++s1, ++s2)
                    {
                        if(*s2 == 0)
                        {
                            // Found only if the line-string do not continue with another word
                            if(*s1 == 0 || *s1 == ' ') return p;
                            break;
                        }
                        if(*s1 == 0 || (tolower(*s1) != tolower(*s2))) break;
                    }
                }
                return nullptr;
            }
            
            /*
             *  Finds the section info related to the index @idx looking in the array @sections
             */
            static SectionInfo* FindByIndex(SectionInfo* sections, int idx)
            {
                return &sections[idx];
            }
    };
    
    
    namespace parser
    {
        /*
         *  inline optimization for isspace 
         */
        inline bool isspace(char c)
        {
            return c >= '\t' && c <= ' ';
        }
    }
    
    /*
     *  PreParseFormat
     *      Pre parses a formating string to make sure some conditions are meet
     *      @scanned:   Used in scan cases, this is the string to be scanned. In case of printf use nullptr here.
     *      @format:    The format...
     *      @out:       The output format after the processing
     *      @min_count: (optional) Minimum number of formats at @scanned and @format
     *      @max_count: (optional) Maximum number of formats at @scanned and @format
     * 
     *      @returns nullptr on failure or @out pointer on success     
     * 
     * 
     *      PS: [] format not support
     */
    inline char* PreParseFormat(const char* scanned, const char* format, char* out,
                                unsigned int min_count = 0, unsigned int max_count = -1)
    {
        using parser::isspace;
        
        char* result = out;
        unsigned int count = 0;
        bool terminate = false, terminated = false;
        
        // Skip first spaces
        if(scanned) { while(isspace(*scanned)) ++scanned; }
        
        for(const char* p = format;  ; )
        {
            // If format identifier...
            if(*p == '%')
            {
                if(!terminated) *out++ = *p++;    // Register it on the out string
                
                // If really a formating, continue the parsing
                if(*p != '%')
                {
                    // Do not read/write this parameter?
                    if(*p != '*') ++count;
                    else if(!terminated) *out++ = *p++;
                    
                    // Skip width
                    while(*p >= '0' && *p <= '9')
                    {
                        if(!terminated) *out++ = *p++;
                    }
                    
                    // Skip length
                    while(*p == 'h' || *p == 'l' || *p == 'L' || *p == 'j' || *p == 'z' || *p == 't')
                    {
                        if(!terminated) *out++ = *p++;
                    }
                    
                    // The actual formating type is at *p !!
                    switch(*p)
                    {
                        // If floating-point format, use %g, not %f !!
                        case 'f': if(!terminated) *out++ = 'g'; break;
                        default:  if(!terminated) *out++ = *p;  break;
                        case 0: break;  // just ignore things, something bad happened
                            
                    }
                    
                    if(*p) p++;
                    
                    // For scan?
                    if(scanned) 
                    {
                        // Go forward in the scanned string
                        while(*scanned && !isspace(*scanned)) ++scanned;    // Skip chars from this fill
                        while(isspace(*scanned))  ++scanned;                // Skip spaces, so we can reach the next fill
                    }
                    else    // Nope, for print
                    {
                        if(count >= max_count)
                            terminate = true;
                    }
                }
            }
            
            // Register current char
            if(!terminated)
            {
                *out = terminate? 0 : *p;
                if(*out == 0) terminated = true;
                ++out;
            }
            
            if(*p != 0) ++p;
            
            // If reached the end of format string or scanned string, test if min and max conditions are met
            if(*p == 0 || terminate || (terminated && (!scanned || *scanned == 0)) || (scanned && *scanned == 0))
            {
                if(!terminated) *out = 0;
                
                // Check if filled at least min_count
                if((count >= min_count) == false)
                    return nullptr;
                // Check if filled less than max_count
                if((count <= max_count) == false)
                    return nullptr;
                if(max_count != -1 && scanned && *scanned != 0)
                    return nullptr;
                    
                // Everything okay
                return result;
            }
        }
        
        // The code shall never reach here
        return nullptr;
    }
    
#if 1   // For testing purposes, set this to 0 to make sure your Scans are count based
    /*
     *  ScanConfigLine
     *      Scans the GTA configuration line @str with format @format
     *      Returns -1 on failure or the number of tokens filled
     */
    inline int ScanConfigLine(const char* str, const char* format, ...)
    {
        char fmt[256];
        int result = -1;
        
        if(const char *s = PreParseFormat(str, format, fmt))
        {
            va_list va; va_start(va, format);
            result = vsscanf(str, s, va);
            va_end(va);
        }
        return result == EOF? -1 : result;  // make sure EOF is -1 on this target platform
    }
#endif
    
    /*
     *  ScanConfigLine
     *      Scans the GTA configuration line @str with format @format
     *      The @format and the scanned string @str must contain at least @min tokens and a maximum of @max tokens
     *      Returns -1 on failure or the number of tokens filled
     * 
     */
    inline int ScanConfigLine(const char* str, unsigned int min, unsigned int max, const char* format, ...)
    {
        char fmt[256];
        int result = -1;
        
        if(const char *s = PreParseFormat(str, format, fmt, min, max))
        {
            va_list va; va_start(va, format);
            result = vsscanf(str, s, va);
            va_end(va);
            
            if(result < min) result = -1;
            else if(result > max) result = -1;
        }
        return result == EOF? -1 : result;  // make sure EOF is -1 on this target platform
    }
     
#if 1   // For testing purposes, set this to 0 to make sure your Prints are count based
    /*
     *  PrintConfigLine
     *      Writes the GTA configuration line with @format to string @str
     *      Returns -1 on failure or the number of bytes written on success.
     */
    inline int PrintConfigLine(char* str, const char* format, ...)
    {
        char fmt[256];
        int result = -1;
        
        if(const char *s = PreParseFormat(nullptr, format, fmt))
        {
            va_list va; va_start(va, format);
            result = vsprintf(str, s, va);
            va_end(va);
        }
        return result < 0? -1 : result;
    }
#endif
    
    /*
     *  PrintConfigLine
     *      Writes the GTA configuration line with @format to string @str
     *      The @format will be truncated to @max tokens
     *      Returns -1 on failure or the number of bytes written on success.
     */
    inline int PrintConfigLine(char* str, unsigned int max, const char* format, ...)
    {
        char fmt[256];
        int result = -1;
        
        if(const char *s = PreParseFormat(nullptr, format, fmt, 0, max))
        {
            va_list va; va_start(va, format);
            result = vsprintf(str, s, va);
            va_end(va);
        }
        return result < 0? -1 : result;
    }

    
    
     /*
      * SectionParser
      *     Parses file @filepath as an GTA config file with sections calling @find_section with the line string to detect which section is this,
      *     and calling @set for each specific section line.
      * 
      *     Functor Notes:
      *         @FindSectionByLine must provide an overload for std::nullptr_t (or not...).
      *                            this overload should return the type of the section variable (see below).
      *                            the section object must provide an operator bool() or something like to test if it is valid.
      *         
      */
    template<class ContainerType, class FindSectionByLine, class SetFromLineData>
    inline bool SectionParser(const char* filepath, ContainerType& map,
                              FindSectionByLine find_section, SetFromLineData set)
    {
        char *line, linebuf[MaxLineSize];
        bool bHandling = false;
        decltype(find_section(nullptr)) section;
        
        if(FILE* f = fopen(filepath, "r"))
        {
            /* Read each config line */
            while(line = ParseConfigLine(fgets(linebuf, sizeof(linebuf), f)))
            {
                if(*line == 0) continue;
                
                /* No section bein handled? */
                if(bHandling == false)
                {
                    if(section = find_section(line))
                        bHandling = true;
                }
                else
                {
                    /* End of section? */
                    if(line[0] == 'e' && line[1] == 'n' && line[2] == 'd')
                        bHandling = false;
                    else
                        set(section, line, map);
                }
            }
            
            fclose(f);
            return true;
        }

        return false;
    } 
    

    /*
     *  Same as SectionParser() above, but handles files that do not contain sections
     */
    template<class ContainerType, class SetFromLineData>
    inline bool SectionParser(const char* filepath, ContainerType& map,
                              SetFromLineData set)
    {
        char *line, linebuf[MaxLineSize];
        
        if(FILE* f = fopen(filepath, "r"))
        {
            SectionInfo* section = nullptr;
            
            /* Read each config line */
            while(line = ParseConfigLine(fgets(linebuf, sizeof(linebuf), f)))
            {
                if(*line == 0) continue;
                set(section, line, map);
            }
            fclose(f);
            return true;
        }
        
        return false;
    }

    /*
     *  SectionBuilder
     *      Builds the file @filepath with sections using @lines as content, calling @find_section with the lines <key,value>
     *      to detect which section is it, and calling @get for each section specific line.
     */
    template<class VectorPairType, class FindSectionById, class GetFromLineData>
    inline bool SectionBuilder(const char* filepath, const VectorPairType& lines,
                              FindSectionById find_section, GetFromLineData get)
    {
        if(FILE* f = fopen(filepath, "w"))
        {
            SectionInfo* section = nullptr;
            
            /* Function to switch from reading one section to another,
             * automatically puting a section start (and end) at the IPL file */
            auto UpdateSection = [&f, &section](SectionInfo* newsec)
            {
                if(section != newsec)
                {
                    fprintf(f, "%s%s\n", (section? "end\n" : ""), (newsec? newsec->name : ""));
                    section = newsec;
                }
            };

            /* Iterate on the lines data pair */
            for(auto& pair : lines)
            {
                char buf[MaxLineSize];
                auto& key   = pair.first.get();
                auto& value = pair.second.get();

                /* Find if the section is valid */
                if(SectionInfo* section =  find_section(key, value))
                {
                    /* Updates the current section we're working on the file */
                    UpdateSection(section);

                    /* Gets data into buf */
                    if(get(section, buf, key, value))
                    {
                        fprintf(f, "%s\n", buf);
                    }
                }
            }

            /* Finish up section, so, if there's a section open, close it */
            UpdateSection(nullptr);

            /* Close file and return success */
            fclose(f);
            return true;
        }
        return false;
    }
    

    /*
     *  Same as the SectionBuilder() above, but without sections
     */
    template<class VectorPairType, class GetFromLineData>
    inline bool SectionBuilder(const char* filepath, const VectorPairType& lines,
                              GetFromLineData get)
    {
        if(FILE* f = fopen(filepath, "w"))
        {
            SectionInfo* section = nullptr;
            
            /* Iterate on the lines data pair */
            for(auto& pair : lines)
            {
                char buf[MaxLineSize];
                auto& key   = pair.first.get();
                auto& value = pair.second.get();

                /* Gets data into buf */
                if(get(section, buf, key, value))
                {
                    fprintf(f, "%s\n", buf);
                }
            }

            /* Close file and return success */
            fclose(f);
            return true;
        }
        return false;
    }
    
    /*
     *  If any section is equal to this, that means the section
     *  is undefined and the data was taken from a readme file
     */
    inline SectionInfo* ReadmeSection()
    {
        static SectionInfo x = { -1, 0 };
        return &x;
    }
    
    inline bool IsReadmeSection(SectionInfo* sec)
    {
        return (sec == ReadmeSection());
    }
    
    
    /* Some default FindSectionByLine and SetFromLineData functors for SectionParser() function */

    namespace parser
    {
        // Used for parsing that contains an <key,value> pair
        template<class TraitsType>
        struct KeyValue
        {
            typedef typename TraitsType::key_type                key_type;
            typedef typename TraitsType::mapped_type             value_type;
            typedef typename TraitsType::container_type          container_type;
            typedef decltype(value_type::FindSectionByLine(""))  SectionType;

            // Section finder
            struct Find
            {
                SectionType operator()(const char* line)
                { return value_type::FindSectionByLine(line); }
                SectionType operator()(const key_type& key, const value_type& value)
                { return value_type::FindSectionById(value.section); }
            };
            
            // Map setter
            struct Set
            {
                bool operator()(SectionType section, const char* line, container_type& map)
                {
                    value_type value;
                    key_type   key;
                    
                    if(value.set(section, line) && key.set(value))
                    {
                        map[key] = value;
                        return true;
                    }
                    
                    return false;
                }
            };
            
            //
            struct Get
            {
                bool operator()(SectionType section, char* line, const key_type& key, const value_type& value)
                {
                    return value.get(line);
                }
            };
        };
        
        // Used for parsing traits that contains only key, value is dummy
        template<class TraitsType>
        struct KeyOnly
        {
            typedef typename TraitsType::key_type                key_type;
            typedef typename TraitsType::mapped_type             value_type;
            typedef typename TraitsType::container_type          container_type;
            typedef decltype(key_type::FindSectionByLine(""))    SectionType;
            
            // Section finder
            struct Find
            {
                SectionType operator()(const char* line)
                { return key_type::FindSectionByLine(line); }
                SectionType operator()(const key_type& key, const value_type& value)
                { return key_type::FindSectionById(key.section); }
            };
            
            // Map setter
            struct Set
            {
                bool operator()(SectionType section, const char* line, container_type& map)
                {
                    key_type key;
                    if(key.set(section, line))
                    {
                        map[key];
                        return true;
                    }
                    return false;
                }
            };
            
            //
            struct Get
            {
                bool operator()(SectionType section, char* line, const key_type& key, const value_type& value)
                {
                    return key.get(line);
                }
            };
        };

    }

    /* Works directly with the above structures */
    
    template<class HandlerType, class ContainerType>
    inline bool SectionParser(const char* filepath, ContainerType& map)
    {
        return SectionParser(filepath, map, typename HandlerType::Find(), typename HandlerType::Set());
    }
    
    template<class HandlerType, class VectorPairType>
    inline bool SectionBuilder(const char* filepath, const VectorPairType& lines)
    {
        return SectionBuilder(filepath, lines, typename HandlerType::Find(), typename HandlerType::Get());
    }
    
    template<class HandlerType, class ContainerType>
    inline bool SectionParserNoSection(const char* filepath, ContainerType& map)
    {
        return SectionParser(filepath, map, typename HandlerType::Set());
    }
    
    template<class HandlerType, class VectorPairType>
    inline bool SectionBuilderNoSection(const char* filepath, const VectorPairType& lines)
    {
        return SectionBuilder(filepath, lines, typename HandlerType::Get());
    }
    
}

#endif	/* MODLOADER_UTIL_PARSER_HPP */

