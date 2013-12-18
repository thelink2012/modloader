/* 
 * San Andreas modloader
 * Copyright (C) 2013  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */

#ifndef CMODLOADER_HPP
#define	CMODLOADER_HPP

#include <windows.h>
#include <list>
#include <map>
#include <string>
#include <modloader.hpp>
#include <modloader_util_path.hpp>

namespace modloader
{
    extern void Log(const char* msg, ...);
    extern void Error(const char* msg, ...);
    extern void LogException(LPEXCEPTION_POINTERS pException);
    
    /*
     * CModLoader
     *      Mod Loader Core Class
     */
    class CModLoader
    {
        private:
            struct ModInfo;
            struct FileInfo;
            
            struct ModInfo
            {
                std::list<FileInfo> files;

                std::string  path;          /* relative to main game dir (ex: "modloader/mymod") */
                std::string  fullPath;      /* full path */
                std::string  name;          /* modname */
                unsigned int id;            /* Note: This id is (probably) unique on each game call */
            };
            
            struct FileInfo
            {
                ModLoaderPlugin* handler;
                ModInfo*         parentMod;
                ModLoaderFile    data;
                
                std::string     fileName;       /* name */
                std::string     fileExtension;  /* ext */
                std::string     filePath;       /* full */
                
                unsigned int    id;         /* Note: This id is (probably) unique on each game call */
                bool            isDir;
            };

        private:
            modloader_t loader;
            
            bool bWorking;      /* true when the gameloader has been started up (Startup())
                                 * and false when the loader has shut down (Shutdown())
                                 */
            
            std::string gamePath;
            std::string modsPath;
            std::string cachePath;
            
            unsigned int currentModId;
            unsigned int currentFileId;

            std::list<ModInfo> mods;
            std::list<ModLoaderPlugin> plugins;
            std::map<std::string, std::vector<ModLoaderPlugin*>> extMap;
            
        private:
            void LoadPlugins();
            void UnloadPlugins();
            void PerformSearch();
            ModLoaderPlugin* FindFileHandler(const ModLoaderFile& file);
            
            bool UnloadPlugin(ModLoaderPlugin& plugin, bool bRemoveFromList = false);
            
            void PosProcess();
            
            /*
             *  Freeup memory taken by mods information
             */
            void ClearFilesData()
            {
                mods.clear();
            }
            
            /*
             *  Handle all files readen
             */
            void HandleFiles()
            {
                Log("\nHandling files...");
                CSetCurrentDirectory xdir("modloader\\");
                for(auto& mod : this->mods)
                {
                    CSetCurrentDirectory xdir((mod.name + "\\").c_str());
                    for(auto& file : mod.files)
                        this->HandleFile(file);
                }
            }
            
            /*
             *  Sorts plugins by priority and name order
             */
            void SortPlugins()
            {
                this->plugins.sort([](const ModLoaderPlugin& a, const ModLoaderPlugin& b)
                {
                    return (a.priority != b.priority?
                            a.priority < b.priority :
                            strcmp(a.name, b.name) < 0);
                });
            }
            
            /*
             *  Builds extensions std::map, used in the file checking algorithm
             */
            void BuildExtensionMap()
            {
                for(auto& plugin : this->plugins)
                {
                    for(size_t i = 0, n = plugin.extable_len;
                        i < n; ++i)
                        this->extMap[plugin.extable[i]].push_back(&plugin);
                }
            }
         
            /*
             *  Call plugin OnStartup method
             */
            void StartupPlugin(ModLoaderPlugin& data)
            {
                CSetCurrentDirectory xdir(this->loader.gamepath);
                
                Log("Starting up plugin \"%s\"", data.name);
                if(data.OnStartup && data.OnStartup(&data))
                {
                    Log("Failed to startup plugin '%s', unloading it.", data.name);
                    this->UnloadPlugin(data, true);
                }
            }
            
            /*
             *  Call plugins OnStartup method
             */
            void StartupPlugins()
            {
                Log("\nStarting up plugins...");
                for(auto& plugin : this->plugins)
                    StartupPlugin(plugin);
            }
            
            
            
        public:
            CModLoader() : bWorking(false), currentModId(0), currentFileId(0)
            { }
            
            void Patch();
            bool Startup();
            bool Shutdown();
            
            void ReadModf(const char* modfolder);
            void ReadFile(ModLoaderFile& file, CModLoader::ModInfo& mod);
            bool HandleFile(FileInfo& file);
            
            bool LoadPlugin(const char* pluginPath, bool bDoStuffNow);
            bool UnloadPlugin(const char* pluginName);
            
    };

}

/* Plugins are equal only (and only if) they point to the same data space */
inline bool operator==(const modloader::ModLoaderPlugin& a, const modloader::ModLoaderPlugin& b)
{
    return (&a == &b);
}


#endif	/* CMODLOADER_HPP */

