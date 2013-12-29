/* 
 * std-img -- Standard IMG Loader Plugin for San Andreas Mod Loader
 * Copyright (C) 2013  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 *  std-img main header file
 * 
 */
#ifndef IMG_H
#define	IMG_H

#include <windows.h>

#include <modloader.hpp>
#include <modloader_util.hpp>
#include <modloader_util_hash.hpp>
#include <modloader_util_path.hpp>

#include <cctype>

#include <string>
#include <map>
#include <list>
#include <vector>

using namespace modloader;

extern class CThePlugin* imgPlugin;
extern void ApplyPatches();

/* Gets the file size from filename */
inline __int64 GetFileSize(const char* filename)
{
    WIN32_FILE_ATTRIBUTE_DATA fad; LARGE_INTEGER size;

    if(!GetFileAttributesExA(filename, GetFileExInfoStandard, &fad))
            return 0;

    size.HighPart = fad.nFileSizeHigh;
    size.LowPart = fad.nFileSizeLow;
    return size.QuadPart;
}

class CThePlugin : public modloader::CPlugin
{
    public:
        static const int default_priority = 48;
        
        struct FileInfo
        {
            std::string name;
            std::string path;
            uint32_t fileHash;
            size_t fileSize, fileSizeBlocks;    /* fileSize (normal) and fileSize in 2KiB blocks */
            bool bProcessed;
            bool bIsPlayerFile;
            bool bImported;
            
            
            FileInfo() :
                bProcessed(false), bIsPlayerFile(false), bImported(false)
            {}
            
            /* Eats the file path and outputs file information into myself */
            void Process()
            {
                this->bProcessed = true;

                this->fileSize = GetFileSize(this->path.c_str());
                /* aligned fileSize in 2KiB blocks */
                this->fileSizeBlocks = ((fileSize % 2048) == 0? fileSize / 2048 : (fileSize / 2048) + 1);
                
                this->fileHash = hash(name, ::toupper);
            }
            
            size_t GetSizeInBlocks()
            {
                if(!this->bProcessed) this->Process();
                return fileSizeBlocks;
            }
            
    
        };
        
        struct ImgInfo
        {
            /* This structure will represent an img file from the user */
            
            typedef std::map<std::string, FileInfo> imgFiles_t;
            
            bool isPlayerContent;           /* is player.img equivalent content? */
            bool isMainContent;             /* is our custom files? */
            bool isCustomContent;           /* is neither player or main content */
            bool isOriginal;                /* is one of the original game files */
            bool isReady;                   /* Tells if the ImgInfo is ready to be used */

            std::string pathMod;            /* img file path relative to mod dir */
            std::string path;               /* the full img file path (relative to game dir) */
            size_t pathModHash, pathHash;   /* hashes for fast compare */
            
            /* map<InsideImgFileName, InComputerPath> */
            imgFiles_t imgFiles;
            
            /* Sorted files */
            std::map<uint32_t, FileInfo*> imgFilesSorted;

            ImgInfo(int i = 0)
            {
                Setup(i);
            }
            
            ImgInfo(const modloader::ModLoaderFile& file)
            {
                Setup();
                Setup(file);
            }
            
            void Process()
            {
                /* Lets build the list of files sorted in hashing order. */
                {
                    /* Process files and push them into sorted list... */
                    for(auto& x : imgFiles)
                    {
                        x.second.Process();
                        if(x.second.fileHash && x.second.fileHash != -1)
                            imgFilesSorted[x.second.fileHash] = &x.second;
                        
                        imgPlugin->NewChunkLoaded();
                    }
                }
            }
            
            void Setup(int i = 0)
            {
                isReady         = true;
                isMainContent   = (i & 1) != 0;
                isPlayerContent = (i & 2) != 0;
                isCustomContent = !isMainContent && !isPlayerContent;
                isOriginal      = false;
            }
            
            void Setup(const modloader::ModLoaderFile& file)
            {
                this->path        = NormalizePath(GetFilePath(file));
                this->pathMod     = NormalizePath(file.filepath);
                this->pathHash    = hash(this->path);
                this->pathModHash = hash(this->pathMod);
            }
            
        };
        
        std::string pedIfp;                              /* ped.ifp replacement path */
        
        ImgInfo mainContent;                             /* main contents */
        std::list<ImgInfo> imgFiles;                     /* list of img files to import */
        
        std::map<unsigned short, FileInfo*> importList;  /* map of objects (model/ifp/col/etc) index
                                                          * and it's respective file pointer */
        
        std::map<unsigned long,  FileInfo*> playerFiles; /* map of the offset of a certain file in the game player.img
                                                          * and it's equivalent file pointer in our mods folder */
        
        
        CThePlugin() : mainContent(1)
        { }
        
        
        /* Plugin Callbacks */
        const char* GetName();
        const char* GetAuthor();
        const char* GetVersion();
        bool OnStartup();
        bool OnShutdown();
        bool CheckFile(modloader::ModLoaderFile& file);
        bool ProcessFile(const modloader::ModLoaderFile& file);
        //bool PosProcess();
        bool OnLoad();
        bool OnReload();
        const char** GetExtensionTable();
        
        
        /* <--> */
        bool ProcessImgFolder(const modloader::ModLoaderFile& file);
        bool ProcessImgFile(const modloader::ModLoaderFile& file);
        void AddFileToImg(ImgInfo& img, const ModLoaderFile& file, const char* filename2 = 0);
        
 
};



#endif

