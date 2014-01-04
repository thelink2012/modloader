/* 
 * std-img -- Standard IMG Loader Plugin for San Andreas Mod Loader
 * Copyright (C) 2013  LINK/2012 <dma_2012@hotmail.com>
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
#include "CDirectoryEntry.h"
#include "img.h"
#include "Injector.h"
#include "CImgDescriptor.h"
#include <modloader_util_injector.hpp>
#include <modloader_util_path.hpp>
using namespace modloader;

static HANDLE GetAbstractHandle(int, HANDLE);

// Hooks and other util stuff
extern "C"
{
    struct scoped_lock
    {
        CRITICAL_SECTION* c;

        /* Enter on ctor, Leave on dtor */
        scoped_lock(CRITICAL_SECTION& cs)
        { c = &cs; EnterCriticalSection(&cs); }
        ~scoped_lock()
        { LeaveCriticalSection(c); }
    };
    
    // Some vars connected within the game
    CStreamingInfo* ms_aInfoForModel;
    DWORD *pStreamCreateFlags;
    CImgDescriptor* images;
    
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
        std::list<std::string> buffers;
        return buffers.emplace(buffers.end(), str)->c_str();
    }
    
    
    //
    static int iModelBeingLoaded = -1;          // Model currently passing throught CdStreamRead
                                                // CallGetAbstractHandle should take care of it
    
    // Returns the file handle to be used for iModelBeingLoaded
    HANDLE CallGetAbstractHandle(HANDLE hFile)
    {
        if(iModelBeingLoaded == -1) return hFile;
        return GetAbstractHandle(iModelBeingLoaded, hFile);
    }
};


/*
 *  CAbstractStreaming
 *      Streaming of files on disk
 */
class CAbstractStreaming
{
    public:
        friend int __stdcall CdStreamThread();
        
        typedef CThePlugin::ModelFile       ModelFile;
        typedef CThePlugin::ModelFileRef    ModelFileRef;
        
        struct SFile    // Handle and ModelFile reference
        {
            HANDLE handle;
            ModelFile& file;
            int index;
            
            SFile(HANDLE handle, ModelFile& file, int idx)
                : handle(handle), file(file), index(idx)
            {}
            
            bool operator==(const SFile& b)
            {
                return file.hash == b.file.hash;
            }
            
            bool operator==(HANDLE h)
            {
                return handle == h;
            }
        };
       
    protected:
        CRITICAL_SECTION cs;                            // this must be used together with @files list for thread-safety
        std::list<SFile> files;                         // list of files currently open for reading
        
    public:
        std::map<unsigned int, ModelFileRef> imports;   // map of imported models index and it's reference
        std::map<unsigned int, ModelFileRef> clothes;   // map of clothes offset at player.img and it's reference in-disk
        
    public:
        
        CAbstractStreaming()
        { InitializeCriticalSection(&cs); }
        
        ~CAbstractStreaming()
        { DeleteCriticalSection(&cs); }
        
        void LoadAbstractCdDirectory();                     // Simulates the loading of a cdimage directory
        
        SFile* OpenModel(ModelFile& file, int index);       // Opens a new file for the model
        void CloseModel(SFile* file);                       // Closes the previosly open model
        
        void Import(ModelFile&, int index);                 // Imports a model into index
        void Reimport(int index);                           // Reimports model at index, reupdating it's information from disk

} abstract;



/*
 *  GetAbstractHandle
 *      Returns another file from the abstract streaming or returns the received file 
 */
static HANDLE GetAbstractHandle(int index, HANDLE hFile)
{
    CAbstractStreaming::SFile* f = nullptr;
    
    // Try to find the object index in the import list
    auto it = abstract.imports.find(index);
    if(it != abstract.imports.end())
    {
        f = abstract.OpenModel(it->second, it->first);
    }
    
    // Returns the file from the abstract streaming if available
    return (f? f->handle : hFile);
}

/*
 *  Import
 *      Imports a model into @index
 */
void CAbstractStreaming::Import(ModelFile& file, int index)
{
    file.MakeSureHasProcessed();
    this->imports.emplace(index, file);
}

/*
 *  Reimport
 *      Reloads the model data (such as the size of it)
 */
void CAbstractStreaming::Reimport(int index)
{
    auto it = imports.find(index);
    if(it != imports.end())
    {
        // Lock just to be sure we're not in concurrency with the streaming thread
        scoped_lock xlock(this->cs);
        
        // Reprocess the file data
        it->second.get().ProcessFileData();
    }
}

/*
 *  OpenModel
 *      Opens a model file handle 
 */
auto CAbstractStreaming::OpenModel(ModelFile& file, int index) -> SFile*
{
    HANDLE hFile = CreateFileA(file.path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL,
                               OPEN_EXISTING, *pStreamCreateFlags, NULL);
    
    if(hFile == INVALID_HANDLE_VALUE)
    {
        imgPlugin->Log("Failed to open file \"%s\" for abstract streaming; error code: 0x%X",
                       file.path.c_str(), GetLastError());
        return nullptr;
    }
    else
    {
        scoped_lock xlock(this->cs);
        return &(*this->files.emplace(files.end(), hFile, file, index));
    }
}

/*
 *  CloseModel
 *      Closes a model file handle 
 */
void CAbstractStreaming::CloseModel(SFile* file)
{
    scoped_lock xlock(this->cs);
    
    // Close the file handle
    CloseHandle(file->handle);
    
    // Remove this file from the open files list
    this->files.remove(*file);
}


/*
 *  LoadAbstractCdDirectory
 *      Simulates cdimage entries from files on disk
 * 
 */
void CAbstractStreaming::LoadAbstractCdDirectory()
{
    typedef function_hooker<0x5B61B8, size_t(void*, void*, size_t)> rcount_hook;
    typedef function_hooker<0x5B61E1, size_t(void*, void*, size_t)> rentry_hook;
    typedef function_hooker_fastcall<0x5B6449, char(CStreamingInfo*, int, int*, int*)> rreg_hook;

    // TODO BUILD STUFF
    
    // Basic vars:
    static decltype(imgPlugin->models)::iterator models_it;
    static decltype(imgPlugin->models)::iterator curr_model_it;
    auto Log = imgPlugin->Log;
    void (*LoadCdDirectory)(void*, int)  = memory_pointer(0x5B6170).get();
    
    Log("Loading abstract cd directory...");

    // Make the models iterator point to the beggining
    models_it = imgPlugin->models.begin();
    
    // Return the number of entries to read
    auto rcount = make_function_hook<rcount_hook>([](rcount_hook::func_type, void*&, void*& buf, size_t&)
    {
        int& mcount = *(int*)(buf);
        
        // We'll read models that aren't clothes
        mcount = imgPlugin->models.size() - abstract.clothes.size();
        //imgPlugin->Log("Models count on abstract cd: %u", mcount);
        
        return sizeof(mcount);
    });
    
    // Reads abstract cd entries based on the models that are on the disk
    auto rentry = make_function_hook<rentry_hook>([](rentry_hook::func_type, void*&, void*& buf, size_t&)
    {
        CThePlugin::ModelFile* file = nullptr;
        CDirectoryEntry* entry = (CDirectoryEntry*)(buf);
        
        // Advance models iterator until a non-cloth model is found
        while(models_it->second.get().isClothes) ++models_it;
        file = &models_it->second.get();
        
        // Setup abstract entry
        strncpy((char*)entry->filename, file->name.data(), 23);
        entry->filename[23]  = 0;
        entry->fileOffset    = 0;
        entry->sizePriority1 = 0;
        entry->sizePriority2 = file->GetSizeInBlocks();
        
        curr_model_it = models_it++;
        return sizeof(entry);
    });
    
    // Registers the existence of a model index under our ownership
    auto rreg   = make_function_hook<rreg_hook>([](rreg_hook::func_type GetCdPosnAndSize, CStreamingInfo*& self, int&, int*& posn, int*& size)
    {
        int index = self - ms_aInfoForModel;                 // self model index
        char result = GetCdPosnAndSize(self, 0, posn, size);
        if(!result) abstract.Import(curr_model_it->second, index);
        return result;
    });
    
    // Make the game load our abstract directory
    LoadCdDirectory(nullptr, 0);
    
    // Remove the hooks we did
    rcount.restore();
    rentry.restore();
    rreg.restore();
    
    // We're done simulating a cd directory
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
        CAbstractStreaming::SFile* sfile = nullptr;
        
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
                scoped_lock xlock(abstract.cs);
                auto it = std::find(abstract.files.begin(), abstract.files.end(), hFile);
                if(it != abstract.files.end())
                {
                    bIsAbstract = true;
                    
                    // Setup vars based on abstract file
                    sfile  = &(*it);
                    offset = 0;
                    size   = sfile->file.GetSize();
                    bsize  = sfile->file.GetSizeInBlocks();
                    index  = sfile->index;
                    filename = sfile->file.name.c_str();
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
                // Ignore EOF because the files on disk (not on cdimage files) won't be 2KiB aligned
                else if(GetLastError() == ERROR_HANDLE_EOF)
                {
                    bResult = true;
                }
            }
            
            // There's some real problem if we can't load a abstract model
            if(bIsAbstract && !bResult)
            {
                imgPlugin->Log("Warning: Failed to load abstract model file %s", filename);
            }
            
            // Set the cdstream status, 0 for "okay" and 254 for "failed to read"
            cd->status = bResult? 0 : 254;
        }
        
        // Remove from the queue what we just readed
        RemoveFirstInQueue(&cdinfo.queue);
        
        // Cleanup
        if(bIsAbstract) abstract.CloseModel(sfile);
        cd->nSectorsToRead = 0;
        if(cd->bLocked) ReleaseSemaphore(cd->semaphore, 1, 0);
        cd->bInUse = false;
    }
    
    return 0;
}









void CThePlugin::StreamingPatch()
{
    typedef function_hooker<0x40CF34, int(int streamNum, void* buf, int sectorOffset, int sectorCount)> cdread_hook;
    typedef function_hooker<0x5B8E1B, size_t()> loadcd_hook;
    
    uintptr_t addr;
    bool isHoodlum = ReadMemory<uint8_t>(0x406A20, true) == 0xE9;
    
    // Setup some references
    ms_aInfoForModel   = memory_pointer(0x8E4CC0).get();
    pStreamCreateFlags = memory_pointer(0x8E3FE0).get();
    images             = ReadMemory<CImgDescriptor*>(0x407613 + 1, true);
    
    // Making our our code for the stream thread would make things so much better
    MakeJMP(0x406560, (void*) CdStreamThread);
    
    // We need to know the next model to be read before the CdStreamRead call happens
    MakeCALL(0x40CCA6, (void*) HOOK_RegisterNextModelRead);
    MakeNOP(0x40CCA6 + 5, 2);
    
    // We need to return a new hFile if the file is on disk
    addr = isHoodlum? 0x156C2FB : 0x406A5B; // isHOODLUM? HOODLUM : ORIGINAL
    MakeCALL(addr, (void*) HOOK_NewFile);
    MakeNOP(addr+5, 1);
    
    // Load our on-disk information as it was inside a cdimage
    make_function_hook<loadcd_hook>([](loadcd_hook::func_type LoadCdDirectory1)
    {
        static bool bFirstTime = true;
        if(bFirstTime)
        {
            abstract.LoadAbstractCdDirectory();
            bFirstTime = false;
        }
        return LoadCdDirectory1();
    });

    // We need to know the model index that will pass throught CallGetAbstractHandle
    make_function_hook<cdread_hook>([](cdread_hook::func_type CdStreamRead, int& streamNum, void*& buf, int& sectorOffset, int& sectorCount)
    {
        iModelBeingLoaded = iNextModelBeingLoaded;
        int result = CdStreamRead(streamNum, buf, sectorOffset, sectorCount);
        iModelBeingLoaded = iNextModelBeingLoaded = -1;
        return result;
    });
    

    
    
    
    
    
    
    
    
    
    
    
    /*
        readImgFileFromDat = MakeCALL(0x5B915B, (void*) HOOK_ReadImgFileFromDat).get();
        
        addPath = MakeCALL(0x5B63E8, (void*)(HOOK_AllocateOrFindPath)).get();
        CExternalScripts__Allocate = MakeCALL(0x5B6419, (void*)(HOOK_AllocateOrFindExternalScript)).get();
     * 
     * ------> CLOTHES
     *  CLOSE FILE
     *  CLOTHES PLAYER BUG
     *  SPECIAL MODEL BUG
     *  RELOAD
    */
        
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
     *          but we need to do more than that, because this string is still used in CStreaming::ReadImgContent
     *          to open the file and read the header.
     * 
     *          So what we did? The first field in CImgDescriptor is a char[40] array to store the name, so, we turned this field into an:
     *              union {
     *                  char name[40];
     *                  
     *                  struct {
     *                      char dummy[2];      // Will container the dummy string "?\0" (0x003F)
     *                      char pad[2];        // Just padding okay?
     *                      char* customName;   // Pointer to a static buffer containing the new file name
     *                  };
     *              };
     * 
     *          Then we hook CStreaming::ReadImgContents to give the pointer customName instead of &name to CFileMgr::Open
     *          Very dirty, isn't it?
     * 
     *          One problem is that the customName will be a buffer allocated for the entire program lifetime,
     *          but I don't think it's a problem because the imgDescripts are initialized only once.
     * 
     */
    if(true)
    {
        addr = isHoodlum? 0x1564B90 : 0x406886;         // streamNames hook
        MakeNOP(addr, 10);  // on Steam this 10 changes
        MakeCALL(addr, (void*) HOOK_SetStreamName);
        
        addr = isHoodlum? 0x01567BC2 : 0x407642;        // imgDescriptor hook
        MakeNOP(addr, 10);  // on Steam this 10 changes
        MakeCALL(addr, (void*) HOOK_SetImgDscName);
        
        typedef function_hooker<0x5B6183, void*(const char*, const char*)>  ldcd_hook;
        
        // hook to read new field at img descriptor
        make_function_hook<ldcd_hook>([](ldcd_hook::func_type fopen, const char*& filename, const char*& mode)
        {
            if(filename)
            {
                // If it's our dummy string, take the new string from the customName pointer
                if(filename[0] == '?' && filename[1] == '\0')
                    filename = *(char**)(filename + 4);
            }
            else
            {
                // It's our custom call, ignore fopen but actually open a valid stream
                filename = modloader::szNullFile;
            }
            
            // Continue into fopen
            return fopen(filename, mode);
        });
    }
}


