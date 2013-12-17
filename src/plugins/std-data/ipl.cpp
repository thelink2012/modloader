/*
 * Copyright (C) 2013  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#include "data/ipl.h"
#include <functional>
#include <modloader_util_container.hpp>
#include <modloader_util_file.hpp>
using namespace modloader;

// TODO need to test all ipl sections but INST and ENEX

namespace data
{
    
/* Constants and typesdefs */
typedef TraitsIPL::container_type   container_type;
static const integer no_lod = -1;   /* for SObject.lod */
static const integer is_lod = -2;   /* for SObject.lod */

struct IplSectionTableItem
{
    uint8_t     id;
    const char* name;
};

/* Section table for lookup */
static IplSectionTableItem sections[] =
{
    { IPL_NONE, "____" },
    { IPL_INST, "inst" },
    { IPL_CULL, "cull" },
    { IPL_PATH, "path" },
    { IPL_GRGE, "grge" },
    { IPL_ENEX, "enex" },
    { IPL_PICK, "pick" },
    { IPL_JUMP, "jump" },
    { IPL_TCYC, "tcyc" },
    { IPL_AUZO, "auzo" },
    { IPL_MULT, "mult" },
    { IPL_CARS, "cars" },
    { IPL_OCCL, "occl" },
    { IPL_ZONE, "zone" },
    { IPL_NONE, nullptr }
};

/* Search for section in sections table */
IplSectionTableItem* FindSectionFromName(const char* line)
{
    for(IplSectionTableItem* p = sections; p->name; ++p)
    {
        if(!strncmp(p->name, line, 4))
            return p;
    }
    return nullptr;
}

/* Search for section in sections table */
IplSectionTableItem* FindSectionFromId(uint8_t id)
{
    if(id >= IPL_INST && id <= IPL_ZONE)
        return &sections[id];
    return nullptr;
}





/*
 *  Parses an IPL file and put its content in a traits map 
 */
bool TraitsIPL::Parser(const char* filename, container_type& map, bool isDefault)
{
    IplSectionTableItem* section = 0;
    char* line, linebuf[512];
    
    if(FILE* f = fopen(filename, "r"))
    {
        std::vector<SDataIPL_INST::SObject> instList;
        instList.reserve(4098);
        
        /* Read config line from file */
        while(line = ParseConfigLine(fgets(linebuf, sizeof(linebuf), f)))
        {
            if(*line == 0) continue;
            
            /* If no section assigned, try to assign one */
            if(section == nullptr)
            {
                section = FindSectionFromName(line);
            }
            else
            {
                /* End of section? */
                 if(line[0] == 'e' && line[1] == 'n' && line[2] == 'd')
                 {
                     section = nullptr;
                 }
                 else
                 {
                     /* Assign line data to SDataIPL key */
                     
                     /* Inst section must be handled in a special way... */
                     if(section->id == IPL_INST)
                     {
                         /* Store inst data for later post-processing */
                         auto& inst = AddNewItemToContainer(instList);
                         if(!SDataIPL_INST::set(line, inst))
                            instList.pop_back();
                     }
                     /* Other sections can be handled normally */
                     else
                     {
                        /* Scan line into key and put it into the data map */
                        SDataIPL::key_type key;

                        if(key.set(section->id, line))
                            map[key];
                     }
                     
                 }
            }
        }
        
        /* Done with the file */
        fclose(f);

        
        /*
         *   Post process INST section data taken into instList
         *   If this file isDefault, it's integrity must be preserved at a maximum cost because of streamed IPLs...
         */
        {
            SDataIPL::key_type key;
            key.section = IPL_INST;
            SDataIPL_INST& data = key.inst;

            /* Identify all LOD entries */
            if(!isDefault)
            {
                for(auto& obj : instList)
                {
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
                        data.obj.lod = -1;
                        data.lod = instList.at(obj.lod);
                    }

                    key.isDefault = isDefault;
                    data.isDefault = isDefault;

                    /* Add entry to @map */
                    map[key];
                }
            }
        }
        
        return true;
    }
    
    return false;
}


/*
 *  Builds a new IPL file from an array of traits data
 */
bool TraitsIPL::Build(const char* filename, const std::vector<pair_ref_type>& lines)
{
    if(FILE* f = fopen(filename, "w"))
    {
        int8_t newsec, section = -1;
        unsigned int instIndex = 0;
       
        fputs("# IPL generated by modloader\n", f);
        
        /* Function to switch from reading one section to another,
         * automatically puting a section start (and end) at the IPL file */
        auto UpdateSection = [&f, &section](uint8_t newsec, const char* secname)
        {
            if(section != newsec)
            {
                PrintConfigLine(f, "%s%s\n", (section != -1? "end\n" : ""), secname);
                section = newsec;
            }
        };
        
        /* Writes an inst object,
         * effectivelly increasing the instIndex variable if successful */
        auto WriteInstObject = [&f, &instIndex](char* buf, const SDataIPL_INST::SObject& inst, int lod)
        {
            if(SDataIPL_INST::get(buf, inst, lod))
            {
                if(PrintConfigLine(f, "%s\n", buf))
                {
                    ++instIndex;
                    return true;
                }
            }
            return false;
        };
        
        
        /* Iterate on the IPL data */
        for(auto& pair : lines)
        {
            char buf[512];
            auto& key = pair.first.get();
            
            /* Find if the section is valid */
            if(IplSectionTableItem* section =  FindSectionFromId(key.section))
            {
                /* Updates the current section we're working on the file */
                UpdateSection(section->id, section->name);

                /* Inst is a complex section, we need to handle it manually */
                if(section->id == IPL_INST)
                {
                    /*
                     *  If item has an lod built in it (item.lod), write the lod just after it
                     *  If item has no lod built in, test if it has a lod at a fixed line, and use it.
                     *  Otherwise use nolod (-1)
                     */
                    auto& item = key.inst;
                    WriteInstObject(buf, item.obj, (!item.has_lod? (item.obj.lod >= 0? item.obj.lod : -1) : instIndex + 1) );
                    if(item.has_lod) WriteInstObject(buf, item.lod, -1);
                }
                else
                {
                    // Get data into buf string, then write it into the file */
                    if(key.get(buf)) PrintConfigLine(f, "%s\n", buf);
                }
            }
        }
        
        /* Finish up section, so, if there's a section open, close it */
        UpdateSection(-1, "");
        
        /* Close IPL and return success */
        fclose(f);
        return true;
    }
    return false;
}




} // namespace
