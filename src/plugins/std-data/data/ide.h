/*
 * Copyright (C) 2013  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 *  IDE structures
 *
 */
#ifndef TRAITS_IDE_H
#define TRAITS_IDE_H

#include "traits.h"
#include "ranges.h"
#include <map>

namespace data
{
    using namespace data::util;

    enum
    {
        IDE_NONE,
        IDE_OBJS,
        IDE_TOBJ,
        IDE_ANIM,
        IDE_PEDS,
        IDE_WEAP,
        IDE_CARS,
        IDE_HIER,
        IDE_TXDP,
        IDE_2DFX        // Not used and probably not supported in SA
    };
    
    /*
     *  The base for each IDE structure
     *  This base provides only a id, that's it
     */
    struct SDataIDE_BASE
    {
        obj_id  id;
    };

    
    
    /* OBJS and TOBJ section structure */
    template<bool isTOBJ>
    struct SDataIDE_OBJS_BASE : public SDataIDE_BASE
    {
        size_t  hash;   // hash of everything below, except drawdist (and time[] if isTOBJ == false)
        
        string24 model;
        string24 texture;
        integer obj_count;      // 0, 1, 2 or 3; 0 actually means 1 but that information wasn't passed to us
        complex drawdist[3];
        flags flag;
        integer time[2];
        
        /* Formating information */
        
        static const char* format(integer obj_count, size_t& count)
        {
            const char* fmt;
            if(isTOBJ == false) /* TOBJ contains two additional %d for time on/off */
            {
                /* Find format from obj count, each object adds a %f of draw distance */
                switch(obj_count)
                {
                    case 0: count = 5; fmt = "%d %s %s %f %d"; break;
                    case 1: count = 6; fmt = "%d %s %s %d %f %d"; break;
                    case 2: count = 7; fmt = "%d %s %s %d %f %f %d"; break;
                    case 3: count = 8; fmt = "%d %s %s %d %f %f %f %d"; break;
                    default: count = 1; fmt = ""; break;
                }
            }
            else
            {
                /* Find format from obj count, each object adds a %f of draw distance */
                switch(obj_count)
                {
                    case 0: count =  7; fmt = "%d %s %s %f %d %d %d"; break;
                    case 1: count =  8; fmt = "%d %s %s %d %f %d %d %d"; break;
                    case 2: count =  9; fmt = "%d %s %s %d %f %f %d %d %d"; break;
                    case 3: count = 10; fmt = "%d %s %s %d %f %f %f %d %d %d"; break;
                    default: count = 1; fmt = ""; break;
                }
            }
            return fmt;
        }
        
        const char* format() const { size_t a; return format(obj_count, a); }
        size_t count()       const { size_t a; return format(obj_count, a), a; }
        //static size_t min_count()  { return ; }
        //static size_t max_count()  { return ; }
        
         
        // Compare two OBJS sections
        bool operator==(const SDataIDE_OBJS_BASE& b) const
        {
            const SDataIDE_OBJS_BASE& a = *this;
            if(EQ(hash))
            {
                /* Rest of the structure is equal, compare the draw distance */
                switch(obj_count)
                {
                    case 3: return EQ(drawdist[0]) && EQ(drawdist[1]) && EQ(drawdist[2]);
                    case 2: return EQ(drawdist[0]) && EQ(drawdist[1]);
                    case 0: case 1: return EQ(drawdist[0]);
                }
            }
            return false;
        }
        
        /* Sets data from string */
        bool set(const char* line)
        {
            const char* fmt;
            size_t c;
            
            /* Check out if basic formating, without object count information */
            fmt = format(0, c);
            if(ScanConfigLine(line, c, c, fmt,
                              &id, model.buf, texture.buf, &drawdist[0].f, &flag, &time[0], &time[1]) > 0)
            {
                this->obj_count = 0;
            }
            /* Nope, extract object count... */
            else if(sscanf(line, "%d %s %s %d", &id, &model.buf, &texture.buf, &obj_count) == 4)
            {
                int result = -1;
                
                /* Do the actual scan now... */
                fmt = format(obj_count, c);
                switch(obj_count)
                {
                    case 1:
                        result = ScanConfigLine(line, c, c, fmt,
                                 &id, model.buf, texture.buf, &obj_count,
                                 &drawdist[0].f, &flag, &time[0], &time[1]);
                        break;
                        
                    case 2:
                        result = ScanConfigLine(line, c, c, fmt,
                                 &id, model.buf, texture.buf, &obj_count,
                                 &drawdist[0].f, &drawdist[1].f, &flag, &time[0], &time[1]);
                        break;
                        
                    case 3:
                        result = ScanConfigLine(line, c, c, fmt,
                                 &id, model.buf, texture.buf, &obj_count,
                                 &drawdist[0].f, &drawdist[1].f, &drawdist[2].f, &flag, &time[0], &time[1]);
                        break;
                        
                }
                
                /* Check if result is okay */
                if(result <= 0) return false;
            }
            else
            {
                /* Nothin' */
                return false;
            }

            /* Calculate hashes */
            model.recalc(::toupper);
            texture.recalc(::toupper);
            
            /* Hash structure, except drawdist */
            {
                modloader::hash_transformer<> h;

                // Hash common information
                h.transform(id).transform(model.hash).transform(texture.hash)
                 .transform(obj_count).transform(flag);
                // Hash TOBJ information
                if(isTOBJ) h.transform(time);

                // Get final hash
                this->hash = h.final();
            }
            
            return true;
        }
      
        /* Gets data from string */
        bool get(char* line) const
        {
            int result = -1;
            switch(obj_count)
            {
                case 0:
                    result = PrintConfigLine(line, count(), format(),
                                 id, model.buf, texture.buf,
                                 drawdist[0].f, flag, time[0], time[1]);
                    break;
                    
                case 1:
                    result = PrintConfigLine(line, count(), format(),
                                 id, model.buf, texture.buf, obj_count,
                                 drawdist[0].f, flag, time[0], time[1]);
                    break;
                    
                case 2:
                    result = PrintConfigLine(line, count(), format(),
                                 id, model.buf, texture.buf, obj_count,
                                 drawdist[0].f, drawdist[1].f, flag, time[0], time[1]);
                    break;
                    
                case 3:
                    result = PrintConfigLine(line, count(), format(),
                                 id, model.buf, texture.buf, obj_count,
                                 drawdist[0].f, drawdist[1].f, drawdist[2].f, flag, time[0], time[1]);
                    break;
            }
            return result > 0;
        }
    };
    
    
    /*
     *  typedef for OBJS and TOBJ structure 
     *  They're just SDataIDE_OBJS_BASE with isTOBJ as true or false
     */
    typedef SDataIDE_OBJS_BASE<false> SDataIDE_OBJS;
    typedef SDataIDE_OBJS_BASE<true>  SDataIDE_TOBJ;
    
    
    
    /* ANIM section structure */
    struct SDataIDE_ANIM : public SDataIDE_BASE
    {
        string24 model;
        string24 texture;
        string16 anim;
        complex drawdist;
        flags flag;
        
        /* Formating information */
        const char* format() const { return "%d %s %s %s %f %d"; }
        size_t count()       const { return min_count(); }
        static size_t min_count()  { return 6; }
        static size_t max_count()  { return min_count(); }
        
        // Compare two ANIM sections
        bool operator==(const SDataIDE_ANIM& b) const
        {
            const SDataIDE_ANIM& a = *this;
            return EQ(id) && EQ(model) && EQ(texture) && EQ(anim) && EQ(drawdist) && EQ(flag);
        }
        
        /* Sets data from string */
        bool set(const char* line)
        {
            if(ScanConfigLine(line, min_count(), max_count(), format(),
                              &id, model.buf, texture.buf, anim.buf, &drawdist.f, &flag)
                              > 0)
            {
                // Calculate string hash
                model.recalc(::toupper);
                texture.recalc(::toupper);
                anim.recalc(::toupper);
                return true;
            }
            return false;
        }
      
        /* Gets data from string */
        bool get(char* line) const
        {
            return PrintConfigLine(line, count(), format(),
                              id, model.buf, texture.buf, anim.buf, drawdist.f, flag) > 0;
        }
    };
    
    
    /* PEDS section structure */
    struct SDataIDE_PEDS : public SDataIDE_BASE
    {
        // Wow, this structure is huge, could we optimize it's size? Currently it has ~300 bytes, damn!
        
        size_t hash;    // Hash of the structure below
        struct
        {
            string24 model;
            string24 texture;
            string24 pedtype;
            string24 behaviour;
            string24 animgrp;
            flags    drive_mask;
            flags    flag;
            string16 anim;
            integer  radio[2];
            string16 voice_type;
            string64 voice[2];      // WOWWWWWW, HUGEEEEEEEE
        };
        
        /* Formating information */
        const char* format() const { return "%d %s %s %s %s %s %x %x %s %d %d %s %s %s"; }
        size_t count()       const { return min_count(); }
        static size_t min_count()  { return 14; }
        static size_t max_count()  { return min_count(); }
        
        // Compare two PEDS sections
        bool operator==(const SDataIDE_PEDS& b) const
        {
            const SDataIDE_PEDS& a = *this;
            return EQ(hash);
            /*
            return EQ(id) && EQ(model) && EQ(texture) && EQ(pedtype) && EQ(behaviour) && EQ(animgrp)
                && EQ(drive_mask) && EQ(flag) && EQ(anim) && EQ(radio[0]) && EQ(radio[1])
                && EQ(voice_type) && EQ(voice[0]) && EQ(voice[1]);
            */
        }
        
        /* Sets data from string */
        bool set(const char* line)
        {
            if(ScanConfigLine(line, min_count(), max_count(), format(),
                              &id, model.buf, texture.buf, pedtype.buf, behaviour.buf, animgrp.buf,
                              &drive_mask, &flag, anim.buf, &radio[0], &radio[1],
                              voice_type.buf, voice[0].buf, voice[1].buf) > 0)
            {
                // Calculate string hash
                model.recalc(::toupper);
                texture.recalc(::toupper);
                pedtype.recalc(::toupper);
                behaviour.recalc(::toupper);
                animgrp.recalc(::toupper);
                anim.recalc(::toupper);
                voice_type.recalc(::toupper);
                voice[0].recalc(::toupper);
                voice[1].recalc(::toupper);
                
                // Find out the entire structure hash for faster comparision
                this->hash = modloader::hash_transformer<>()
                    .transform(id).transform(model.hash).transform(texture.hash).transform(pedtype.hash)
                    .transform(behaviour.hash).transform(animgrp.hash).transform(drive_mask).transform(flag)
                    .transform(anim.hash).transform(radio).transform(voice_type.hash)
                    .transform(voice[0].hash).transform(voice[1].hash)
                    .final();
                
                return true;
            }
            return false;
        }
      
        /* Gets data from string */
        bool get(char* line) const
        {
            return PrintConfigLine(line, count(), format(),
                              id, model.buf, texture.buf, pedtype.buf, behaviour.buf, animgrp.buf,
                              drive_mask, flag, anim.buf, radio[0], radio[1],
                              voice_type.buf, voice[0].buf, voice[1].buf) > 0;
        }
    };

    
    /* WEAP section structure */
    struct SDataIDE_WEAP : public SDataIDE_BASE
    {
        string24 model;
        string24 texture;
        string16 anim;
        integer  clumps;    // ignored by the game... but let's use it
        complex  drawdist;
        
        uint8_t nopt;
        
        union  // Optional arguments
        {
            flags    flag;      // not even readen by the game, but let's use it
        };
        
        /* Formating information */
        const char* format() const { return "%d %s %s %s %d %f    %d"; }
        size_t count()       const { return min_count() + nopt; }
        static size_t min_count()  { return 6; }
        static size_t max_count()  { return 7; }
        
        // Compare two WEAP sections
        bool operator==(const SDataIDE_WEAP& b) const
        {
            const SDataIDE_WEAP& a = *this;
            if(EQ(nopt))
            {
                if(EQ(id) && EQ(model) && EQ(texture) && EQ(anim) && EQ(clumps) && EQ(drawdist))
                    return nopt == 1? EQ(flag) : true;
            }
            return false;
        }
        
        /* Sets data from string */
        bool set(const char* line)
        {
            int nargs = ScanConfigLine(line, min_count(), max_count(), format(),
                              &id, model.buf, texture.buf, anim.buf,
                              &clumps, &drawdist.f, &flag);
            
            if(nargs > 0)
            {
                // Calculate num optional opts
                this->nopt = nargs - min_count();
                
                // Calculate string hash
                model.recalc(::toupper);
                texture.recalc(::toupper);
                anim.recalc(::toupper);
                return true;
            }
            return false;
        }
      
        /* Gets data from string */
        bool get(char* line) const
        {
            return PrintConfigLine(line, count(), format(),
                              id, model.buf, texture.buf, anim.buf,
                              clumps, drawdist.f, flag) > 0;
        }
    };

    /* CARS section structure */
    struct SDataIDE_CARS : public SDataIDE_BASE
    {
        size_t hash;    // Hash for structure below
        struct
        {
            string24    model;
            string24    texture;
            string8     type;
            string16    handling;
            string16    gamename;
            string16    anim;
            string16    xclass;
            integer     frq;
            integer     lvl;
            flags       compr;
        };
        
        uint8_t nopt;   // num opt parameters received
        
        struct      // optional arguments
        {
            integer     wheel;
            complex     wheelScale1;
            complex     wheelScale2;
            integer     unk;
        };
        
        
        
        /* Formating information */
        const char* format() const { return "%d %s %s %s %s %s %s %s %d %d %x   %d %f %f %d"; }
        size_t count()       const { return min_count() + nopt; }
        static size_t min_count()  { return 11; }
        static size_t max_count()  { return 15; }
        
        // Compare two CARS sections
        bool operator==(const SDataIDE_CARS& b) const
        {
            const SDataIDE_CARS& a = *this;
            
            /*if(EQ(nopt) && EQ(id) && EQ(model) && EQ(texture) && EQ(type) && EQ(handling) && EQ(gamename)
            && EQ(anim) && EQ(xclass) && EQ(frq) && EQ(lvl) && EQ(compr))*/
            
            // If non-optional arguments and num optional arguments are equal...
            if(EQ(nopt) && EQ(hash))
            {
                // Check non-optional arguments
                switch(nopt)
                {
                    case 0: return true;
                    case 1: return EQ(wheel);
                    case 2: return EQ(wheel) && EQ(wheelScale1);
                    case 3: return EQ(wheel) && EQ(wheelScale1) && EQ(wheelScale2);
                    case 4: return EQ(wheel) && EQ(wheelScale1) && EQ(wheelScale2) && EQ(unk);
                }
            }
            
            // Nope
            return false;
        }
        

        /* Sets data from string */
        bool set(const char* line)
        {
            int nargs = ScanConfigLine(line, min_count(), max_count(), format(),
                              &id,
                              model.buf, texture.buf, type.buf, handling.buf, gamename.buf, anim.buf, xclass.buf,
                              &frq, &lvl, &compr, &wheel, &wheelScale1.f, &wheelScale2.f, &unk);

            
            // Has at least the non-optional arguments?
            if(nargs > 0)
            {
                // Calculate num optional arguments received
                this->nopt = nargs - min_count();
                
                // Calculate hash for each string
                model.recalc(::toupper);
                texture.recalc(::toupper);
                type.recalc(::toupper);
                handling.recalc(::toupper);
                gamename.recalc(::toupper);
                anim.recalc(::toupper);
                xclass.recalc(::toupper);
                
                // Structure hash
                this->hash = modloader::hash_transformer<>()
                    .transform(id).transform(model.hash).transform(texture.hash).transform(type.hash)
                    .transform(handling.hash).transform(gamename.hash).transform(anim.hash).transform(xclass.hash)
                    .transform(frq).transform(lvl).transform(compr)
                    .final();
                
                return true;
            }
            return false;
        }
      
        /* Gets data from string */
        bool get(char* line) const
        {
            return (PrintConfigLine(line, count(), format(),
                    id,
                    model.buf, texture.buf, type.buf, handling.buf, gamename.buf, anim.buf, xclass.buf,
                    frq, lvl, compr, wheel, wheelScale1.f, wheelScale2.f, unk) > 0);
        }
        
    };

    /* TXDP section structure */
    struct SDataIDE_TXDP // : public SDataIDE_BASE - don't use this base, we aren't id based
    {
        string32 child;     // consider this as an id...
        string32 parent;
        
        /* Formating information */
        const char* format() const { return "%s %s"; }
        size_t count()       const { return min_count(); }
        static size_t min_count()  { return 2; }
        static size_t max_count()  { return min_count(); }
        
        // Compare two TXDP sections
        bool operator==(const SDataIDE_TXDP& b) const
        {
            const SDataIDE_TXDP& a = *this;
            return EQ(child) && EQ(parent);
        }
        
        /* Sets data from string */
        bool set(const char* line)
        {
            if(ScanConfigLine(line, min_count(), max_count(), format(), child.buf, parent.buf) > 0)
            {
                // Calculate string hash
                child.recalc(::toupper);
                parent.recalc(::toupper);
                return true;
            }
            return false;
        }
      
        /* Gets data from string */
        bool get(char* line) const
        {
            return PrintConfigLine(line, count(), format(), child.buf, parent.buf) > 0;
        }
    };
    
    /* HIER section structure */
    struct SDataIDE_HIER : public SDataIDE_BASE
    {
        string24 model;
        string24 texture;

        uint8_t nopt;   // num optional args received
        
        struct  // optional arguments
        {
            string16 anim;      // ignored by the game
            complex  drawdist;  // ignored by the game
        };
        
        
        /* Formating information */
        const char* format() const { return "%d %s %s   %s %f"; }
        size_t count()       const { return min_count() + nopt; }
        static size_t min_count()  { return 3; }
        static size_t max_count()  { return 5; }
        
        // Compare two HIER sections
        bool operator==(const SDataIDE_HIER& b) const
        {
            const SDataIDE_HIER& a = *this;
            if(EQ(nopt))
            {
                if(EQ(id) && EQ(model) && EQ(texture))
                {
                    switch(nopt)
                    {
                        case 0: return true;
                        case 1: return EQ(anim);
                        case 2: return EQ(anim) && EQ(drawdist);
                    }
                }
            }
            return false;
        }
        
        /* Sets data from string */
        bool set(const char* line)
        {
            anim.buf[0] = 0;
            
            int nargs = ScanConfigLine(line, min_count(), max_count(), format(),
                                       &id, model.buf, texture.buf, anim.buf, &drawdist.f);
            if(nargs > 0)
            {
                this->nopt = nargs - min_count();
                
                // Calculate string hash
                model.recalc(::toupper);
                texture.recalc(::toupper);
                anim.recalc(::toupper);
                return true;
            }
            return false;
        }
      
        /* Gets data from string */
        bool get(char* line) const
        {
            return PrintConfigLine(line, count(), format(),
                                   id, model.buf, texture.buf, anim.buf, drawdist.f) > 0;
        }
    };
 
    
    
    
    

    struct SDataIDE
    {
        /* Key for an IDE item */
        struct key_type
        {
            bool isTxdp;            // txdp section is not based in indexes
            union
            {
                obj_id id;          // Used if isTxdp = false
                hash child_txd;     // Used if isTxdp = true
            };
            
            // Set key based on value
            bool set(const SDataIDE& b)
            {
                if(isTxdp = (b.section == IDE_TXDP))
                    child_txd.hash = b.txdp.child.hash;
                else
                    id = b.base.id;
                return true;
            }
            
            // For sorting...
            bool operator<(const key_type& b) const
            {
                const key_type& a = *this;
                return isTxdp? LE(child_txd) : LE(id);
            }
        };
        
        uint8_t section; /* ide section this data is from */
        
        /* actual data, depending on the section... */
        union
        {
            SDataIDE_BASE   base;       // base of each section (except txdp), stores ID
            SDataIDE_OBJS   objs;
            SDataIDE_TOBJ   tobj;
            SDataIDE_ANIM   anim;
            SDataIDE_PEDS   peds;
            SDataIDE_WEAP   weap;
            SDataIDE_CARS   cars;
            SDataIDE_HIER   hier;
            SDataIDE_TXDP   txdp;
        };

        /*
        *  Sections table 
        */
        static SectionInfo* GetTable()
        {
            /* Section table for lookup */
            static SectionInfo sections[] =
            {
                { IDE_NONE, ""      },
                { IDE_OBJS, "objs"  },
                { IDE_TOBJ, "tobj"  },
                { IDE_ANIM, "anim"  },
                { IDE_PEDS, "peds"  },
                { IDE_WEAP, "weap"  },
                { IDE_CARS, "cars"  },
                { IDE_HIER, "hier"  },
                { IDE_TXDP, "txdp"  },
                { IDE_2DFX, "2dfx"  },
                { IDE_NONE, nullptr }
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
        
        /*
         *  Just two dummy objects to represent if the set() method was called
         *  to detect a vehmods.ide line or vehicles.ide line from a readme file 
         */
        static SectionInfo* ReadmeVehmods()
        { static SectionInfo dummy; return &dummy; }
        static SectionInfo* ReadmeVehicles()
        { static SectionInfo dummy; return &dummy; }
        
        
#if 1
            /* Executes DoFunction operator on current section field */
            template<class ResultType, template<class> class DoFunction>
            static ResultType DoIt(const SDataIDE& a, const SDataIDE& b, ResultType defaultResult)
            {
                switch(a.section)                   
                {                                   
                    case IDE_OBJS:
                        typedef decltype(a.objs) objs;
                        return DoFunction<objs>()(a.objs, b.objs);
                        
                    case IDE_TOBJ:
                        typedef decltype(a.tobj) tobj;
                        return DoFunction<tobj>()(a.tobj, b.tobj);
                        
                    case IDE_ANIM:
                        typedef decltype(a.anim) anim;
                        return DoFunction<anim>()(a.anim, b.anim);
                        
                    case IDE_PEDS:
                        typedef decltype(a.peds) peds;
                        return DoFunction<peds>()(a.peds, b.peds); 
                        
                    case IDE_WEAP:
                        typedef decltype(a.weap) weap;
                        return DoFunction<weap>()(a.weap, b.weap);
                        
                    case IDE_CARS:
                        typedef decltype(a.cars) cars;
                        return DoFunction<cars>()(a.cars, b.cars);
                        
                    case IDE_HIER:
                        typedef decltype(a.hier) hier;
                        return DoFunction<hier>()(a.hier, b.hier);
                        
                    case IDE_TXDP:
                        typedef decltype(a.txdp) txdp;
                        return DoFunction<txdp>()(a.txdp, b.txdp);
                }
                return defaultResult;
            }
            
            /* Executes DoFunction in a section field sending b as argument */
            template<class ResultType, template<class, class> class DoFunction, class A, class T>
            static ResultType DoIt(A& a, T& b, ResultType defaultResult)
            {
                switch(a.section)                   
                {                                   
                    case IDE_OBJS:
                        typedef decltype(a.objs) objs;
                        return DoFunction<objs,T>()(a.objs, b);
                        
                    case IDE_TOBJ:
                        typedef decltype(a.tobj) tobj;
                        return DoFunction<tobj,T>()(a.tobj, b);
                        
                    case IDE_ANIM:
                        typedef decltype(a.anim) anim;
                        return DoFunction<anim,T>()(a.anim, b);
                        
                    case IDE_PEDS:
                        typedef decltype(a.peds) peds;
                        return DoFunction<peds,T>()(a.peds, b); 
                        
                    case IDE_WEAP:
                        typedef decltype(a.weap) weap;
                        return DoFunction<weap,T>()(a.weap, b);
                        
                    case IDE_CARS:
                        typedef decltype(a.cars) cars;
                        return DoFunction<cars,T>()(a.cars, b);
                        
                    case IDE_HIER:
                        typedef decltype(a.hier) hier;
                        return DoFunction<hier,T>()(a.hier, b);
                        
                    case IDE_TXDP:
                        typedef decltype(a.txdp) txdp;
                        return DoFunction<txdp,T>()(a.txdp, b);
                }
                return defaultResult;
            }
#endif
        
        /*
         *  Compare two IDE data lines
         */
        bool operator==(const SDataIDE& b) const
        {
            const SDataIDE& a = *this;
                
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
            return section >= IDE_OBJS && section <= IDE_2DFX;
        }
        
        /* Sets the current data to the data at @line, knowing that the section to handle is @info */
        bool set(SectionInfo* info, const char* line)
        {
            bool bVehiclesReadme = (info == ReadmeVehicles());
            bool bVehmodsReadme  = (info == ReadmeVehmods());
            
            // We only support IDE CARS (vehicles.ide) and IDE OBJS (veh_mods) section detection from a readme file...
            if(bVehmodsReadme || bVehiclesReadme)
            {
                using namespace modloader::parser;

                this->section = IDE_NONE;
                
                // Line must start with a numeric identifier to be a IDE line
                if(isdigit(*line))
                {
                    const char* fmt = bVehiclesReadme? "%d" : "%d %23s";
                    int fmt_count   = bVehiclesReadme?   1  : 2;
                    
                    int id; char buf[24];
                    if(sscanf(line, fmt, &id, buf) == fmt_count)
                    {
                        if(bVehiclesReadme)
                        {
                            // Check if vehicle id is in the right bounds
                            // because it may be a line for GTA III/VC
                            if(is_vehicle(id))
                                this->section = IDE_CARS;
                        }
                        else if(bVehmodsReadme)
                        {
                            // On vehmods, check if the second token is a upgrade model
                            if(IsUpgradeModel(buf))
                                this->section = IDE_OBJS;
                        }
                    }
                }
                
                
                // Did found something to handle?
                if(this->section == IDE_NONE)
                    return false;   // he, nope
            }
            else
            {
                this->section = info->id;
            }
            return DoIt<bool, call_set>(*this, line, false);
        }
        
        /* Gets the current data to the string @output */
        bool get(char* output) const
        {
            return this->IsValidSection()
                && DoIt<bool, call_get>(*this, output, false);
        }
        

        

        
        SDataIDE() :
            section(IDE_NONE)
        {}
        
    };
    
    // The traits object
    struct TraitsIDE : public DataTraitsImplSimple<
            /*ContainerType*/   std::map<SDataIDE::key_type, SDataIDE>,
            /* HasSection */    true, 
            /* HasKeyValue */   true,
            /* IsSorted */      true,
            /* DomFlags */      DomFlags<flag_RemoveIfNotExistInOneCustomButInDefault>
                        >
    {
        static const char* what() { return "object types file"; }
        
        // Specialize handler_type
        struct handler_type : DataTraitsImplSimple::handler_type
        {
            typedef DataTraitsImplSimple::handler_type super; 
            
            // Specialize because we need to know if we're dealing with a veh_mods or vehicles detection.
            struct Set
            {
                bool operator()(SectionType section, const char* line, container_type& map, const char* fileov = 0)
                {
                    // If trying to detect from readme, we need to know if want to detect a veh_mods.ide or a vehicles.ide file
                    if(section == modloader::ReadmeSection() && fileov)
                    {
                        // The paths we use are "data/maps/veh_mods.ide" and "data/vehicles.ide"
                        // Naturally the first different char is the char at index 5, so let's look into it

                        char c = fileov[5];
                        if(c == 'v')  section = SDataIDE::ReadmeVehicles();
                        else if(c == 'm') section = SDataIDE::ReadmeVehmods();
                    }
                    
                    // Continue to normal course
                    return super::Set()(section, line, map);
                }
            };
        };
        
    };
}

#endif	/* TRAITS_IDE_H */


