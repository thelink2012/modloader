/*
 * Copyright (C) 2013-2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 *  std-cleo -- Standard CLEO Loader Plugin for San Andreas Mod Loader
 *      This plugin goes against the main rule of modloader that is not touching any file from the game install.
 *      This plugin copies and removes files from the CLEO/ folder, but it tries to make sure that the modifies are undone.
 *      It is recommend that you use CLEO mods directly in CLEO/ folder!
 * 
 * 
 *      NOTICE:
 *          This plugin is deprecated because of it's faults (the need to copy 'n paste stuff)
 *          std-asi is now responssible for loading cleo scripts and cleo plugins
 *          std-text is now responssible for loading fxt files
 * 
 *          As of this version the plugin does nothing but finishes the old cache file cleaning up what it did before
 *          the update!
 * 
 *          Only the pieces of code related to the caching system is left here.
 * 
 */
#include <modloader.hpp>
#include <modloader_util.hpp>
#include <modloader_util_hash.hpp>
#include <modloader_util_path.hpp>
#include <modloader_util_file.hpp>
#include <modloader_util_injector.hpp>
using namespace modloader;





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
                if((flags.bIsDir = (file.is_dir != 0)) == false)
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
        
        char cacheDirPath[MAX_PATH];
        char cacheFilePath[MAX_PATH];
        char cacheCleoPath[MAX_PATH];

        bool StartCacheFile();
        bool FinishCacheFile(bool bIsStartup = false);

        
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
    return "DC1";   // Deprecation Candidate 1, lol
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
    
    return true;
}

bool CThePlugin::OnShutdown()
{
    return true;
}

bool CThePlugin::CheckFile(modloader::ModLoaderFile& file)
{
    return false;
}

bool CThePlugin::ProcessFile(const modloader::ModLoaderFile& file)
{
    return true;
}

bool CThePlugin::PosProcess()
{
    return true;
}

/*
 *  Creates the cache file responsible for restoration of the original state of the CLEO/ folder
 */
bool CThePlugin::StartCacheFile()
{
    return false;
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
    //CreateImportantFolders();
    
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
