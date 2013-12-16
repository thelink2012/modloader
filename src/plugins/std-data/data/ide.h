/*
 * Copyright (C) 2013  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 *
 */
#ifndef DATAIDE_H
#define	DATAIDE_H

#include "traits.h"


// TODO

namespace data
{
    using namespace data::util;

    enum
    {
        IDE_NONE,
        IDE_OBJS,
        IDE_TOBJS,
        IDE_ANIM,
        IDE_PEDS,
        IDE_WEAP,
        IDE_CARS,
        IDE_HIER,
        IDE_TXDP,
        IDE_2DFX
    };
    
    struct SDataIDE_BASE
    { /* Has nothing, yet at least */ };

    /* Not implemented yet structures: */
    typedef SDataIDE_BASE   SDataIDE_OBJS;
    typedef SDataIDE_BASE   SDataIDE_TOBJS;
    typedef SDataIDE_BASE   SDataIDE_ANIM;
    typedef SDataIDE_BASE   SDataIDE_PEDS;
    typedef SDataIDE_BASE   SDataIDE_WEAP;
    //typedef SDataIDE_BASE   SDataIDE_CARS;
    typedef SDataIDE_BASE   SDataIDE_HIER;
    typedef SDataIDE_BASE   SDataIDE_2DFX;

    /* CARS structure */
    struct SDataIDE_CARS : SDataIDE_BASE
    {
        typedef string32 string;
        typedef SDataIDE_CARS this_type;

        enum eType
        {
            TRAILER, BMX, BIKE, TRAIN, BOAT, PLANE, HELI, QUAD, MTRUCK, CAR
        };

        integer     id;
        string      dff;
        string      txd;
        integer     type;
        string      handling;
        string      gamename;
        string      xclass;
        integer     frq;
        integer     lvl;
        flags       compr;
        integer     wheel;      /* wheel (for cars); lod (for planes) */
        complex     wheelScale; /* (for cars); */


        static bool comp(const this_type& a, const this_type& b)
        {
            return (
                EQ(id) && EQ(dff) && EQ(txd) && EQ(type) && EQ(handling) && EQ(gamename) && EQ(xclass) && EQ(frq)
             && EQ(lvl) && EQ(compr) && EQ(wheel) && EQ(wheelScale)
             );
        }

        bool operator==(const this_type& b)
        {
            return comp(*this, b);
        }
    };

    

    struct SDataIDE
    {
        typedef integer key_type;
        
        uint8_t section;
        
        struct {
            string32 child;
            string32 parent;            
        } txdp;

        union
        {
            SDataIDE_OBJS   objs;
            SDataIDE_TOBJS  tobjs;
            SDataIDE_ANIM   anim;
            SDataIDE_PEDS   peds;
            SDataIDE_WEAP   weap;
            SDataIDE_CARS   cars;
            SDataIDE_HIER   hier;
            SDataIDE_2DFX   a2dfx;
        };
        
        SDataIDE() :
            section(IDE_NONE)
        {}
        
    };
    
    typedef DataTraitsBase<std::map<SDataIDE::key_type, SDataIDE>>    TraitsIDE;

}

#endif	/* DATAIDE_H */


