/*
 * Copyright (C) 2013-2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#ifndef STREAMING_HPP
#define	STREAMING_HPP

#include <map>
#include <set>
#include <string>
#include <deque>
#include <list>
#include <modloader/modloader.hpp>
#include <modloader/util/hash.hpp>
#include <modloader/util/injector.hpp>
#include <modloader/util/container.hpp>

struct CImgDescriptor;
struct CStreamingInfo;

#include "CDirectoryEntry.h"
#include "CStreamingInfo.h"



using namespace modloader;



// Streaming file type
enum class FileType : uint8_t // max 4 bits (value 15)
{
    None            = 0,        // Unknown
    Model           = 1,        // DFF (may be Atomic, but whatever)
    TexDictionary   = 2,        // TXD
    Collision       = 3,        // COL
    StreamedScene   = 4,        // IPL
    Nodes           = 5,        // nodes%d.dat
    AnimFile        = 6,        // IFP
    VehRecording    = 7,        // RRR
    StreamedScript  = 8,        // SCM

    Max             = 16        // Max 4 bits
};

// Behaviour maskes
static const uint64_t hash_mask         = 0x00000000FFFFFFFF;   // Mask for the hash on the behaviour
static const uint64_t is_img_file_mask  = 0x0000000100000000;   // Is .img file
static const uint64_t is_img_dir_mask   = 0x0000000200000000;   // Is .img directory
static const uint64_t is_item_mask      = 0x0000000400000000;   // Is item that goes inside .img file, use the type_mask to see file type
static const uint64_t is_pedifp_mask    = 0x0000000800000000;   // Is ped.ifp file
static const uint64_t type_mask         = 0x00000F0000000000;   // Mask for the file type (see FileType enum)
static const uint32_t type_mask_shr     = 40;                   // How much to shift right to get the FileType

// Finds the file type (as in the type_mask) from an file behaviour
inline FileType GetFileType(const modloader::file& file)
{
    return FileType((file.behaviour & type_mask) >> type_mask_shr);
}

// Sets the file type (as in the type_mask) in an file behaviour
inline void SetFileType(modloader::file& file, FileType type)
{
    file.behaviour = file.hash | is_item_mask | ((uint64_t)(type) << type_mask_shr);
}

// Finds the file type from an extension
inline FileType GetFileTypeFromExtension(const char* ext)
{
    if(ext && ext[0])   // Make sure we have ext, we're allowed to send a null string
    {
        if(!_stricmp(ext, "dff"))         return FileType::Model;
        else if(!_stricmp(ext, "txd"))    return FileType::TexDictionary;
        else if(!_stricmp(ext, "col"))    return FileType::Collision;
        else if(!_stricmp(ext, "ifp"))    return FileType::AnimFile;
        else if(gvm.IsSA()) // The following files are present only in GTA SA
        {
            if(!_stricmp(ext, "scm"))         return FileType::StreamedScript;
            else if(!_stricmp(ext, "ipl"))    return FileType::StreamedScene;
            else if(!_stricmp(ext, "dat"))    return FileType::Nodes;
            else if(!_stricmp(ext, "rrr"))    return FileType::VehRecording;
        }
    }
    return FileType::None;
}



static uint32_t GetSizeInBlocks(uint32_t size)
{
    auto div = std::div(size, 2048);
    return (div.quot + (div.rem? 1 : 0));
}

static uint32_t GetSizeInBlocks(uint64_t size)
{
    return GetSizeInBlocks((uint32_t)size);
}

static void FillDirectoryEntry(CDirectoryEntry& entry, const char* filename, uint32_t size)
{
    strncpy(entry.filename, filename, 23);
    entry.filename[23]  = 0;
    entry.fileOffset    = 0;
    entry.sizePriority1 = 0;
    entry.sizePriority2 = GetSizeInBlocks(size);
}

static void FillDirectoryEntry(CDirectoryEntry& entry, const modloader::file& model)
{
    return FillDirectoryEntry(entry, model.FileName(), (uint32_t) model.Size());
}


// TODO RENAME STUFF BASED ON RESOURCE INSTEAD OF MODEL






/*
 *  CAbstractStreaming
 *      Streaming of files on disk
 */
class CAbstractStreaming
{
    public:
        using id_t      = uint32_t;     // should have sizeof(int) to allow -1 comparision
        using hash_t    = uint32_t;

        template<class T> friend class Refresher;
        friend int __stdcall CdStreamThread();

    protected:
        static const uint8_t AbstractImgId = 0;         // our abstract img id, let's use 0 (TODO SOME OTHER)
        bool bHasInitializedStreaming   = false;
        bool bIsUpdating                = false;
        bool bIs2048AlignedSector;                      // not used, maybe in the future
        CRITICAL_SECTION cs;                            // this must be used together with imported files list for thread-safety

        std::string fbuffer;                            // To avoid a dynamic allocation everytime we open a model

    public:
        struct CdDirectoryItem
        {
            uint32_t hash;      // File name hash
            uint32_t offset;    // File offset in blocks
            uint16_t blocks;    // File size in blocks
            uint8_t  img;       // IMG file id this item is bound to
            FileType type;      // File type

            CdDirectoryItem(const char* filename, const CDirectoryEntry& entry, int img_id)
            {
                auto* ext = strrchr(filename, '.');
                if(ext) ++ext;

                // Has separate filename because entry filename is incomplete (without extension)
                this->hash   = modloader::hash(filename, ::tolower);
                this->offset = entry.fileOffset;
                this->blocks = entry.sizePriority1? entry.sizePriority1 : entry.sizePriority2;
                this->img    = (uint8_t)(img_id);
                this->type   = GetFileTypeFromExtension(ext);
            }
        };

        struct ModelInfo
        {
            const modloader::file* file = nullptr;      // The file this model is bound to
            CdDirectoryItem*  original  = nullptr;      // The original directory entry
            bool isFallingBack          = false;        // If file isn't on disk, we're falling back to the original entry
            bool isSpecialModel         = false;
            bool isClothes              = false;
            bool isForcedClothes        = false;
        };

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
            
                bool operator==(const AbctFileHandle& b) const
                {
                    return (this == &b);    // Dirty, but that's how we should go here
                }
            
                bool operator==(HANDLE h) const
                {
                    return (this->handle == h);
                }
        };

        typedef std::list<std::pair<int, std::deque<CDirectoryEntry>>>  TempCdDir_t;   // <IMG_ID, ENTRY>
        
        typedef std::map<id_t, CdDirectoryItem>     CdDir_t;   // <MODEL_ID, ENTRY>
        CdDir_t cd_dir;

        std::map<std::string, const modloader::file*> raw_models;  // <name, file> (Sorted by name)

        // Immutable pair items (once pair is mapped it'll never change)
        // Those are related to the game (to find out more about indices and clothes) not to our custom files
        std::map<hash_t, id_t> indices;           // <hash, indice>
        //std::map<hash_t, uint32_t> clothes_map;   // <hash, offset>

        // Custom files
        std::map<id_t, ModelInfo>     imports;  // <indice, model>
        //std::map<uint32_t, ModelInfo> clothes;  // <offset, model>

        //std::set<id_t> mToRefresh;

    public:
        CAbstractStreaming();
        ~CAbstractStreaming();
        void Patch();

        void FetchCdDirectory(TempCdDir_t& cd_dir, CImgDescriptor*& descriptor, int id);
        void FetchCdDirectories(TempCdDir_t& cd_dir, void(*LoadCdDirectories)());
        void LoadCdDirectories(TempCdDir_t& cd_dir);
        void LoadCustomCdDirectory(ref_list<const modloader::file*> files);

        void ProcessRefreshes();
        void FilterRefreshList();

        CStreamingInfo* InfoForModel(id_t id = 0);

    public:
        
        // Registers the existence of the model @fielname at the specified @index
        void RegisterModelIndex(const char* filename, id_t index);
        // Registers the stock (original) directory entry for the specified model index
        void RegisterStockEntry(const char* filename, CDirectoryEntry& entry, id_t index, int img_id);

        // Performs installation process in the file, refreshing model and so on
        bool InstallFile(const modloader::file& file);
        bool ReinstallFile(const modloader::file& file);
        bool UninstallFile(const modloader::file& file);
        void Update();




        // TODO
        void BuildClothesMap() {}
        bool IsClothes(const modloader::file& file) {return false;}

        void LoadAllRequestedModels();
        void FlushChannels();
        void RemoveUnusedResources();

        static HANDLE TryOpenAbstractHandle(int index, HANDLE hFile);

        
        bool IsModelOnStreaming(id_t id);
        void RemoveModel(id_t id);
        void RequestModel(id_t id, uint32_t flags);
        void ReloadModel(id_t id);

        void MakeSureModelIsOnDisk(id_t id);

    protected:
        void GenericReadEntry(CDirectoryEntry& entry, const modloader::file* file);
        bool GenericRegisterEntry(CStreamingInfo& model, bool hasModel, const modloader::file* file);


        std::list<AbctFileHandle> stm_files;                    // List of custom files currently open for reading
        AbctFileHandle* OpenModel(ModelInfo& file, int index);  // Opens a new file for the model to be read by the streaming
        void CloseModel(AbctFileHandle* file);                  // Closes the previosly open model, was readen

        uint32_t tempStreamingBufSize;
        std::map<hash_t, const modloader::file*> mToImportList; // null file means uninstall

        // Call those only if the streaming bus is empty
        void ImportModels(ref_list<const modloader::file*> files);
        void UnimportModel(id_t index);

        void SetInfoForModel(id_t index, uint32_t offset, uint32_t blocks, uint8_t img_id)
        {
            auto& model = *this->InfoForModel(index);
            model.offset    = offset;
            model.blocks    = blocks;
            model.img_id    = img_id;
            model.nextOnCd  = -1;                    // TODO find the one pointing to me and do -1 on it
            if(!bHasInitializedStreaming) model.flags = 0;
        }

        void RestoreInfoForModel(id_t index)
        {
            auto it = this->cd_dir.find(index);
            if(it != cd_dir.end())
            {
                auto& cd = it->second;
                this->SetInfoForModel(index, cd.offset, cd.blocks, cd.img);
            }
            else
            {
                this->SetInfoForModel(index, 0, 0, 0);
            }
        }






        id_t FindModelFromHash(hash_t hash) // TODO pair
        {
            auto it = this->indices.find(hash);
            return it != indices.end()? it->second : -1;
        }

        std::pair<hash_t, bool> FindHashFromModel(id_t id)
        {
            auto it = this->cd_dir.find(id);
            return it != cd_dir.end()? std::pair<hash_t, bool>(it->second.hash, true) : std::pair<hash_t, bool>(0, false);
        }

        FileType GetIdType(int id)
        {
            auto it = this->cd_dir.find(id);
            return it != cd_dir.end()? it->second.type : FileType::None;
        }

        id_t InfoForModelIndex(const CStreamingInfo& model)
        {
            return (&model - InfoForModel());
        }

        void BeginUpdate()
        {
            if(this->bIsUpdating == false)
            {
                this->FlushChannels();
                this->bIsUpdating = true;
            }
        }

        void EndUpdate()
        {
            this->bIsUpdating = false;
        }

        bool IsUpdating()
        {
            return this->bIsUpdating;
        }
};



extern CAbstractStreaming streaming;
extern "C" int iNextModelBeingLoaded;
extern "C" int iModelBeingLoaded;
extern "C" const char* GetCdStreamPath(const char* filename);



#endif
