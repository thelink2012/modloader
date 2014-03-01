/*
 * Copyright (C) 2013  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 *  Carmods structures
 *
 */
#ifndef TRAITS_WATER_H
#define TRAITS_WATER_H

#include "traits.h"
#include <map>

namespace data
{
    using namespace data::util;

    enum
    {
        WATER_NONE,
        WATER_PROCESSED
    };
    
    struct SDataWater
    {
        /* Key for an IDE item */
        struct key_type
        {
            uint8_t section;
            uint8_t num_points;
            bool has_flags;
            complex p[4][7];
            integer flags;
            
            /*
             *  Sections table 
             */
            static SectionInfo* GetTable()
            {
                /* Section table for lookup */
                static SectionInfo sections[] =
                {
                    { WATER_NONE,      ""          },
                    { WATER_PROCESSED, "processed" },
                    { WATER_NONE,      nullptr     }
                };
                return sections;
            }

            /* Search for section in sections table */
            static SectionInfo* FindSectionByLine(const char* line)
            {
                return SectionInfo::FindByLine(GetTable(), line);
            }

            /* Search for section in sections table */
            static SectionInfo* FindSectionById(uint8_t id)
            {
                return SectionInfo::FindByIndex(GetTable(), id);
            }          
            
            /* Compare */
            bool operator==(const key_type& b) const
            {
                const key_type& a = *this;
                if(true)
                {
                    // First check basic data...
                    if(a.num_points == b.num_points && a.has_flags == b.has_flags)
                    {
                        // Then check basic points...
                        if(EQ_ARRAY(p[0]) && EQ_ARRAY(p[1]) && EQ_ARRAY(p[2]))
                        {
                            // then check extra point...
                            if(a.num_points == 4) return EQ_ARRAY(p[3]);
                            return true;
                        }
                    }
                }
                return false;
            }

            static int min_count()  { return 21; }
            static int max_count()  { return 29; }
            
            const char* format() const
            {
                switch(num_points)
                {
                    case 3:
                        if(has_flags)
                            return "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %d";
                        else
                            return "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f";
                        
                    case 4:
                        if(has_flags)
                            return "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %d";
                        else
                            return "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f";
                        
                }
                return "; THIS SHOULD NEVER HAPPEN";
            }
            
            size_t count() const
            {
                return num_points * 7 + has_flags;
            }
            
            /* Sets the current data to the data at @line, knowing that the section to handle is @info */
            bool set(const SectionInfo*, const char* line)
            {
                // Processed word... skip it
                if(*line == 'p') return false;
                
                //
                this->section = WATER_PROCESSED;
                
                // Capture all values on format...
                this->num_points = 4;
                this->has_flags = true;
                
                // Scan
                int nargs = ScanConfigLine(line, min_count(), max_count(), format(),
                                 &p[0][0].f, &p[0][1].f, &p[0][2].f, &p[0][3].f, &p[0][4].f, &p[0][5].f, &p[0][6].f,
                                 &p[1][0].f, &p[1][1].f, &p[1][2].f, &p[1][3].f, &p[1][4].f, &p[1][5].f, &p[1][6].f,          
                                 &p[2][0].f, &p[2][1].f, &p[2][2].f, &p[2][3].f, &p[2][4].f, &p[2][5].f, &p[2][6].f,          
                                 &p[3][0].f, &p[3][1].f, &p[3][2].f, &p[3][3].f, &p[3][4].f, &p[3][5].f, &p[3][6].f,
                                 &flags
                            );
                
                // Find params and stuff...
                if(nargs >= min_count())
                {
                    // ...based on num arguments received from the scan
                    switch(nargs)
                    {
                        default: return false;
                        
                        case 21:    // 3 points, no flags
                            this->num_points = 3; this->has_flags = false;
                            break;
                        case 22:    // 3 points, with flags
                            this->num_points = 3; this->has_flags = true;
                            this->flags = (integer) this->p[3][0].f;
                            break;
                            
                        case 28:    // 4 points, no flags
                            this->num_points = 4; this->has_flags = false;
                            break;
                        case 29:    // 4 points, with flags
                            this->num_points = 4; this->has_flags = true;
                            break;
                    }
                    return true;
                }
                return false;
            }
            
            /* Gets the current data to the string @output */
            bool get(char* output) const
            {
                switch(num_points)
                {
                    case 3:
                        return PrintConfigLine(output, count(), format(),
                                 p[0][0].f, p[0][1].f, p[0][2].f, p[0][3].f, p[0][4].f, p[0][5].f, p[0][6].f,
                                 p[1][0].f, p[1][1].f, p[1][2].f, p[1][3].f, p[1][4].f, p[1][5].f, p[1][6].f,          
                                 p[2][0].f, p[2][1].f, p[2][2].f, p[2][3].f, p[2][4].f, p[2][5].f, p[2][6].f,
                                 flags
                            ) > 0;
                    
                    case 4:
                        return PrintConfigLine(output, count(), format(),
                                 p[0][0].f, p[0][1].f, p[0][2].f, p[0][3].f, p[0][4].f, p[0][5].f, p[0][6].f,
                                 p[1][0].f, p[1][1].f, p[1][2].f, p[1][3].f, p[1][4].f, p[1][5].f, p[1][6].f,          
                                 p[2][0].f, p[2][1].f, p[2][2].f, p[2][3].f, p[2][4].f, p[2][5].f, p[2][6].f,          
                                 p[3][0].f, p[3][1].f, p[3][2].f, p[3][3].f, p[3][4].f, p[3][5].f, p[3][6].f,
                                 flags
                            ) > 0;
                        
                }
                return false;
            }
        };
        
        
        /* The value for this is dummy, nothing, things depend only on the key */
        bool operator==(const SDataWater&) const
        { return true; }
    };
    
    
    // The traits object
    struct TraitsWater : public DataTraitsImplSimple<
            /*ContainerType*/   ordered_map<SDataWater::key_type, SDataWater>,
            /* HasSection */    false,
            /* HasKeyValue */   false,
            /* IsSorted */      false,
            /* DomFlags */      DomFlags<flag_RemoveIfNotExistInOneCustomButInDefault>
                        >
    {
        static const char* what() { return "water level"; }
    };

    
}   // namespace

#endif
