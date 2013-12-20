/*
 * Copyright (C) 2013  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 *  gta.dat/default.dat structures
 * 
 */
#ifndef TRAITS_GTA_H
#define	TRAITS_GTA_H

#include "traits.h"
#include <map>
#include <modloader_util_path.hpp>

namespace data
{
    enum
    {
        GTA_NONE,
        GTA_IMG,
        GTA_TXD,
        GTA_DFF,
        GTA_COL,
        GTA_IDE,
        GTA_IPL,
        GTA_HIER,
        GTA_SPLASH,
        GTA_EXIT
    };


    struct SDataGTA
    {

        /*
        *  Sections table 
        */
        static SectionInfo* GetTable()
        {
            static SectionInfo a[] =
            {
                { GTA_NONE,   ""            },
                { GTA_IMG,    "IMG"         },
                { GTA_TXD,    "TEXDICTION"  },
                { GTA_DFF,    "MODELFILE"   },
                { GTA_COL,    "COLFILE"     },
                { GTA_IDE,    "IDE"         },
                { GTA_IPL,    "IPL"         },
                { GTA_HIER,   "HIERFILE"    },
                { GTA_SPLASH, "SPLASH"      },
                { GTA_EXIT,   "EXIT"        },
                { GTA_NONE,   nullptr       }
            };
            return a;
        };

        // Dummy structure...
        bool operator==(const SDataGTA&) const
        {
            return true;
        }
        
        // The actual data is on the key
        struct key_type
        {
            uint8_t     section;
            string64    path;

            // for map ordering and equality comparision
            bool operator<(const key_type& b) const
            {
                const key_type& a = *this;
                return EQ(section)? LE(path) : LE(section);
            }
            
            // 
            bool operator==(const key_type& b) const
            {
                const key_type& a = *this;
                return (EQ(section) && EQ(path));
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
            
            
            // Sets this data from string @line
            bool set(SectionInfo* info, const char* line, bool detecting = false)
            {
                if(info || (info = FindSectionByLine(line)))
                {
                    const char* path = 0;

                    // Find path in the line
                    for(const char* p = line; *p; ++p)
                    {
                        // Find the first space, before it is the section-type and after it the path
                        if(*p == ' ')
                        {
                            // Skip the spaces to get the path
                            while(*++p == ' ') {}
                            // Get the path 
                            path = (*p == 0? nullptr : p);
                            break;
                        }
                    }

                    // If the line has a path (if it doesn't, the user is doing something wrong), continue the set
                    if(path)
                    {
                        std::string normalized = modloader::NormalizePath(path).c_str();
                        
                        // If detecting from a text file and path string do not contain any path slash, ignore it
                        if(detecting && (normalized.find_first_of(modloader::cNormalizedSlash) == normalized.npos))
                        {
                            return false;
                        }
                        
                        // Setup
                        this->section = info->id;
                        strcpy(this->path.buf, normalized.c_str());
                        
                        // Calculate the hash (no need of ::toupper functor because the normalize path is... well... normalized)
                        this->path.recalc();
                        return true;
                    }
                }
                return false;
            }
            
            // Auto-detect section and stuff. Used to read readme files lines.
            bool set(const char* line)
            {
                return set(nullptr, line, true);
            }

            // Gets data into string @line
            bool get(char* line) const
            {
                return PrintConfigLine(line, "%s %s",
                                       FindSectionById(this->section)->name,
                                       this->path.buf);
            }
        };

    };
    
    /* The gta.dat traits */
    struct TraitsGTA : DataTraitsBase< std::map<SDataGTA::key_type, SDataGTA> >
    {
        static bool Parser(const char* filename, container_type& map)
        {
            return modloader::SectionParserNoSection<modloader::parser::KeyOnly<DataTraits>>(filename, map);
        }
        
        static bool Build(const char* filename, const std::vector<DataTraits::pair_ref_type>& lines)
        {
            return modloader::SectionBuilderNoSection<modloader::parser::KeyOnly<DataTraits>>(filename, lines);
        }
        
        bool LoadData()
        {
            return DataTraits::LoadData(path.c_str(), Parser);
        }
    };
    
    
}


#endif

