/*
 * Copyright (C) 2013  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 *  std-sala -- Standard Limit Adjuster Plugin for San Andreas Mod Loader
 *      Built upon Sacky's limit adjuster source code
 * 
 */
#include "sala.h"
#include "sacky.hpp"
#include "modloader_util_path.hpp"
#include <modloader_util.hpp>
#include <modloader_util_container.hpp>
using namespace modloader;

SALimitAdjuster* SALA = 0;

CThePlugin* salaPlugin;
static CThePlugin plugin;

/*
 *  Export plugin object data
 */
extern "C" __declspec(dllexport)
void GetPluginData(modloader_plugin_t* data)
{
    salaPlugin = &plugin;
    modloader::RegisterPluginData(plugin, data, plugin.default_priority);
}



/*
 *  Basic plugin informations
 */
const char* CThePlugin::GetName()
{
    return "std-sala";
}

const char* CThePlugin::GetAuthor()
{
    return "Sacky";
}

const char* CThePlugin::GetVersion()
{
    return "SALA-0.7";
}

const char** CThePlugin::GetExtensionTable()
{
    /* Put the extensions  this plugin handles on @table */
    static const char* table[] = { "ini", "sala", 0 };
    return table;
}

/* Startup */
bool CThePlugin::OnStartup()
{
    if(!SALA) SALA = new SALimitAdjuster();
    return true;
}

/* Shutdown */
bool CThePlugin::OnShutdown()
{
    if(SALA) delete SALA;
    return true;
}


/*
 *  Check if the file is the one we're looking for
 */
bool CThePlugin::CheckFile(modloader::ModLoaderFile& file)
{
    if(!file.is_dir)
    {
        if(!strcmp(file.filename, "salimits.ini", false))
            return true;
        else if(!strcmp(file.filename, "salimits.sala", false))
            return true;
    }
    return false;
}

/*
 * Process the replacement
 */
bool CThePlugin::ProcessFile(const modloader::ModLoaderFile& file)
{
    if(IsFileExtension(file.filext, "ini"))
        this->salimits_ini.emplace_back(GetFilePath(file));
    else if(IsFileExtension(file.filext, "sala"))
        this->salimits_sala.emplace_back(GetFilePath(file));
    return true;
}

/*
 * Called after all files have been processed
 */
bool CThePlugin::PosProcess()
{
    for(auto& f : this->salimits_ini)
        SALA->ReadIni(f.c_str());
    for(auto& f : this->salimits_sala)
        SALA->ReadSalaScript(f.c_str());
    return true;
}
