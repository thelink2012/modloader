/* 
 * Standard Streamer Plugin for Mod Loader
 * Copyright (C) 2013-2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under the MIT License, see LICENSE at top level directory.
 * 
 */
#include <stdinc.hpp>
#include "streaming.hpp"
using namespace modloader;

extern "C"
{
    // Assembly hooks at "asm/" folder
    extern void HOOK_RegisterNextModelRead();
    extern void HOOK_NewFile();
    extern void HOOK_RegisterNextModelRead_VC();
    extern void HOOK_NewFile_VC();
    extern void HOOK_FixBikeSuspLines();

    // Next model read registers. It's important to have those two vars! Don't leave only one!
    int  iNextModelBeingLoaded = -1;            // Set by RegisterNextModelRead, will then be sent to iModelBeingLoaded
    int  iModelBeingLoaded = -1;                // Model currently passing throught CdStreamRead

    // Note: Don't perform pointer arithimetic, or indexing, with this pointer! This have a sizeof = 0 (4 actually).
    // Use the InfoForModel(id) and InfoForModelIndex(info) functions!!!
    CStreamingInfo* ms_aInfoForModel;

    DWORD *pStreamCreateFlags;

    void** pStreamingBuffer;

    uint32_t* streamingBufferSize;

    void(*LoadCdDirectory2)(const char*, int);

    CDirectory* clothesDirectory;

    // Returns the file handle to be used for iModelBeingLoaded (called from Assembly)
    HANDLE CallGetAbstractHandle(HANDLE hFile)
    {
        if(iModelBeingLoaded == -1) return hFile;
        return streaming->TryOpenAbstractHandle(iModelBeingLoaded, hFile);
    }

    // Registers the next model to be loaded (called from Assembly)
    void RegisterNextModelRead(int id)
    {
        iNextModelBeingLoaded = id;
        if(streaming->DoesModelNeedsFallback(id))    // <- make sure the resource hasn't been deleted from disk
        {
            plugin_ptr->Log("Resource id %d has been deleted from disk, falling back to stock model.", id);
            streaming->FallbackResource(id, true);   // forceful but safe since we are before info setup in RequestModelStream
        }
    }

    static HANDLE __stdcall CreateFileForCdStream(
        LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
        LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition,
        DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
    {
        LPCSTR lpActualFileName = streaming->GetCdStreamPath(lpFileName);
        plugin_ptr->Log("Opening file for streaming \"%s\"", lpActualFileName);
        return CreateFileA(
            lpActualFileName, dwDesiredAccess, dwShareMode,
            lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
    }

    
    // Reproduces the code @6BF741
    void FixBikeSuspPtr(char* m_pColData)
    {
        *(char*)(m_pColData + 6) = 4;
        char* ptr = injector::cstd<char*(size_t)>::call<0x72F420>(128); // CMemoryMgr::Malloc
        *(char**)(m_pColData + 16) = ptr;
        *(uint32_t*)(ptr + 10) = 0x47C34FFFu;   // a float actually
        *(uint32_t*)(ptr + 8)  = 0x47C34FFFu;   // a float actually
    }
};



/*
 *  Streaming thread
 *      This thread reads pieces from cd images and in mod loader on disk files
 */
int __stdcall CdStreamThread()
{
    HANDLE* pSemaphore = nullptr;
    Queue* pQueue      = nullptr;
    CdStream** ppStreams = nullptr;

    // Get reference to the addresses we'll use.
    if(gvm.IsSA())
    {
        auto& cdinfo = *memory_pointer(0x8E3FEC).get<CdStreamInfoSA>();
        pSemaphore = &cdinfo.semaphore;
        pQueue = &cdinfo.queue;
        ppStreams = &cdinfo.pStreams;
    }
    else if(gvm.IsVC() || gvm.IsIII())
    {
        pSemaphore = memory_pointer(xVc(0x6F76F4)).get<HANDLE>();
        pQueue = memory_pointer(xVc(0x6F7700)).get<Queue>();
        ppStreams = memory_pointer(xVc(0x6F76FC)).get<CdStream*>();
    }

    // Loop in search of things to load in the queue
    while(true)
    {
        bool bIsAbstract = false;
        CAbstractStreaming::AbctFileHandle* sfile = nullptr;
        
        // Wait until there's something to be loaded...
        WaitForSingleObject(*pSemaphore, -1);
        
        // Take the stream index from the queue
        int i = GetFirstInQueue(pQueue);
        if(i == -1) continue;
        
        CdStream* cd = &((*ppStreams)[i]);
        cd->bInUse = true;          // Mark the stream as under work
        if(cd->status == 0)
        {
            // Setup vars
            uint32_t bsize  = cd->nSectorsToRead;
            uint64_t offset = uint64_t(cd->nSectorOffset)  << 11;   // translate 2KiB based offset to actual offset
            uint32_t size   = uint32_t(bsize) << 11;                // translate 2KiB based size to actual size
            HANDLE hFile    = (HANDLE) cd->hFile;
            bool bResult    = false;
            const char* filename = nullptr; int index = -1; // When abstract those fields are valid
            
            // Try to find abstract file from hFile
            if(true)
            {
                scoped_lock xlock(streaming->cs);
                auto it = std::find(streaming->stm_files.begin(), streaming->stm_files.end(), hFile);
                if(it != streaming->stm_files.end())
                {
                    bIsAbstract = true;
                    
                    // Setup vars based on abstract file
                    sfile  = &(*it);
                    offset = 0;
                    size   = (uint32_t) sfile->info.file->size;
                    bsize  = GetSizeInBlocks(size);
                    index  = sfile->index;
                    filename = sfile->info.file->filepath();
                }
            }
            
            
            // Setup overlapped structure
            LARGE_INTEGER offset_li;
            DWORD nBytesReaden;
            offset_li.QuadPart        = offset;
            cd->overlapped.Offset     = offset_li.LowPart;
            cd->overlapped.OffsetHigh = offset_li.HighPart;
            
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
                    // As noted on MSDN [http://msdn.microsoft.com/en-us/library/windows/desktop/aa363858(v=vs.85).aspx], when
                    // you open a stream with FILE_FLAG_NO_BUFFERING  (R* does that with their cd streams) it gives you maximum
                    // performance if you use overlapped I/O
                    bResult = GetOverlappedResult(hFile, &cd->overlapped, &nBytesReaden, true) != 0;
                }
            }
            
            // There's some real problem if we can't load a abstract model
            if(bIsAbstract && !bResult)
                plugin_ptr->Log("Warning: Failed to load abstract model file %s; error code: 0x%X", filename, GetLastError());
            

            // Set the cdstream status, 0 for "okay" and 254 for "failed to read"
            cd->status = bResult? 0 : 254;
        }
        
        // Remove from the queue what we just readed
        RemoveFirstInQueue(pQueue);
        
        // Cleanup
        if(bIsAbstract) streaming->CloseModel(sfile);
        cd->nSectorsToRead = 0;
        if(cd->bLocked) ReleaseSemaphore(cd->semaphore, 1, 0);
        cd->bInUse = false;
    }
    return 0;
}


/*
 *  CAbstractStreaming::GetCdStreamPath
 *      Gets the actual file path for the specified filepath
 *      For example it may find a new gta3.img path for "MODELS/GTA3.IMG"
 */
const char* CAbstractStreaming::GetCdStreamPath(const char* filepath_)
{
    if(filepath_)   // If null it's our abstract cd
    {
        std::string fpath; bool bBreak = false;
        auto filepath = NormalizePath(filepath_);
        auto filename = filepath.substr(GetLastPathComponent(filepath));

        // Check twice, the first time check for path equality, the second time for filename equality
        for(int i = 0; i < 2 && !bBreak; ++i)
        {
            for(auto& file : this->imgFiles)
            {
                auto& cdpath    = (i == 0? filepath : filename);                    // Sent filepath/filename
                fpath           = (i == 0? file->filedir() : file->filename());    // Custom filepath/filename

                // Check if the ending of cdpath is same as fpath
                if(fpath.length() >= cdpath.length() && std::equal(cdpath.rbegin(), cdpath.rend(), fpath.rbegin()))
                {
                    // Yeah, let's override!!!
                    filepath_ = file->filepath();
                    bBreak = true;
                    break;
                }
            }
        }

        return filepath_;
    }

    // It's abstract, ignore name but actually have a valid openable name
    return modloader::szNullFile;
}


/*
 *  CAbstractStreaming::TryOpenAbstractHandle
 *      Returns another file from the abstract streaming or returns the received file 
 */
HANDLE CAbstractStreaming::TryOpenAbstractHandle(int index, HANDLE hFile)
{
    CAbstractStreaming::AbctFileHandle* f = nullptr;
    
    // Try to find the object index in the import list
    auto it = streaming->imports.find(index);
    if(it != streaming->imports.end())
    {
         // Don't use our custom model if we're falling back to the original file because of an error
        if(it->second.isFallingBack == false)
            f = streaming->OpenModel(it->second, it->first);
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
    DWORD flags = FILE_FLAG_SEQUENTIAL_SCAN | (*pStreamCreateFlags & FILE_FLAG_OVERLAPPED);
    
    HANDLE hFile = CreateFileA(file.file->fullpath(fbuffer).c_str(), GENERIC_READ, FILE_SHARE_READ, NULL,
                               OPEN_EXISTING, flags, NULL);
    
    if(hFile == INVALID_HANDLE_VALUE)
    {
        plugin_ptr->Log("Warning: Failed to open file \"%s\" for abstract streaming; error code: 0x%X",
                       file.file->filepath(), GetLastError());
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
 *  CAbstractStreaming::DoesModelNeedsFallback
 *      If the specified model id is under our control, make entirely sure it's present on disk.
 *      If it isn't, return false for fallback.
 */
bool CAbstractStreaming::DoesModelNeedsFallback(id_t index)
{
    auto it = imports.find(index);
    if(it != imports.end())
    {
        auto& m = it->second;
        if(m.isFallingBack == false)    // If already falling back, don't check for file existence again
        {
            // If file isn't on disk we should fall back to the stock model
            if(!IsPath(m.file->fullpath(fbuffer).c_str()))
                return true;
        }
    }
    return false;
}


/*
 *  CAbstractStreaming::BuildPrevOnCdMap
 *      Builds map to find out nextOnCd defaults field for InfoForMOdel
 */
void CAbstractStreaming::BuildPrevOnCdMap()
{
    CStreamingInfo* res;
    prev_on_cd.clear();
    for(id_t i = 0; res = InfoForModel(i); ++i)
    {
        id_t nextOnCd = res->GetNextOnCd();
        if(nextOnCd != -1) prev_on_cd.emplace(nextOnCd, i);
    }
}



/*
 *  CAbstractStreaming::Patch
 *      Patches the default game streaming pipeline to have our awesome hooks.
 */
void CAbstractStreaming::Patch()
{
    using sinit_hook  = function_hooker<0x5B8E1B, void()>;
    
    #ifndef NDEBUG
    LaunchDebugger();
    #endif

    // Pointers
    ms_aInfoForModel    = ReadMemory<CStreamingInfo*>(0x5B8AE8, true);
    pStreamCreateFlags  = memory_pointer(0x8E3FE0).get();
    pStreamingBuffer    = memory_pointer(0x8E4CAC).get<void*>();
    streamingBufferSize = memory_pointer(0x8E4CA8).get<uint32_t>();
    LoadCdDirectory2    = ReadRelativeOffset(0x5B8310 + 1).get<void(const char*, int)>();
    clothesDirectory    = gvm.IsSA()? ReadMemory<CDirectory*>(lazy_ptr<0x5A419B>(), true) : nullptr;

    // See data.cpp
    this->DataPatch();

    // TODO CHECK THIS ms_imageOffsets THING IN VC (@ CStreaming::LoadCdDirectory)

#if 0
    // Initialise the streaming
    make_static_hook<sinit_hook>([this](sinit_hook::func_type LoadCdDirectory1)
    {
        plugin_ptr->Log("Initializing the streaming...");

        // Load standard cd directories.....
        TempCdDir_t tmp_cd_dir;
        this->FetchCdDirectories(tmp_cd_dir, LoadCdDirectory1);
        this->LoadCdDirectories(tmp_cd_dir);
        this->BuildPrevOnCdMap();
        tmp_cd_dir.clear();

        // Do custom setup TODO FIX
        this->BuildClothesMap();                                // Find out clothing hashes and remove clothes from raw_models
        this->LoadAbstractCdDirectory(refs_mapped(raw_models)); // Load abstract directory, our custom files

        // Mark streaming as initialized
        this->bHasInitializedStreaming = true;
        this->raw_models.clear();
    });

    // Standard models
    if(true)
    {
        // Making our our code for the stream thread would make things so much better
        MakeJMP(0x406560, raw_ptr(CdStreamThread));

        // We need to know the next model to be read before the CdStreamRead call happens
        if(gvm.IsSA())
        {
            MakeCALL(0x40CCA6, raw_ptr(HOOK_RegisterNextModelRead));
            MakeNOP(0x40CCA6 + 5, 2);
        }
        else if(gvm.IsVC()) // TODO III
        {
            MakeCALL(0x40CCA6, raw_ptr(HOOK_RegisterNextModelRead_VC));
            MakeCALL(xVc(0x40B738), raw_ptr(HOOK_RegisterNextModelRead_VC));
            MakeNOP(0x40CCA6 + 5, 1);
            MakeNOP(xVc(0x40B738) + 5, 1);
        }

        // We need to return a new hFile if the file is on disk
        if(gvm.IsSA())
        {
            MakeCALL(0x406A5B, raw_ptr(HOOK_NewFile));
            MakeNOP(0x406A5B + 5, 1);
        }
        else if(gvm.IsVC()) // TODO III
        {
            MakeCALL(xVc(0x408521), raw_ptr(HOOK_NewFile_VC));
        }

        if(true)
        {
            using cdread_hook = function_hooker<0x40CF34, int(int, void*, int, int)>;
            using cdread_hook2 = function_hooker<xVc(0x40B76A), int(int, void*, int, int)>;
            using cdread_hook3 = function_hooker<xVc(0x40B780), int(int, void*, int, int)>;

            // We need to know the model index that will pass throught CallGetAbstractHandle
            auto f = [](cdread_hook::func_type CdStreamRead, int& streamNum, void*& buf, int& sectorOffset, int& sectorCount)
            {
                iModelBeingLoaded = iNextModelBeingLoaded;
                auto result = CdStreamRead(streamNum, buf, sectorOffset, sectorCount);
                iModelBeingLoaded = iNextModelBeingLoaded = -1;
                return result;
            };

            make_static_hook<cdread_hook>(f);

            if(gvm.IsVC()) // TODO III
            {
                make_static_hook<cdread_hook2>(f);
                make_static_hook<cdread_hook3>(f);
            }
        }
    }


    // Special models
    if(gvm.IsSA() || gvm.IsVC()) // TODO III
    {
        static DirectoryInfo* pRQSpecialEntry;    // Stores the special entry we're working with
        using findspecial_hook  = function_hooker_thiscall<0x409F76, char(void*, const char*, unsigned int&, unsigned int&)>;
        using rqspecial_hook    = function_hooker<0x409FD9, char(int, int)>;
        
        // Hook call to CDirectory::FindItem (1) to find out which directory entry we're dealing with...
        make_static_hook<findspecial_hook>([this](findspecial_hook::func_type FindItem, void*& dir, const char*& name, unsigned int& offset, unsigned int& blocks)
        {
            pRQSpecialEntry = injector::thiscall<DirectoryInfo*(void*, const char*)>::call<0x532450>(dir, name);  // CDirectory::FindItem
            return FindItem(dir, name, offset, blocks); // (above call and this call are different)
        });

        // Hook call to CStreaming::RequestModel and do special processing if the entry is related to the abstract streaming
        make_static_hook<rqspecial_hook>([this](rqspecial_hook::func_type RequestModel, int& index, int& flags)
        {
            // Before this being called the game built the InfoForModel entry for this index based on the stock entry pRQSpecialEntry
            // Register stock entry, so it can be restored during gameplay if necessary
            {
                DirectoryInfo entry  = *pRQSpecialEntry;                   // Make copy to do proper processing
                uint32_t img_id;

                if(gvm.IsSA())
                {
                    img_id               = pRQSpecialEntry->m_dwFileOffset >> 24;
                    entry.m_dwFileOffset = pRQSpecialEntry->m_dwFileOffset & 0xFFFFFF; 
                }
                else
                {
                    img_id               = 0; // on III/VC there's no img_id, it's based on m_dwFileOffset itself.
                    entry.m_dwFileOffset = pRQSpecialEntry->m_dwFileOffset;
                }

                this->RegisterStockEntry(strcat(entry.m_szFileName, ".dff"), entry, index, img_id);
            }

            // Try to find abstract special entry related to this request....
            // If it's possible, quickly import our entry into this special index
            // otherwise quickly remove our previous entry at this special index if any there
            auto it = this->special.find(mhash(pRQSpecialEntry->m_szFileName));
            if(it != this->special.end())
                this->QuickImport(index, it->second, true);
            else
                this->QuickUnimport(index);

            // Ahhhhh!! finally do the actual request :)
            return RequestModel(index, flags);
        });
    }

    // Clothes
    if(gvm.IsSA())
    {
        using log_hook = function_hooker<0x532341, int(const char*)>;
        using ldclothd_hook = function_hooker_thiscall<0x5A6A01, void(CDirectory*, const char*)>;
        using rqcloth_hook  = function_hooker<0x40A106, char(int, int)>;

        // When reading clothes directory again (it happens everytime player clump is rebuilt, i.e. when muscle stats change or clothes change)
        // make sure the entries are fine with our entries
        make_static_hook<ldclothd_hook>([this](ldclothd_hook::func_type ReadDir, CDirectory*& dir, const char*& filepath)
        {
            ReadDir(dir, filepath);
            this->FixClothesDirectory();
        });

        // Right, this hook produces the actual overriding of the clothing files with ours
        make_static_hook<rqcloth_hook>([this](rqcloth_hook::func_type RequestModel, int& index, int& flags)
        {
            // Check if we have a overrider for this clothing item (based on directory offset)
            DirectoryInfo* entry;
            auto it = this->clothes.find(InfoForModel(index)->GetOffset());
            if(it != clothes.end() && (entry = this->FindClothEntry(it->second->hash)))
            {
                // Yep, we have a overrider, save stock entry and quickly import our abstract model
                this->RegisterStockEntry(it->second->filename(), *entry, index, InfoForModel(index)->GetImgId());
                this->QuickImport(index, it->second, false, true);
            }
            else
            {
                // Hmmm.. nope, quickly unimport anything we had imported on this index before
                this->QuickUnimport(index);
            }

            return RequestModel(index, flags);
        });

        // Make sure to log about "Too many objects without model info structure" into our logging stream
        make_static_hook<log_hook>([](log_hook::func_type printf, const char* msg)
        {
            plugin_ptr->Log("Warning: %s", msg);
            return printf(msg);
        });
    }

    // Fixes for stores that do not return the previous registered entry, always registers a new one
    if(true)
    {
        using addcol_hook = function_hooker<0x5B630B, id_t(const char*)>;
        using addr3_hook  = function_hooker<0x5B63E8, id_t(const char*)>;
        using addscm_hook = function_hooker_thiscall<0x5B6419, id_t(void*, const char*)>;

        // Although streamed COLs exist in Vice too, they are checked, so we don't need to check ourselves
        if(gvm.IsSA())
        {
            TraitsSA traits; // see comment above

            // CColStore finding method is dummie, so we need to avoid duplicate cols by ourselves
            make_static_hook<addcol_hook>([&](addcol_hook::func_type AddColSlot, const char*& name)
            {
                return this->FindOrRegisterResource(name, "col", traits.col_start, AddColSlot, name);
            });
        }

        // The following files are in SA only
        if(gvm.IsSA())
        {
            TraitsSA traits;

            // CVehicleRecording do not care about duplicates, but we should
            make_static_hook<addr3_hook>([&](addr3_hook::func_type RegisterRecordingFile, const char*& name)
            {
                return this->FindOrRegisterResource(name, "rrr", traits.rrr_start, RegisterRecordingFile, name);
            });
            
            // CStreamedScripts do not care about duplicates but we should
            make_static_hook<addscm_hook>([&](addscm_hook::func_type RegisterScript, void*& self, const char*& name)
            {
                return this->FindOrRegisterResource(name, "scm", traits.scm_start, RegisterScript, self, name);
            });
        }
    }
#endif


    // CdStream path overiding
    if(gvm.IsSA())  // TODO VC III
    {
        static void*(*OpenFile)(const char*, const char*);
        static void*(*RwStreamOpen)(int, int, const char*);

        static auto OpenFileHook = [](const char* filename, const char* mode)
        {
            return OpenFile(streaming->GetCdStreamPath(filename), mode);
        };

        static auto RwStreamOpenHook = [](int a, int b, const char* filename)
        {
            return RwStreamOpen(a, b, streaming->GetCdStreamPath(filename));
        };

        // Resolve the cd stream filenames by ourselves on the following calls
        {
            auto pOpenFile     = raw_ptr((decltype(OpenFile))(OpenFileHook));
            OpenFile = MakeCALL(0x5B6183, pOpenFile).get();
            MakeCALL(0x532361, pOpenFile);
            MakeCALL(0x5AFC9D, pOpenFile);
            auto pRwStreamOpen = raw_ptr((decltype(RwStreamOpen))(RwStreamOpenHook));
            RwStreamOpen = MakeCALL(0x5AFBEF, pRwStreamOpen).get();
            MakeCALL(0x5B07E9, pRwStreamOpen);
        }

        // Pointers to archieve the ds:[CreateFileA] overriding, we also have to deal with SecuROM obfuscation there!
        static void* pCreateFileForCdStream = (void*) &CreateFileForCdStream;

        if(gvm.IsSA() && gvm.IsHoodlum())
        {
            static uintptr_t SRXorCreateFileForCdStream = 0x214D4C48 ^ (uintptr_t)(&pCreateFileForCdStream);
            memory_pointer_raw p;

            if(gvm.IsUS())
                p = raw_ptr(0x01564B56 + 1);
            else if(gvm.IsEU())
                p = raw_ptr(0x01564AED + 1);
            else
                plugin_ptr->Error("SRXorCreateFileForCdStream patch failed");

            injector::WriteMemory(p, &SRXorCreateFileForCdStream, true);
        }
        else
            injector::WriteMemory(0x40685E + 2, &pCreateFileForCdStream, true);
    }

    // Some fixes to allow the refreshing process to happen
    if(gvm.IsSA())
    {
        // Fix issue with CBike having some additional fields on SetupSuspensionLines that gets deallocated when
        // we destroy it's model or something. Do just like CQuad and other does, checks if the pointer is null and then allocate it.
        MakeNOP(0x6B89CE, 6);
        MakeCALL(0x6B89CE, HOOK_FixBikeSuspLines);
    }
}

