/*
 * Copyright (C) 2013  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 *
 */
#ifndef DATAIPL_H
#define	DATAIPL_H

namespace data
{
    using namespace data::util;
    
    enum
    {
        IPL_NONE,
        IPL_INST,
        IPL_CULL,
        IPL_PATH,
        IPL_GRGE,
        IPL_ENEX,
        IPL_PICK,
        IPL_JUMP,
        IPL_TCYC,
        IPL_AUZO,
        IPL_MULT,
        IPL_CARS,
        IPL_OCCL,
        IPL_ZONE
    };
    
    struct SDataIPL_INST
    {
        obj_id  id;
        // won't use modelname because it makes no difference
        integer interior;
        vec3    pos;
        vec4    rot;
        integer lod;
        
        bool operator<(const SDataIPL_INST& b) const
        {
            const SDataIPL_INST& a = *this;
            return (LE(id) && LE(interior) && LE(pos) && LE(rot) && LE(lod));
        }
    };
    
    struct SDataIPL
    {
        struct key_type
        {
            uint8_t section;
            
            union
            {
                SDataIPL_INST inst;
            };
            
            
            key_type(uint8_t sec = IPL_NONE) :
                section(sec)
            {}
            
            bool operator<(const key_type& b) const
            {
                switch(section)
                {
                    case IPL_INST:
                        return (inst < b.inst);
                        
                        // TODO
                }
            }
        };

        bool operator==(const SDataIPL&) const
        {
            /* The algorithm for IPLs depend only on the key */
            return true;
        }
    };
    
    struct TraitsIPL : DataTraitsBase<SDataIPL>
    {
        static bool Parser(const char* filename, DataTraits::container_type& map);
        static bool Build(const char* filename, const std::vector<DataTraits::pair_ref_type>& lines);
        bool LoadData() { return DataTraits::LoadData(path.c_str(), Parser); }
    };
};


#endif	/* DATAIPL_H */

