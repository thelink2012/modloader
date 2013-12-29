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
        HANDLING_FINAL,  // modloader finalization data
        HANDLING_DATA,
        HANDLING_BOAT,
        HANDLING_BIKE,
        HANDLING_PLANE,
        HANDLING_ANIM
    };


    struct SDataHandling_BASE
    {
        string16 name;
    };

    
    // Basic handling data structure
    struct SDataHandling_DATA : public SDataHandling_BASE
    {
        complex mass;
        complex turnmass;
        complex dragmult;
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
        
        // For space optimization put those char fields here:
        char drivetype, enginetype, babs, flight, rlight;
        
        /* Formating information */
        const char* format() const { return "%s %f %f %f %f %f %f %d %f %f %f %d %f %f %f %c %c %f %f %c %f %f %f %f %f %f %f %f %f %f %d %x %x %c %c %d"; }
        size_t count()       const { return min_count(); }
        static size_t min_count()  { return 36; }
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
                if(EQ(mass) && EQ(turnmass) && EQ(dragmult) && EQ(cmassx) && EQ(cmassy) && EQ(cmassz)
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
                            &mass.f, &turnmass.f, &dragmult.f, &cmassx.f, &cmassy.f, &cmassz.f, &submerg,
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
            return get(line, 0) > 0;
        }
        
        int get(char* line, int dummy_ret_int) const
        {
            return(PrintConfigLine(line, max_count(), format(),
                            name.buf,
                            mass.f, turnmass.f, dragmult.f, cmassx.f, cmassy.f, cmassz.f, submerg,
                            tractionmult.f, tractionloss.f, tractionbias.f, ngears, maxvelocity.f, eacceleration.f,
                            einertia.f, drivetype, enginetype, bdeceleration.f, bbias.f, babs, sterlock.f, sforce.f, sdamping.f,
                            sspd.f, sulimit.f, sllimit.f, sfrbias.f, sadmult.f, seatoffset.f,
                            damagemult.f, monetary, mflags, hflags, flight, rlight, animgroup
                           ));
        }
    };
    
    // Boat section structure
    struct SDataHandling_BOAT : public SDataHandling_BASE
    {
        complex a[14];
        
        /* Formating information */
        static char idchar()       { return '%'; }
        const char* format() const { return "%c %s %f %f %f %f %f %f %f %f %f %f %f %f %f %f"; }
        size_t count()       const { return min_count(); }
        static size_t min_count()  { return 14 + 1 + 1; }
        static size_t max_count()  { return min_count(); }
        
        
        // Compare two boat sections
        bool operator==(const SDataHandling_BOAT& b) const
        {
            const SDataHandling_BOAT& a = *this;
            return(EQ(name) && EQ_ARRAY(a));
        }
        
        /* Sets data from string */
        bool set(const char* line)
        {
            char c = idchar();
            if(ScanConfigLine(line, min_count(), max_count(), format(),
                            &c, name.buf,
                            &a[0].f, &a[1].f, &a[2].f, &a[3].f, &a[4].f, &a[5].f, &a[6].f, &a[7].f, &a[8].f, &a[9].f, &a[10].f,
                            &a[11].f, &a[12].f, &a[13].f
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
            return(get(line, 0) > 0);
        }
        
        int get(char* line, int dummy_ret_int) const
        {
            char c = idchar();
            return(PrintConfigLine(line, max_count(), format(),
                            c, name.buf,
                            a[0].f, a[1].f, a[2].f, a[3].f, a[4].f, a[5].f, a[6].f, a[7].f, a[8].f, a[9].f, a[10].f,
                            a[11].f, a[12].f, a[13].f
                           ));
        }
    };
    
    // Bike section structure
    struct SDataHandling_BIKE : public SDataHandling_BASE
    {
        complex a[15];
        
        /* Formating information */
        static char idchar()       { return '!'; }
        const char* format() const { return "%c %s %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f"; }
        size_t count()       const { return min_count(); }
        static size_t min_count()  { return 15 + 1 + 1; }
        static size_t max_count()  { return min_count(); }
        
        
        // Compare two boat sections
        bool operator==(const SDataHandling_BIKE& b) const
        {
            const SDataHandling_BIKE& a = *this;
            return(EQ(name) && EQ_ARRAY(a));
        }
        
        /* Sets data from string */
        bool set(const char* line)
        {
            char c = idchar();
            if(ScanConfigLine(line, min_count(), max_count(), format(),
                            &c, name.buf,
                            &a[0].f, &a[1].f, &a[2].f, &a[3].f, &a[4].f, &a[5].f, &a[6].f, &a[7].f, &a[8].f, &a[9].f, &a[10].f,
                            &a[11].f, &a[12].f, &a[13].f, &a[14].f
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
            return(get(line, 0) > 0);
        }
        
        /* Gets data from string */
        int get(char* line, int dummy_ret_int) const
        {
            char c = idchar();
            return(PrintConfigLine(line, max_count(), format(),
                            c, name.buf,
                            a[0].f, a[1].f, a[2].f, a[3].f, a[4].f, a[5].f, a[6].f, a[7].f, a[8].f, a[9].f, a[10].f,
                            a[11].f, a[12].f, a[13].f, a[14].f
                           ));
        }
    };

    
    // Plane section structure
    struct SDataHandling_PLANE : public SDataHandling_BASE
    {
        complex a[21];
        
        /* Formating information */
        static char idchar()       { return '$'; }
        const char* format() const { return "%c %s %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f"; }
        size_t count()       const { return min_count(); }
        static size_t min_count()  { return 21 + 1 + 1; }
        static size_t max_count()  { return min_count(); }
        
        
        // Compare two boat sections
        bool operator==(const SDataHandling_PLANE& b) const
        {
            const SDataHandling_PLANE& a = *this;
            return(EQ(name) && EQ_ARRAY(a));
        }
        
        /* Sets data from string */
        bool set(const char* line)
        {
            char c = idchar();
            if(ScanConfigLine(line, min_count(), max_count(), format(),
                            &c, name.buf,
                            &a[0].f, &a[1].f, &a[2].f, &a[3].f, &a[4].f, &a[5].f, &a[6].f, &a[7].f, &a[8].f, &a[9].f, &a[10].f,
                            &a[11].f, &a[12].f, &a[13].f, &a[14].f, &a[15].f, &a[16].f, &a[17].f, &a[18].f, &a[19].f, &a[20].f
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
            return (get(line, 0) > 0);
        }
        
        /* Gets data from string */
        int get(char* line, int dummy_ret_int) const
        {
            char c = idchar();
            return(PrintConfigLine(line, max_count(), format(),
                            c, name.buf,
                            a[0].f, a[1].f, a[2].f, a[3].f, a[4].f, a[5].f, a[6].f, a[7].f, a[8].f, a[9].f, a[10].f,
                            a[11].f, a[12].f, a[13].f, a[14].f, a[15].f, a[16].f, a[17].f, a[18].f, a[19].f, a[20].f
                           ));
        }
    };
    
    
    // Anim section structure
    struct SDataHandling_ANIM // : public SDataHandling_BASE
    {
        integer  id;
        integer  a[20];
        complex  b[13];
        flags    flag;
        
        /* Formating information */
        static char idchar()       { return '^'; }
        const char* format() const { return "%c %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %f %f %f %f %f %f %f %f %f %f %f %f %f %d"; }
        size_t count()       const { return min_count(); }
        static size_t min_count()  { return 35 + 1; }
        static size_t max_count()  { return min_count(); }
        
        
        // Compare two anim sections
        bool operator==(const SDataHandling_ANIM& b) const
        {
            const SDataHandling_ANIM& a = *this;
            return((EQ(id) || a.id == -127 || b.id == -127) && EQ(flag) && EQ_ARRAY(a) && EQ_ARRAY(b));
            
            // id '-127' used for internal usage to tell the comparer to ignore the id during the comparision
        }
        
        /* Sets data from string */
        bool set(const char* line)
        {
            char c = idchar();
            if(ScanConfigLine(line, min_count(), max_count(), format(),
                            &c, &id,
                            &a[0], &a[1], &a[2], &a[3], &a[4], &a[5], &a[6], &a[7], &a[8], &a[9], &a[10],
                            &a[11], &a[12], &a[13], &a[14], &a[15], &a[16], &a[17], &a[18], &a[19],
                            &b[0].f, &b[1].f, &b[2].f, &b[3].f, &b[4].f, &b[5].f, &b[6].f, &b[7].f, &b[8].f, &b[9].f, &b[10].f,
                            &b[11].f, &b[12].f,
                            &flag
                           ) > 0)
            {
                return true;
            }
            return false;
        }
      
        /* Gets data from string */
        bool get(char* line) const
        {
            return(get(line, 0) > 0);
        }
        
        int get(char* line, int dummy_ret_int) const
        {
            char c = idchar();
            return(PrintConfigLine(line, max_count(), format(),
                            c, id,
                            a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7], a[8], a[9], a[10],
                            a[11], a[12], a[13], a[14], a[15], a[16], a[17], a[18], a[19],
                            b[0].f, b[1].f, b[2].f, b[3].f, b[4].f, b[5].f, b[6].f, b[7].f, b[8].f, b[9].f, b[10].f,
                            b[11].f, b[12].f,
                            flag
                           ));
        }
        
    };
    
    
    // Finalization section structure
    struct SDataHandling_FINAL // : public SDataHandling_BASE
    {   // This structure is HUGE!
        
        SDataHandling_DATA  data;
        SDataHandling_ANIM  anim;
        
        uint8_t additional;     // HANDLING_NONE, HANDLING_BOAT, HANDLING_PLANE or HANDLING_BIKE
        bool    hasAnim;        // Tells if anim field is valid
        
        // The additonal handling data
        union
        {
            SDataHandling_BOAT  boat;
            SDataHandling_BIKE  bike;
            SDataHandling_PLANE plane;
        };
        
        
        // Compare two sections
        bool operator==(const SDataHandling_FINAL& b) const
        {
            const SDataHandling_FINAL& a = *this;
            
            // Check for basic fields first
            if(EQ(hasAnim) && EQ(additional) && EQ(data.name))
            {
                // Compare main data
                if(EQ(data))
                {
                    // Compare anim if necessary
                    if(hasAnim == false || EQ(anim))
                    {
                        // Compare additional data
                        switch(additional)
                        {
                            case HANDLING_NONE:  return true;
                            case HANDLING_BOAT:  return EQ(boat);
                            case HANDLING_BIKE:  return EQ(bike);
                            case HANDLING_PLANE: return EQ(plane);
                        }
                    }
                }
            }
            
            return false;
        }
        
        /* Sets data from string */
        bool set(const char* line)
        {
            return false;   // Nope
        }
      
        /* Gets data from string */
        bool get(char* line) const
        {
            int len = 0;

            len += data.get(&line[len], 0);
            
            // Get anim if necessary
            if(hasAnim)
            {
                line[len++] = '\n';
                len += anim.get(&line[len], 0);
            }
                
            // Get additional handling if necessary
            if(additional != HANDLING_NONE)
            {
                line[len++] = '\n';
                
                switch(additional)
                {
                    case HANDLING_BOAT:  len += boat.get(&line[len], 0); break;
                    case HANDLING_BIKE:  len += bike.get(&line[len], 0); break;
                    case HANDLING_PLANE: len += plane.get(&line[len], 0); break;
                    default: return false;  // This should never happen
                }
            }
            
            return true;
        }
    };
    
    
    
    

    struct SDataHandling
    {
        /* Key for an IDE item */
        struct key_type
        {
            uint8_t section;
            union
            {
                integer id;          // Used if section == HANDLING_ANIM
                hash    name;        // Used if section != HANDLING_ANIM
            };
            
            // Set key based on value
            bool set(const SDataHandling& b)
            {
                this->section = b.section;
                
                if(this->section == HANDLING_ANIM)
                    this->id = b.anim.id;
                else
                    this->name.hash = b.base.name.hash;
                
                return true;
            }
            
            // For sorting...
            bool operator<(const key_type& b) const
            {
                const key_type& a = *this;
                
                if(EQ(section)) // If sections are equal
                {
                    // Return which data should come first based on id/name
                    return (section == HANDLING_ANIM? LE(id) : LE(name));
                }
                
                // Otherwise return which section should come first
                return LE(section);
            }
        };
        
        uint8_t section; /* section this data is from */
        
        /* actual data, depending on the section... */
        union
        {
            SDataHandling_BASE   base;  // base for each of the objects below except anim
            SDataHandling_FINAL  final;
            SDataHandling_DATA   data;
            SDataHandling_BOAT   boat;
            SDataHandling_BIKE   bike;
            SDataHandling_PLANE  plane;
            SDataHandling_ANIM   anim;
        };

        
        char* TryToFixLine(const char* line, char* output);  // See handling.cpp
        
        
        /*
        *  Sections table 
        */
        static SectionInfo* GetTable()
        {
            /* Section table for lookup */
            static SectionInfo sections[] =
            {
                { HANDLING_NONE,  ""  },
                { HANDLING_FINAL, ""  },
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
            for(SectionInfo* p = GetTable() + 3; p->name; ++p)
            {
                if(p->name[0] == line[0])
                {
                    return p;
                }
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
                    case HANDLING_FINAL:
                        typedef decltype(a.final) final;
                        return DoFunction<final>()(a.final, b.final);
                    
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
                    case HANDLING_FINAL:
                        typedef decltype(a.final) final;
                        return DoFunction<final,T>()(a.final, b);
                        
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
            return section >= HANDLING_FINAL && section <= HANDLING_ANIM;
        }
        
        /* Sets the current data to the data at @line, knowing that the section to handle is @section */
        bool set(SectionInfo* info, const char* line)
        {
            if(IsReadmeSection(info))
            {
                this->section = HANDLING_DATA;  // Only supports reading basic data from readme files
                return DoIt<bool, call_set>(*this, line, false);
            }
            else
            {
                this->section = info->id;
                if(!DoIt<bool, call_set>(*this, line, false))
                {
                    if(!IsValidSection()) return false;
                    
                    char line_fixed[512];
                    bool bLog = strncmp(line, "$ RCRAIDER", 10) != 0;  // Let's not log R* mistake at RCRAIDER line, he
   
                    if(bLog) Log("Warning: Bad formated handling.cfg line found somewhere, trying to fix it: %s", line);
                    
                    // Let's try to fix this damn line
                    if(*TryToFixLine(line, line_fixed))
                    {
                        // Log the possible fixed line
                        if(bLog) Log("\tFixed line: %s", line_fixed);
                    }
                    else
                    {
                        // Oh, why?
                        if(bLog) Log("\tCould not fix the line, too bad");
                    }
                    
                    return DoIt<bool, call_set>(*this, line_fixed, false);
                }
                return true;
            }
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
        
        // The following avois data finalization before readme files have been completly readen
        // After reading the readme files, set it to true
        static bool& CanFinalizeData()
        {
            static bool bCanFinalizeData = false;
            return bCanFinalizeData;
        }

        // The following is implemented in handling.cpp
        static bool Parser(const char* filename, container_type& map);
        static bool Build(const char* filename, const std::vector<typename super::pair_ref_type>& lines);
        static bool FinalizeData(container_type& map);    // Finalize data, returns false if there's nothing to finalize
        
        bool FinalizeData()
        {
            if(CanFinalizeData())
                return FinalizeData(this->map);
            return false;
        }
        
        bool LoadData()
        {
            if(DataTraits::LoadData(path.c_str(), Parser))
            {
                this->FinalizeData();
                return true;
            }
            return false;
        }
        
    };
}

#endif



