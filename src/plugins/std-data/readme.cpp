/*
 * Copyright (C) 2013  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 *  Readme reader
 *
 */
#include "data.h"

void CThePlugin::ProcessReadmeFile(const char* filename)
{
    // TODO, in the final release, improve this to show the line number only, not the string at the line
    
    char buf[2048], *line;
    size_t nLine;
    
    Log("Reading readme file %s", filename);
    
    SDataGTA::key_type gtaKey;
    auto& gta = traits.gta.Additional("data/gta.dat");

    if(FILE* f = fopen(filename, "r"))
    {
        for(nLine = 0; line = ParseConfigLine(fgets(buf, sizeof(buf), f)); ++nLine)
        {
            if(*line == 0) continue;

            if(gtaKey.set(line))
            {
                Log("\tFound gta config line at line %d: %s", nLine, line);
                gta.map[gtaKey];
            }
        }
        fclose(f);
    }
}
