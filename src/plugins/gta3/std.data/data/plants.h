/*
 * Copyright (C) 2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 *  Carmods structures
 *
 */
#ifndef TRAITS_PLANTS_H
#define TRAITS_PLANTS_H

#include "traits.h"
#include <map>

namespace data
{
    using namespace data::util;
    
    struct SDataPlants
    {
        string24 surface;
        integer  a[10];
        complex  b[7];
        
        
        /* Key for an plant item */
        struct key_type
        {
            hash surface;

            // Set key based on value
            bool set(const SDataPlants& b)
            {
                this->surface.hash = b.surface.hash;
                return true;
            }
            
            // For sorting...
            bool operator<(const key_type& b) const
            {
                const key_type& a = *this;
                return LE(surface);
            }
        };
        
        /*
         *  Compare two plants data lines
         */
        bool operator==(const SDataPlants& b) const
        {
            const SDataPlants& a = *this;
            return EQ(surface) && EQ_ARRAY(a) && EQ_ARRAY(b);
        }
        
        /* Formating information */
        const char* format() const { return "%s %d %d %d %d %d %d %d %d %d %d %f %f %f %f %f %f %f"; }
        size_t count()       const { return min_count(); }
        static size_t min_count()  { return 18; }
        static size_t max_count()  { return min_count(); }
        
        /* Sets the current data to the data at @line */
        bool set(SectionInfo*, const char* line)
        {
            if(ScanConfigLine(line, min_count(), max_count(), format(),
                            surface.buf,
                            &a[0], &a[1], &a[2], &a[3], &a[4], &a[5], &a[6], &a[7], &a[8], &a[9],
                            &b[0].f, &b[1].f, &b[2].f, &b[3].f, &b[4].f, &b[5].f, &b[6].f
                ) > 0)
            {
                // Calculate hash
                this->surface.recalc();
                return true;
            }
            return false;
        }
        
        /* Gets the current data to the string @output */
        bool get(char* output) const
        {
            return(PrintConfigLine(output, count(), format(),
                            surface.buf,
                            a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7], a[8], a[9],
                            b[0].f, b[1].f, b[2].f, b[3].f, b[4].f, b[5].f, b[6].f
                ) > 0);
        }

        
        SDataPlants()
        {}
        
    };
    
    
    // The traits object
    struct TraitsPlants : public DataTraitsImplSimple<
            /*ContainerType*/   std::map<SDataPlants::key_type, SDataPlants>,
            /* HasSection */    false, 
            /* HasKeyValue */   true,
            /* IsSorted */      true,
            /* DomFlags */      DomFlags<flag_RemoveIfNotExistInOneCustomButInDefault>
                        >
    {
        static const char* what() { return "plant surface properties"; }
    };

    
}   // namespace

#endif
