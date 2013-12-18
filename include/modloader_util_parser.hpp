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
                /* On each section on the array... */
                for(SectionInfo* p = sections; p->name; ++p)
                {
                    /* ...Compare first chars from line with the section name */
                    for(const char *s1 = line, *s2 = p->name;  ; ++s1, ++s2)
                    {
                        if(*s2 == 0) return p;
                        if(*s1 == 0 || (toupper(*s1) != toupper(*s2))) break;
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
            /* Read each config line */
            while(line = ParseConfigLine(fgets(linebuf, sizeof(linebuf), f)))
            {
                if(*line == 0) continue;
                set(nullptr, line, map);
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
                    PrintConfigLine(f, "%s%s\n", (section? "end\n" : ""), (newsec? newsec->name : ""));
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
                        PrintConfigLine(f, "%s\n", buf);
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
            /* Iterate on the lines data pair */
            for(auto& pair : lines)
            {
                char buf[MaxLineSize];
                auto& key   = pair.first.get();
                auto& value = pair.second.get();

                /* Gets data into buf */
                if(get(nullptr, buf, key, value))
                {
                    PrintConfigLine(f, "%s\n", buf);
                }
            }

            /* Close file and return success */
            fclose(f);
            return true;
        }
        return false;
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
                { return value.section; }
            };
            
            // Map setter
            template<class T>
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

