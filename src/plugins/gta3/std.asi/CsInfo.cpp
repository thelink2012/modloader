/*
 * Copyright (C) 2013-2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 *  std.asi -- Standard ASI Loader Plugin for Mod Loader
 *      Injects CLEO scripts into cleo search
 *
 */
#include <stdinc.hpp>
#include "asi.h"
using namespace modloader;


/*
 *  Constructs a CsInfo
 *  This stores information about cleo scripts 
 */
ThePlugin::CsInfo::CsInfo(const modloader::file* file) : file(file)
{
    // Do basic initial setup
    this->bIsMission = false;                   
    this->iVersion = CLEO_VER_NONE;
    
    std::string path = file->filepath();

    // Get path used to search files when a script tries to open a file (fopen etc)
    if(IsFileInsideFolder(file->filedir(), true, "CLEO"))  // If inside a CLEO folder, use the path before it
        this->folder = path.substr(0, GetLastPathComponent(path, 2));
    else                                        // Use this path
        this->folder = path.substr(0, GetLastPathComponent(path, 1));

    // Get script attributes
    if(file->is_ext("cm"))
    {
        // Custom Mission
        this->bIsMission = true;
    }
    else
    {
        if(IsFileInsideFolder(file->filedir(), false, "CLEO")
        && !IsFileInsideFolder(file->filedir(), true, "CLEO"))
        {
            // Ignore scripts that are inside a subfolder in CLEO folder
            this->bIsMission = true;
        }
        else
        {
            // Get .cs version
            if(auto* p = strrchr(file->filename(), '.'))
                this->GetVersionFromExtension(p+1, this->iVersion);
        }
    }
}


/*
 *  Checks if extension is a cleo script and gets it's compatibility version
 *  Returns true if it is an cleo script and sets version for the proper version
 *  Returns false if it is not an cleo script and sets version to CLEO_VER_NONE
 */
bool ThePlugin::CsInfo::GetVersionFromExtension(const char* ext, char& version)
{
    if(!_strnicmp(ext, "cs", 2))
    {
        // Checks if there's no compatibility version on the extension
        if(ext[2] == 0)
        {
            version = CLEO_VER_NONE;
            return true;
        }
        else if(ext[2] > 0 && ext[3] == 0)  // Check if there's a compatibility version
        {
            version = tolower(ext[2]);
            return true;
        }
    }
    
    // Not a cleo script
    version = CLEO_VER_NONE;
    return false;
}
