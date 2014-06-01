/*
 * Copyright (C) 2013-2014  LINK/2012 <dma_2012@hotmail.com>
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
        static const int default_priority = 48;
        
        const char* GetName();
        const char* GetAuthor();
        const char* GetVersion();
        
        bool OnStartup();
        bool OnShutdown();
        
        bool CheckFile(modloader::modloader::file& file);
        bool ProcessFile(const modloader::modloader::file& file);
        bool PosProcess();
        bool OnReload();
        
        const char** GetExtensionTable();

        // Object that represents an abstract model file
        struct ModelFile
        {
            private:
                size_t size;            // Normal size
                size_t blocks;          // Size in 2KiB blocks
                bool bProcessed;        // Processed the above values?
                
            public:
                std::string name;       // Model name (e.g. 'cheetah.dff')
                std::string path;       // Path to the model
                size_t hash;            // Model name hash
                bool isClothes;         // Present on player.img file too, should be handled as a clothing item
                bool isForcedClothes;   // Mayn't be present on player.img file but is to be handled as a clothing item
            
            ModelFile(const char* filename, const char* filepath)
                : name(filename), path(filepath), isClothes(false), isForcedClothes(false),
                                                  bProcessed(false)
            {
                // Get the model name hash
                this->hash = modloader::hash(this->name, ::toupper);
                
                // Check out if it's a forced clothing item
                std::string lpath = path;
                modloader::tolower(lpath);
                this->isForcedClothes = lpath.find("player") != lpath.npos;
            }
            
            // Check if this model is a clothing item (either by default or forced)
            bool IsClothes()
            {
                return isClothes || isForcedClothes;
            }
            
            // Check if this model is a clothing item also present on player.img
            bool IsAlsoPresentOnPlayerImg()
            {
                return isClothes;
            }
            
            // Find out the 'size' and 'blocks' field for this object
            void ProcessFileData()
            {
                this->bProcessed = true;
                this->size   = (size_t) modloader::GetFileSize(path.c_str());
                auto div     = std::div(this->size, 2048);
                this->blocks = div.quot + (div.rem? 1 : 0);
            }
            
            // Make sure this structure has been processed and so the 'size' and 'blocks parameter are set
            void MakeSureHasProcessed()
            {
                if(this->bProcessed == false) this->ProcessFileData();
            }
            
            // Gets the file size
            size_t GetSize()
            {
                MakeSureHasProcessed();
                return this->size;
            }
            
            // Gets the file size in 2KiB blocks (e.g. if GetSize() == 2048, this will return 1)
            size_t GetSizeInBlocks()
            {
                MakeSureHasProcessed();
                return this->blocks;
            }
        };
        
        // Object that represents an img file loaded by modloader
        struct ImgFile
        {
            std::string name;   // File Name
            std::string path;   // File Path
            
            ImgFile(const char* filename, const char* filepath)
                : name(filename), path(filepath)
            { }
        };

    private:
        std::list<ModelFile> modelsFiles;   // List of .dff, .txd, .col, .ifp, etc files 
        
    public:
        typedef std::reference_wrapper<ModelFile> ModelFileRef;
        
        std::string pedIfp;                     // ped.ifp replacement
        std::list<ImgFile> imgFiles;            // List of .img files
        std::map<size_t, ModelFileRef> models;  // Pointer to the modelsFiles above, except it's using a hash for fast-lookup

        void ReadImgFolder(const modloader::modloader::file& file);
        void BuildModelsMap();
        void ReplaceStandardImg();
        
        void StreamingPatch();
        void ReloadModels();
        
};

#endif	/* IMG_H */

