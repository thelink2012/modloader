/*
 * Copyright (C) 2013  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 *  Readme reader
 *
 */
#include "data.h"

static auto NullFile = traits.gta.NullFile;


void CThePlugin::ProcessReadmeFile(const char* filename)
{
    char buf[2048], *line;
    return;
    SDataGTA::key_type gtaKey;
    auto& gta = traits.gta.AddFile("data/gta.dat", nullptr, NullFile);
    return;
#if 0    
    if(FILE* f = fopen(filename, "r"))
    {
        while(line = fgets(buf, sizeof(buf), f))
        {
            if(gtaKey.get(line))
            {
                gta.map[gtaKey];
            }
        }
        fclose(f);
    }
#endif
}
