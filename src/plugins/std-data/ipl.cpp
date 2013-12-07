/*
 * Copyright (C) 2013  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */

#include "data/traits.h"
#include <modloader_util.hpp>
using namespace modloader;

#include <functional>

namespace data {

static bool InstHandler(const char* line, TraitsIPL::container_type& map)
{
    char dummy[64];
    SDataIPL::key_type key(IPL_INST);
    SDataIPL_INST& inst = key.inst;
    
    if(sscanf_cfg(11, line, "%d %s %d %f %f %f %f %f %f %f %d",
           &inst.id, dummy, &inst.interior,
           &inst.pos[0], &inst.pos[1], &inst.pos[2],
           &inst.rot[0], &inst.rot[1], &inst.rot[2], &inst.rot[3],
           &inst.lod))
    {
        map[key];
        return true;
    }
    
    return false;
}

    
bool TraitsIPL::Parser(const char* filename, TraitsIPL::container_type& map)
{
    
    static SSectionHandler<decltype(map)> handlers[] =
    {
        { "inst", InstHandler },
        { 0, 0 }
    };
    return SectionParser(filename, map, handlers);
}

bool TraitsIPL::Build(const char* filename, const std::vector<DataTraits::pair_ref_type>& lines)
{
    if(FILE* f = fopen(filename, "w"))
    {
        uint8_t newsec, section = -1;
       
        auto UpdateSection = [&section, &f](uint8_t newsec, const char* secname)
        {
            if(section != newsec)
            {
                fprintf(f, "%s%s\n", (section != -1? "end\n" : ""), secname);
                section = newsec;
            }
        };
        
        for(auto& pair : lines)
        {
            switch(newsec = pair.first.section)
            {
                case IPL_INST:
                {
                    const char* dummy = "dummy";
                    auto& inst = pair.first.inst;
                    UpdateSection(newsec, "inst");
                    
                    fprintf(f, "%d %s %d %f %f %f %f %f %f %f %d\n",
                        inst.id, dummy, inst.interior,
                        inst.pos[0], inst.pos[1], inst.pos[2],
                        inst.rot[0], inst.rot[1], inst.rot[2], inst.rot[3],
                        inst.lod);
                    break;
                }
                    
                default:
                    // ????
                    break;
                    
            }
        }
        
        if(section != -1) fprintf(f, "end\n");
        fclose(f);
        return true;
    }
    return false;
}


} // namespace
