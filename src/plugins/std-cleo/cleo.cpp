/*
 * Copyright (C) 2013  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 *  std-cleo -- Standard CLEO Loader Plugin for San Andreas Mod Loader
 *      This plugin goes against the main rule of modloader that is not touching any file from the game install.
 *      This plugin copies and removes files from the CLEO/ folder, but it tries to make sure that the modifies are undone.
 *      It is recommend that you use CLEO mods directly in CLEO/ folder!
 * 
 */
#include <modloader.hpp>
#include <modloader_util.hpp>
#include <modloader_util_hash.hpp>
#include <modloader_util_path.hpp>
#include <modloader_util_file.hpp>
#include <modloader_util_injector.hpp>
using namespace modloader;

#include <list>
#include <vector>
#include <set>
#include <map>

#include "Injector.h"
#include "CText.h"

/*
 *  The plugin object
 */
class CThePlugin* cleoPlugin;
class CThePlugin : public modloader::CPlugin
{
    public:
        const char* GetName();
        const char* GetAuthor();
        const char* GetVersion();
        bool OnStartup();
        bool OnShutdown();
        bool CheckFile(modloader::ModLoaderFile& file);
        bool ProcessFile(const modloader::ModLoaderFile& file);
        bool PosProcess();
        bool OnLoad();
        
        const char** GetExtensionTable();

    public:
        
        /* Cache file header */
        struct CacheHeader
        {
            static const uint32_t currentVersion = 1;
            
            char magic[4];      /* "\0CCC" */
            uint32_t version;   /* Current cache file version */
            
            CacheHeader()
            {
                magic[0] = 0; magic[1] = 'C'; magic[2] = 'C'; magic[3] = 'C';
                version = currentVersion;
            }
            
            /* Checks if the file header is okay */
            bool Check()
            {
                if(magic[0] == 0 && magic[1] == 'C' && magic[2] == 'C' && magic[3] == 'C')
                {
                    if(version == currentVersion)
                        return true;
                }
                return false;
            }
        };
        
        /* The structure of the cache file, this data is repeated for each copied file */
        struct CacheStruct
        {
            char        srcPath[128];       /* Source directory of the copy process */
            char        dstPath[128];       /* Destination directory of the copy process */
            size_t      hash;               /* Source file hash */
            
            union {
                uint32_t i;
                
                struct {
                    bool bExists : 1;   /* File existed before copying ours? Create backup at $CACHE/CLEO */
                    bool bIsDir  : 1;   /* This is actually a directory, just create the directory dstPath */
                    bool bIsFXT  : 1;   /* If FXT copy to CLEO_TEXT */
                    bool bIsConf : 1;   /* If is config file, such as ini... Copy from CLEO folder to the mod folder on shutdown */
                    bool bIsCLEO : 1;   /* True when the extension is .cleo, that's a CLEO plugin */
                };

            } flags;
            
            
            /*
             *  Constructs without any data initialization 
             */
            CacheStruct()
            {
            }
            
            /*
             *  Construct this using @file as a CLEO file
             */
            CacheStruct(const ModLoaderFile& file) : hash(0)
            {
                SetupFlags(file);
                strcpy(srcPath, GetFilePath(file).c_str());
                strcpy(dstPath, (std::string(!flags.bIsFXT? "CLEO\\" : "CLEO\\CLEO_TEXT\\") + file.filename).c_str());
                PosConstruct();
            }
            
            /*
             *  Construct this using @file as a CLEO file inside @modir 
             */
            CacheStruct(const ModLoaderFile& modir, const ModLoaderFile& file) : hash(0)
            {
                SetupFlags(file);
                strcpy(srcPath, (GetFilePath(modir) + file.filepath).c_str());
                strcpy(dstPath, (std::string("CLEO\\") + file.filepath).c_str());
                PosConstruct();
            }
            
            void SetupFlags(const ModLoaderFile& file)
            {
                flags.i = 0; hash = 0;
                if((flags.bIsDir = file.is_dir) == false)
                {
                    flags.bIsFXT  = IsFileExtension(file.filext, "fxt");
                    flags.bIsConf = IsFileExtension(file.filext, "ini");
                    flags.bIsCLEO = IsFileExtension(file.filext, "cleo");
                }
            }
            
            void PosConstruct()
            {
                // If it's a CLEO plugin, change file extension to something else
                // 'cause otherwise we won't be able to delete it on Startup since CLEO will have it's module loaded.
                if(flags.bIsCLEO)
                {
                    if(char* p = strrchr(dstPath, '.'))
                    {
                        strcpy(p+1, "_cleo_");
                    }
                }
            }
            
            
        };
        
        bool bHaveCLEO;
        std::vector<HMODULE>   plugins;
        std::list<CacheStruct> files;
        std::map<size_t, std::string> text; // for gxt hook
        char cacheDirPath[MAX_PATH];
        char cacheFilePath[MAX_PATH];
        char cacheCleoPath[MAX_PATH];

        bool StartCacheFile();
        bool FinishCacheFile(bool bIsStartup = false);
        
        
        /*
         *  Parses the FXT file @filename calling @AddText foreach key-value pari 
         */
        bool ParseFXT(const char* filename);
        
        /*
         *  Adds a GXT key-value pair to the @text map for use in our GxtHook 
         */
        void AddText(const char* key, const char* value)
        {
            text[modloader::hash(key, ::toupper)] = value;
        }
        
        
        /*
         *  Creates all necessary folders for this plugin to work properly 
         */
        void CreateImportantFolders()
        {
            MakeSureDirectoryExistA("CLEO");
            MakeSureDirectoryExistA("CLEO\\CLEO_TEXT");
            MakeSureDirectoryExistA(cacheDirPath);
            MakeSureDirectoryExistA(cacheCleoPath);
        }
        
        /*
         * Checks if the user has the CLEO library installed
         */
        bool DoesHaveCLEO()
        {
            /* You have CLEO if either CLEO folder exists or cleo.asi exists */
            return (IsDirectoryA("CLEO") || IsPathA("cleo.asi"));
        }

        
        typedef const char* (__fastcall *CText__Locate_Type)(CText*, int, const char*);
        
        /*
         *  Address for CLEO's GXT HOOK
         */
        static CText__Locate_Type& CLEO_GxtHook()
        {
            static CText__Locate_Type hook = 0;
            return hook;
        }
        
        /*
         *  Hooks CText::Locate taking in consideration CLEO already hooked it 
         */
        static const char* __fastcall GxtHook(CText* self, int, const char* key)
        {
            auto& text = cleoPlugin->text;
            
            // Try to find @key in our text map
            auto it = text.find(modloader::hash(key, ::toupper));
            if(it != text.end()) return it->second.c_str();
            
            // Nothing found, try with CLEO call
            return CLEO_GxtHook()(self, 0, key);
        }
        
        
} plugin;

/*
 *  Export plugin object data
 */
extern "C" __declspec(dllexport)
void GetPluginData(modloader_plugin_t* data)
{
    cleoPlugin = &plugin;
    modloader::RegisterPluginData(plugin, data);
}



/*
 *  Basic plugin informations
 */
const char* CThePlugin::GetName()
{
    return "std-cleo";
}

const char* CThePlugin::GetAuthor()
{
    return "LINK/2012";
}

const char* CThePlugin::GetVersion()
{
    return "RC2";
}

const char** CThePlugin::GetExtensionTable()
{
    /* Put the extensions  this plugin handles on @table */
    static const char* table[] = { "cs", "cm", "cleo", "fxt", 0 };
    return table;
}

/*
 *  Startup / Shutdown (do nothing)
 */
bool CThePlugin::OnStartup()
{
    scoped_chdir xdir(this->modloader->gamepath);
    
    /* Setup vars */
    sprintf(cacheDirPath, "%s%s", this->modloader->cachepath, "std-cleo\\");
    sprintf(cacheFilePath, "%s%s", cacheDirPath, "cache.bin");
    sprintf(cacheCleoPath, "%s%s", cacheDirPath, "CLEO\\");

    /* Finish the cache if it still exist, probably because an abnormal termination of the game */
    FinishCacheFile(true);
    
    if(bHaveCLEO = DoesHaveCLEO())
    {
        //
        CreateImportantFolders();
    }
    else
        Log("CLEO not found! std-cleo plugin features will be disabled.");
    
    return true;
}

bool CThePlugin::OnShutdown()
{
    if(bHaveCLEO)
    {
        scoped_chdir xdir(this->modloader->gamepath);
        std::for_each(this->plugins.begin(), this->plugins.end(), FreeLibrary); // free .cleo plugins
        FinishCacheFile();
    }
    return true;
}

/*
 *  Check if the file is the one we're looking for
 */
bool CThePlugin::CheckFile(modloader::ModLoaderFile& file)
{
    if(bHaveCLEO)
    {
        if(file.is_dir)
        {
            /* me can handle folder named CLEO */
            return (!strcmp(file.filename, "CLEO", false)? true : false);
        }
        else
        {
            /* Check if an CLEO handled extension */
            for(const char** p = GetExtensionTable(); *p; ++p)
            {
                if(IsFileExtension(file.filext, *p))
                    return true;
            }
        }
    }
    return false;
}

/*
 * Process the replacement
 */
bool CThePlugin::ProcessFile(const modloader::ModLoaderFile& file)
{
    if(bHaveCLEO)
    {
        if(file.is_dir)
        {
            // This is the CLEO directory, go recursive to copy all files inside this folder
            scoped_chdir xdir(file.filepath);
            
            ForeachFile("*.*", true, [this,&file](ModLoaderFile& mf)
            {
                this->files.emplace_back(file, mf);
                return true;
            });
        }
        else
        {
            this->files.emplace_back(file);
        }
    }
    return true;
}

/*
 * Called after all files have been processed
 */
bool CThePlugin::PosProcess()
{
    if(bHaveCLEO)
    {
        // Add some chunks for us in the loading bar
        this->SetChunks(this->files.size());
        this->SetChunkLimiter();
        
        // Make our GXT hook
        if(ReadMemory<uint8_t>(0x6A0050, true) == 0xE9) 
        {
            // CLEO already did it's GXT hook here, good!
            this->CLEO_GxtHook() = GetBranchDestination(0x6A0050).get();
            
            // Do ours now
            MakeJMP(0x6A0050, raw_ptr(CThePlugin::GxtHook));
        }
        else
            Log("Failed to install std-cleo gxt hook because CLEO.asi didn't install it's hook yet");
    }
    
    return true;
}

/*
 *  Called on the loading screen 
 */
bool CThePlugin::OnLoad()
{
    this->StartCacheFile();
    return true;
}

/*
 *  Creates the cache file responsible for restoration of the original state of the CLEO/ folder
 */
bool CThePlugin::StartCacheFile()
{
    std::set<std::string> pluginsPath;  // set of normalize strings pointing to files in .cleo folder
                                        // don't repeat a plugin file twice
    
    std::vector<char> filebuf;
    CacheHeader header;
   
    Log("Starting cache file");
    CreateImportantFolders();
    
    FILE* f = fopen(cacheFilePath, "wb");
    if(f == NULL)
    {
        Log("Failed to open cache file for writing (%s)", cacheFilePath);
        return false;
    }
    
    if(fwrite(&header, sizeof(header), 1, f) != 1)
    {
        Log("Failed to write header to cache file. Cache corruption may happen.");
    }
    else
    {
        Log("Creating backup CLEO folder");
        
        // Destroy old backup
        if(!DestroyDirectoryA(cacheCleoPath))
        {
            Log("Failed to destroy backup CLEO folder in-cache. Error code: 0x%X.", GetLastError());
        }
        
        // Create new backup
        if(!CopyDirectoryA("CLEO\\", cacheCleoPath))
        {
            Log("Failed to create backup CLEO folder. Error code: 0x%X. Aborting.", GetLastError());
        }
        /* For each file, write to the cache it's information */
        else for(auto& file : this->files)
        {
            this->NewChunkLoaded();
            
            if(file.flags.bIsDir)
            {
                /* Just make sure we have that directory created */
                MakeSureDirectoryExistA(file.dstPath);
                Log("Created directory \"%s\"", file.dstPath);
                continue;
            }
            else 
            {
                // If FXT file, don't even copy it into CLEO, we will handle it ourselfs
                if(file.flags.bIsFXT)
                {
                    this->ParseFXT(file.srcPath);
                    continue;
                }
                
                // Check if file already exist in the destination CLEO folder
                if(IsPathA(file.dstPath))
                    file.flags.bExists = true;
            }
            
            /* Take the file hash */
            if(!ReadEntireFile(file.srcPath, filebuf))
            {
                Log("Failed to read source file for hashing (\"%s\"). Skipping it.", file.srcPath);
                continue;
            }
            else
            {
                file.hash = hash((void*)filebuf.data(), filebuf.size());
            }
            
            /* Finally make the file copy at main game path CLEO/ */
            if(!CopyFileA(file.srcPath, file.dstPath, FALSE))
            {
                Log("Failed to copy source file (%s) to destination (%s)\n\terror code: 0x%X. Skipping it.",
                    file.srcPath, file.dstPath, GetLastError());
                continue;
            }
            else
            {
                if(file.flags.bIsCLEO)  // Is .cleo plugin?
                {
                    // Push it's path for later processing
                    pluginsPath.emplace(NormalizePath(file.dstPath));
                }
            }
            
            /* Write file information to the cache and flush immediately, we don't want to lose information on some crash */
            if(fwrite(&file, sizeof(file), 1, f) != 1)
            {
                Log("Failed to write file info into the cache. Cache corruption may happen.");
                continue;
            }
            else fflush(f);
        }
    }
    
    // Load each CLEO plugin
    for(auto& path : pluginsPath)
    {
        if(HMODULE module = LoadLibraryA(path.c_str()))
        {
            Log("CLEO plugin has been loaded: %s", path.c_str());
            this->plugins.emplace_back(module); // for later FreeLibrary
        }
        else
        {
            Log("Failed to load .CLEO plugin (%s)\n\terror code: 0x%X.", path.c_str(), GetLastError());
        }
    }
    
    // I don't need this data anymore, fre memory
    files.clear();
    
    fclose(f);
    Log("Cache file has been started");
    return true;
}

/*
 *  Restores the CLEO/ folder 
 */
bool CThePlugin::FinishCacheFile(bool bIsStartup)
{
    std::vector<char> filebuf;
    CacheHeader header;
    CacheStruct file;
    
    Log("Finishing cache file");
    CreateImportantFolders();
    
    FILE* f = fopen(cacheFilePath, "rb");
    if(f == NULL)
    {
        if(bIsStartup)
            Log("Nothing to finish");
        else
            Log("Failed to open cache file for reading: %s", cacheFilePath);
        return false;
    }
    
    // Check if the header is alright
    if(fread(&header, sizeof(header), 1, f) != 1 || !header.Check())
    {
        Log("Failed to read or validate cache file header. File may be corrupted.");
    }
    else
    {
        // Read each entry in the file
        while(fread(&file, sizeof(file), 1, f) == 1)
        {
            if(file.flags.bIsDir)
            {
                // Ignore 'cause is dir
                Log("Ignoring file \"%s\" because it is a directory", file.dstPath);
                continue;
            }
            
            // Take the hash to see if the file is still the same as the one I copied
            if(!ReadEntireFile(file.dstPath, filebuf))
            {
                Log("Failed to read file \"%s\". Skipping it.", file.dstPath);
                continue;
            }
            
            // Check if file has changed...
            if(file.hash != hash((void*)filebuf.data(), filebuf.size()))
            {
                // If config file, update it in the source path
                if(file.flags.bIsConf)
                {
                    Log("Config file \"%s\" has changed. Copying it back to source directory \"%s\"",
                        file.dstPath, file.srcPath);
                    
                    if(!CopyFileA(file.dstPath, file.srcPath, FALSE))
                        Log("Failed to update config file, ignoring this failure");
                }
                else
                {
                    // Not config file, nothing should've changed since then
                    Log("Failed to validate hash for file \"%s\". Skipping it.", file.dstPath);
                    continue;
                }
            }
            
            // Delete the file
            if(true)
            {
                if(DeleteFileA(file.dstPath))
                    Log("File has been deleted: \"%s\"", file.dstPath);
                else
                    Log("Failed to delete file \"%s\"", file.dstPath);
            }
            
            
            // If file existed before, take it from the backup folder and put back on the CLEO folder
            if(file.flags.bExists)
            {
                Log("Restoring original file for \"%s\"", file.dstPath);
                
                char src[MAX_PATH];
                sprintf(src, "%s%s", cacheDirPath, file.dstPath);
                if(!CopyFileA(src, file.dstPath, FALSE))
                    Log("Failed to restore file \"%s\"", file.dstPath);
            }
            
        }
        
        // Check if successful read all entries
        if(!feof(f) && ferror(f))
            Log("Failed to read cache file. Reading terminated without EOF reached.");
        else
        {
            fclose(f); f = 0;                   /* close cache before deleting it */
            DeleteFileA(cacheFilePath);         /* delete cache */
            DestroyDirectoryA(cacheCleoPath);   /* delete bakup */
        }
    }
    
    Log("Cache file has been finished");
    if(f) fclose(f);
    return true;
}

/*
 *  Parses a CLEO FXT file into our text map 
 */
bool CThePlugin::ParseFXT(const char* filename)
{
    if(FILE* f = fopen(filename, "r"))
    {
        char buf[1024];
        
        Log("Reading FXT file \"%s\"", filename);
        
        // Alright, that format is pretty easy
        while(fgets(buf, sizeof(buf), f))
        {
            char *key = 0, *value = 0;
            
            // Parses the fxt line
            for(char* p = buf; *p; ++p)
            {
                // # is used as comments...
                if(*p == '#')
                {
                    *p = 0;
                    break;
                }
                
                // If no key found yet...
                if(key == 0)
                {
                    // ...and the current iterating character is a space...
                    if(isspace(*p))
                    {
                        // ..we've just found that the key is there
                        *p = 0;
                        key = buf;
                    }
                }
                // But, if key has been found but no value found yet...
                else if(value == 0)
                {
                    // ...we are actually reading the first character from the value!
                    value = p;
                }
            }
            
            // Adds into the text map only if found both key and value
            if(key && value) this->AddText(key, value);
        }
        
        // Done
        fclose(f);
        return true;
    }
    else
        Log("Failed to open FXT file \"%s\" for reading.", filename);
    
    return false;
}

