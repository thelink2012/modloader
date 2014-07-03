/*
 * Copyright (C) 2013-2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#ifndef STREAMING_HPP
#define	STREAMING_HPP

#include <map>
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



using namespace modloader;



// Streaming file type
enum class FileType : uint8_t // max 4 bits (value 15)
{
    None            = 0,        // Unknown
    Clump           = 1,        // DFF (may be Atomic, but whatever)
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
        if(!_stricmp(ext, "dff"))         return FileType::Clump;
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








/*
 *  CAbstractStreaming
 *      Streaming of files on disk
 */
class CAbstractStreaming
{
    public:
        using id_t      = uint32_t;
        using hash_t    = uint32_t;
        friend int __stdcall CdStreamThread();

    protected:
        static const uint8_t AbstractImgId = 0;         // our abstract img id, let's use 0 (TODO SOME OTHER)
        bool bHasInitializedStreaming   = false;
        bool bIs2048AlignedSector;                      // not used, maybe in the future
        CRITICAL_SECTION cs;                            // this must be used together with imported files list for thread-safety

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
                // Has separate filename because entry filename is incomplete (without extension)
                this->hash   = modloader::hash(filename, ::tolower);
                this->offset = entry.fileOffset;
                this->blocks = entry.sizePriority1? entry.sizePriority1 : entry.sizePriority2;
                this->img    = (uint8_t)(img_id);
                this->type   = GetFileTypeFromExtension(entry.filename);
            }
        };

        struct ModelInfo
        {
            const modloader::file* file = nullptr;      // The file this model is bound to
            CdDirectoryItem*  original  = nullptr;      // The original directory entry
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

    public:
        CAbstractStreaming();
        ~CAbstractStreaming();
        void Patch();

        void FetchCdDirectory(TempCdDir_t& cd_dir, CImgDescriptor*& descriptor, int id);
        void FetchCdDirectories(TempCdDir_t& cd_dir, void(*LoadCdDirectories)());
        void LoadCdDirectories(TempCdDir_t& cd_dir, void(*LoadCdDirectory)(CImgDescriptor*, int));
        void LoadCustomCdDirectory(ref_list<const modloader::file*> files, void(*LoadCdDirectory)(CImgDescriptor*, int));

    public:
        
        // Registers the existence of the model @fielname at the specified @index
        void RegisterModelIndex(const char* filename, id_t index);
        // Registers the stock (original) directory entry for the specified model index
        void RegisterStockEntry(const char* filename, CDirectoryEntry& entry, id_t index, int img_id);

        // Performs installation process in the file, refreshing model and so on
        bool InstallFile(const modloader::file& file);
        bool ReinstallFile(const modloader::file& file);
        bool UninstallFile(const modloader::file& file);

        // Imports or Removes a model file (non-clothing)
        uint32_t ImportModel(const modloader::file& file);
        uint32_t RemoveModel(const modloader::file& file);
        bool TellToRefreshModel(uint32_t id){return true;/**/;}

        // TODO
        void BuildClothesMap() {}
        bool IsClothes(const modloader::file& file) {return false;}

        static HANDLE TryOpenAbstractHandle(int index, HANDLE hFile);

    protected:
        void GenericReadEntry(CDirectoryEntry& entry, const modloader::file* file);
        bool GenericRegisterEntry(CStreamingInfo& model, bool hasModel, const modloader::file* file);


        std::list<AbctFileHandle> stm_files;                    // List of custom files currently open for reading
        AbctFileHandle* OpenModel(ModelInfo& file, int index);  // Opens a new file for the model to be read by the streaming
        void CloseModel(AbctFileHandle* file);                  // Closes the previosly open model, was readen
};
extern CAbstractStreaming streaming;


#endif
