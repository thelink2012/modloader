/*
 * Copyright (C) 2013-2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#include "asi.h"
#include <modloader_util.hpp>
#include <modloader_util_path.hpp>
using namespace modloader;


/*
 *  Constructs a CsInfo
 *  This stores information about cleo scripts 
 */
CThePlugin::CsInfo::CsInfo(const modloader::ModLoaderFile& file)
{
    // Do basic initial setup
    this->path = GetFilePath(file);
    this->bIsMission = false;                   
    this->iVersion = CLEO_VER_NONE;
    
    // Get path used to search files when a script tries to open a file (fopen etc)
    if(IsFileInsideFolder(file.filepath, true, "CLEO"))  // If inside a CLEO folder, use the path before it
        this->folder = path.substr(0, GetLastPathComponent(path, 2));
    else                                        // Use this path
        this->folder = path.substr(0, GetLastPathComponent(path, 1));

    // Get script attributes
    if(IsFileExtension(file.filext, "cm"))
    {
        // Custom Mission
        this->bIsMission = true;
    }
    else
    {
        // Get .cs version
        if(char* p = strrchr(file.filename, '.'))
            this->GetVersionFromExtension(p+1, this->iVersion);
    }
}


/*
 *  Checks if extension is a cleo script and gets it's compatibility version
 *  Returns true if it is an cleo script and sets version for the proper version
 *  Returns false if it is not an cleo script and sets version to CLEO_VER_NONE
 */
bool CThePlugin::CsInfo::GetVersionFromExtension(const char* p, char& version)
{
    // Checks if extension starts with 'cs' or 'clo' (dose leaks)
    if(!strnicmp(p, "cs", 2) || !strnicmp(p, "clo", 3))
    {
        // Checks if there's no compatibility version on the extension
        if(p[2] == 0)
        {
            // Nope, no compatibility
            version = CLEO_VER_NONE;
            return true;
        }
        else if(p[2] > 0 && p[3] == 0)  // Check if there's a compatibility version
        {
            // Yep, get it
            version = tolower(p[2]);
            return true;
        }
    }
    
    // Not a cleo script
    version = CLEO_VER_NONE;
    return false;
}
