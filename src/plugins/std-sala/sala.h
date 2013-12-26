/*
 * Copyright (C) 2013  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 *  std-sala -- Standard Limit Adjuster Plugin for San Andreas Mod Loader
 *      Built upon Sacky's limit adjuster source code
 * 
 */
#include <list>
#include <modloader.hpp>
#include <modloader_util.hpp>
using namespace modloader;

/*
 *  The plugin object
 */
extern class CThePlugin* salaPlugin;
class CThePlugin : public modloader::CPlugin
{
    public:
        static const int default_priority = 45;
        
        std::list<std::string> salimits_ini;
        std::list<std::string> salimits_sala;
        
        const char* GetName();
        const char* GetAuthor();
        const char* GetVersion();
        bool OnStartup();
        bool OnShutdown();
        bool CheckFile(modloader::ModLoaderFile& file);
        bool ProcessFile(const modloader::ModLoaderFile& file);
        bool PosProcess();
        
        const char** GetExtensionTable();
        

};
