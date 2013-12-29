/*
 * Copyright (C) 2013  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 *
 *      This header file include all data traits
 * 
 *      To create a new trait and mixer, it's kinda of simple:
 *          1. Build a header file with the structures for the trait and the trait itself (see examples at "data/")
 *          2. Include the header where data.h can see it (this header is a good place to do so)
 *          3. Add a CDataFS specialization for your trait (see data.h)
 *          4. Add a entry at CheckFile and ProcessFile in data.cpp for your CDataFS (see data.cpp)
 *          5. Make a file mixer replacing a fopen call (see data.cpp)
 *          6. [optional] Add your trait plus a overriding filepath in the tuple of pairs at OnSplash (see data.cpp),
 *             this will make line detection from readme files.
 *          7. It's done, it looks complex, but it's not!
 * 
 */
#ifndef TRAITS_HPP
#define	TRAITS_HPP

#include "data/ide.h"
#include "data/ipl.h"

#include "data/gta.h"

#include "data/handling.h"
#include "data/carmods.h"

#endif

