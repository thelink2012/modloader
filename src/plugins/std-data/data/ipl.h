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
            return ScanConfigLine(line, count(), format(),
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
            vec4 mirror;        // when formating == 0
            integer unknown3;   // when formating == 1
        };
        
        
        /* Complete formating information */
        static const char* format(uint8_t formating, size_t& count)
        {
            switch(formating)
            {
                case 0:  count = 14; return "%f %f %f %f %f %f %f %f %f %d %f %f %f %f";
                case 1:  count = 11; return "%f %f %f %f %f %f %f %f %f %d %d";
                default: count = -1; return "";
            }
        }
        
        /* Formating information */
        const char* format() const { size_t a; return (format(formating, a));    }
        size_t count()       const { size_t a; return (format(formating, a), a); }

        /* Comparision */
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
                        case 0: return EQ(mirror);
                        case 1: return EQ(unknown3);
                    }
                }
            }
            return false;
        }
        
        /* Sets data from string */
        bool set(const char* line)
        {
            /* Try with formating types 0 and 1 */
            for(int i = 0; i < 2; ++i)
            {
                /* Try with this formating */
                switch(this->formating = i)
                {
                    case 0: /* vec4 at the end */
                        if(ScanConfigLine(line, count(), format(),
                                  &center[0].f, &center[1].f, &center[2].f,
                                  &unknown1.f, &length.f, &bottom.f, &width.f, &unknown2.f, &top.f, &flag,
                                  &mirror[0].f, &mirror[1].f, &mirror[2].f, &mirror[3].f)) return true;
                        break;
                    
                    case 1: /* integer at the end */
                        if(ScanConfigLine(line, count(), format(),
                                  &center[0].f, &center[1].f, &center[2].f,
                                  &unknown1.f, &length.f, &bottom.f, &width.f, &unknown2.f, &top.f, &flag,
                                  &unknown3)) return true;
                        break;
                }
            }
            
            /* Nope? formating state is undefined. */
            return false;
        }
        
        /* Gets data to string */
        bool get(char* line) const
        {
            switch(this->formating)
            {
                case 0: /* vec4 at the end */
                    return PrintConfigLine(line, format(),
                                center[0].f, center[1].f, center[2].f,
                                unknown1.f, length.f, bottom.f, width.f, unknown2.f, top.f, flag,
                                mirror[0].f, mirror[1].f, mirror[2].f, mirror[3].f);
                    break;
                
                case 1: /* integer at the end */
                    return PrintConfigLine(line, format(),
                                center[0].f, center[1].f, center[2].f,
                                unknown1.f, length.f, bottom.f, width.f, unknown2.f, top.f, flag,
                                unknown3);
            }
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
            struct  // when formating == 0 --- this is Garage eXtender data
            {       // (see http://gtaforums.com/topic/536465-garage-extender/)
                
                integer num_cars;
                flags grgx_type;
                flags door_style;
            };
            struct  // when formating == 1
            {       // game's default data
            };
        };
        
        
        /* Complete formating information */
        static const char* format(uint8_t formating, size_t& count)
        {
            switch(formating)
            {
                case 0:  count = 14; return "%f %f %f %f %f %f %f %f %d %d %s %d %d %d";
                case 1:  count = 11; return "%f %f %f %f %f %f %f %f %d %d %s";
                default: count = -1; return "";
            }
        }
        
        /* Formating information */
        const char* format() const { size_t a; return (format(formating, a));    }
        size_t count()       const { size_t a; return (format(formating, a), a); }
        
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
                        case 0: return (EQ(num_cars) && EQ(grgx_type) && EQ(door_style));
                        case 1: return true;
                    }
                }
            }
            return false;
        }
        
        /* Sets data from string */
        bool set(const char* line)
        {
            bool bResult = false;
            
            /* Try with formating types 0 and 1 */
            for(int i = 0; i < 2; ++i)
            {
                /* Try with this formating */
                switch(this->formating = i)
                {
                    case 0: /* grgx data at the end */
                        bResult = ScanConfigLine(line, count(), format(),
                                        &pos1[0].f, &pos1[1].f, &pos1[2].f,
                                        &depth[0].f, &depth[1].f,
                                        &pos2[0].f, &pos2[1].f, &pos2[2].f,
                                        &flag, &type, name.buf,
                                        &num_cars, &grgx_type, &door_style);
                        break;
                    
                    case 1: /* no grgx data at the end */
                        bResult = ScanConfigLine(line, count(), format(),
                                        &pos1[0].f, &pos1[1].f, &pos1[2].f,
                                        &depth[0].f, &depth[1].f,
                                        &pos2[0].f, &pos2[1].f, &pos2[2].f,
                                        &flag, &type, name.buf);
                        break;
                }
                
                /* If found correct formating, go ahead and return,
                 * but before we need to calculate the name hash */
                if(bResult)
                {
                    name.recalc(::toupper);
                    return true;
                }
            }
            
            return false;
        }
        
        /* Gets data to string */
        bool get(char* line) const
        {
            switch(this->formating)
            {
                case 0: /* grgx data at the end */
                    return PrintConfigLine(line, format(),
                                pos1[0].f, pos1[1].f, pos1[2].f,
                                depth[0].f, depth[1].f,
                                pos2[0].f, pos2[1].f, pos2[2].f,
                                flag, type, name.buf,
                                num_cars, grgx_type, door_style);
                    
                case 1: /* no grgx data at the end */
                    return PrintConfigLine(line, format(),
                                pos1[0].f, pos1[1].f, pos1[2].f,
                                depth[0].f, depth[1].f,
                                pos2[0].f, pos2[1].f, pos2[2].f,
                                flag, type, name.buf);
            }
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
            if(ScanConfigLine(line, count(), format(),
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
        
        /* Formating information */
        const char* format() const { return "%d %f %f %f"; }
        size_t count()       const { return 4; }
        
        bool operator==(const SDataIPL_PICK& b) const
        {
            const SDataIPL_PICK& a = *this;
            return EQ(id) && EQ(pos);
        }
        
        /* Sets data from string */
        bool set(const char* line)
        {
            return ScanConfigLine(line, count(), format(),
                                  &id, &pos[0].f, &pos[1].f, &pos[2].f);
        }
        
        /* Gets data to string */
        bool get(char* line) const
        {
            return PrintConfigLine(line, format(),
                                  id, pos[0].f, pos[1].f, pos[2].f);
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
        
        /* Formating information */
        const char* format() const { return "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %d"; }
        size_t count()       const { return 16; }
        
        bool operator==(const SDataIPL_JUMP& b) const
        {
            const SDataIPL_JUMP& a = *this;
            return(EQ(start1) && EQ(start2) && EQ(target1) && EQ(target2) && EQ(camerapos) && EQ(reward));
        }

        /* Sets data from string */
        bool set(const char* line)
        {
            return ScanConfigLine(line, count(), format(),
                                  &start1[0].f, &start1[1].f, &start1[2].f,
                                  &start2[0].f, &start2[1].f, &start2[2].f,
                                  &target1[0].f, &target1[1].f, &target1[2].f,
                                  &target2[0].f, &target2[1].f, &target2[2].f,
                                  &camerapos[0].f, &camerapos[1].f, &camerapos[2].f,
                                  &reward);
        }
        
        /* Gets data to string */
        bool get(char* line) const
        {
            return ScanConfigLine(line, count(), format(),
                                  start1[0].f, start1[1].f, start1[2].f,
                                  start2[0].f, start2[1].f, start2[2].f,
                                  target1[0].f, target1[1].f, target1[2].f,
                                  target2[0].f, target2[1].f, target2[2].f,
                                  camerapos[0].f, camerapos[1].f, camerapos[2].f,
                                  reward);
        }
    };
    
     /* Tcyc section */
    struct SDataIPL_TCYC
    {
        vec3 pos1;      // box edge 1
        vec3 pos2;      // box edge 2
        integer unk1;
        integer unk2;
        complex unk3;
        vec3 opt;       // those parameters are optional... in fact, this isn't a vector... default is (100.0, 1.0, 1.0)
        uint8_t nopt;   // num opt parameters received
        
        /* Formating information */
        const char* format() const { return 0; }   // format and count are 'dynamic'
        size_t count()       const { return 0; }   //
        
        bool operator==(const SDataIPL_TCYC& b) const
        {
            const SDataIPL_TCYC& a = *this;
            
            /* Check if num options received is the same */
            if(EQ(nopt))
            {
                /* Comparen non-optional parameters */
                if(EQ(pos1) && EQ(pos2) && EQ(unk1) && EQ(unk2) && EQ(unk3))
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
        
         /* Sets data from string */
        bool set(const char* line)
        {
            const int ndefault = 9;
            
            // Scan config line getting the number of parameters successfully caught
            int nargs = ScanConfigLine(line, "%f %f %f %f %f %f %d %d %f %f %f %f",
                                       &pos1[0].f, &pos1[1].f, &pos1[2].f,
                                       &pos2[0].f, &pos2[1].f, &pos2[2].f,
                                       &unk1, &unk2, &unk3.f,
                                       &opt[0].f, &opt[1].f, &opt[2].f);
            
            // Check if successfully caught at least the number of non-optional parameters
            if(nargs >= ndefault)
            {
                // Setup the number optional arguments field and return success...
                this->nopt = nargs - ndefault;
                return true;
            }
            return false;
        }
        
        /* Gets data from string */
        bool get(char* line) const
        {
            char buf[256];
            bool bResult = false;
            
            // Print common data into line
            if(!PrintConfigLine(line, "%f %f %f %f %f %f %d %d %f ",
                    pos1[0].f, pos1[1].f, pos1[2].f,
                    pos2[0].f, pos2[1].f, pos2[2].f,
                    unk1, unk2, unk3.f))
                return false;
            
            // Print optional data into temporary buffer
            switch(nopt)
            {
                case 0: bResult = true; buf[0] = '\0'; break;
                case 1: bResult = PrintConfigLine(buf, "%f", opt[0].f); break;
                case 2: bResult = PrintConfigLine(buf, "%f %f", opt[0].f, opt[1].f); break;
                case 3: bResult = PrintConfigLine(buf, "%f %f %f", opt[0].f, opt[1].f, opt[2].f); break;
            }
            
            // Append optional data into line
            if(bResult) strcat(line, buf);
            return bResult;
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
        
        /* Complete formating information */
        static const char* format(uint8_t formating, size_t& count)
        {
            switch(formating)
            {
                case 0:  count = 9;  return "%s %d %d %f %f %f %f %f %f";
                case 1:  count = 7;  return "%s %d %d %f %f %f %f";
                default: count = -1; return "";
            }
        }
        
        /* Formating information */
        const char* format() const { size_t a; return (format(formating, a));    }
        size_t count()       const { size_t a; return (format(formating, a), a); }
        
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
        
        /* Sets data from string */
        bool set(const char* line)
        {
            bool bResult = false;
            
            /* Try with formating types 0 and 1 */
            for(int i = 0; i < 2; ++i)
            {
                /* Try with this formating */
                switch(this->formating = i)
                {
                    case 0: /* box data */
                        bResult = ScanConfigLine(line, count(), format(),
                                        &name.buf, &id, &state,
                                        &box[0][0].f, &box[0][1].f,  &box[0][2].f,
                                        &box[1][0].f, &box[1][1].f,  &box[1][2].f);
                        break;
                    
                    case 1: /* sphere data */
                        bResult = ScanConfigLine(line, count(), format(),
                                        &name.buf, &id, &state,
                                        &sphere[0].f, &sphere[1].f, &sphere[2].f, &sphere[3].f);
                        break;
                }
                
                /* If found correct formating, go ahead and return,
                 * but before we need to calculate the name hash */
                if(bResult)
                {
                    name.recalc(::toupper);
                    return true;
                }
            }
            
            return false;
        }
        
        /* Gets data to string */
        bool get(char* line) const
        {
            switch(this->formating)
            {
                case 0: /* box data */
                    return PrintConfigLine(line, format(),
                                name.buf, id, state,
                                box[0][0].f, box[0][1].f,  box[0][2].f,
                                box[1][0].f, box[1][1].f,  box[1][2].f);
                    
                case 1: /* sphere data */
                    return PrintConfigLine(line, format(),
                                name.buf, id, state,
                                sphere[0].f, sphere[1].f, sphere[2].f, sphere[3].f);
            }
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
        
        /* Formating information */
        const char* format() const { return "%f %f %f %f %d %d %d %d %d %d %d %d"; }
        size_t count()       const { return 12; }
        
        bool operator==(const SDataIPL_CARS& b) const
        {
            const SDataIPL_CARS& a = *this;
            return (EQ(pos) && EQ(id) && EQ(color[0]) && EQ(color[1]) && EQ(force)
                 && EQ(alarm) && EQ(lock) && EQ(unk[0]) && EQ(unk[1]));
        }
        
        /* Sets data from string */
        bool set(const char* line)
        {
            return ScanConfigLine(line, count(), format(),
                        &pos[0].f, &pos[1].f, &pos[2].f, &pos[3].f,
                        &id, &color[0], &color[1], &force, &alarm, &lock,
                        &unk[0], &unk[1]);
        }
        
        /* Gets data from string */
        bool get(char* line) const
        {
            return PrintConfigLine(line, format(),
                        pos[0].f, pos[1].f, pos[2].f, pos[3].f,
                        id, color[0], color[1], force, alarm, lock,
                        unk[0], unk[1]);
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
        
        /* Formating information */
        const char* format() const { return 0; }    // dynamic
        size_t count()       const { return 0; }    //
        
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
        
        /* Sets data from string */
        bool set(const char* line)
        {
            const int ndefault = 7;
            
            // Scan config line getting the number of parameters successfully caught
            int nargs = ScanConfigLine(line, "%f %f %f %f %f %f %f %f %f %d",
                                       &mid[0].f, &mid[1].f, &bottom.f,
                                       &width[0].f, &width[1].f, &height.f, &rotation.f,
                                       &opt1[0].f, &opt1[1].f, &opt2);
            
            // Check if successfully caught at least the number of non-optional parameters
            if(nargs >= ndefault)
            {
                // Setup the number optional arguments field and return success...
                this->nopt = nargs - ndefault;
                return true;
            }
            return false;
        }
        
        /* Gets data from string */
        bool get(char* line) const
        {
            char buf[256];
            bool bResult = false;
            
            // Print common data into line
            if(!PrintConfigLine(line, "%f %f %f %f %f %f %f ",
                mid[0].f, mid[1].f, bottom.f,
                width[0].f, width[1].f, height.f, rotation.f))
                    return false;
            
            // Print optional data into temporary buffer
            switch(nopt)
            {
                case 0: bResult = true; buf[0] = '\0'; break;
                case 1: bResult = PrintConfigLine(buf, "%f", opt1[0].f); break;
                case 2: bResult = PrintConfigLine(buf, "%f %f", opt1[0].f, opt1[1].f); break;
                case 3: bResult = PrintConfigLine(buf, "%f %f %d", opt1[0].f, opt1[1].f, opt2); break;
            }
            
            // Append optional data into line
            if(bResult) strcat(line, buf);
            return bResult;
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
        
        /* Formating information */
        const char* format() const { return "%s %d %f %f %f %f %f %f %d %s"; }
        size_t count()       const { return 10; }
        
        bool operator==(const SDataIPL_ZONE& b) const
        {
            const SDataIPL_ZONE& a = *this;
            return (EQ(name) && EQ(type) && EQ(box[0]) && EQ(box[1]) && EQ(island) && EQ(label));
        }
        
        /* Sets data from string */
        bool set(const char* line)
        {
            if(ScanConfigLine(line, count(), format(),
                              name.buf, &type,
                              &box[0][0].f, &box[0][1].f, &box[0][2].f,
                              &box[1][0].f, &box[1][1].f, &box[1][2].f,
                              &island, label.buf))
            {
                // Calculate hash and return success
                name.recalc(::toupper);
                label.recalc(::toupper);
                return true;
            }
            return false;
        }
        
        /* Gets data from string */
        bool get(char* line) const
        {
            return PrintConfigLine(line, format(),
                        name.buf, type,
                        box[0][0].f, box[0][1].f, box[0][2].f,
                        box[1][0].f, box[1][1].f, box[1][2].f,
                        island, label.buf);
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
            bool IsValidSection() const
            {
                return (section >= IPL_INST && section <= IPL_ZONE);
            }
            
            
#if 1
            /*
             *  Sections table 
             */
            static SectionInfo* GetTable()
            {
                /* Section table for lookup */
                static SectionInfo sections[] =
                {
                    { IPL_NONE, ""     },
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
#endif
           
            
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
            template<class ResultType, template<class, class> class DoFunction, class A, class T>
            static ResultType DoIt(A& a, T& b, ResultType defaultResult)
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
            bool set(const SectionInfo* pSection, const char* line)
            {
                this->section = pSection->id;
                return DoIt<bool, call_set>(*this, line, false);
            }
            
            /* Gets the current data to the string @output */
            bool get(char* output) const
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
        static bool Parser(const char* filename, DataTraits::container_type& map, bool isDefault);      // implemented at ipl.cpp
        static bool Build(const char* filename, const std::vector<DataTraits::pair_ref_type>& lines);   //
        
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

