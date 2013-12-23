
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
#include <modloader_util_hash.hpp>
#include <modloader_util_path.hpp>
#include <modloader_util_file.hpp>
using namespace modloader;

#include <list>
#include <vector>

/*
 *  The plugin object
 */
class CThePlugin : public modloader::CPlugin
{
    public:
        static const int default_priority = 50;
        
        const char* GetName();
        const char* GetAuthor();
        const char* GetVersion();
        int OnStartup();
        int OnShutdown();
        int CheckFile(const modloader::ModLoaderFile& file);
        int ProcessFile(const modloader::ModLoaderFile& file);
        int PosProcess();
        
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
                };
                
            } flags;
            
            
            CacheStruct()
            {
                hash = 0;
                flags.i = 0;
                srcPath[0] = dstPath[0] = 0;
            }
            
            /*
             *  Construct this using @file as a CLEO file
             */
            CacheStruct(const ModLoaderFile& file)
            {
                flags.bIsDir   = file.is_dir;
                if(!flags.bIsDir) flags.bIsFXT   = IsFileExtension(file.filext, "fxt"); 
                strcpy(srcPath, GetFilePath(file).c_str());
                strcpy(dstPath, (std::string(!flags.bIsFXT? "CLEO\\" : "CLEO\\CLEO_TEXT\\") + file.filename).c_str());
            }
            
            /*
             *  Construct this using @file as a CLEO file inside @modir 
             */
            CacheStruct(const ModLoaderFile& modir, const ModLoaderFile& file)
            {
                std::string src = GetFilePath(modir) + file.filepath;
                std::string dst = std::string("CLEO\\") + file.filepath;

                /* Setup */
                flags.bIsDir = file.is_dir;
                if(!flags.bIsDir) flags.bIsFXT = IsFileExtension(file.filext, "fxt");
                strcpy(srcPath, src.c_str());
                strcpy(dstPath, dst.c_str());
            }
            
        };
        
        bool bHaveCLEO;
        std::list<CacheStruct> files;
        char cacheDirPath[MAX_PATH];
        char cacheFilePath[MAX_PATH];
        char cacheCleoPath[MAX_PATH];
        
        bool StartCacheFile();
        bool FinishCacheFile(bool bIsStartup = false);
        
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
        
} plugin;

/*
 *  Export plugin object data
 */
extern "C" __declspec(dllexport)
void GetPluginData(modloader_plugin_t* data)
{
    modloader::RegisterPluginData(plugin, data);
    plugin.data->priority = plugin.default_priority;
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
    return "1.0";
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
int CThePlugin::OnStartup()
{
    /* Setup vars */
    sprintf(cacheDirPath, "%s%s", this->modloader->cachepath, "std-cleo\\");
    sprintf(cacheFilePath, "%s%s", cacheDirPath, "cache.bin");
    sprintf(cacheCleoPath, "%s%s", cacheDirPath, "CLEO\\");
    bHaveCLEO     = DoesHaveCLEO();

    CreateImportantFolders();
    
    if(bHaveCLEO)
    {
        /* Finish the cache if it still exist, probably because an abnormal termination of the game */
        FinishCacheFile(true);
    }
    else
        Log("CLEO not found! std-cleo plugin features will be disabled.");
    
    return 0;
}

int CThePlugin::OnShutdown()
{
    if(bHaveCLEO)
    {
        FinishCacheFile();
    }
    return 0;
}

/*
 *  Check if the file is the one we're looking for
 */
int CThePlugin::CheckFile(const modloader::ModLoaderFile& file)
{
    if(bHaveCLEO)
    {
        if(file.is_dir)
        {
            /* me can handle folder named CLEO */
            return (!strcmp(file.filename, "CLEO", false)? MODLOADER_YES : MODLOADER_NO);
        }
        else
        {
            /* Check if an CLEO handled extension */
            for(const char** p = GetExtensionTable(); *p; ++p)
            {
                if(IsFileExtension(file.filext, *p))
                    return MODLOADER_YES;
            }
        }
    }
    return MODLOADER_NO;
}

/*
 * Process the replacement
 */
int CThePlugin::ProcessFile(const modloader::ModLoaderFile& file)
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
    return 0;
}

/*
 * Called after all files have been processed
 */
int CThePlugin::PosProcess()
{
    if(bHaveCLEO)
    {
        return !StartCacheFile();
    }
    return 0;
}

/*
 *  Creates the cache file responsible for restoration of the original state of the CLEO/ folder
 */
bool CThePlugin::StartCacheFile()
{
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
        DestroyDirectoryA(cacheCleoPath);
        if(!CopyDirectoryA("CLEO\\", cacheCleoPath))
        {
            Log("Failed to create backup CLEO folder. Aborting.");
        }
        /* For each file, write to the cache it's information */
        else for(auto& file : this->files)
        {
            if(file.flags.bIsDir)
            {
                /* Just make sure we have that directory created */
                MakeSureDirectoryExistA(file.dstPath);
                Log("Created directory \"%s\"", file.dstPath);
                continue;
            }
            else if(IsPathA(file.dstPath))
            {
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
                Log("Failed to copy source file (%s) to destination (%s), error code: %d. Skipping it.",
                    file.srcPath, file.dstPath, GetLastError());
                continue;
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
    
    fclose(f);
    Log("Cache file has been started");
    files.clear();  /* I don't need this data anymore */
    return true;
}

/*
 *  Restores the CLEO/ folder 
 * 
 * 
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
            Log("Failed to open cache file for reading (%s)", cacheFilePath);
        return false;
    }
    
    /* Check if the header is alright */
    if(fread(&header, sizeof(header), 1, f) != 1 || !header.Check())
    {
        Log("Failed to read or validate cache file header. File may be corrupted.");
    }
    else
    {
        /* Read each entry in the file */
        while(fread(&file, sizeof(file), 1, f) == 1)
        {
            if(file.flags.bIsDir)
            {
                /* Ignore 'cause is dir */
                Log("Ignoring file \"%s\" because it is a directory", file.dstPath);
                continue;
            }
            
            /* Take the hash to see if the file is still the same as the one I copied */
            if(!ReadEntireFile(file.dstPath, filebuf)
            || file.hash != hash((void*)filebuf.data(), filebuf.size()))
            {
                Log("Failed to read or validate hash for file \"%s\". Skipping it.", file.dstPath);
                continue;
            }
            
            /* Delete the file */
            if(true)
            {
                if(DeleteFileA(file.dstPath))
                    Log("File has been deleted: \"%s\"", file.dstPath);
                else
                    Log("Failed to delete file \"%s\"", file.dstPath);
            }
            
            
            /* If file existed before, take it from the backup folder and put back on the CLEO folder */
            if(file.flags.bExists)
            {
                Log("Restoring original file for \"%s\"", file.dstPath);
                
                char src[MAX_PATH];
                sprintf(src, "%s%s", cacheDirPath, file.dstPath);
                if(!CopyFileA(src, file.dstPath, FALSE))
                    Log("Failed to restore file \"%s\"", file.dstPath);
            }
            
        }
        
        /* Check if successful read all entries */
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
