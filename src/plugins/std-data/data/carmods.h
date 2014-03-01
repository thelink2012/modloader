/*
 * Copyright (C) 2013  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 *  Carmods structures
 *
 */
#ifndef TRAITS_CARMODS_H
#define TRAITS_CARMODS_H

#include "traits.h"
#include "ranges.h" // for IsUpgradeModel
#include <map>

namespace data
{
    using namespace data::util;

    enum
    {
        CARMODS_NONE,
        CARMODS_LINK,
        CARMODS_MODS,
        CARMODS_WHEEL
    };

    
    // Link section structure
    struct SDataCarmods_LINK
    {
        string24 link1;
        string24 link2;
        
        /* Formating information */
        const char* format() const { return "%s %s"; }
        size_t count()       const { return min_count(); }
        static size_t min_count()  { return 2; }
        static size_t max_count()  { return min_count(); }
        
        // Compare two LINK sections
        bool operator==(const SDataCarmods_LINK& b) const
        {
            const SDataCarmods_LINK& a = *this;
            
            // Check if data is swapped
            if(a.link1 == b.link2)
            {
                // Compare in a swapped manner
                return a.link2 == b.link1;
            }
            else
            {
                // Compare in a normal manner
                return EQ(link1) && EQ(link2);
            }
        }
        
        /* Sets data from string */
        bool set(const char* line)
        {
            if(ScanConfigLine(line, min_count(), max_count(), format(), link1.buf, link2.buf) > 0)
            {
                // Calculate string hash
                link1.recalc(::toupper);
                link2.recalc(::toupper);
                return true;
            }
            return false;
        }
      
        /* Gets data from string */
        bool get(char* line) const
        {
            return PrintConfigLine(line, count(), format(), link1.buf, link2.buf) > 0;
        }
    };
    
    // Mods section structure
    struct SDataCarmods_MODS    // This structure is HUGE! Around 500 bytes
    {
        string24 carname;
        string24 mods[18];
        uint8_t  nmods;
        
        /* Formating information */
        const char* format() const { return "%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s"; }
        int count()       const { return min_count() + nmods; }
        static int min_count()  { return 1; }
        static int max_count()  { return 1 + 18; }
        
        // Compare two MODS sections
        bool operator==(const SDataCarmods_MODS& b) const
        {
            const SDataCarmods_MODS& a = *this;
            
            // Check if num mods is equal and then the carname used
            if(EQ(nmods) && EQ(carname))
            {
                // Compare the car mods
                for(size_t i = 0; i < nmods; ++i)
                {
                    if(EQ(mods[i]) == false) return false;
                }
                // Everything is equal
                return true;
            }
            return false;
        }
        
        /* Sets data from string */
        bool set(const char* line)
        {
            int nargs = ScanConfigLine(line, min_count(), max_count(), format(),
                            carname.buf,
                            mods[0].buf, mods[1].buf, mods[2].buf, mods[3].buf, mods[4].buf, mods[5].buf, mods[6].buf,
                            mods[7].buf, mods[8].buf, mods[9].buf, mods[10].buf, mods[11].buf, mods[12].buf, mods[13].buf,
                            mods[14].buf, mods[15].buf, mods[16].buf, mods[17].buf
                        );
            
            if(nargs >= min_count())
            {
                this->nmods = nargs - min_count();
                this->carname.recalc(::toupper);
                return true;
            }
            return false;
        }
      
        /* Gets data from string */
        bool get(char* line) const
        {
            return PrintConfigLine(line, count(), format(),
                            carname.buf,
                            mods[0].buf, mods[1].buf, mods[2].buf, mods[3].buf, mods[4].buf, mods[5].buf, mods[6].buf,
                            mods[7].buf, mods[8].buf, mods[9].buf, mods[10].buf, mods[11].buf, mods[12].buf, mods[13].buf,
                            mods[14].buf, mods[15].buf, mods[16].buf, mods[17].buf
                        ) > 0;
        }
    };
    
    // Mods section structure
    struct SDataCarmods_WHEEL    // This structure is HUGE! Around 400 bytes
    {
        integer id;
        string24 wheels[15];
        uint8_t  nwheels;
        
        /* Formating information */
        const char* format() const { return "%d %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s"; }
        int count()       const { return min_count() + nwheels; }
        static int min_count()  { return 1; }
        static int max_count()  { return 1 + 15; }
        
        // Compare two wheels sections
        bool operator==(const SDataCarmods_WHEEL& b) const
        {
            const SDataCarmods_WHEEL& a = *this;
            
            // Check if num wheels is equal and then the id used
            if(EQ(nwheels) && EQ(id))
            {
                // Compare the wheels
                for(size_t i = 0; i < nwheels; ++i)
                {
                    if(EQ(wheels[i]) == false) return false;
                }
                // Everything is equal
                return true;
            }
            return false;
        }
        
        /* Sets data from string */
        bool set(const char* line)
        {
            int nargs = ScanConfigLine(line, min_count(), max_count(), format(),
                            &id,
                            wheels[0].buf, wheels[1].buf, wheels[2].buf, wheels[3].buf, wheels[4].buf, wheels[5].buf, wheels[6].buf,
                            wheels[7].buf, wheels[8].buf, wheels[9].buf, wheels[10].buf, wheels[11].buf, wheels[12].buf, wheels[13].buf,
                            wheels[14].buf
                        );
            
            if(nargs >= min_count())
            {
                this->nwheels = nargs - min_count();
                return true;
            }
            return false;
        }
      
        /* Gets data from string */
        bool get(char* line) const
        {
            return PrintConfigLine(line, count(), format(),
                            id,
                            wheels[0].buf, wheels[1].buf, wheels[2].buf, wheels[3].buf, wheels[4].buf, wheels[5].buf, wheels[6].buf,
                            wheels[7].buf, wheels[8].buf, wheels[9].buf, wheels[10].buf, wheels[11].buf, wheels[12].buf, wheels[13].buf,
                            wheels[14].buf
                        ) > 0;
        }
    };

    
    
    struct SDataCarmods
    {
        /* Key for an IDE item */
        struct key_type
        {
            uint8_t section;
            union
            {
                struct {                // Used if section == CARMODS_LINK
                    hash link1;
                    hash link2;
                };
                
                hash carname;           // Used if section == CARMODS_MODS
                integer id;             // Used if section == CARMODS_WHEEL
            };
            
            
            // Set key based on value
            bool set(const SDataCarmods& b)
            {
                this->section = b.section;
                
                switch(this->section)
                {
                    case CARMODS_LINK:
                        link1.hash = b.link.link1.hash;
                        link2.hash = b.link.link2.hash;
                        break;
                        
                    case CARMODS_MODS:
                        carname.hash = b.mods.carname.hash;
                        break;
                        
                    case CARMODS_WHEEL:
                        id = b.wheel.id;
                        break;
                        
                    default:
                        return false;
                }
                return true;
            }
            
            // For sorting...
            bool operator<(const key_type& b) const
            {
                const key_type& a = *this;
                
                if(EQ(section))
                {
                    switch(this->section)
                    {
                        case CARMODS_LINK:
                            if(a.link1 == b.link2)  // Let's see if the link positions are swapped
                                return false;       // Both are equal keys, so they're not less than
                            return LE(link1);       // Not swapped, return default less than

                        case CARMODS_MODS:  return LE(carname);
                        case CARMODS_WHEEL: return LE(id);
                        default:             return false;
                    }
                }
                
                return LE(section);
            }
        };
        
        uint8_t section; /* section this data is from */
        
        /* actual data, depending on the section... */
        union
        {
            SDataCarmods_LINK   link;
            SDataCarmods_MODS   mods;
            SDataCarmods_WHEEL  wheel;
        };

        /*
        *  Sections table 
        */
        static SectionInfo* GetTable()
        {
            /* Section table for lookup */
            static SectionInfo sections[] =
            {
                { CARMODS_NONE,     ""      },
                { CARMODS_LINK,     "link"  },
                { CARMODS_MODS,     "mods"  },
                { CARMODS_WHEEL,    "wheel" },
                { CARMODS_NONE,     nullptr }
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
        
#if 1
            /* Executes DoFunction operator on current section field */
            template<class ResultType, template<class> class DoFunction>
            static ResultType DoIt(const SDataCarmods& a, const SDataCarmods& b, ResultType defaultResult)
            {
                switch(a.section)                   
                {             
                    case CARMODS_LINK:
                        typedef decltype(a.link) link;
                        return DoFunction<link>()(a.link, b.link);
                    
                    case CARMODS_MODS:
                        typedef decltype(a.mods) mods;
                        return DoFunction<mods>()(a.mods, b.mods);
                        
                    case CARMODS_WHEEL:
                        typedef decltype(a.wheel) wheel;
                        return DoFunction<wheel>()(a.wheel, b.wheel);
                }
                return defaultResult;
            }
            
            /* Executes DoFunction in a section field sending b as argument */
            template<class ResultType, template<class, class> class DoFunction, class A, class T>
            static ResultType DoIt(A& a, T& b, ResultType defaultResult)
            {
                switch(a.section)                   
                {             
                    case CARMODS_LINK:
                        typedef decltype(a.link) link;
                        return DoFunction<link,T>()(a.link, b);
                    
                    case CARMODS_MODS:
                        typedef decltype(a.mods) mods;
                        return DoFunction<mods,T>()(a.mods, b);
                        
                    case CARMODS_WHEEL:
                        typedef decltype(a.wheel) wheel;
                        return DoFunction<wheel,T>()(a.wheel, b);
                }
                return defaultResult;
            }
#endif
        
        /*
         *  Compare two carmods data lines
         */
        bool operator==(const SDataCarmods& b) const
        {
            const SDataCarmods& a = *this;
                
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
            return section >= CARMODS_LINK && section <= CARMODS_WHEEL;
        }
        
        /* Sets the current data to the data at @line, knowing that the section to handle is @info */
        bool set(SectionInfo* info, const char* line)
        {
            if(IsReadmeSection(info))
            {
                using namespace modloader::parser;
                
                this->section = CARMODS_NONE;   // if no section is found, DoIt will return false with this value

                // Find out if the first token is a numeric digit
                bool isDigit = true;
                for(const char* x = line; *x && !isspace(*x); ++x)
                {
                    if(isdigit(*x) == false)
                    {
                        isDigit = false;
                        break;
                    }
                }
                
                // If the first token on the line is a digit, that means it might be a wheel section line
                if(isDigit)
                {
                    // Scan the digit and the string after the digit
                    int id; char buf[24];
                    if(sscanf(line, "%d %23s", &id, buf) == 2)
                    {
                        // Check if the id isn't too high, otherwise it is a IDE line not a carmods line
                        if(id < 10)
                        {
                            // If the string after the digit starts with wheel, well, that's our line!
                            if(modloader::starts_with(buf, "wheel_", false))
                                this->section = CARMODS_WHEEL;
                        }
                    }
                }
                else if(isalnum(*line))
                {
                    // It's not a digit, so it's either a link or mods section or neither
                    char buf1[24], buf2[24];
                    if(sscanf(line, "%23s %23s", buf1, buf2) == 2)
                    {
                        // Check if second string is a upgrade model name
                        if(IsUpgradeModel(buf2))
                        {
                            // So we have to make a choice between LINK and MODS section
                            // It's simple, if first string is a upgrade model too (not a car model) it's a LINK section
                            this->section = IsUpgradeModel(buf1)? CARMODS_LINK : CARMODS_MODS;
                        }
                    }
                }
                
                // Done trying to understand the line, see if any result...
                if(this->section != CARMODS_NONE)
                    return DoIt<bool, call_set>(*this, line, false);
                return false;
            }
            else
            {
                this->section = info->id;
                return DoIt<bool, call_set>(*this, line, false);
            }
        }
        
        /* Gets the current data to the string @output */
        bool get(char* output) const
        {
            return this->IsValidSection()
                && DoIt<bool, call_get>(*this, output, false);
        }

        
        SDataCarmods() :
            section(CARMODS_NONE)
        {}
        
    };
    
    
    // The traits object
    struct TraitsCarmods : public DataTraitsImplSimple<
            /*ContainerType*/   std::map<SDataCarmods::key_type, SDataCarmods>,
            /* HasSection */    true, 
            /* HasKeyValue */   true,
            /* IsSorted */      true,
            /* DomFlags */      DomFlags<flag_RemoveIfNotExistInOneCustomButInDefault>
                        >
    {
        static const char* what() { return "vehicle upgrades file"; }
    };

    
}   // namespace

#endif
