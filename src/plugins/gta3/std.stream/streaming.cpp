/* 
 * Copyright (C) 2013-2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 *
 */
#include "streaming.hpp"
#include "CdStreamInfo.h"
#include <modloader/util/injector.hpp>
#include <modloader/util/path.hpp>
using namespace modloader;

/*
    uiUnk2 flag
        0x02 - cannot delete
        0x04 - streaming is not owner (?)
        0x08 - dependency
        0x10 - first priority request
        0x20 - don't delete on makespacefor

        notes:
        (0x20|0x4) -> gives model alpha=255 otherwise alpha=0
*/

// TODO needs to refresh ifp and others too
// TODO remove is hoodlum
// TODO avoid CdStream optimization of nextOnCd / lastposn / etc (?)
// TODO fix non find (only add) on LoadCdDirectory (GAME FOR COL/IFP/RRR/ETC)
// TODO special model
// TODO clothes
// TODO proper logging
// TODO proper IsClothes (player_parachute.scm isn't a clothing item)

CAbstractStreaming streaming;


extern "C"
{
    // Assembly hooks at "asm/" folder
    auto* ms_aInfoForModel = injector::ReadMemory<CStreamingInfo*>(0x5B8AE8, true);
    extern void HOOK_RegisterNextModelRead();
    extern void HOOK_NewFile();
 
    static HANDLE __stdcall CreateFileForCdStream(
        LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
        LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition,
        DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
    {
        LPCSTR lpActualFileName = streaming.GetCdStreamPath(lpFileName);
        plugin_ptr->Log("Opening file for streaming \"%s\"", lpActualFileName);
        return CreateFileA(
            lpActualFileName, dwDesiredAccess, dwShareMode,
            lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
    }
};







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
 *  CAbstractStreaming::InfoForModel
 *      Returns the streaming info pointer for the specified resource id
 */
CStreamingInfo* CAbstractStreaming::InfoForModel(id_t id)
{
    return &ms_aInfoForModel[id];
}

/*
 *  CAbstractStreaming::IsModelOnStreaming
 *      Checks if the specified model is on the streaming, either on the bus to get loaded or already loaded.
 */
bool CAbstractStreaming::IsModelOnStreaming(id_t id)
{
    return InfoForModel(id)->load_status != 0;
}

/*
 *  CAbstractStreaming::RequestModel
 *      Requests a resource into the streaming, essentially loading it
 *      Notice the model will not be available right after the call, it's necessary to wait for it.
 *      Flags are unknown... still researching about them.
 */
void CAbstractStreaming::RequestModel(id_t id, uint32_t flags)
{
    injector::cstd<void(int, int)>::call<0x4087E0>(id, flags);
}

/*
 *  CAbstractStreaming::RemoveModel
 *      Removes a resource from the streaming, essentially unloading it
 *      Flags are unknown (and optional) so leave it as default.
 */
void CAbstractStreaming::RemoveModel(id_t id)
{
    injector::cstd<void(int)>::call<0x4089A0>(id);
}

/*
 *  CAbstractStreaming::LoadAllRequestedModels
 *      Free ups the streaming bus by loading everything previosly requested
 */
void CAbstractStreaming::LoadAllRequestedModels()
{
    plugin_ptr->Log("Loading requested resources...");
    injector::cstd<void()>::call<0x5619D0>();       // CTimer::StartUserPause
    injector::cstd<void(int)>::call<0x40EA10>(0);   // CStreaming::LoadAllRequestedModels
    injector::cstd<void(int)>::call<0x40EA10>(0);   // CStreaming::LoadAllRequestedModels
    injector::cstd<void()>::call<0x561A00>();       // CTimer::EndUserPause
}

/*
 *  CAbstractStreaming::FlushChannels
 *      Flushes the streaming channels loading anything on the bus
 */
void CAbstractStreaming::FlushChannels()
{
    plugin_ptr->Log("Flushing streaming channels...");
    injector::cstd<void()>::call<0x5619D0>();       // CTimer::StartUserPause
    injector::cstd<void(int)>::call<0x40E460>(0);   // CStreaming::FlushChannels
    injector::cstd<void()>::call<0x561A00>();       // CTimer::EndUserPause
}

/*
 *  CAbstractStreaming::RemoveUnusedResources
 *      Free ups the streaming by removing any unused resource
 */
void CAbstractStreaming::RemoveUnusedResources()
{
    plugin_ptr->Log("Removing unused resources...");
    injector::cstd<void()>::call<0x40CF80>();               // CStreaming::RemoveAllUnusedModels
    injector::cstd<char(uint32_t)>::call<0x40CFD0>(0x20);   // CStreaming::RemoveLeastUsedModel
}






/*
 *  CAbstractStreaming::InstallFile
 *      Installs a model, clothing or any streamable file, refreshing them after the process
 */
bool CAbstractStreaming::InstallFile(const modloader::file& file)
{
    // If the streaming hasn't initialized we cannot assume much things about the streaming
    // One thing to keep in mind is that in principle stuff should load in alpha order (for streamed scenes etc)
    if(!this->bHasInitializedStreaming)
    {
        // Just push it to this list and it will get loaded when the streaming initializes
        // At this point we don't know if this is a clothing item or an model, for that reason "raw"
        // The initializer will take care of filtering clothes and models from the list
        this->raw_models[file.FileName()] = &file;
        return true;
    }
    else
    {
        // We cannot do much at this point, too many calls may come, repeated calls, uninstalls, well, many things will still happen
        // so we'll delay the actual install to the next frame, put everything on an import list
        this->BeginUpdate();

        if(!IsClothes(file))
        {
            this->mToImportList[file.hash] = &file;
            return true;
        }
        else
        {
            // TODO clothes
        }
    }
    return false;
}

/*
 *  CAbstractStreaming::UninstallFile
 *      Uninstalls a specific file
 */
bool CAbstractStreaming::UninstallFile(const modloader::file& file)
{
    // Ahhh, see the comments at InstallFile.....
    if(!this->bHasInitializedStreaming)
    {
        // Streaming hasn't initialized, just remove it from our raw list
        raw_models.erase(file.FileName());
        return true;
    }
    else
    {
        this->BeginUpdate();

        if(!IsClothes(file))
        {
            // Mark the specified file [hash] to be vanished
            this->mToImportList[file.hash] = nullptr;
            return true;
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
    // Reinstalling works the same way as installing
    return InstallFile(file);
}


/*
 *  CAbstractStreaming::Update
 *      Updates the abstract streaming after a serie of install/uninstall
 */
void CAbstractStreaming::Update()
{
    if(this->IsUpdating())
    {
        // Refresh necessary files
        this->ProcessRefreshes();
        this->EndUpdate();
    }
}




/*
 *  CAbstractStreaming::ImportModels
 *      Imports the files in the list into the abstract streaming
 */
void CAbstractStreaming::ImportModels(ref_list<const modloader::file*> files)
{
    LoadAbstractCdDirectory(files);
}

/*
 *  CAbstractStreaming::UnimportModel
 *      Removes the imported index from the abstract streaming
 */
void CAbstractStreaming::UnimportModel(id_t index)
{
    // Remove the entry from the special directory
    auto it_imp = imports.find(index);
    if(it_imp != imports.end())
    {
        for(auto it = special_dir.begin(); it != special_dir.end(); ++it)
        {
            // We're removing it from the special directory but it's still in the ModelsDirectory, we can't do much about it...
            // Let it go.
            if(it->second == it_imp->second.file)
            {
                special_dir.erase(it);
                break;
            }
        }
    }

    this->RestoreInfoForModel(index);
    this->QuickUnimport(index);
}

/*
 *  CAbstractStreaming::QuickImport
 *      Quickly import into the specified index the specified file with the specified flags
 *      This is a raw import technique and should be used ONLY when necessary
 *      It does not perform any kind of checking such as if the streaming buffer size is enought
 */
void CAbstractStreaming::QuickImport(id_t index, const modloader::file* file, bool isSpecialModel)
{
    plugin_ptr->Log("Importing model file for index %d at \"%s\"", index, file->FileBuffer());

    // Add import into the import table
    auto& imp = imports[index];
    imp.file = file;
    imp.isFallingBack = false;
    imp.isSpecialModel = isSpecialModel;

    // Register the existence of such a model and setup info for it
    this->RegisterModelIndex(file->FileName(), index);
    this->SetInfoForModel(index, 0, GetSizeInBlocks(file->Size()));
}

/*
 *  CAbstractStreaming::QuickUnimport
 *      Quickly unimport the specified index
 *      Analogue to QuickImport. This version of unimporting do not mess with the info for model structure.
 */
void CAbstractStreaming::QuickUnimport(id_t index)
{
    plugin_ptr->Log("Removing imported model file at index %d", index, index);
    this->imports.erase(index);
}

/*
 *  CAbstractStreaming::RegisterModelIndex
 *      Registers the existence of an index assigned to a specific filename.
 */
void CAbstractStreaming::RegisterModelIndex(const char* filename, id_t index)
{
    this->indices[modloader::hash(filename, ::tolower)] = index;
}

/*
 *  CAbstractStreaming::RegisterStockEntry
 *      Registers the stock/default/original cd directory data of an index, important so it can be restored later when necessary.
 */
void CAbstractStreaming::RegisterStockEntry(const char* filename, CDirectoryEntry& entry, id_t index, int img_id)
{
    // Please note entry here is incomplete because the game null terminated the string before the extension '.', so use filename
    cd_dir[index] = CdDirectoryItem(filename, entry, img_id);
}






/*
 *  CAbstractStreaming::MakeSureModelIsOnDisk
 *      If the specified model id is under our control, make entirely sure it's present on disk.
 *      If it isn't, we'll fallback to the original mode on img files.
 */
void CAbstractStreaming::MakeSureModelIsOnDisk(id_t index)
{
    auto it = imports.find(index);
    if(it != imports.end())
    {
        auto& m = it->second;
        if(m.isFallingBack == false)    // If already falling back, don't check again
        {
            // If file isn't on disk we should fall back to the stock model
            if(!IsPath(m.file->FullPath(fbuffer).c_str()))
            {
                plugin_ptr->Log("Model file \"%s\" has been deleted, falling back to stock model.", m.file->FileBuffer());
                this->RestoreInfoForModel(index);
                m.isFallingBack = true;
            }
        }
    }
}

/*
 *  Streaming thread
 *      This thread reads pieces from cd images and in mod loader on disk files
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
                plugin_ptr->Log("Warning: Failed to load abstract model file %s; error code: 0x%X", filename, GetLastError());
            

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


/*
 *  CAbstractStreaming::Patch
 *      Patches the default game streaming pipeline to have our awesome hooks.
 */
void CAbstractStreaming::Patch()
{
    typedef function_hooker<0x5B8E1B, void()> sinit_hook;
    typedef function_hooker<0x40CF34, int(int, void*, int, int)> cdread_hook;
    bool isHoodlum = injector::address_manager::singleton().IsHoodlum(); // TODO REMOVE, PUT ON ADDR TRANSLATOR

    // Initialise the streaming
    make_static_hook<sinit_hook>([this](sinit_hook::func_type LoadCdDirectory1)
    {
        plugin_ptr->Log("Initializing the streaming...");

        TempCdDir_t cd_dir;
        this->FetchCdDirectories(cd_dir, LoadCdDirectory1);         // Fetch...
        this->LoadCdDirectories(cd_dir);                            // ...and load
        this->LoadAbstractCdDirectory(refs_mapped(raw_models));       // Load custom

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
        MakeCALL(0x40CCA6, raw_ptr(HOOK_RegisterNextModelRead));
        MakeNOP(0x40CCA6 + 5, 2);

        // We need to return a new hFile if the file is on disk
        MakeCALL(!isHoodlum? 0x406A5B : 0x0156C2FB, raw_ptr(HOOK_NewFile));
        MakeNOP((!isHoodlum? 0x406A5B : 0x0156C2FB)+5, 1);

        // We need to know the model index that will pass throught CallGetAbstractHandle
        make_static_hook<cdread_hook>([](cdread_hook::func_type CdStreamRead, int& streamNum, void*& buf, int& sectorOffset, int& sectorCount)
        {
            iModelBeingLoaded = iNextModelBeingLoaded;
            auto result = CdStreamRead(streamNum, buf, sectorOffset, sectorCount);
            iModelBeingLoaded = iNextModelBeingLoaded = -1;
            return result;
        });
    }

    // Special models
    if(true)
    {
        typedef function_hooker_thiscall<0x409F76, char(void*, const char*, unsigned int&, unsigned int&)> findspecial_hook;
        typedef function_hooker<0x409FD9, char(int, int)> rqspecial_hook;
        static CDirectoryEntry* pRQSpecialEntry;    // Stores the special entry we're working with

        // Hook call to CDirectory::FindItem (1) to find out which directory entry we're dealing with...
        make_static_hook<findspecial_hook>([this](findspecial_hook::func_type FindItem, void*& dir, const char*& name, unsigned int& offset, unsigned int& blocks)
        {
            pRQSpecialEntry = injector::thiscall<CDirectoryEntry*(void*, const char*)>::call<0x532450>(dir, name);  // CDirectory::FindItem
            return FindItem(dir, name, offset, blocks); // (above call and this call are different)
        });

        // Hook call to CStreaming::RequestModel and do special processing if the entry is related to the abstract streaming
        make_static_hook<rqspecial_hook>([this](rqspecial_hook::func_type RequestModel, int& index, int& flags)
        {
            // Before this being called the game built the InfoForModel entry for this index based on the stock entry pRQSpecialEntry
            // Register stock entry, so it can be restored during gameplay if necessary
            CDirectoryEntry entry = *pRQSpecialEntry;                   // Make copy to do proper processing
            auto img_id      = pRQSpecialEntry->fileOffset >> 24;       // Extract img id from file offset
            entry.fileOffset = pRQSpecialEntry->fileOffset & 0xFFFFFF;  // Extract actual file offset
            this->RegisterStockEntry(strcat(entry.filename, ".dff"), entry, index, img_id);

            // Try to find abstract special entry related to this request....
            // If it's possible, quickly import our entry into this special index
            // otherwise quickly remove our previous entry at this special index if any there
            auto it = this->special_dir.find(modloader::hash(pRQSpecialEntry->filename, ::tolower));
            if(it != this->special_dir.end())
                this->QuickImport(index, it->second, true);
            else
                this->QuickUnimport(index);

            // Ahhhhh!! finally do the actual request :)
            return RequestModel(index, flags);
        });
    }

    // CdStream path overiding
    if(true)
    {
        // Do not use function_hooker here in this context, it would break many scoped hooks in this plugin.
        
        static void*(*OpenFile)(const char*, const char*);
        static void*(*RwStreamOpen)(int, int, const char*);

        static auto OpenFileHook = [](const char* filename, const char* mode)
        {
            return OpenFile(streaming.GetCdStreamPath(filename), mode);
        };

        static auto RwStreamOpenHook = [](int a, int b, const char* filename)
        {
            return RwStreamOpen(a, b, streaming.GetCdStreamPath(filename));
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
        static void* pCreateFileForCdStream = &CreateFileForCdStream;
        static uintptr_t SRXorCreateFileForCdStream = 0x214D4C48 ^ (uintptr_t)(&pCreateFileForCdStream);  // Used on the obfuscated executable

        if(gvm.IsSA() && gvm.IsHoodlum())
            injector::WriteMemory(raw_ptr(0x1564B56 + 1), &SRXorCreateFileForCdStream, true);
        else
            injector::WriteMemory(0x40685E + 2, &pCreateFileForCdStream, true);
    }
}

