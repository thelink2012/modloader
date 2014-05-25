/*
 * Copyright (C) 2013-2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#ifndef LOADER_HPP
#define	LOADER_HPP

#include <modloader.hpp>
#include <modloader_util_path.hpp>
#include <cstdint>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>

// TODO FIX THE PRIORITY SYSTEM
// TODO UNORDMAP???
// TODO UINT64 FOR IDS

using namespace modloader;

static const char* modurl = "https://github.com/thelink2012/sa-modloader";

// Functor for sorting based on priority and name 
template<class T>
struct PriorityPred
{
    bool operator()(const T& a, const T& b) const
    {
        return (a.priority != b.priority?
                a.priority > b.priority :               // That's right, higher priority means lower
                compare(a.name, b.name, true) < 0);
    }
};

template<class T>
struct ByName
{
    struct EqualTo
    {
        bool operator()(const T& a, const T& b) const
        {
            return compare(a.name, b.name, false) == 0;
        }
    };
    
    struct LessThan
    {
        bool operator()(const T& a, const T& b) const
        {
            return compare(a.name, b.name, false) < 0;
        }
    };
    
    struct Hash
    {
        std::hash<decltype(T::name)> hash;
        
        bool operator()(const T& a) const
        {
            return hash(a.name);
        }
    };
};

extern class Loader loader;

// The Mod Loader Core
class Loader : public modloader_t
{
    public:
        static const int default_priority = 50;         // Default priority for mods
        static const int default_cmd_priority = 80;     // Default priority for mods sent by command line
        
        class ModInformation;
        class FileInformation;
        class PluginInformation;
        class FolderInformation;
        typedef std::vector<PluginInformation*> PluginVector;
        
        enum class FileFlags : decltype(ModLoaderFile::flags)
        {
            None        = 0,
            IsDirectory = MODLOADER_FF_IS_DIRECTORY,    // File is a directory
        };
        
        enum class Status : uint8_t
        {
            Unchanged,          // Unchanged since last install
            Added,              // Added to filesystem since last install
            Updated,            // Updated at filesystem since last install
            Removed,            // Removed from the filesystem since last install
        };
        
        enum class HandleStatus
        {
            No      = MODLOADER_CHECK_NO,
            Yes     = MODLOADER_CHECK_YES,
            CallMe  = MODLOADER_CHECK_CALL_ME
        };
        
        // Information about a Mod Loader plugin
        class PluginInformation : public ModLoaderPlugin 
        {
            protected:
                friend class Loader;
                bool Startup();         // Calls OnStartup event
                bool Shutdown();        // Calls OnShutdown event
               
            public:
                PluginInformation(void* module, const char* modulename, modloader_fGetPluginData GetPluginData)
                {
                    // Fill basic information
                    std::memset(this, 0, sizeof(ModLoaderPlugin));
                    this->pModule   = module;
                    this->modloader = &loader;
                    
                    // Fill the plugin structure with the rest of the informations
                    if(GetPluginData) GetPluginData(this);
                    
                    // Override priority
                    auto it = loader.plugins_priority.find(NormalizePath(modulename));
                    if(it != loader.plugins_priority.end())
                    {
                         Log("\tOverriding priority, from %d to %d", priority, it->second);
                         this->priority = it->second;
                    }
                }
                
                HandleStatus FindHandleStatus(const ModLoaderFile& m)
                {
                    return CheckFile? (HandleStatus)(CheckFile(this, &m)) : HandleStatus::No;
                }
        };
        
        // Information about a file
        class FileInformation : public ModLoaderFile // TODO
        {
            public: // protected: -- Only FolderInformation should touch this struct
                ModInformation&                 parent;         // The mod this file belongs to
                PluginInformation&              handler;        // The plugin that will handle this file
                std::string                     pathbuf;        // Path buffer (as used in base.buffer)
                PluginVector                    callme;         // Those plugins should receive this file, but they won't handle it
                
                // The following information is used for the in-game install/uninstall system
                bool       installed;   // Is the mod installed?
                Status     status;      // File status
                
                FileInformation(ModInformation& parent, std::string&& xpathbuf, const ModLoaderFile& m,
                                PluginInformation& xhandler, PluginVector&& xcallme)
                
                    : parent(parent), handler(xhandler), pathbuf(std::move(xpathbuf)), callme(std::move(xcallme)),
                      installed(false), status(Status::Added)
                {
                    std::memcpy(this, &m, sizeof(ModLoaderFile));
                }
        };
        
        // Information about a mod folder
        class ModInformation
        {
            public: // protected: -- Only FolderInformation should touch this struct
                friend class FolderInformation;
                FolderInformation&          parent;         // Owner of this mod
                uint32_t                    id;             // Unique mod id
                uint32_t                    priority;       // Mod priority
                std::string                 path;           // Path for this mod (relative to game dir), normalized
                std::string                 name;           // Name for this mod, this is the filename in path (normalized)
                std::string                 fs_name;        // Name for this mod on the filesystem (non normalized)
                std::map<std::string, FileInformation>  files;          // Files inside this mod
                Status                      status;         // Mod status
                
                ModInformation(std::string name, FolderInformation& parent, uint32_t id)
                    : fs_name(name), name(NormalizePath(name)), parent(parent), status(Status::Added), id(id)
                {
                    this->priority = parent.GetPriority(this->name);
                    MakeSureStringIsDirectory(this->path = parent.GetPath() + this->name);
                }
                
                void Scan();
                void UninstallNecessaryFiles();
                void InstallNecessaryFiles();
        };
        
        // Information about a "modloader" folder
        class FolderInformation // TODO
        {
            // TODO NEED TO: sort by priority (any but map/set) AND name (map/set)
            
            public: // Type definitions
                typedef std::map<std::string, FolderInformation>                    FolderInformationList;
                typedef FolderInformationList::iterator                             FolderInformationIterator;
                typedef std::map<std::string, ModInformation>                       ModInformationList;
                typedef ModInformationList::iterator                                ModInformationIterator;
                typedef std::list<std::reference_wrapper<FolderInformation>>        FolderInformationRefList;

            public: // Methods
                FolderInformation(const std::string& path, FolderInformation* parent = nullptr)
                    : path(path), parent(parent), status(Status::Added)
                {}
                
                
                // Clears all buffers from this structure and makes it as if it was at the initial stage
                void Clear();
                
                
                // Checks if the specified mod should be ignored
                bool IsIgnored(std::string name);
                
                // Checks if the specified file should be ignored
                bool IsFileIgnored(std::string name);
                

                
                // Adds a child mod list folder
                FolderInformation& AddChild(std::string path);
                
                // Adds a mod to the container
                ModInformation& AddMod(std::string name);
                
                // Adds a priority to a mod. If priority is zero, it ignores the mod.
                void SetPriority(std::string name, int priority);
                
                // Gets a priority to a mod.
                int GetPriority(std::string name);

                // Adds a mod to the mod inclusion list (to get loaded even if ExcludeAll is true)
                void Include(std::string name);
                
                // Makes a specified file glob ignored
                void IgnoreFileGlob(std::string glob);
                
                // Sets flags
                void SetIgnoreAll(bool bSet);
                void SetExcludeAll(bool bSet);
                void SetForceExclude(bool bSet);
                
                
                
                // Gets me and my childs in a reference list
                FolderInformationRefList GetAll();
                
                // Rebuilds the glob string for exclude_globs
                void RebuildIncludeModsGlob();
                void RebuildExcludeFilesGlob();
                
                
                void Scan();
                void Update();
                
                const std::string& GetPath() { return path; }
                
            protected:
                friend class Loader;
                Status status;    // Folder status
                
            private:    // Data
                std::string path;               // Path relative to game dir
                FolderInformation* parent;      // Parent folder
                
                ModInformationList       mods;      // All mods on this folder
                FolderInformationList    childs;    // All child mod folders on this mod folder (sub sub "modloader/" folders)
                
                // List of settings, all strings are normalized!!!!!
                std::map<std::string, int> mods_priority;   // List of priorities to be applied to mods
                std::set<std::string> include_mods;         // All mod globs inside this list shall be included when bExcludeAll is true
                std::set<std::string> exclude_files;        // All file globs inside this list shall be ignored
                std::string include_mods_glob;              // include_mods built into a single glob
                std::string exclude_files_glob;             // exclude_files built into a single glob
                
                // Folder flags
                struct flags_t
                {
                    bool bIgnoreAll;        // When true, no mod will be readen
                    bool bExcludeAll;       // When true, no mod gets loaded but the ones at include_mods list (set by INI)
                    bool bForceExclude;     // When true, have the same effect as exclude all (set by command line)
                    
                    // Defaults
                    flags_t() : bIgnoreAll(false), bExcludeAll(false), bForceExclude(false)
                    {}
                    
                } flags;
        };

        
        typedef std::map<std::string, PluginVector> ExtMap;
        
    protected:
        friend class scoped_gdir;
        
        // Configs
        bool            bRunning;               // True when the loader was started up, false otherwise
        bool            bEnableLog;             // Enable logging to the log file
        bool            bImmediateFlush;        // Enable immediately flushing the log file
        bool            bEnablePlugins;         // Enable the loading of ML plugins
        
        // Unique ids
        uint32_t        currentModId;           // Current id for the unique mod id
        uint32_t        currentFileId;          // Current id for the unique file id
        
        // Directories
        std::string     gamePath;               // Full game path
        std::string     cachePath;              // Cache path (relative to game path)
        std::string     pluginPath;             // Plugins path (relative to game path)
        
        // Modifications and Plugins
        FolderInformation               mods;               // All mods are contained on this folder
        ExtMap                          extMap;             // List of extensions and the plugins that takes care of it
        std::map<std::string, int>      plugins_priority;   // List of priorities to be applied to plugins
        std::list<PluginInformation>    plugins;            // List of plugins
        
        
    private: // Logging
        void OpenLog();     // Open log stream
        void CloseLog();    // Closes log stream
 
    private: // Plugins Management
        
        bool StartupPlugin(PluginInformation& plugin);
        
        // Loads / unloads all plugins
        void LoadPlugins();
        void UnloadPlugins();
        
        // Loads / Unloads plugins during run-time
        bool LoadPlugin(std::string filename);
        bool UnloadPlugin(PluginInformation& plugin);

        // Rebuilds the extMap object
        void RebuildExtensionMap();
        
    private: // Basic configuration
        void ReadBasicConfig(const char*);
        void ParseCommandLine();
        
    public:
        
        // Constructor
        Loader() : mods("modloader/")
        {}
        
        // Patches the game code to run this core
        void Patch();
        
        // Start or Shutdown the loader
        void Startup();
        void Shutdown();
        
        // Called on specific game circustances
        void OnReload();    // On Game Reloading
        
        
        void ScanAndInstallFiles();
        PluginInformation* FindHandlerForFile(const ModLoaderFile& m, PluginVector& out_callme);
        
        // Logging functions
        static void Log(const char* msg, ...);
        static void vLog(const char* msg, va_list va);
        static void Error(const char* msg, ...);
        static void LogException(void* pExceptionPointers);
        
        uint32_t PickUniqueModId()  { return ++currentModId; }
        uint32_t PickUniqueFileId() { return ++currentFileId; }
        
        
        
        
        
        
        
        
        
        template<class M>
        static void MarkStatus(M& map, Loader::Status status)
        {
            for(auto& pair : map)
            {
                auto& x = pair.second;
                x.status = status;
            }
        }


        template<class T, class M>
        static void FindStatus(T& info, M& map, bool fine)
        {
            if(!fine)
            {
                // Failed to scan this, it probably doesn't exist
                info.status = Status::Removed;
            }
            else if(info.status != Status::Added)
            {
                for(auto& pair : map)
                {
                    auto& x = pair.second;
                    if(x.status != Status::Unchanged)
                    {
                        // Something changed here on this scan
                        info.status = Status::Updated;
                        break;
                    }
                }
            }
        }
};




// Plugins are equal only (and only if) they point to the same data space
inline bool operator==(const Loader::PluginInformation& a, const Loader::PluginInformation& b)
{
    return (&a == &b);
}

struct scoped_gdir : public modloader::scoped_chdir
{
    scoped_gdir(const char* newdir) : scoped_chdir((!newdir[0]? loader.gamePath : loader.gamePath + newdir).data())
    { }
};








// What should be taken into consideration:
//  Zipped Files
//  Game Menu
//  Normalized

/*
CheckFile
InstallFile
UninstallFile
PosProcess
*/

/*
---> File Information
    uint8_t     Flags                                   : 1=Is Directory
    uint8_t     Filepath length
    uint8_t     Index to the filename in the filepath
    uint8_t     Index to the extension in the filepath
    char*       Pointer to the file normalized filepath (see NormalizePath function)
    uint64_t    File size
    uint64_t    File time (Windows FILETIME, that's 100-nanosecond intervals since January 1, 1601 (UTC))
    uint64_t    reserved1 (0)
*/





#endif
