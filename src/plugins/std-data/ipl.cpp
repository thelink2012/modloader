/*
 * Copyright (C) 2013  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 *  IPL Parser and Builder
 *  This code is not in the header because it's kinda huge and the header is already huge
 */
#include "data.h"
#include "data/ipl.h"
#include <functional>
#include <modloader_util_container.hpp>
#include <modloader_util_parser.hpp>
using namespace modloader;


namespace data
{
    
/* Constants and typesdefs */
typedef std::vector<SDataIPL_INST::SObject> InstListType;
typedef TraitsIPL::container_type   container_type;
static const integer no_lod = -1;   /* for SObject.lod */
static const integer is_lod = -2;   /* for SObject.lod */

/*
 * IPL handling is very specific in the INST section, let's derive from KeyOnly
 * functor and customize it to handle INST section
 */
struct KeyOnlyForIPL : public parser::KeyOnly<TraitsIPL>
{
    typedef KeyOnly     super;
        
    /* The set functor for SectionParser */
    struct Set
    {
        InstListType& instList;
            
        Set(InstListType& instList) : instList(instList)
        {}
            
        bool operator()(SectionInfo* section, const char* line, container_type& map)
        {
            /* Inst section must be handled in a special way... */
            if(section->id == IPL_INST)
            {
                /* Store inst data for later post-processing */
                instList.emplace_back();
                auto& inst = instList.back();
                if(!SDataIPL_INST::set(line, inst))
                {
                    instList.pop_back();
                    return false;
                }
                return true;
            }
            else
            {
                /* Other sections can be handled normally */
                return super::Set()(section, line, map);
            }
        }
    };
    
    /* The get function for SectionBuilder */
    struct Get
    {
        unsigned int instIndex;
        
        Get() : instIndex(0)
        {}
        
        /* Writes an @inst object into the string @buf acumulating the @buf length at @lenaccum */
        bool WriteInstObject(char* buf, const SDataIPL_INST::SObject& inst, int lod, size_t& lenaccum)
        {
            char getbuf[512];
            if(SDataIPL_INST::get(getbuf, inst, lod))
            {
                /* Write the content at the null terminator (seeing lenaccum) */
                int result = sprintf(&buf[lenaccum], "%s", getbuf);
                if(result >= 0)
                {
                    ++instIndex;        // Increase the current inst being written index (for LODs)
                    lenaccum += result; // Accumulate the length
                    return true;
                }
            }
            return false;
        };
        
        bool operator()(SectionType section, char* line, const key_type& key, const value_type& value)
        {
            // If inst section, we need to handle in a special way
            if(section->id == IPL_INST)
            {
                size_t total_len = 0;
                auto& item = key.inst;
                
                /* If object has no lod = but it's .lod field is a valid lod, use it, otherwise use -1
                 * If object has lod, write the lod object just after it
                 */
                int obj_lod = (!item.has_lod? (item.obj.lod >= 0? item.obj.lod : -1) : instIndex + 1);
                if(WriteInstObject(line, item.obj, obj_lod, total_len))
                {
                    if(item.has_lod)
                    {
                        // End line the last object
                        line[total_len++] = '\n';
                        // Write LOD object into line
                        return WriteInstObject(line, item.lod, -1, total_len);
                    }
                    return true;
                }
            }
            else
            {
                // Normal handling
                return super::Get()(section, line, key, value);
            }
            return false;
        }
        
        
    };
    
};

/*
 *  Builds a new IPL file from an array of traits data
 */
bool TraitsIPL::Build(const char* filename, const std::vector<pair_ref_type>& lines)
{
    return SectionBuilder<KeyOnlyForIPL>(filename, lines);
}


/*
 *  Parses an IPL file and put its content in a traits map 
 */
bool TraitsIPL::Parser(const char* filename, container_type& map, bool isDefault)
{
    InstListType instList;
    instList.reserve(4096);
    
    if(SectionParser(filename, map, KeyOnlyForIPL::Find(), KeyOnlyForIPL::Set(instList)))
    {
        /*
         *   Post process INST section data taken into instList
         *   If this file isDefault, it's integrity must be preserved at a maximum cost because of streamed IPLs...
         */
        
        SDataIPL::key_type key;
        key.section = IPL_INST;
        SDataIPL_INST& data = key.inst;

        /* Identify all LOD entries */
        if(!isDefault)
        {
            for(auto& obj : instList)
            {
                obj.linked = false;
                if(obj.lod != no_lod && obj.lod != is_lod)
                {
                    instList.at(obj.lod).lod = is_lod;
                }
            }
        }

        /* Add entries to @map */
        for(auto& obj : instList)
        {
            /* Add object to map only if it isn't a LOD */
            if(obj.lod != is_lod)
            {
                data.obj = obj;

                /* Copy the LOD from this object into it's data memory
                * (But it should not happen if the data is a default data because default line order matters!) */
                if(data.has_lod = (obj.lod != no_lod && !isDefault))
                {
                    // Check if lod is being linked twice or more
                    auto& lod_obj = instList.at(obj.lod);

                    if(lod_obj.linked)
                    {
                        data.obj.lod = -1;
                        data.has_lod = false;
                        dataPlugin->Log("Warning: LOD at index %d being used two or more times at scene file \"%s\"",
                                        obj.lod, filename);
                    }
                    else
                    {
                        data.obj.lod = -1;
                        lod_obj.linked = true;
                        data.lod = lod_obj;
                    }
                }

                key.isDefault = isDefault;
                data.isDefault = isDefault;

                /* Add entry to @map */
                map[key];
            }
        }
            
        return true;
    }
    
    return false;
}


} // namespace
