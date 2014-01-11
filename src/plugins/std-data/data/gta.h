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


    struct SDataGTA // fix me original sorting is important
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
            int         index;      // Set by the handler Set object, see TraitsGTA
                                    // This index is unique for each path

         /*
            const char* format() const { return "%s %s"; }
            size_t count()       const { return min_count(); }
            static size_t min_count()  { return 2; }
            static size_t max_count()  { return min_count(); }
         */
 
            // for map ordering and equality comparision
            bool operator<(const key_type& b) const
            {
                const key_type& a = *this;
                return EQ(section)? LE(index) : LE(section);
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
            bool set(SectionInfo* info, const char* line)
            {
                bool detecting = IsReadmeSection(info);

                if(info = FindSectionByLine(line))
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

            // Gets data into string @line
            bool get(char* line) const
            {
                return PrintConfigLine(line, "%s %s",
                                       FindSectionById(this->section)->name,
                                       this->path.buf);
            }
        };

    };
    
    // The traits object
    struct TraitsGTA  : DataTraitsBase< std::map<SDataGTA::key_type, SDataGTA> >
    {
        /*
         *  Specialized set/get structure 
         *  Why, you might ask?
         * 
         *  The order of files loaded by this trait matters, so we need to keep that order in someway.
         *  For example, IPL files ENEX section loading order matters so much...
         * 
         *  To keep the loading order, we will make a map of <hash, index> and each SDataGTA::set we do in the SDataGTA
         *  we will send the key hash to receive a index, then we set this index in SDataGTA::index
         * 
         */
        struct handler_type : public modloader::parser::KeyOnly<TraitsGTA>
        {
            typedef std::map<size_t, int> index_map;
            
            // Get index map
            static index_map& get_index_map()
            {
                /*
                 *  Memory on this map will be present on memory for the entire duration of the program
                 *  May not be a problem, but might be good to fix it
                 *  Maybe on Build() call something like clear_index_map() ?
                 */
                static index_map a;
                return a;
            }
            
            // Advances the index and return it
            static int get_index()
            {
                static int index = 0;
                return ++index;
            }
            
            // Indices from readmes must start at higher values to not come before normal indices
            static int get_readme_index()
            {
                static int index = 1000000000;
                return ++index;
            }

            /*
             * Get index based on normalized path hash
             * If index already exist in index map, return it, otherwise return a new index
             * 
             */
            static int get_index(size_t hash, bool bIsReadmeSection)
            {
                index_map& map = get_index_map();
                auto it = map.find(hash);
                if(it == map.end())
                {
                    return (map[hash] = (bIsReadmeSection? get_readme_index() : get_index()));
                }
                return it->second;
            }
            
            // Map setter, overriden for our purposes to set a index for the key
            struct Set
            {
                bool operator()(SectionType section, const char* line, container_type& map, const char* fileov = 0)
                {
                    key_type key;
                    if(key.set(section, line))
                    {
                        key.index = get_index(key.path.hash, IsReadmeSection(section));
                        map[key];
                        return true;
                    }
                    return false;
                }
            };
            
        };
        
        /* What is this? */
        static const bool is_sorted     = true;
        static int domflags()       { return flag_RemoveIfNotExistInOneCustomButInDefault; }
        static const char* what()   { return "level file"; }
        
        /* File loading methods */
        static bool Parser(const char* filename, DataTraits::container_type& map)
        {
            return SectionParserTemplate<false, handler_type>()(filename, map);
        }
        
        static bool Build(const char* filename, const std::vector<DataTraits::pair_ref_type>& lines)
        {
            return SectionBuilderTemplate<false, handler_type>()(filename, lines);
        }
        
        bool LoadData()     // Calls Parser...
        { return DataTraits::LoadData(path.c_str(), Parser);  }
    };

}


#endif

