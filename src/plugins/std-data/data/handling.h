/*
 * Copyright (C) 2013  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 *  IDE structures
 *
 */
#ifndef TRAITS_HANDLING_H
#define TRAITS_HANDLING_H

#include "traits.h"
#include "ranges.h"
#include <map>

namespace data
{
    using namespace data::util;

    enum
    {
        HANDLING_NONE,
        HANDLING_DATA,
        HANDLING_BOAT,
        HANDLING_BIKE,
        HANDLING_PLANE,
        HANDLING_ANIM
    };


    struct SDataHandling_BASE
    {
        string16 name;
        integer id;
        
        bool operator==(const SDataHandling_BASE& b) const 
        {
            return false;
        }
        
        bool set(const char*) { return false; }
        bool get(char*) const { return false; }
    };

    //typedef SDataHandling_BASE  SDataHandling_DATA;
    typedef SDataHandling_BASE  SDataHandling_BOAT;
    typedef SDataHandling_BASE  SDataHandling_BIKE;
    typedef SDataHandling_BASE  SDataHandling_PLANE;
    typedef SDataHandling_BASE  SDataHandling_ANIM;
    
    
    struct SDataHandling_DATA : public SDataHandling_BASE
    {
        complex mass;
        complex turnmass;
        complex dragmult;
        complex notused;
        complex cmassx;
        complex cmassy;
        complex cmassz;
        integer submerg;
        complex tractionmult;
        complex tractionloss;
        complex tractionbias;
        integer ngears;
        complex maxvelocity;
        complex eacceleration;
        complex einertia;
        //char    drivetype;
        //char    enginetype;
        complex bdeceleration;
        complex bbias;
        //char    babs;
        complex sterlock;
        complex sforce;
        complex sdamping;
        complex sspd;
        complex sulimit;
        complex sllimit;
        complex sfrbias;
        complex sadmult;
        complex seatoffset;
        complex damagemult;
        integer monetary;
        flags   mflags;
        flags   hflags;
        //char    flight;
        //char    rlight;
        integer animgroup;
        
        char drivetype, enginetype, babs, flight, rlight;
        
        /* Formating information */
        const char* format() const { return "%s %f %f %f %f %f %f %f %d %f %f %f %d %f %f %f %c %c %f %f %c %f %f %f %f %f %f %f %f %f %f %d %x %x %c %c %d"; }
        size_t count()       const { return min_count(); }
        static size_t min_count()  { return 3; }
        static size_t max_count()  { return min_count(); }
        
        // Compare two handling sections
        bool operator==(const SDataHandling_DATA& b) const
        {
            const SDataHandling_DATA& a = *this;
            
            // Compare less-expensive things first
            if(EQ(name) && EQ(mflags) && EQ(hflags) && EQ(drivetype) && EQ(enginetype) && EQ(babs)
            && EQ(flight) && EQ(rlight) && EQ(submerg) && EQ(ngears) && EQ(monetary) && EQ(animgroup))
            {
                // Okay for now, let's continue with the complex types
                // THE FOLLOWING COMPARISION CHAIN WILL BE SLOWWWWWWWWWW, anything we can do for it?
                if(EQ(mass) && EQ(turnmass) && EQ(dragmult) && EQ(notused) && EQ(cmassx) && EQ(cmassy) && EQ(cmassz)
                && EQ(tractionmult) && EQ(tractionloss) && EQ(tractionbias) && EQ(maxvelocity) && EQ(eacceleration)
                && EQ(einertia) && EQ(bdeceleration) && EQ(bbias) && EQ(sterlock) && EQ(sforce) && EQ(sdamping)
                && EQ(sspd) && EQ(sulimit) && EQ(sllimit) && EQ(sfrbias) && EQ(sadmult) && EQ(seatoffset)
                && EQ(damagemult))
                    return true;
            }
            return false;
        }
        
        /* Sets data from string */
        bool set(const char* line)
        {
            if(ScanConfigLine(line, min_count(), max_count(), format(),
                            name.buf,
                            &mass.f, &turnmass.f, &dragmult.f, &notused.f, &cmassx.f, &cmassy.f, &cmassz.f, &submerg,
                            &tractionmult.f, &tractionloss.f, &tractionbias.f, &ngears, &maxvelocity.f, &eacceleration.f,
                            &einertia.f, &drivetype, &enginetype, &bdeceleration.f, &bbias.f, &babs, &sterlock.f, &sforce.f, &sdamping.f,
                            &sspd.f, &sulimit.f, &sllimit.f, &sfrbias.f, &sadmult.f, &seatoffset.f,
                            &damagemult.f, &monetary, &mflags, &hflags, &flight, &rlight, &animgroup
                           ) > 0)
            {
                name.recalc(::toupper);
                return true;
            }
            return false;
        }
      
        /* Gets data from string */
        bool get(char* line) const
        {
            return(PrintConfigLine(line, max_count(), format(),
                            name.buf,
                            mass.f, turnmass.f, dragmult.f, notused.f, cmassx.f, cmassy.f, cmassz.f, submerg,
                            tractionmult.f, tractionloss.f, tractionbias.f, ngears, maxvelocity.f, eacceleration.f,
                            einertia.f, drivetype, enginetype, bdeceleration.f, bbias.f, babs, sterlock.f, sforce.f, sdamping.f,
                            sspd.f, sulimit.f, sllimit.f, sfrbias.f, sadmult.f, seatoffset.f,
                            damagemult.f, monetary, mflags, hflags, flight, rlight, animgroup
                           ) > 0);
        }
    };
    

    
    struct SDataHandling
    {
        /* Key for an IDE item */
        struct key_type
        {
            bool isAnim;            // anim section is not based in names
            union
            {
                integer id;          // Used if isAnim = true
                hash    name;        // Used if isAnim = false
            };
            
            // Set key based on value
            bool set(const SDataHandling& b)
            {
                if(isAnim = (b.section == HANDLING_ANIM))
                    id = b.anim.id;
                else
                    name.hash = b.base.name.hash;
                return true;
            }
            
            // For sorting...
            bool operator<(const key_type& b) const
            {
                const key_type& a = *this;
                return isAnim? LE(id) : LE(name);
            }
        };
        
        uint8_t section; /* section this data is from */
        
        /* actual data, depending on the section... */
        union
        {
            SDataHandling_BASE   base;  // base for each of the objects below except anim
            SDataHandling_DATA   data;
            SDataHandling_BOAT   boat;
            SDataHandling_BIKE   bike;
            SDataHandling_PLANE  plane;
            SDataHandling_ANIM   anim;
        };

        /*
        *  Sections table 
        */
        static SectionInfo* GetTable()
        {
            /* Section table for lookup */
            static SectionInfo sections[] =
            {
                { HANDLING_NONE,  ""  },
                { HANDLING_DATA,  ""  },
                { HANDLING_BOAT,  "%" },
                { HANDLING_BIKE,  "!" },
                { HANDLING_PLANE, "$" },
                { HANDLING_ANIM,  "^" },
                { HANDLING_NONE, nullptr }
            };
            return sections;
        }
        
        /* Search for section in sections table */
        static SectionInfo* FindSectionByLine(const char* line)
        {
            for(SectionInfo* p = GetTable() + 2; p->name; ++p)
            {
                if(p->name[0] == line[0]) return p;
            }
            return FindSectionById(HANDLING_DATA);
        }
        
        /* Search for section in sections table */
        static SectionInfo* FindSectionById(uint8_t id)
        {
            return &GetTable()[id];
        }
        
#if 1
            /* Executes DoFunction operator on current section field */
            template<class ResultType, template<class> class DoFunction>
            static ResultType DoIt(const SDataHandling& a, const SDataHandling& b, ResultType defaultResult)
            {
                switch(a.section)                   
                {                                   
                    case HANDLING_DATA:
                        typedef decltype(a.data) data;
                        return DoFunction<data>()(a.data, b.data);
                        
                    case HANDLING_BOAT:
                        typedef decltype(a.boat) boat;
                        return DoFunction<boat>()(a.boat, b.boat);
                        
                    case HANDLING_BIKE:
                        typedef decltype(a.bike) bike;
                        return DoFunction<bike>()(a.bike, b.bike);
                        
                    case HANDLING_PLANE:
                        typedef decltype(a.plane) plane;
                        return DoFunction<plane>()(a.plane, b.plane); 
                        
                    case HANDLING_ANIM:
                        typedef decltype(a.anim) anim;
                        return DoFunction<anim>()(a.anim, b.anim);
                }
                return defaultResult;
            }
            
            /* Executes DoFunction in a section field sending b as argument */
            template<class ResultType, template<class, class> class DoFunction, class A, class T>
            static ResultType DoIt(A& a, T& b, ResultType defaultResult)
            {
                switch(a.section)                   
                {                                   
                    case HANDLING_DATA:
                        typedef decltype(a.data) data;
                        return DoFunction<data,T>()(a.data, b);
                        
                    case HANDLING_BOAT:
                        typedef decltype(a.boat) boat;
                        return DoFunction<boat,T>()(a.boat, b);
                        
                    case HANDLING_BIKE:
                        typedef decltype(a.bike) bike;
                        return DoFunction<bike,T>()(a.bike, b);
                        
                    case HANDLING_PLANE:
                        typedef decltype(a.plane) plane;
                        return DoFunction<plane,T>()(a.plane, b); 
                        
                    case HANDLING_ANIM:
                        typedef decltype(a.anim) anim;
                        return DoFunction<anim,T>()(a.anim, b);
                }
                return defaultResult;
            }
#endif
        
        /*
         *  Compare two handling data lines
         */
        bool operator==(const SDataHandling& b) const
        {
            const SDataHandling& a = *this;
                
            /* First check if sections are the same */
            if(a.section == b.section)
            {
                /* Then execute std::equal_to on the section field */
                return DoIt<bool, std::equal_to>(a, b, false);
            }
            
            return false;
        }
        
        /*
         *  Checks if section is valid 
         */
        bool IsValidSection() const
        {
            return section >= HANDLING_DATA && section <= HANDLING_ANIM;
        }
        
        /* Sets the current data to the data at @line, knowing that the section to handle is @section */
        bool set(SectionInfo* info, const char* line)
        {
            // We only support IDE CARS section detection from a readme file...
            if(IsReadmeSection(info))
            {
                // TODO
                this->section = HANDLING_NONE;
            }
            else
            {
                this->section = info->id;
            }
            return DoIt<bool, call_set>(*this, line, false);
        }
        
        /* Gets the current data to the string @output */
        bool get(char* output) const
        {
            return this->IsValidSection()
                && DoIt<bool, call_get>(*this, output, false);
        }
        

        

        
        SDataHandling() :
            section(HANDLING_NONE)
        {}
        
    };
    
    // The traits object
    struct TraitsHandling : public DataTraitsImplSimple<
            /*ContainerType*/   std::map<SDataHandling::key_type, SDataHandling>,
            /* HasSection */    true, 
            /* HasKeyValue */   true,
            /* IsSorted */      true,
            /* DomFlags */      DomFlags<flag_RemoveIfNotExistInAnyCustom>
                        >
    {
        static const char* what() { return "handling file"; }
        
        static bool Parser(const char* filename, container_type& map)
        {
            return modloader::SectionParser<handler_type>(filename, map, false);
        }
        
        static bool Build(const char* filename, const std::vector<typename super::pair_ref_type>& lines)
        {
            return modloader::SectionBuilder<handler_type>(filename, lines, false);
        }
        
        bool LoadData()
        {
            return DataTraits::LoadData(path.c_str(), Parser);
        }
        
    };
}

#endif



