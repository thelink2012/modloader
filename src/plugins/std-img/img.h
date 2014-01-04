/*
 * Copyright (C) 2013  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#ifndef IMG_H
#define	IMG_H

#include <modloader.hpp>
#include <modloader_util_hash.hpp>
#include <modloader_util_path.hpp>
#include <string>
#include <list>
#include <map>

/*
 *  The plugin object
 */
extern class CThePlugin* imgPlugin;
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
        
        //bool OnSplash();
        //bool OnLoad();
        //bool OnReload();
        
        const char** GetExtensionTable();

        struct ModelFile
        {
            private:
                size_t size;
                size_t blocks;
                bool bProcessed;
                
            public:
                std::string name;
                std::string path;
                size_t hash;
                bool isClothes;
                int nTimes;
            
            ModelFile(const char* filename, const char* filepath)
                : name(filename), path(filepath), isClothes(false), bProcessed(false), nTimes(0)
            {
                this->hash = modloader::hash(this->name, ::toupper);
            }
            
            void ProcessFileData()
            {
                this->bProcessed = true;
                
                this->size   = modloader::GetFileSize(path.c_str());
                auto div     = std::div(this->size, 2048);
                this->blocks = div.quot + (div.rem? 1 : 0);
            }
            
            void MakeSureHasProcessed()
            {
                if(this->bProcessed == false) this->ProcessFileData();
            }
            
            size_t GetSize()
            {
                MakeSureHasProcessed();
                return this->size;
            }
            
            size_t GetSizeInBlocks()
            {
                MakeSureHasProcessed();
                return this->blocks;
            }
        };
        
        struct ImgFile
        {
            std::string name;
            std::string path;
            
            ImgFile(const char* filename, const char* filepath)
                : name(filename), path(filepath)
            {
            }
        };
        
        typedef std::reference_wrapper<ModelFile> ModelFileRef;
        
    private:
        std::list<ModelFile> modelsFiles;   // List of .dff, .txd, .col, .ifp, etc files 
        
    public:
        std::string pedIfp;                     // ped.ifp replacement
        std::list<ImgFile> imgFiles;            // List of .img files
        std::map<size_t, ModelFileRef> models;  // Pointer to the modelsFiles above, except it's using a hash for fast-lookup

        void ReadImgFolder(const modloader::ModLoaderFile& file);
        void BuildModelsMap();
        void ReplaceStandardImg();
        
        void StreamingPatch();
        
        
};

#endif	/* IMG_H */

