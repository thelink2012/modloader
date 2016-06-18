/*
 * Copyright (C) 2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under the MIT License, see LICENSE at top level directory.
 * 
 */
#ifndef STREAMING_HPP
#define	STREAMING_HPP

#include <list>
#include <map>

#include <modloader/modloader.hpp>
#include <modloader/util/hash.hpp>
#include <modloader/util/injector.hpp>
#include <modloader/util/container.hpp>
#include "CDirectory.h"
#include "CStreamingInfo.h"

#include <traits/gta3/sa.hpp>
#include <traits/gta3/vc.hpp>
#include <traits/gta3/iii.hpp>

using namespace modloader;

// Streaming file type
enum class ResType : uint8_t // max 4 bits (value 15)
{
    None            = 0,        // Unknown
    Model           = 1,        // DFF
    TexDictionary   = 2,        // TXD
    Collision       = 3,        // COL
    StreamedScene   = 4,        // IPL
    Nodes           = 5,        // nodes%d.dat
    AnimFile        = 6,        // IFP
    VehRecording    = 7,        // RRR
    StreamedScript  = 8,        // SCM

    Max             = 16        // Max 4 bits
};

// Type of non streamed resource
enum class NonStreamedType : uint8_t
{
    TexDictionary,
    ColFile,
    AtomicFile,
    ClumpFile,
};

// Behaviour maskes
static const uint64_t hash_mask         = 0x00000000FFFFFFFF;   // Mask for the hash on the behaviour
static const uint64_t is_img_file_mask  = 0x0000000100000000;   // Is .img file
static const uint64_t is_item_mask      = 0x0000000400000000;   // Is item that goes inside .img file, use the type_mask to see file type
static const uint64_t is_fcloth_mask    = 0x0000000800000000;   // Is a item that SHOULD FOR SURE be a clothing item (placed inside a folder named player.img or smth)
static const uint64_t is_pedifp_mask    = 0x0000001000000000;   // Is ped.ifp file
static const uint64_t type_mask         = 0x00000F0000000000;   // Mask for the file type (see ResType enum)
static const uint32_t type_mask_shr     = 40;                   // How much to shift right to get the ResType (from type_mask)

// Finds the file type (as in the type_mask) from an file behaviour
inline ResType GetResType(const modloader::file& file)
{
    return ResType((file.behaviour & type_mask) >> type_mask_shr);
}

// Sets the file type (as in the type_mask) in an file behaviour
inline void SetResType(modloader::file& file, ResType type)
{
    file.behaviour = file.hash | is_item_mask | ((uint64_t)(type) << type_mask_shr);
}

// Finds the file type from an extension
inline ResType GetResTypeFromExtension(const char* ext)
{
    if(ext && ext[0])   // Make sure we have ext, we're allowed to send a null string
    {
        if(!_stricmp(ext, "dff")) return ResType::Model;
        if(!_stricmp(ext, "txd")) return ResType::TexDictionary;
        if(!_stricmp(ext, "col")) return ResType::Collision; // although not streamed on III
        if(!_stricmp(ext, "ifp")) return ResType::AnimFile;  // although not streamed on III
        if(gvm.IsVC() || gvm.IsSA())
        {
            if(gvm.IsSA())
            {
                if(!_stricmp(ext, "scm")) return ResType::StreamedScript;
                if(!_stricmp(ext, "ipl")) return ResType::StreamedScene;
                if(!_stricmp(ext, "dat")) return ResType::Nodes;
                if(!_stricmp(ext, "rrr")) return ResType::VehRecording;
            }
        }
    }
    return ResType::None;
}

// Converts the specified size in bytes to cd stream blocks
inline uint32_t GetSizeInBlocks(uint32_t size)
{
    auto div = std::div(size, 2048);
    return (div.quot + (div.rem? 1 : 0));
}

// Converts the specified size in bytes to cd stream blocks
inline uint32_t GetSizeInBlocks(uint64_t size)
{
    return GetSizeInBlocks((uint32_t)size);
}

// Fills the specified directory entry
inline void FillDirectoryEntry(DirectoryInfo& entry, const char* filename, uint32_t offset, uint64_t size_in_bytes)
{
    strncpy(entry.m_szFileName, filename, 23);
    entry.m_szFileName[23]  = 0;
    entry.m_dwFileOffset    = offset;
    entry.m_usCompressedSize__   = 0;
    entry.m_usSize          = GetSizeInBlocks(size_in_bytes);
}





/*
 *  CAbstractStreaming
 *      Abstraction around the game's streaming->
 *      This system is capable of loading files from disk and refreshing them.
 */
class CAbstractStreaming
{
    public: // Friends
        template<class T> friend class Refresher;
        friend int __stdcall CdStreamThread();

    private:
        bool bHasInitializedStreaming   = false;    // Has the streaming initialized?
        bool bIsUpdating                = false;    // Is updating the streaming (for a future refresh)?
        CRITICAL_SECTION cs;                        // This must be used together with imported files list for thread-safety
        std::string fbuffer;                        // File buffer to avoid a dynamic allocation everytime we open a model
        std::list<const modloader::file*> imgFiles; // List of img files imported with Mod Loader

    public:
        // Basic types
        using id_t      = uint32_t;     // should have sizeof(int) to allow -1 comparision
        using hash_t    = uint32_t;

        //
        using NonStreamedInfo_t = std::pair<const modloader::file*, NonStreamedType>;

        // Temporary cd directory for basic information extracting, used during initialization
        using TempCdDir_t = std::list<std::pair<int, std::deque<DirectoryInfo>>>;
        
        // This one stores the extracted informations from the temp cd dir and more.
        using CdDir_t     = std::map<id_t, struct CdDirectoryItem>;

        // Detailed information about a cd directory item
        struct CdDirectoryItem
        {
            uint32_t hash;      // File name hash
            uint32_t offset;    // File offset in blocks
            uint16_t blocks;    // File size in blocks
            uint8_t  img;       // IMG file id this item is bound to
            ResType type;       // File type

            CdDirectoryItem() = default;

            CdDirectoryItem(const char* filename, const DirectoryInfo& entry, int img_id)
            {
                auto* ext = strrchr(filename, '.');
                if(ext) ++ext;

                // Has separate filename because entry filename is incomplete (without extension)
                this->hash   = mhash(filename);
                this->offset = entry.m_dwFileOffset;
                this->blocks = gvm.IsSA() && entry.m_usCompressedSize__? entry.m_usCompressedSize__ : entry.m_usSize;
                this->img    = (uint8_t)(img_id);
                this->type   = GetResTypeFromExtension(ext);
            }
        };

        // Imported model information
        struct ModelInfo
        {
            const modloader::file* file = nullptr;      // The file this model is bound to
            bool isFallingBack          = false;        // If file isn't on disk, we're falling back to the original entry
            bool isSpecialModel         = false;        // Is special model (as in CStreaming::RequestSpecialModel)
            bool isClothes              = false;        // Is clothing item (SA-only, player clothing item)
        };

        // Abstract file handle (on-disk-file)
        struct AbctFileHandle
        {
            protected:
                friend class CAbstractStreaming;
                friend int __stdcall CdStreamThread();

                HANDLE     handle;                      // OS file handle
                ModelInfo& info;                        // Model information
                id_t       index;                       // Model index this is related to

            public:
                AbctFileHandle(HANDLE handle, ModelInfo& info, int idx)
                    : handle(handle), info(info), index(idx)
                {}
            
                // Quickly check if this handle is the same as another
                bool operator==(const AbctFileHandle& b) const
                {
                    return (this == &b);    // Dirty, but that's how we should go here
                }
            
                // Check if this handle has the specified OS file handle bound to it
                bool operator==(HANDLE h) const
                {
                    return (this->handle == h);
                }
        };


        // Information maps, those maps are here to help the abstract streaming with some main streaming information
        std::map<hash_t, id_t>      indices;            // Association between all game resources name hashes and indices
        std::map<hash_t, uint32_t>  clothes_map;        // Association between all clothes name hashes and it's item offset
        std::map<id_t, struct CdDirectoryItem> cd_dir;  // Important information about all resource files in the game
        std::map<id_t, id_t> prev_on_cd;                // Original InfoForModel next on cd information, <next, prev>

        // Custom files
        std::map<std::string, const modloader::file*>   raw_models; // Models installed before the streaming initialization ---- (sorted by name!!)
        std::map<id_t, ModelInfo>                       imports;    // Imported abstract models
        std::map<hash_t, const modloader::file*>        special;    // Abstract special models ---- (imported by me) (hashed filename has no extension)
        std::map<uint32_t, const modloader::file*>      clothes;    // Imported abstract clothes --- (key is offset)
        std::map<hash_t, NonStreamedInfo_t>             non_stream; // Non streamed resources (requested by default.dat/gta.dat) (.second.first may be nullptr)

        // Information for refreshing
        std::map<hash_t, const modloader::file*>        to_import;  // Files to be imported by the refresher --- (null pointer on mapped piece means uninstall)
        bool to_rebuild_player = false;                             // Should rebuild the player?
        uint32_t newcloth_blocks = 0;                               // On the player rebuilding process, realloc the streaming buffer if necessary because of this clothing item size (in blocks)

        // Abstract streaming
        std::list<AbctFileHandle> stm_files;                        // List of abstract files currently open for reading

        // Dynamic cross-game structures caching
        size_t sizeof_CStreamingInfo;                               // The size of the CStreamingInfo structure

    public:
        CAbstractStreaming();
        ~CAbstractStreaming();
        void Patch();
        void DataPatch();
        void InitRefreshInterface();
        void ShutRefreshInterface();

        // Some analogues to the game CStreaming (may perform some additional work)
        void RequestModel(id_t id, uint32_t flags);
        void RemoveModel(id_t id);
        void FlushChannels();
        void LoadAllRequestedModels();
        void RemoveUnusedResources();
        bool IsModelOnStreaming(id_t id);
        bool IsModelAvailable(id_t id);
        CStreamingInfo* InfoForModel(id_t id);

        // Checks if file is clothing item
        bool IsClothes(const modloader::file* file);

        // Performs installation process in the file, refreshing model and so on
        bool InstallFile(const modloader::file& file);
        bool ReinstallFile(const modloader::file& file);
        bool UninstallFile(const modloader::file& file);
        void Update();

        // Install img file to override
        bool AddImgFile(const modloader::file& f)
        {
            if(!bHasInitializedStreaming) imgFiles.emplace_back(&f);
            return !bHasInitializedStreaming;
        }

        // Uninstall previosly installed img file
        bool RemImgFile(const modloader::file& f)
        {
            if(!bHasInitializedStreaming) imgFiles.remove(&f);
            return !bHasInitializedStreaming;
        }

        // Returns a hash for a model file name
        static hash_t mhash(const char* filename)
        {
            return modloader::hash(filename, ::tolower);
        }

        // Misc (used by hooks)
        const char* GetCdStreamPath(const char* filepath);
        bool DoesModelNeedsFallback(id_t id);
        static HANDLE TryOpenAbstractHandle(int index, HANDLE hFile);

    private:
        // Fetching and loading of cd directories
        void FetchCdDirectory(TempCdDir_t& cd_dir, const char*& filename, int id);
        void FetchCdDirectories(TempCdDir_t& cd_dir, std::function<void()> LoadCdDirectories);
        void LoadCdDirectories(TempCdDir_t& cd_dir);
        void LoadAbstractCdDirectory(ref_list<const modloader::file*> files);
        void BuildPrevOnCdMap();

        // Refreshing
        void ProcessRefreshes();

        // Abstract streaming file managing
        AbctFileHandle* OpenModel(ModelInfo& file, int index);
        void CloseModel(AbctFileHandle* file);
        
        // Registering
        void RegisterModelIndex(const char* filename, id_t index);
        void RegisterStockEntry(const char* filename, DirectoryInfo& entry, id_t index, int img_id);
        void RegisterClothingItem(const char* filename, int index);
       
        // Importing of abstract files
        void ImportModels(ref_list<const modloader::file*> files);
        void UnimportModel(id_t index);
        bool ImportCloth(const modloader::file* file);
        bool UnimportCloth(const modloader::file* file);

        // Imports to the main game streaming (this should be used only in specific cases)
        void QuickImport(id_t index, const modloader::file* file,  bool isSpecialModel = false, bool isCloth = false);
        void QuickUnimport(id_t index);

    private: // Helpers to change info for model structure

        // Sets the info for model structure for the resource id
        void SetInfoForModel(id_t id, uint32_t offset, uint32_t blocks)
        {
            auto& model = *this->InfoForModel(id);
            model.SetStreamData(offset, blocks);
            model.SetNextOnCd(-1);
            ClearNextOnCdPointingTo(id);
            if(!bHasInitializedStreaming) model.ClearStreamFlags();
        }

        // Restores the info for model structure for the specified resource id
        void RestoreInfoForModel(id_t id)
        {
            auto it = this->cd_dir.find(id);
            if(it != cd_dir.end())  // Has a stock directory entry?
            {
                this->SetInfoForModel(id, it->second.offset, it->second.blocks);
                RestoreNextOnCdPointingTo(id);
            }
            else
                this->SetInfoForModel(id, 0, 0);
        }

        // Remove the next on cd pointing to the specified model....
        void ClearNextOnCdPointingTo(id_t id)
        {
            auto prev_pair = prev_on_cd.find(id);
            if(prev_pair != prev_on_cd.end()) InfoForModel(prev_pair->second)->SetNextOnCd(-1);
        }

        // Restore the next on cd pointing to the specified model....
        void RestoreNextOnCdPointingTo(id_t id)
        {
            auto prev_pair = prev_on_cd.find(id);
            if(prev_pair != prev_on_cd.end()) InfoForModel(prev_pair->second)->SetNextOnCd(id);
        }

    public://protected:

        std::string TryLoadNonStreamedResource(std::string filepath, NonStreamedType type);

        void RemoveNonStreamedFromRawModels();

        // Checks if a file is not part of the streaming engine.
        bool IsNonStreamed(const modloader::file* file)
        {
            if(gvm.IsIII())
            {
                switch(GetResType(*file))
                {
                    // GetResTypeFromExtension accepts giving those types on III, but they're not
                    // streamed in this game, only static loaded and such.
                    case ResType::Collision:
                    case ResType::AnimFile:
                        return true;
                }
            }

            return (this->non_stream.count(file->hash) > 0);
        }

        // Makes a previosly imported file use the stock resource info instead the one on disk.
        // The force parameter may only be true from an call before info setup in CStreaming::RequestModelStream, otherwise it's undefined behaviour.
        // Notice: This doesn't remove the file from the imported resource list and needs the resource to not be on the streaming
        // The user cases for this function are recuperation from failures. Don't do anything fancy here, critical call! Use with care!
        bool FallbackResource(id_t index, bool force = false)
        {
            auto imp = this->imports.find(index);
            if(imp != imports.end() && imp->second.isFallingBack == false)
            {
                if(force == false && this->IsModelOnStreaming(index))
                {
                    plugin_ptr->Log("Failed to fallback resource id %d because it's loaded", index);
                    return false;
                }
                else
                {
                    plugin_ptr->Log("Falling back resource id %d to stock resource", index);
                    this->RestoreInfoForModel(imp->first);
                    imp->second.isFallingBack = true;
                    return true;
                }
            }
            return false;
        }


    private: // Resources helper methods

        template<class T>
        friend void PerformStandardRefresh(CAbstractStreaming& s);

        id_t InfoForModelIndex(const CStreamingInfo& model);

        // Finds resource index from it's filename hash, returns -1 if none
        id_t FindModelFromHash(hash_t hash)
        {
            auto it = this->indices.find(hash);
            return it != indices.end()? it->second : -1;
        }

        // Find the resource filename hash from it's id, returns a pair where the second element tells whether the operation was successfull
        std::pair<hash_t, bool> FindHashFromModel(id_t id)
        {
            auto it = this->cd_dir.find(id);
            return it != cd_dir.end()? std::pair<hash_t, bool>(it->second.hash, true) : std::pair<hash_t, bool>(0, false);
        }

        // Gets the resource type of a resource id
        ResType GetIdType(id_t id)
        {
            auto it = this->cd_dir.find(id);
            return it != cd_dir.end()? it->second.type : ResType::None;
        }


    private: // Abstract clothes managing
        void BuildClothesMap();
        void FixClothesDirectory();
        DirectoryInfo* FindClothEntry(hash_t hash);
        
        // Sparse entries in the clothing directory are entries that DO NOT EXIST but they actually represent a abstract file for us
        // When I say they do not exist they do not exist neither in player.img!!!!
        std::vector<DirectoryInfo> sparse_dir_entries;  // Cached sparse entries
        uint32_t cloth_dir_sparse_start;                // The starting m_dwFileOffset for sparse entries
        uint32_t cloth_dir_sparse_curr;                 // Current m_dwFileOffset returned by TakeClothSparseOffset

        // Taking and checking for sparse entries
        uint32_t TakeClothSparseOffset()            { return ++cloth_dir_sparse_curr; }
        bool IsClothSparseOffset(uint32_t offset)   { return offset >= cloth_dir_sparse_start; }


    private: // Updating imports (during install/uninstall) requieres to be inside this block

        // Call this during the Install/Uninstall to begin or continue an update
        void BeginUpdate()
        {
            if(this->IsUpdating() == false)
            {
                this->FlushChannels();      // Bus must be empty before we can install/uninstall stuff
                this->bIsUpdating = true;
            }
        }

        // Call this after the refresh to finish the update
        void EndUpdate()
        {
            this->bIsUpdating = false;
        }

        // Checks if an update is happening
        bool IsUpdating()
        {
            return this->bIsUpdating;
        }

    protected: // Helpers

        // Helper to avoid duplicate of resources
        template<class FuncT, class ...Args>
        id_t FindOrRegisterResource(const char* name, const char* extension, uint32_t base_index, FuncT RegisterResource, Args&&... args)
        {
            char buf[64]; sprintf(buf, "%s.%s", name, extension);
            auto index = this->FindModelFromHash(mhash(buf));
            if(index != -1) return index - base_index;
            return RegisterResource(std::forward<Args>(args)...);
        }

        // Updates/Reallocates the streaming buffer on destruction according to the new streaming items added during the object lifetime
        // Just construct, call AddItem with the sizes in blocks and on destruction it will do the hard work.
        struct StreamingBufferUpdater
        {
            private:
                uint32_t realStreamingBufferSize;   // Current actual streaming buffer size
                uint32_t tempStreamingBufSize;      // Temporary (we'll be updating this var) streaming buffer size

            public:
                StreamingBufferUpdater();
                ~StreamingBufferUpdater();

                // Tell the streaming buffer updater about a new streaming item size
                void AddItem(uint32_t sizeInBlocks)
                {
                    if(sizeInBlocks > tempStreamingBufSize)
                        tempStreamingBufSize = sizeInBlocks;
                }
        };

};


extern CAbstractStreaming* streaming;
extern "C" int iNextModelBeingLoaded;
extern "C" int iModelBeingLoaded;
extern "C" struct CStreamingInfo* ms_aInfoForModel;
extern "C" struct CDirectory* clothesDirectory;;
extern "C" DWORD *pStreamCreateFlags;
extern "C" void** pStreamingBuffer;
extern "C" uint32_t* streamingBufferSize;
extern "C" void(*LoadCdDirectory2)(const char*, int);

#endif
