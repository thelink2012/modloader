/*
 * Copyright (C) 2013  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 *  IPL structures
 * 
 */
#ifndef TRAITS_IPL_H
#define	TRAITS_IPL_H

#include "traits.h"
#include <ordered_map.hpp>
#include <algorithm>
#include <modloader_util_file.hpp>

namespace data
{
    using namespace data::util;
    using modloader::ScanConfigLine;
    using modloader::PrintConfigLine;
    
    /* Sections */
    enum
    {
        IPL_NONE,
        IPL_INST,
        IPL_CULL,
        IPL_PATH,   /* Not present in SA */
        IPL_GRGE,
        IPL_ENEX,
        IPL_PICK,
        IPL_JUMP,
        IPL_TCYC,
        IPL_AUZO,
        IPL_MULT,   /* Completly unknown section, neither format or use are known */
        IPL_CARS,
        IPL_OCCL,
        IPL_ZONE
    };

    /* Inst section */
    struct SDataIPL_INST
    {
        /* Structure that represents an single object in the map, essentially the data taken from inst...end */
        struct SObject
        {
            // won't use modelname because it makes no difference

            obj_id  id;
            integer interior;
            vec3    pos;
            vec4    rot;
            integer lod;

            bool operator==(const SObject& b) const
            {
                // Won't compare @lod because it is a relative comparision... the caller will make the proper comparision.
                const SObject& a = *this;
                return EQ(id) && EQ(interior) && EQ(pos) && EQ(rot);
            }
        };

        SObject obj;        /* object */
        SObject lod;        /* lod for object */
        bool has_lod;       /* true if @lod contains valid data */
        bool isDefault;     /* true if this is data taken from the standard game file */

        /* Formating information */
        static const char* format() { return "%d %s %d %f %f %f %f %f %f %f %d"; }
        static size_t count()       { return 11; }
        
        /* Proper inst comparision */
        bool operator==(const SDataIPL_INST& b) const
        {
            const SDataIPL_INST& a = *this;
            
            /* If both base objects are equal, we can continue the comparision... */
            if(EQ(obj))
            {
                /* If objects are equal and one of the objects is default, ignore the lod check */
                if(a.isDefault || b.isDefault)
                {
                    if(a.obj.lod != b.obj.lod)
                        ;// we have a problem, Log it?
                    return true;
                }
                /* Compare lods */
                else if(EQ(has_lod))
                {
                    if(has_lod) return EQ(lod);
                    return true;
                }
            }
            
            return false;
        }

        /* Sets an SObject with string data */
        static bool set(const char* line, SObject& inst)
        {
            char dummy[64];
            return ScanConfigLine(count(), line, format(),
                        &inst.id, dummy, &inst.interior,
                        &inst.pos[0].f, &inst.pos[1].f, &inst.pos[2].f,
                        &inst.rot[0].f, &inst.rot[1].f, &inst.rot[2].f, &inst.rot[3].f,
                        &inst.lod);
        }
        
        /* Gets an SObject to string data. Note the lod will be from the function argument. */
        static bool get(char* line, const SObject& inst, int lod)
        {
            return PrintConfigLine(line, format(),
                    inst.id, "dummy", inst.interior,
                    inst.pos[0].f, inst.pos[1].f, inst.pos[2].f,
                    inst.rot[0].f, inst.rot[1].f, inst.rot[2].f, inst.rot[3].f,
                    lod);
        }
        
        /*
         * get and set must be done manually (see funcs above) for IPL_INST due to it's complexity related to LODs
         */
        bool set(const char* line)
        {
            return false;
        }
        
        bool get(char* line) const
        {
            return false;
        }
    };
    
    /* Cull section */
    struct SDataIPL_CULL
    {
        vec3 center;
        complex unknown1;
        complex length;
        complex bottom;
        complex width;
        complex unknown2;
        complex top;
        flags   flag;
        
        uint8_t formating;   // not an actual parameter
        union
        {
            integer unknown3;   // when formating == 0
            vec4 mirror;        // when formating == 1
        };
        
        bool operator==(const SDataIPL_CULL& b) const
        {
            const SDataIPL_CULL& a = *this;
            
            /* Formating types must be equal... */
            if(EQ(formating))
            {
                /* Compare common fields between types */
                if(EQ(center) && EQ(unknown1) && EQ(length) && EQ(bottom)
                && EQ(width) && EQ(unknown2) && EQ(top) && EQ(flag))
                {
                    /* Compare formating type specific data */
                    switch(formating)
                    {
                        case 0: return EQ(unknown3);
                        case 1: return EQ(mirror);
                    }
                }
            }
            return false;
        }

        const char* format()
        {
            return "";
        }
        
        bool set(const char* line)
        {
            return false;
        }
        
        bool get(char* line) const
        {
            return false;
        }
    };
    
    /* Grge section */
    struct SDataIPL_GRGE
    {
        vec3 pos1;
        vec2 depth;
        vec3 pos2;
        flags flag;
        integer type;
        string8 name;
        
        uint8_t formating;
        union
        {
            struct  // when formating == 1 --- this is Garage eXtender data
            {       // (see http://gtaforums.com/topic/536465-garage-extender/)
                
                integer num_cars;
                flags grgx_type;
                flags door_style;
            };
        };
        
        bool operator==(const SDataIPL_GRGE& b) const
        {
            const SDataIPL_GRGE& a = *this;
            
            /* Formating types must be equal... */
            if(EQ(formating))
            {
                /* Compare common fields between types */
                if(EQ(pos1) && EQ(depth) && EQ(pos2) && EQ(flag) && EQ(type) && EQ(name))
                {
                    /* Compare formating type specific data */
                    switch(formating)
                    {
                        case 0: return true;
                        case 1: return (EQ(num_cars) && EQ(grgx_type) && EQ(door_style));
                    }
                }
            }
            return false;
        }

        const char* format()
        {
            return "";
        }
        
        bool set(const char* line)
        {
            return false;
        }
        
        bool get(char* line) const
        {
            return false;
        }
    };
    
    /* Enex section */
    struct SDataIPL_ENEX
    {
        vec4     entrance;
        vec2     width;
        complex  unknown1;
        vec4     exit;
        integer  interior;
        flags    flag;
        string16 name;  /* needs to be quoted with '"' */
        integer  sky;
        integer  unknown2;
        integer  time1;
        integer  time2;
        
        /* Formating information */
        static const char* format() { return "%f %f %f %f %f %f %f %f %f %f %f %d %d %s %d %d %d %d"; }
        static size_t count()       { return 18; }
        
        /* Comparer */
        bool operator==(const SDataIPL_ENEX& b) const
        {
            const SDataIPL_ENEX& a = *this;
            return (EQ(entrance) && EQ(width) && EQ(unknown1) && EQ(exit) && EQ(interior)
                 && EQ(flag) && EQ(name) && EQ(sky) && EQ(unknown2) && EQ(time1) && EQ(time2));
        }

        /* Sets data from string */
        bool set(const char* line)
        {
            if(ScanConfigLine(count(), line, format(),
                &entrance[0].f, &entrance[1].f, &entrance[2].f, &entrance[3].f,
                &width[0].f, &width[1].f,
                &unknown1.f,
                &exit[0].f, &exit[1].f, &exit[2].f, &exit[3].f,
                &interior, &flag, &name.buf[0], &sky, &unknown2, &time1, &time2
             ))
            {
                /* Calculate the name hash */
                name.recalc(::toupper);
                return true;
            }
            return false;
        }
        
        /* Gets data to string */
        bool get(char* line) const
        {
            return PrintConfigLine(line, format(),
                    entrance[0].f, entrance[1].f, entrance[2].f, entrance[3].f,
                    width[0].f, width[1].f,
                    unknown1.f,
                    exit[0].f, exit[1].f, exit[2].f, exit[3].f,
                    interior, flag, name.buf, sky, unknown2, time1, time2
                 );
        }
    };
    
    /* Pick section */
    struct SDataIPL_PICK
    {
        obj_id id;
        vec3   pos;
        
        bool operator==(const SDataIPL_PICK& b) const
        {
            const SDataIPL_PICK& a = *this;
            return EQ(id) && EQ(pos);
        }
        

        const char* format()
        {
            return "";
        }
        
        bool set(const char* line)
        {
            return false;
        }
        
        bool get(char* line) const
        {
            return false;
        }
    };
    
    /* Jump section */
    struct SDataIPL_JUMP
    {
        vec3    start1;
        vec3    start2;
        vec3    target1;
        vec3    target2;
        vec3    camerapos;
        integer reward;
        
        bool operator==(const SDataIPL_JUMP& b) const
        {
            const SDataIPL_JUMP& a = *this;
            return(EQ(start1) && EQ(start2) && EQ(target1) && EQ(target2) && EQ(camerapos) && EQ(reward));
        }

        const char* format()
        {
            return "";
        }
        
        bool set(const char* line)
        {
            return false;
        }
        
        bool get(char* line) const
        {
            return false;
        }
    };
    
     /* Tcyc section */
    struct SDataIPL_TCYC
    {
        vec3 pos1;      // box edge 1
        vec3 pos2;      // box edge 2
        vec3 unk;       // dunno what is this, not even use if it's a vector
        vec3 opt;       // those parameters are optional... in fact, this isn't a vector... default is (100.0, 1.0, 1.0)
        uint8_t nopt;   // num opt parameters received
        
        bool operator==(const SDataIPL_TCYC& b) const
        {
            const SDataIPL_TCYC& a = *this;
            
            /* Check if num options received is the same */
            if(EQ(nopt))
            {
                /* Comparen non-optional parameters */
                if(EQ(pos1) && EQ(pos2) && EQ(unk))
                {
                    /* Compare option parameters */
                    switch(nopt)
                    {
                        case 0: return true;
                        case 1: return EQ(opt[0]);
                        case 2: return EQ(opt[0]) && EQ(opt[1]);
                        case 3: return EQ(opt[0]) && EQ(opt[1]) && EQ(opt[2]);
                    }
                }
            }
            return false;
        }
        
        const char* format()
        {
            return "";
        }
        
        bool set(const char* line)
        {
            return false;
        }
        
        bool get(char* line) const
        {
            return false;
        }
    };
    
    /* Auzo section */
    struct SDataIPL_AUZO
    {
        string8 name;
        integer id;
        integer state;
        
        uint8_t formating;
        union
        {
            vec3 box[2];    // when formating == 0
            vec4 sphere;    // when formating == 1
        };
        
        bool operator==(const SDataIPL_AUZO& b) const
        {
            const SDataIPL_AUZO& a = *this;
            
            /* Formating types must be equal... */
            if(EQ(formating))
            {
                /* Compare common fields between types */
                if(EQ(name) && EQ(id) && EQ(state))
                {
                    /* Compare formating type specific data */
                    switch(formating)
                    {
                        case 0: return EQ(box[0]) && EQ(box[1]);
                        case 1: return EQ(sphere);
                    }
                }
            }
            return false;
        }
        

        const char* format()
        {
            return "";
        }
        
        bool set(const char* line)
        {
            return false;
        }
        
        bool get(char* line) const
        {
            return false;
        }
    };    
    
    /* Cars section */
    struct SDataIPL_CARS
    {
        vec4    pos;
        /* XXX we can hash the following fields into one hash */
        obj_id  id;
        integer color[2];
        integer force;
        integer alarm;
        integer lock;
        integer unk[2];
        
        bool operator==(const SDataIPL_CARS& b) const
        {
            const SDataIPL_CARS& a = *this;
            return (EQ(pos) && EQ(id) && EQ(color[0]) && EQ(color[1]) && EQ(force)
                 && EQ(alarm) && EQ(lock) && EQ(unk[0]) && EQ(unk[1]));
        }
        

        const char* format()
        {
            return "";
        }
        
        bool set(const char* line)
        {
            return false;
        }
        
        bool get(char* line) const
        {
            return false;
        }
    };
    
     /* Occl section */
    struct SDataIPL_OCCL
    {
        vec2 mid;
        complex bottom;
        vec2 width;
        complex height;
        complex rotation;
        vec2    opt1;       // optional parameter 1 (not sure if this is a vector)
        integer opt2;       // optional parramter 2
        uint8_t nopt;       // number optional parameters
        
        bool operator==(const SDataIPL_OCCL& b) const
        {
            const SDataIPL_OCCL& a = *this;
            
            /* Check if num options received is the same */
            if(EQ(nopt))
            {
                /* Compare non-optional parameters */
                if(EQ(mid) && EQ(bottom) && EQ(width) && EQ(height) && EQ(rotation))
                {
                    /* Compare optional parameters */
                    switch(nopt)
                    {
                        case 0: return true;
                        case 1: return EQ(opt1[0]);
                        case 2: return EQ(opt1[0]) && EQ(opt1[1]);
                        case 3: return EQ(opt1[0]) && EQ(opt1[1]) && EQ(opt2);
                    }
                }
            }
            return false;
        }
        

        const char* format()
        {
            return "";
        }
        
        bool set(const char* line)
        {
            return false;
        }
        
        bool get(char* line) const
        {
            return false;
        }
    };
    
     /* Zone section */
    struct SDataIPL_ZONE
    {
        string8 name;
        integer type;
        vec3    box[2];
        integer island;
        string8 label;
        
        bool operator==(const SDataIPL_ZONE& b) const
        {
            const SDataIPL_ZONE& a = *this;
            return (EQ(name) && EQ(type) && EQ(box[0]) && EQ(box[1]) && EQ(island) && EQ(label));
        }
        

        const char* format()
        {
            return "";
        }
        
        bool set(const char* line)
        {
            return false;
        }
        
        bool get(char* line) const
        {
            return false;
        }
    };
    
    
    /* An IPL line..... */
    struct SDataIPL
    {
        template<class T, class U>
        struct call_get
        {
            bool operator()(const T& a, U& b) { return a.get(b); }
        };
        
        template<class T, class U>
        struct call_set
        {
            bool operator()(T& a, const U& b) { return a.set(b); }
        };
        
        
        /* The key from this is the actual line data... */
        struct key_type
        {
            /* section must be the first item on this structure, so it can be initialized with { } */
            
            int8_t section; /* ipl section this data is from */
            bool isDefault; /* is from main game folder */
            
            /* actual data, depending on the section... */
            union
            {
                SDataIPL_INST inst;
                SDataIPL_CULL cull;
                SDataIPL_GRGE grge;
                SDataIPL_ENEX enex;
                SDataIPL_PICK pick;
                SDataIPL_JUMP jump;
                SDataIPL_TCYC tcyc;
                SDataIPL_AUZO auzo;
                SDataIPL_CARS cars;
                SDataIPL_OCCL occl;
                SDataIPL_ZONE zone;
            };

            /*
             *  Checks if section is valid 
             */
            bool IsValidSection()
            {
                return (section >= IPL_INST && section <= IPL_ZONE);
            }
            
#if 1
            /* Executes DoFunction operator on current section field */
            template<class ResultType, template<class> class DoFunction>
            static ResultType DoIt(const key_type& a, const key_type& b, ResultType defaultResult)
            {
                switch(a.section)                   
                {                                   
                    case IPL_INST:
                        typedef decltype(a.inst) inst;
                        return DoFunction<inst>()(a.inst, b.inst);
                        
                    case IPL_CULL:
                        typedef decltype(a.cull) cull;
                        return DoFunction<cull>()(a.cull, b.cull);
                        
                    case IPL_GRGE:
                        typedef decltype(a.grge) grge;
                        return DoFunction<grge>()(a.grge, b.grge);
                        
                    case IPL_ENEX:
                        typedef decltype(a.enex) enex;
                        return DoFunction<enex>()(a.enex, b.enex); 
                        
                    case IPL_PICK:
                        typedef decltype(a.pick) pick;
                        return DoFunction<pick>()(a.pick, b.pick);
                        
                    case IPL_JUMP:
                        typedef decltype(a.jump) jump;
                        return DoFunction<jump>()(a.jump, b.jump);
                        
                    case IPL_TCYC:
                        typedef decltype(a.tcyc) tcyc;
                        return DoFunction<tcyc>()(a.tcyc, b.tcyc);
                        
                    case IPL_AUZO:
                        typedef decltype(a.auzo) auzo;
                        return DoFunction<auzo>()(a.auzo, b.auzo);
                        
                    case IPL_CARS:
                        typedef decltype(a.cars) cars;
                        return DoFunction<cars>()(a.cars, b.cars);
                        
                    case IPL_OCCL:
                        typedef decltype(a.occl) occl;
                        return DoFunction<occl>()(a.occl, b.occl);
                        
                    case IPL_ZONE:
                        typedef decltype(a.zone) zone;
                        return DoFunction<zone>()(a.zone, b.zone);
                }
                return defaultResult;
            }
            
            /* Executes DoFunction in a section field sending b as argument */
            template<class ResultType, template<class, class> class DoFunction, class T>
            static ResultType DoIt(key_type& a, T& b, ResultType defaultResult)
            {
                switch(a.section)                   
                {                                   
                    case IPL_INST:
                        typedef decltype(a.inst) inst;
                        return DoFunction<inst,T>()(a.inst, b);
                        
                    case IPL_CULL:
                        typedef decltype(a.cull) cull;
                        return DoFunction<cull,T>()(a.cull, b);
                        
                    case IPL_GRGE:
                        typedef decltype(a.grge) grge;
                        return DoFunction<grge,T>()(a.grge, b);
                        
                    case IPL_ENEX:
                        typedef decltype(a.enex) enex;
                        return DoFunction<enex,T>()(a.enex, b); 
                        
                    case IPL_PICK:
                        typedef decltype(a.pick) pick;
                        return DoFunction<pick,T>()(a.pick, b);
                        
                    case IPL_JUMP:
                        typedef decltype(a.jump) jump;
                        return DoFunction<jump,T>()(a.jump, b);
                        
                    case IPL_TCYC:
                        typedef decltype(a.tcyc) tcyc;
                        return DoFunction<tcyc,T>()(a.tcyc, b);
                        
                    case IPL_AUZO:
                        typedef decltype(a.auzo) auzo;
                        return DoFunction<auzo,T>()(a.auzo, b);
                        
                    case IPL_CARS:
                        typedef decltype(a.cars) cars;
                        return DoFunction<cars,T>()(a.cars, b);
                        
                    case IPL_OCCL:
                        typedef decltype(a.occl) occl;
                        return DoFunction<occl,T>()(a.occl, b);
                        
                    case IPL_ZONE:
                        typedef decltype(a.zone) zone;
                        return DoFunction<zone,T>()(a.zone, b);
                }
                return defaultResult;
            }
#endif
            
            /* */
            bool operator==(const key_type& b) const
            {
                const key_type& a = *this;
                
                /* First check if sections are the same */
                if(a.section == b.section)
                {            
                    /* Then execute std::equal_to on the section field */
                    return DoIt<bool, std::equal_to>(a, b, false);
                }
                
                /* The code shall never reach this point */
                return false;
            }
            
            /* Sets the current data to the data at @line, knowing that the section to handle is @section */
            bool set(uint8_t section, const char* line)
            {
                this->section = section;
                return DoIt<bool, call_set>(*this, line, false);
            }
            
            /* Gets the current data to the string @output */
            bool get(char* output)
            {
                return this->IsValidSection()
                    && DoIt<bool, call_get>(*this, output, false);
            }
        };

        /* The value for this is dummy, nothing, things depend only on the key */
        
        bool operator==(const SDataIPL&) const
        { return true; }
    };
    
    
    
    /* The IPL traits */
    struct TraitsIPL : DataTraitsBase< ordered_map<SDataIPL::key_type, SDataIPL> >
    {
        static bool Parser(const char* filename, DataTraits::container_type& map, bool isDefault);
        static bool Build(const char* filename, const std::vector<DataTraits::pair_ref_type>& lines);
        bool LoadData()
        {
            return DataTraits::LoadData(path.c_str(), [this](const char* filename, container_type& map)
            {
                return Parser(filename, map, this->isDefault);
            });
        }
    };
    
};


#endif

