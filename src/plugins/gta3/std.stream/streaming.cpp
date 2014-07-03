/* 
 * std-img -- Standard IMG Loader Plugin for San Andreas Mod Loader
 * Copyright (C) 2013-2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 * Abstract streaming for files on disk
 * Why we call it "abstract"? I don't know, that just came to my mind.
 *
 */
#include <windows.h>
#include "CdStreamInfo.h"
#include "CStreamingInfo.h"
#include "CDirectory.h"
#include "streaming.hpp"
#include "CImgDescriptor.h"
#include <modloader/util/injector.hpp>
#include <modloader/util/hash.hpp>
#include <modloader/util/path.hpp>
#include <modloader/util/modloader.hpp>
using namespace modloader;

// TODO avoid CdStream optimization of uiUnknown1 / lastposn / etc
// TODO fix non find (only add) on LoadCdDirectory (GAME)
// TODO special model
// TODO remove is hoodlum
// TODO cd stream falling back

void TestChangeModel();

CAbstractStreaming streaming;

// Hooks and other util stuff
extern "C"
{
    int* streamingBufferSize = memory_pointer(0x8E4CA8).get();
    auto LoadCdDirectory2 = ReadRelativeOffset(0x5B8310 + 1).get<void(CImgDescriptor*, int)>();

    // Some vars connected within the game
    CStreamingInfo* ms_aInfoForModel                = memory_pointer(0x8E4CC0).get();
    DWORD *pStreamCreateFlags= memory_pointer(0x8E3FE0).get();
    CImgDescriptor* images= ReadMemory<CImgDescriptor*>(0x407613 + 1, true);
    int *CVehicleRecording__NumPlayBackFiles = ReadMemory<int*>(0x45A001 + 1, true);
    int *CVehicleRecording__StreamingArray   = ReadMemory<int*>(0x459FF0 + 2, true);
    int (__fastcall *FindStreamedScript)(void*, int, const char* name)  = memory_pointer(0x4706F0).get();
    int (__fastcall *CDirectory__CDirectory)(CDirectory*, int, size_t count, CDirectoryEntry*)              = memory_pointer(0x5322F0).get();
    int (__fastcall *CDirectory__ReadDirFile)(CDirectory*, int, const char* file)             = memory_pointer(0x532350).get();
    
    auto CDirectory__FindItem2 = (CDirectoryEntry* (__thiscall*)(CDirectory*, const char*)) memory_pointer(0x532450).get<void>();

    CDirectory* playerImgDirectory                  = ReadMemory<CDirectory*>(0x5A69ED + 1, true);
    CDirectoryEntry* playerImgEntries                    = ReadMemory<CDirectoryEntry*>(0x5A69E3 + 1, true);
    
    // Hooks
    int  iNextModelBeingLoaded = -1;            // Set by HOOK_RegisterNextModelRead, will then be sent to iModelBeingLoaded
    extern void HOOK_RegisterNextModelRead();
    extern void HOOK_NewFile();
    extern void HOOK_SetStreamName();
    extern void HOOK_SetImgDscName();
    
    // Allocates a buffer that lives for the entire life time of the app
    // Used by HOOK_SetImgDscName
    const char* AllocBufferForString(const char* str)
    {
        static std::list<std::string> buffers;
        return buffers.emplace(buffers.end(), str)->c_str();
    }
    
    
    //
    static int iModelBeingLoaded = -1;          // Model currently passing throught CdStreamRead
                                                // CallGetAbstractHandle should take care of it
    
    // Returns the file handle to be used for iModelBeingLoaded
    HANDLE CallGetAbstractHandle(HANDLE hFile)
    {
        if(iModelBeingLoaded == -1) return hFile;
        return streaming.TryOpenAbstractHandle(iModelBeingLoaded, hFile);
    }

    static const char* GetCdStreamPath(const char* filename)
    {
        if(filename)
        {
            // If it's our dummy string, take the new string from the customName pointer
            if(filename[0] == '?' && filename[1] == '\0') return *(char**)(filename + 4);
            return filename;
        }
        // It's custom, ignore name but actually have a valid openable name
        return modloader::szNullFile;
    }
};


/*
 *  PerformDirectoryRead
 *      Reads a CD Directory (abstract or real) using callbacks
 *      @size                   The amount of entries on the CD directory (Ignored on III/VC)
 *      @ReadEntry              Callback to fill the entry, return false on failure (i.e. end of entries)
 *      @RegisterSpecialEntry   Called when a special entry is registered (special models etc, models that aren't attached to a id at all)
 *      @RegisterEntry          Called when a normal entry is registered (entries attached to ids), receives the CStreamingInfo that you'll attach to
                                 and a boolean specifying if the model is already attached (if it is, it's mostly like it shouldn't be attached again).
                                 You should return this boolean, or another if you want a different behaviour (i.e. you want repeated entries to override the previous
        @Run                    Callback to run the directory reader after hooks have been placed
 */
static void PerformDirectoryRead(size_t size,
    std::function<void()> Run,
    std::function<bool(CDirectoryEntry&)> ReadEntry,
    std::function<void(CDirectoryEntry&)> RegisterSpecialEntry  = nullptr,
    std::function<bool(CStreamingInfo&, bool)> RegisterEntry    = nullptr
    )
{
    typedef function_hooker<0x5B6183, void*(const char*, const char*)> nf_hook;
    typedef function_hooker<0x5B61B8, size_t(void*, void*, size_t)> cf_hook;
    typedef function_hooker<0x5B61E1, size_t(void*, void*, size_t)> rf_hook;
    typedef function_hooker_thiscall<0x5B627A, void(CDirectory*, CDirectoryEntry* entry)> sf_hook;
    typedef function_hooker_thiscall<0x5B6449, char(CStreamingInfo*, int*, int*)> gf_hook;

    nf_hook nf; // Open Null File
    cf_hook cf; // Entry Count
    rf_hook rf; // Read Entry
    sf_hook sf; // Special Entry
    gf_hook gf; // Register Entry

    // Open a null but valid file handle, so it can be closed (fclose) etc
    nf = make_function_hook2<nf_hook>([](nf_hook::func_type OpenFile, const char*&, const char*& mode)
    {
        return OpenFile(modloader::szNullFile, mode);
    });

    if(gvm.IsSA())  // Only SA needs such thing as a counter, III/VC uses EOF as end of entries indicator
    {
        cf = make_function_hook2<cf_hook>([&size](cf_hook::func_type, void*&, void*& buf, size_t&)
        {
            *reinterpret_cast<size_t*>(buf) = size;
            return sizeof(size_t);
        });
    }

    // Fills a directory entry
    rf = make_function_hook2<rf_hook>([&ReadEntry](rf_hook::func_type, void*&, void*& buf_, size_t&)
    {
        auto& buf = *reinterpret_cast<CDirectoryEntry*>(buf_);
        if(!ReadEntry(buf))
        {
            // Avoid this getting read by the game
            buf.filename[0] = 'A'; buf.filename[1] = '.'; buf.filename[2] = '\0';
            return size_t(0);
        }
        return sizeof(buf);
    });

    // Registers a special entry
    sf = make_function_hook2<sf_hook>([&RegisterSpecialEntry](sf_hook::func_type CDirectory__AddItem, CDirectory*& dir, CDirectoryEntry*& entry)
    {
        CDirectoryEntry* xentry = nullptr;

        // The standard game doesn't have this behaviour, but it's useful to have....
        // If we already have a special entry, override it. Originally it would be added again without emotion!
        if(xentry = CDirectory__FindItem2(dir, entry->filename))
        {
            memcpy(xentry, entry, sizeof(*xentry));
            entry = xentry;
        }

        if(RegisterSpecialEntry) RegisterSpecialEntry(*entry);  // Tell the callback about this registering
        if(!xentry) CDirectory__AddItem(dir, entry);            // If not overriding an entry, just add it
    });

    // Registers a normal entry
    gf = make_function_hook2<gf_hook>([&RegisterEntry](gf_hook::func_type GetCdPosnAndSize, CStreamingInfo*& model, int*& pOffset, int*& pSize)
    {
        char r = GetCdPosnAndSize(model, pOffset, pSize);
        if(RegisterEntry) r = (char) RegisterEntry(*model, !!r);    // Override function result
        return r;
    });

    // Run the cd directory reader
    Run();
}




/*
 *  Constructs the abstract streaming object 
 */
CAbstractStreaming::CAbstractStreaming()
{
    DWORD SectorsPerCluster, BytesPerSector, NumberOfFreeClusters, TotalNumberOfClusters;

    InitializeCriticalSection(&cs);
    
    /*
     *  Find out if the number of bytes per sector from this disk is 2048 aligned, this allows no-buffering for 2048 bytes aligned files.
     *
     *  FIXME: The game might not work if it is installed in another disk other than the main disk because of the NULL parameter
     *  in GetDiskFreeSpace(). R* did this mistake and I'm doing it again because I'm too lazy to fix R* mistake.
     * 
     */
    if(GetDiskFreeSpaceA(0, &SectorsPerCluster, &BytesPerSector, &NumberOfFreeClusters, &TotalNumberOfClusters)
    && BytesPerSector)
    {
        this->bIs2048AlignedSector  = (2048 % BytesPerSector) == 0;
    }
    else this->bIs2048AlignedSector = false;
}

CAbstractStreaming::~CAbstractStreaming()
{
    DeleteCriticalSection(&cs);
}


/*
 *  CAbstractStreaming::RegisterModelIndex
 *      Registers the existence of an index
 */
void CAbstractStreaming::RegisterModelIndex(const char* filename, id_t index)
{
    if(this->indices.emplace(modloader::hash(filename, ::tolower), index).second == false)
        plugin_ptr->Log("Warning: Model %s appears more than once in abstract streaming", filename);
}

/*
 *  CAbstractStreaming::RegisterStockEntry
 *      Registers the original directory data of an index
 */
void CAbstractStreaming::RegisterStockEntry(const char* filename, CDirectoryEntry& entry, id_t index, int img_id)
{
    if(this->cd_dir.emplace(std::piecewise_construct,
        std::forward_as_tuple(index),
        std::forward_as_tuple(filename, entry, img_id)).second == false)
    {
        // Please note @entry here is incomplete because the game null terminated the string before the extension '.'
        // So let's use @filename
        plugin_ptr->Log("Warning: Stock entry %s appears more than once in abstract streaming", filename);
    }
}




/*
 *  CAbstractStreaming::InstallFile
 *      Installs a model, clothing or any streamable file, refreshing them after the process
 */
bool CAbstractStreaming::InstallFile(const modloader::file& file)
{
    if(!bHasInitializedStreaming)
    {
        // Just push it to this list and it will get loaded when the streaming initializes
        raw_models[file.FileName()] = &file;
        return true;
    }
    else
    {
        if(!IsClothes(file))
        {
            auto id = ImportModel(file);
            if(id != -1) return TellToRefreshModel(id);
        }
        else
        {
            // TODO clothes
        }
        
    }
    return false;
}

/*
 *  CAbstractStreaming::ReinstallFile
 *      Does the same as InstallFile
 */
bool CAbstractStreaming::ReinstallFile(const modloader::file& file)
{
    // Reinstalling works the same as installing
    return InstallFile(file);
}

/*
 *  CAbstractStreaming::UninstallFile
 *      Uninstalls a specific file
 */
bool CAbstractStreaming::UninstallFile(const modloader::file& file)
{
    if(!bHasInitializedStreaming)
    {
        // Streaming hasn't initialized, just remove it from our raw list
        raw_models.erase(file.FileName());
        return true;
    }
    else
    {
        if(!IsClothes(file))
        {
            auto id = RemoveModel(file);
            if(id != -1) return TellToRefreshModel(id);
        }
        else
        {
            // TODO clothes
        }
    }
    return false;
}


uint32_t CAbstractStreaming::ImportModel(const modloader::file& file)
{
    return -1;
}

uint32_t CAbstractStreaming::RemoveModel(const modloader::file& file)
{
    return -1;
}




/*
 *  CAbstractStreaming::TryOpenAbstractHandle
 *      Returns another file from the abstract streaming or returns the received file 
 */
HANDLE CAbstractStreaming::TryOpenAbstractHandle(int index, HANDLE hFile)
{
    CAbstractStreaming::AbctFileHandle* f = nullptr;
    
    // Try to find the object index in the import list
    auto it = streaming.imports.find(index);
    if(it != streaming.imports.end())
    {
        f = streaming.OpenModel(it->second, it->first);
    }
    
    // Returns the file from the abstract streaming if available
    return (f? f->handle : hFile);
}

/*
 *  CAbstractStreaming::OpenModel
 *      Opens a abstract model file handle 
 */
auto CAbstractStreaming::OpenModel(ModelInfo& file, int index) -> AbctFileHandle*
{
    static std::string fbuffer(MAX_PATH, '\0'); // To avoid a dynamic allocation everytime we open a model
    DWORD flags = FILE_FLAG_SEQUENTIAL_SCAN | (*pStreamCreateFlags & FILE_FLAG_OVERLAPPED);
    
    HANDLE hFile = CreateFileA(file.file->FullPath(fbuffer).c_str(), GENERIC_READ, FILE_SHARE_READ, NULL,
                               OPEN_EXISTING, flags, NULL);
    
    if(hFile == INVALID_HANDLE_VALUE)
    {
        plugin_ptr->Log("Failed to open file \"%s\" for abstract streaming; error code: 0x%X",
                       file.file->FileBuffer(), GetLastError());
        return nullptr;
    }
    else
    {
        scoped_lock xlock(this->cs);
        return &(*this->stm_files.emplace(stm_files.end(), hFile, file, index));
    }
}

/*
 *  CAbstractStreaming::CloseModel
 *      Closes a abstract model file handle 
 */
void CAbstractStreaming::CloseModel(AbctFileHandle* file)
{
    scoped_lock xlock(this->cs);
    
    // Close the file handle
    CloseHandle(file->handle);
    
    // Remove this file from the open files list
    this->stm_files.remove(*file);
}


/*
 *  Streaming thread
 *      This thread reads pieces from cdimages and in modloader files itself
 */
int __stdcall CdStreamThread()
{
    DWORD nBytesReaden;
    
    // Get reference to the addresses we'll use
    CdStreamInfo& cdinfo = *memory_pointer(0x8E3FEC).get<CdStreamInfo>();
    
    // Loop in search of things to load in the queue
    while(true)
    {
        int i = -1;
        CdStream* cd;
        bool bIsAbstract = false;
        CAbstractStreaming::AbctFileHandle* sfile = nullptr;
        
        // Wait until there's something to be loaded...
        WaitForSingleObject(cdinfo.semaphore, -1);
        
        // Take the stream index from the queue
        i = GetFirstInQueue(&cdinfo.queue);
        if(i == -1) continue;
        
        cd = &cdinfo.pStreams[i];
        cd->bInUse = true;          // Mark the stream as under work
        if(cd->status == 0)
        {
            // Setup vars
            size_t bsize  = cd->nSectorsToRead;
            size_t offset = cd->nSectorOffset  << 11;       // translate 2KiB based offset to actual offset
            size_t size   = bsize << 11;                    // translate 2KiB based size to actual size
            HANDLE hFile  = (HANDLE) cd->hFile;
            bool bResult  = false;
            const char* filename = nullptr; int index = -1; // When abstract those fields are valid
            
            // Try to find abstract file from hFile
            if(true)
            {
                scoped_lock xlock(streaming.cs);
                auto it = std::find(streaming.stm_files.begin(), streaming.stm_files.end(), hFile);
                if(it != streaming.stm_files.end())
                {
                    bIsAbstract = true;
                    
                    // Setup vars based on abstract file
                    sfile  = &(*it);
                    offset = 0;
                    size   = (size_t) sfile->info.file->Size();
                    bsize  = GetSizeInBlocks(size);
                    index  = sfile->index;
                    filename = sfile->info.file->FileBuffer();
                }
            }
            
            
            // Setup overlapped structure
            cd->overlapped.Offset     = offset;
            cd->overlapped.OffsetHigh = 0;
            
            // Read the stream
            if(ReadFile(hFile, cd->lpBuffer, size, &nBytesReaden, &cd->overlapped))
            {
                bResult = true;
            }
            else
            {
                if(GetLastError() == ERROR_IO_PENDING)
                {
                    // This happens when the stream was open for async operations, let's wait until everything has been read
                    bResult = GetOverlappedResult(hFile, &cd->overlapped, &nBytesReaden, true) != 0;
                }
            }
            
            // There's some real problem if we can't load a abstract model
            if(bIsAbstract && !bResult)
            {
                plugin_ptr->Log("Warning: Failed to load abstract model file %s; error code: 0x%X", filename, GetLastError());
            }
            
            // Set the cdstream status, 0 for "okay" and 254 for "failed to read"
            cd->status = bResult? 0 : 254;
        }
        
        // Remove from the queue what we just readed
        RemoveFirstInQueue(&cdinfo.queue);
        
        // Cleanup
        if(bIsAbstract) streaming.CloseModel(sfile);
        cd->nSectorsToRead = 0;
        if(cd->bLocked) ReleaseSemaphore(cd->semaphore, 1, 0);
        cd->bInUse = false;
    }
    
    return 0;
}







void CAbstractStreaming::GenericReadEntry(CDirectoryEntry& entry, const modloader::file* file)
{
    strncpy(entry.filename, file->FileName(), sizeof(entry.filename));
    entry.fileOffset = 0;
    entry.sizePriority2 = GetSizeInBlocks(file->Size());
    entry.sizePriority1 = 0;
}

bool CAbstractStreaming::GenericRegisterEntry(CStreamingInfo& model, bool hasModel, const modloader::file* file)
{
    auto index = &model - ms_aInfoForModel;

    if(!hasModel) RegisterModelIndex(file->FileName(), index);

    ModelInfo a;
    a.file = file;
    imports.emplace(index, a);

    model.iBlockOffset = 0;
    model.iBlockCount = GetSizeInBlocks(file->Size());
    model.uiUnknown1 = -1;                    // TODO find the one pointing to me and do -1 on it
    model.uiUnknown2_ld = 0;                  // TODO what to do with this one?? It's set during gameplay
    model.ucImgId = AbstractImgId;
    return true;
}





/*
 *  CAbstractStreaming::FetchCdDirectories
 *      Fetches (but do not load) the cd directories into @cd_dir
 */
void CAbstractStreaming::FetchCdDirectories(TempCdDir_t& cd_dir, void(*LoadCdDirectories)())
{
    using namespace std::placeholders;
    typedef function_hooker<0x5B8310, void(CImgDescriptor*, int)> fetchcd_hook;

    auto fetcher = make_function_hook2<fetchcd_hook>(std::bind(&CAbstractStreaming::FetchCdDirectory, this, std::ref(cd_dir), _2, _3));
    return LoadCdDirectories();
}


/*
 *  CAbstractStreaming::FetchCdDirectory
 *      Fetches (but do not load) the cd directory file at @descriptor=>name into @cd_dir
 */
void CAbstractStreaming::FetchCdDirectory(TempCdDir_t& cd_dir, CImgDescriptor*& descriptor, int id)
{
    auto filename = GetCdStreamPath(descriptor->name);

    if(FILE* f = fopen(filename, "rb"))
    {
        uint32_t count = -1;
        CDirectoryEntry entry;      // Works for III/VC and SA!!!!

        auto& deque = cd_dir.emplace(cd_dir.end(), std::piecewise_construct, 
                                        std::forward_as_tuple(id), std::forward_as_tuple())->second;

        // The header is present only in SA IMG
        if(gvm.IsSA())
        {
            fread(&count, sizeof(count), 1, f); // "VER2"
            fread(&count, sizeof(count), 1, f); // actual file count
        }

        // Read entry by entry and push to @cd_dir deque
        while(count-- && fread(&entry, sizeof(entry), 1, f))
        {
            if(entry.sizePriority1 != 0)
            {
                entry.sizePriority2 = entry.sizePriority1;
                entry.sizePriority1 = 0;
            }

            deque.emplace_back(entry);
        }

        fclose(f);
    }
    else
        plugin_ptr->Log("Failed to open cd stream \"%s\" for fetching", filename);
}


/*
 *  CAbstractStreaming::LoadCdDirectories
 *      Loads the cd directories previosly stored in @cd_dir
 */
void CAbstractStreaming::LoadCdDirectories(TempCdDir_t& cd_dir, void(*LoadCdDirectory)(CImgDescriptor*, int))
{
    for(auto& cd : cd_dir)
    {
        auto id = cd.first;                 // img id
        auto it = cd.second.begin();        // iterator on the cd
        auto curr = it;
        CDirectoryEntry* entry = nullptr;
        
        // Fills the entry buffer with the current iterating entry (also sets ^entry with the current entry)
        auto ReadEntry = [&](CDirectoryEntry& buf)
        {
            if(it != cd.second.end())
            {
                // There's entries, advance the iterator, fill buffer, and setup entry
                curr = it++;
                buf = *curr;
                entry = &buf;
                return true;
            }
            else
            {
                // No more entries
                entry = nullptr;
                return false;
            }
        };

        // Register special entries
        auto RegisterSpecialEntry = [](CDirectoryEntry& entry)
        {
            // TODO
        };

        // Register standard entries
        auto RegisterEntry = [&](CStreamingInfo& model, bool hadModel)
        {
            // If it's going to register the model (newly!), let's register on abstract streaming too
            if(!hadModel && entry)  
            {
                auto index = &model - ms_aInfoForModel;
                RegisterModelIndex(curr->filename, index);
                RegisterStockEntry(curr->filename, *entry, index, id);
            }
            return hadModel;
        };

        // Read this cd directory using the above callbacks
        PerformDirectoryRead(cd.second.size(), std::bind(LoadCdDirectory, nullptr, id), ReadEntry, RegisterSpecialEntry, RegisterEntry);
    }
}


/*
 *  CAbstractStreaming::LoadCustomCdDirectory
 *      Loads a abstract (fake) custom cd directories related to our disk files @files
 */
void CAbstractStreaming::LoadCustomCdDirectory(ref_list<const modloader::file*> files, void(*LoadCdDirectory)(CImgDescriptor*, int))
{
    // TODO RESIZE STREAMBUF AFTER STREAM INIT WHEN IT COMES

    // Setup iterators
    auto it = files.begin();
    auto curr = it;

    // Fill entry buffer and advance iterator
    auto ReadEntry = [&](CDirectoryEntry& entry)
    {
        if(it != files.end())
        {
            GenericReadEntry(entry, it->get());
            curr = it++;
            return true;
        }
        return false;
    };

    // Be aware of existence of special entries here
    auto RegisterSpecialEntry = [](CDirectoryEntry& entry)
    {
        // TODO
    };

    // Be aware of existence of standard entries here
    auto RegisterEntry = [&](CStreamingInfo& model, bool hasModel)
    {
        return GenericRegisterEntry(model, hasModel, curr->get());
    };

    // Read the custom cd directory
    PerformDirectoryRead(files.size(), std::bind(LoadCdDirectory, nullptr, AbstractImgId), ReadEntry, RegisterSpecialEntry, RegisterEntry);
}














void CAbstractStreaming::Patch()
{
    typedef function_hooker<0x5B8E1B, void()> sinit_hook;
    typedef function_hooker<0x40CF34, int(int, void*, int, int)> cdread_hook;
    bool isHoodlum = injector::address_manager::singleton().IsHoodlum(); // TODO REMOVE, PUT ON ADDR TRANSLATOR

    // Initialise the streaming
    make_static_hook<sinit_hook>([this](sinit_hook::func_type LoadCdDirectory1)
    {
        TempCdDir_t cd_dir;
        this->FetchCdDirectories(cd_dir, LoadCdDirectory1);                     // Fetch...
        this->LoadCdDirectories(cd_dir, LoadCdDirectory2);                      // ...and load
        this->LoadCustomCdDirectory(refs_mapped(raw_models), LoadCdDirectory2); // Load custom

        // Mark streaming as initialized
        this->bHasInitializedStreaming = true;
        this->raw_models.clear();
    });


#if 1
    TestChangeModel();
#endif

    // We need to know the model index that will pass throught CallGetAbstractHandle
    make_static_hook<cdread_hook>([](cdread_hook::func_type CdStreamRead, int& streamNum, void*& buf, int& sectorOffset, int& sectorCount)
    {
        iModelBeingLoaded = iNextModelBeingLoaded;
        auto result = CdStreamRead(streamNum, buf, sectorOffset, sectorCount);
        iModelBeingLoaded = iNextModelBeingLoaded = -1;
        return result;
    });


    // Making our our code for the stream thread would make things so much better
    MakeJMP(0x406560, raw_ptr(CdStreamThread));
    
    // We need to know the next model to be read before the CdStreamRead call happens
    MakeCALL(0x40CCA6, raw_ptr(HOOK_RegisterNextModelRead));
    MakeNOP(0x40CCA6 + 5, 2);
    
    // We need to return a new hFile if the file is on disk
    MakeCALL(!isHoodlum? 0x406A5B : 0x0156C2FB, raw_ptr(HOOK_NewFile));
    MakeNOP((!isHoodlum? 0x406A5B : 0x0156C2FB)+5, 1);


    /*
     *  We need to hook some game code where imgDescriptor[x].name and SteamNames[x][y] is set because they have a limit in filename size,
     *  and we do not want that limit.
     * 
     *  This is a real dirty solution, but well...
     *  
     *  How does it work?
     *      First of all, we hook the StreamNames string copying to copy our dummy string "?\0"
     *                    this is simple and not dirty, the game never accesses this string again,
     *                    only to check if there's a string there (and if there is that means the stream is open)
     *  
     *      Then comes the dirty trick:
     *          We need to hook the img descriptors (CImgDescriptor array) string copying too,
     *          but we need to do more than that, because this string is still used in CStreaming::LoadCdDirectory
     *          to open the file and read the header.
     * 
     *          So what we did? The first field in CImgDescriptor is a char[40] array to store the name, so, we turned this field into an:
     *              union {
     *                  char name[40];
     *                  
     *                  struct {
     *                      char dummy[2];      // Will container the dummy string "?\0" (0x003F)
     *                      char pad[2];        // Padding to 4 bytes boundary
     *                      char* customName;   // Pointer to a static buffer containing the new file name
     *                  };
     *              };
     * 
     *          Then we hook CStreaming::LoadCdDirectory to give the pointer customName instead of &name to CFileMgr::Open
     *          Very dirty, isn't it?
     */
    if(true)
    {
        // WARNING: DO NOT USE FUNCTION_HOOKER HERE IN THIS CONTEXT, OTHERWISE IT WILL BREAK MANY OF THE SCOPED HOOKS ABOVE!
        static void*(*OpenFile)(const char*, const char*);
        static auto OpenFileHook = [](const char* filename, const char* mode)
        {
            return OpenFile(GetCdStreamPath(filename), mode);
        };

        OpenFile = MakeCALL(0x5B6183, raw_ptr((decltype(OpenFile))(OpenFileHook))).get();

        size_t nopcount = injector::address_manager::singleton().IsSteam()? 0xC : 0xA;
        
        MakeNOP(!isHoodlum? 0x406886 : 0x01564B90, nopcount);
        MakeCALL(!isHoodlum? 0x406886 : 0x01564B90, raw_ptr(HOOK_SetStreamName));
        
        MakeNOP(!isHoodlum? 0x407642 : 0x01567BC2, nopcount);
        MakeCALL(!isHoodlum? 0x407642 : 0x01567BC2, raw_ptr(HOOK_SetImgDscName));
    }
}











void TestChangeModel()
{
    typedef function_hooker<0x53ECBD, void(int)> rr_hook;
    make_static_hook<rr_hook>([](rr_hook::func_type Idle, int& i)
    {
        static uintptr_t pVehicle;
        auto xVehicle = ReadMemory<uintptr_t>(0xBA18FC);
        if(xVehicle) pVehicle = xVehicle;

        if(GetAsyncKeyState(VK_NUMPAD8))
        {
            auto RequestModel = (void (*)(int, int)) 0x4087E0;
            auto RemoveModel  = (void (*)(int)) 0x4089A0;
            auto LoadAll      = (void (*)(int)) 0x40EA10;

            static const int id = 542;

            if(pVehicle)
            {
                auto Destroy = (void (__thiscall*)(uintptr_t)) ReadMemory<uintptr_t>(raw_ptr(ReadMemory<uintptr_t>(pVehicle) + 0x20));
                auto Create  = (void (__thiscall*)(uintptr_t, int)) ReadMemory<uintptr_t>(raw_ptr(ReadMemory<uintptr_t>(pVehicle) + 0x14));

                Destroy(pVehicle);

                RemoveModel(id);
                LoadAll(0);
                RequestModel(id, 2);
                LoadAll(0);

                Create(pVehicle, id);
                injector::thiscall<void(uintptr_t)>::call(*(uintptr_t*)((*(uintptr_t*)pVehicle) + 0xC0), pVehicle);
            }
        }
        return Idle(i);
    });
}

