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
#include <set>
#include <modloader.hpp>
#include <modloader_util_path.hpp>
#include <modloader_util_container.hpp>

extern int LogException(char* buffer, LPEXCEPTION_POINTERS pException);

namespace modloader
{
    extern void Log(const char* msg, ...);
    extern void Error(const char* msg, ...);
    
    // Logs an LPEXCEPTION_POINTER into the logging stream
    inline void LogException(LPEXCEPTION_POINTERS pException)
    {
        char buffer[512];
        if(LogException(buffer, pException))
        {
            // Transfer log inside buffer into the logging stream
            Log(buffer);
        }
    }
    
    /*
     *  Functor for sorting based on priority and name 
     */
    template<class T>
    struct PriorityPred
    {
        bool operator()(const T& a, const T& b) const
        {
            return (a.priority != b.priority?
                    a.priority > b.priority :
                    compare(a.name, b.name, true) < 0);
        }
    };
    
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

                std::string  path;          // relative to main game dir (ex: "modloader/mymod")
                std::string  fullPath;      // full path
                std::string  name;          // modname
                unsigned int id;            // Note: This id is (probably) unique on each game call
                
                unsigned int priority;      // The mod priority
                
                ModInfo() : priority(50)
                {}
            };
            
            struct FileInfo
            {
                ModLoaderPlugin* handler;
                ModInfo*         parentMod;
                ModLoaderFile    data;
                
                std::string     fileName;       // name
                std::string     fileExtension;  // ext
                std::string     filePath;       // full
                
                std::vector<ModLoaderPlugin*> callMe;
                
                unsigned int    id;             // Note: This id is (probably) unique on each game call
                bool            isDir;
            };

            struct ModFolderInfo
            {
                /*
                 *  This class is not well written at all, it was build ONLY and ONLY for the modloader purposes...
                 *  So...
                 */
                
                typedef std::list<std::reference_wrapper<ModFolderInfo>>    ModFolderInfoList;
                
                ModFolderInfo* parent;  // My parent, the guy who found me!
                std::string path;       // The path, relative to the gamedir, to this modloader folder
               
                // Booleans
                struct flags_t
                {
                    bool bIgnoreAll;        // When true, no mod will be readen
                    bool bExcludeAll;       // When true, no mod gets loaded but the ones at include_mods list (set by INI)
                    bool bForceExclude;     // When true, have the same effect as exclude all (set by command line)
                    
                    flags_t()
                        : bIgnoreAll(false), bExcludeAll(false), bForceExclude(false)
                    {}
                
                } flags;
                
                // Lists
                std::list<ModFolderInfo> childs;    // My childs, own
                std::list<ModInfo> mods;            // My mods
                
                // Sorted lists (every path in those lists are normalized, as in NormalizePath() function)
                std::set<std::string> include_mods;     // All mod names inside this list shall be included when bExcludeAll is true
                std::set<std::string> exclude_mods;     // All mod names inside this list shall be ignored
                std::set<std::string> exclude_files;    // All file names inside this list shall be ignored
                std::set<std::string> exclude_files_ext;// All extensions in this list shall be ignored
                std::map<std::string, int> mods_priority;// list of priorities to be applied to mods <key> (normalized)
                
                //
                ModFolderInfo(const char* path, ModFolderInfo* parent = nullptr)
                    : path(path), parent(parent)
                {}
                
                ModFolderInfo(const std::string& path, ModFolderInfo* parent = nullptr)
                    : path(path), parent(parent)
                {}
                //
                
                // Clear all buffers from this structure, freeing up memory
                void clear()
                {
                    mods.clear();
                    childs.clear();
                }
               
                // Give a new child to self!
                void AddChild(const std::string& path)
                {
                    childs.emplace_back(path, this);
                }
                
                // Adds a priority to a mod
                void AddPriority(const char* path, int priority)
                {
                    if(priority == 0) return AddIgnoredMod(path);
                    mods_priority.emplace(NormalizePath(path), priority);
                }
                
                // 
                void AddIncludedMod(const char* name)
                {
                    include_mods.emplace(NormalizePath(name));
                }
                
                // Add ignored mod name
                void AddIgnoredMod(const char* name)
                {
                    exclude_mods.emplace(NormalizePath(name));
                }
                
                // Add ignored file name
                void AddIgnoredFile(const char* file)
                {
                    // Is to ignore a specific extension? Use the exclude extensions list
                    if(file[0] == '*' && file[1] == '.')
                    {
                        // Add it into the excluded extensions...
                        if(file[2] != '\0')
                            exclude_files_ext.emplace(NormalizePath(&file[2]));
                    }
                    else
                    {
                        // An actual file name... put on the excluded files list
                        exclude_files.emplace(NormalizePath(file));
                    }
                }
                
                // Checks if the file is excluded, as when calling AddIgnoredMod
                bool IsModIgnored(ModLoaderFile& file)
                {
                    if(flags.bExcludeAll || flags.bForceExclude)
                    {
                        if(include_mods.find(NormalizePath(file.filename)) != include_mods.end())
                            return false;
                        
                        // Don't check with parents!
                        return true;
                    }
                    else
                    {
                        // See if our config ignores...
                        if(exclude_mods.find(NormalizePath(file.filename)) != exclude_mods.end())
                            return true;
                        
                        // Don't check with parents!
                        return false;
                    }
                    

                }
                
                // Checks if the mod is excluded, as when calling AddIgnoredFile
                bool IsFileIgnored(ModLoaderFile& file)
                {
                    // Check if our config ignores...
                    if(exclude_files.find(NormalizePath(file.filename)) != exclude_files.end()
                    || exclude_files_ext.find(NormalizePath(file.filext)) != exclude_files_ext.end())
                    {
                        return true;
                    }
                    
                    // ...otherwise check our parent config...
                    return parent? parent->IsFileIgnored(file) : false;
                }
                

                // Returns me and my childs ModFolderInfo structure in a list
                ModFolderInfoList GetAllFolders()
                {
                    ModFolderInfoList list;
                    
                    // Put myself on the list
                    list.emplace_back(*this);
                    
                    // Put my child modloader folders,  my childs childs [...] into the list
                    for(auto& child : this->childs)
                        list.splice(list.end(), child.GetAllFolders());
                    
                    // Done
                    return list;
                }
                
                // Process mods list, ordering them by priority
                void ProcessPriorities()
                {
                    // First, give priority to mods that need it
                    for(auto& mod : this->mods)
                    {
                        auto it = this->mods_priority.find(NormalizePath(mod.name));
                        if(it != this->mods_priority.end())
                        {
                            mod.priority = it->second;
                        }
                    }
                    
                    // Now sort mods
                    this->mods.sort(PriorityPred<ModInfo>());
                }
                
            };
            
        private:
            modloader_t loader; /* C interface */
            
            bool bWorking;      /* true when the gameloader has been started up (Startup())
                                 * and false when the loader has shut down (Shutdown())
                                 */
            
            bool bEnableLog;            // Enables logging in modloader.log 
            bool bEnablePlugins;        // Enables loading modloader plugins (dll)
            
            std::string gamePath;       // full game path
            std::string cachePath;      // cache path
            std::string pluginsPath;    // plugins path
            
            // unique ids
            unsigned int currentModId;
            unsigned int currentFileId;
            
            /* mods and plugins information */
            ModFolderInfo modsfolder;                   // all mods are contained here
            std::map<std::string, int> plugins_priority;// list of priorities to be applied to plugins <key> (normalized)
            std::list<ModLoaderPlugin> plugins;         // all plugins
            std::map<std::string, std::vector<ModLoaderPlugin*>> extMap;
            
        private:
            /* Logging management */
            void OpenLog();
            void CloseLog();
            
            void SetupLoadbarChunks();
            
            /* Plugin management methods */
            void LoadPlugins();                     // Loads plugins from plugins folder
            void UnloadPlugins();                   // Unloads all plugins
            bool UnloadPlugin(ModLoaderPlugin& plugin, bool bRemoveFromList = false);
            bool LoadPlugin(const char* pluginPath, bool bDoStuffNow);
            bool UnloadPlugin(const char* pluginName);
            void StartupPlugins();                  // Call all plugins startup notification
            bool StartupPlugin(ModLoaderPlugin& data);

            
            /* Mods searching methods */
            void PerformSearch();                       // Searchs for mods starting from base modloader folder
            void PerformSearch(ModFolderInfo& folder);  // Search folder for mods
            ModLoaderPlugin* FindFileHandler(ModLoaderFile& file,  decltype(FileInfo::callMe)&); 
            void ReadModf(ModFolderInfo&, const std::string& folder);
            void ReadFile(ModFolderInfo&, ModLoaderFile& file, CModLoader::ModInfo& mod);
            
            /* Mods handling methods */
            void HandleFiles();                     // Handles all the files with their respective plugins
            bool HandleFile(FileInfo& file);        // Handle file with it's respective plugin
            void PosProcess();                      // Pos process all the plugins
            void DoLoadCall(bool isLoadBar);

            /* INI Config */
            void ReadBasicConfig(const char* filename);
            void LoadConfigFromINI(const char* filename, ModFolderInfo& modfolder);
            
            /* Command line parsing */
            void ParseCommandLine();
            
            /* Freeup memory taken by mods information */
            void ClearFilesData()
            {
                modsfolder.clear();
            }
            
            /* Sorts plugins by priority and name order */
            void SortPlugins()
            {
                this->plugins.sort(PriorityPred<ModLoaderPlugin>());
            }
            
            /* Builds extensions std::map, used in the file checking algorithm */
            void BuildExtensionMap()
            {
                // Foreach plugin...
                for(auto& plugin : this->plugins)
                {
                    // ... push the extensions handled by it into the extension map
                    for(size_t i = 0, n = plugin.extable_len;
                        i < n; ++i)
                        this->extMap[plugin.extable[i]].emplace_back(&plugin);
                }
            }

            
            
        public:
            CModLoader()
                : bWorking(false), bEnableLog(true), bEnablePlugins(true),
                    currentModId(0), currentFileId(0),
                    modsfolder("modloader\\")
            { }
            
            void Patch();       // Patches the game, so it will be "aware" about modloader
            bool Startup();     // Startups modloader
            bool Shutdown();    // Shutdowns modloader

            void OnLoadBar();   // Called when the loadbars gets started
            void OnReload();    // Called when the users loads a game (first time or not)
    };

}

/* Plugins are equal only (and only if) they point to the same data space */
inline bool operator==(const modloader::ModLoaderPlugin& a, const modloader::ModLoaderPlugin& b)
{
    return (&a == &b);
}


#endif	/* CMODLOADER_HPP */

