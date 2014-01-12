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
#include "img.h"
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
    int *CVehicleRecording__NumPlayBackFiles;
    int *CVehicleRecording__StreamingArray;
    int (__fastcall *FindStreamedScript)(void*, int, const char* name);
    int (__fastcall *CDirectory__CDirectory)(CDirectory*, int, size_t count, CDirectoryEntry*);
    int (__fastcall *CDirectory__ReadDirFile)(CDirectory*, int, const char* file);
    CDirectory* playerImgDirectory;
    CDirectoryEntry* playerImgEntries;
    
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
        return GetAbstractHandle(iModelBeingLoaded, hFile);
    }
};


/*
 *  CAbstractStreaming
 *      Streaming of files on disk
 * 
 *      We could do some optimizations to file loads from disk, but well, don't optimize until it's necessary to.
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
        bool bIs2048AlignedSector;                      // not used, maybe in the future
        CRITICAL_SECTION cs;                            // this must be used together with @files list for thread-safety
        std::list<SFile> files;                         // list of files currently open for reading
        
    public:
        std::map<unsigned int, ModelFileRef> imports;   // map of imported models index and it's reference
        std::map<unsigned int, ModelFileRef> clothes;   // map of clothes offset at player.img and it's reference in-disk

    public:
        
        CAbstractStreaming();
        ~CAbstractStreaming();
        
        void LoadAbstractCdDirectory();                 // Simulates the loading of a cdimage directory
        
        SFile* OpenModel(ModelFile& file, int index);   // Opens a new file for the model
        void CloseModel(SFile* file);                   // Closes the previosly open model
        
        void Import(ModelFile&, int index);             // Imports a model into index, if there's any model at index, nothing gets imported
        void Reimport(int index);                       // Reimports model at index, reupdating it's information from disk
        void Reimport(ModelFile&, bool bLock = true);   // Does the same as above
        void Reimport(ModelFile&, int index);           // Imports a model into index, overriding any model proviosly there
        void RemoveImport(int index);                   // Removes a import. Note this won't restore modelinfo!

        void BuildClothesMap();                         // Builds clothes information (clothes map)
        void RequestClothes(int index);                 // Imports a cloth based on aInfoForModel[index] block offset
        void FixClothesDirectory(CDirectory&);          // Fixes clothes directory with our sizes
        
        // Reloads all models information
        void ReloadModels()
        {
            scoped_lock xlock(this->cs);
            for(auto& pair : this->imports) this->Reimport(pair.second.get(), false);
        }

        // Removes model import at @index if @pModel is nullptr
        // or reimport @index if @pModel isn't nullptr
        void RemoveOrReimport(ModelFile* pModel, int index)
        {
            if(pModel) this->Reimport(*pModel, index);
            else       this->RemoveImport(index);
        }
        
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
 *  Constructs the abstract streaming object 
 */
CAbstractStreaming::CAbstractStreaming()
{
    DWORD SectorsPerCluster, BytesPerSector, NumberOfFreeClusters, TotalNumberOfClusters;

    InitializeCriticalSection(&cs);
    
    /*
     *  Findout if the number of bytes per sector from this disk is 2048 aligned, this allows no-buffering for 2048 bytes aligned files.
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

/*
 *  Destructor for the abstract streaming object
 */
CAbstractStreaming::~CAbstractStreaming()
{
    DeleteCriticalSection(&cs);
}


/*
 *  Import
 *      Imports a model into @index
 */
void CAbstractStreaming::Import(ModelFile& file, int index)
{
    imgPlugin->Log("Importing model file for index %d at \"%s\"", index, file.path.c_str());
    
    file.MakeSureHasProcessed();
    this->imports.emplace(index, file);
    
}

/*
 *  Reimport
 *      Imports a new model into @index
 */
void CAbstractStreaming::Reimport(ModelFile& file, int index)
{
    scoped_lock xlock(this->cs);
    
    auto it = this->imports.find(index);
    if(it != this->imports.end())
        it->second = file;
    else
        this->imports.emplace(index, file);

    file.ProcessFileData();
}

/*
 *  Reimport
 *      Reloads the model data (such as the size of it)
 */
void CAbstractStreaming::Reimport(int index)
{
    auto it = imports.find(index);
    if(it != imports.end()) this->Reimport(it->second.get());
}

/*
 *  Reimport
 *      Reloads the model data (such as the size of it)
 */
void CAbstractStreaming::Reimport(ModelFile& file, bool bLock)
{
    if(bLock)
    {
        // Lock just to be sure we're not in concurrency with the streaming thread
        scoped_lock xlock(this->cs);
        // Reprocess the file data
        file.ProcessFileData();
    }
    else
    {
        file.ProcessFileData();
    }
}

/*
 * RemoveImport
 *      Erases a item from the import table 
 */
void CAbstractStreaming::RemoveImport(int index)
{
    scoped_lock xlock(this->cs);
    imports.erase(index);
}


/*
 *  OpenModel
 *      Opens a model file handle 
 */
auto CAbstractStreaming::OpenModel(ModelFile& file, int index) -> SFile*
{
    DWORD flags = FILE_FLAG_SEQUENTIAL_SCAN | (*pStreamCreateFlags & FILE_FLAG_OVERLAPPED);
    
    HANDLE hFile = CreateFileA(file.path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL,
                               OPEN_EXISTING, flags, NULL);
    
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
 *  RequestClothes
 *      Clothes object indices are in range 384-393, one index for each part of the body
 *      So when a cloth gets requested it must import "the file" into the specified index
 */
void CAbstractStreaming::RequestClothes(int index)
{
    // If index is a texture or body-id we can take our time to see if there's the need to import a file
    if(index >= 20000 || (index >= 384 && index <= 393))
    {
        auto it = this->clothes.find(ms_aInfoForModel[index].iBlockOffset);
        if(it != this->clothes.end())
        {
            // We have a replacement for this cloth! Reimport our file into index @index
            this->Reimport(it->second, index);
        }
        else
        {
            // Clear import at index 'cuz it is not our file
            this->RemoveImport(index);
        }
    }
    else
    {
        imgPlugin->Log("Warning: RequestClothes failed for index %d", index);
    }
}

/*
 *  FixClothesDirectory
 *      Fix CDirectory size fields for imported clothes 
 */
void CAbstractStreaming::FixClothesDirectory(CDirectory& dir)
{
    scoped_lock xlock(this->cs);
    
    // For each entry in the cd directory...
    for(size_t i = 0; i < dir.m_dwCount; ++i)
    {
        auto& entry = dir.m_pEntries[i];
        
        // See if we have a replacement for the cloth at entry
        auto  it = this->clothes.find(entry.fileOffset);
        if(it != this->clothes.end())
        {
            // Yes, let's fix the cloth size at the cd directory
            entry.sizePriority1 = 0;
            entry.sizePriority2 = it->second.get().GetSizeInBlocks();
        }
    }
    
}

/*
 * BuildClothesMap
 *      Finds out which model files under our control are clothes.
 */
void CAbstractStreaming::BuildClothesMap()
{
    // Findout about some pointers...
    char* playerImgPath         = ReadMemory<char*>(0x5A69F7 + 1, true);
    int   playerImgNumEntries   = ReadMemory<int>  (0x5A69E8 + 1, true);
    
    // Build game's cd directory entries for player.img cdimage
    CDirectory__CDirectory(playerImgDirectory, 0, playerImgNumEntries, playerImgEntries);
    CDirectory__ReadDirFile(playerImgDirectory, 0, playerImgPath);

    this->clothes.clear();
    
    // Find files on modloader folder that is present in player.img cdimage too
    for(size_t i = 0; i < playerImgDirectory->m_dwCount; ++i)
    {
        CDirectoryEntry& entry = playerImgDirectory->m_pEntries[i];
        
        auto it = imgPlugin->models.find( modloader::hash(entry.filename, ::toupper) );
        if(it != imgPlugin->models.end())
        {
            ModelFile& model = it->second;
            
            // Register the clothes file we've just found...
            this->clothes.emplace(entry.fileOffset, model);
            model.isClothes = true;
            
            imgPlugin->Log("Found clothes file \"%s\"", model.path.c_str());
        }
    }
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

    // Basic vars:
    static decltype(imgPlugin->models)::iterator models_it;
    static decltype(imgPlugin->models)::iterator curr_model_it;
    auto Log = imgPlugin->Log;
    void (*LoadCdDirectory)(void*, int)  = memory_pointer(0x5B6170).get();
    
    Log("Loading abstract cd directory...");

    // Build the clothes map right now so we know how much "real models" we have in hands
    this->BuildClothesMap();
    
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

        // Advance models iterator until a non-clothes model is found
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
    Log("Abstract cd directory has been loaded");
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
            }
            
            // There's some real problem if we can't load a abstract model
            if(bIsAbstract && !bResult)
            {
                imgPlugin->Log("Warning: Failed to load abstract model file %s; error code: 0x%X", filename, GetLastError());
            }

            // TODO should I fill the "trash" part of the 2KiB aligned buffer (cd->lpBuffer) with null bytes ?
            
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





/*
 *  Reload models information
 */
void CThePlugin::ReloadModels()
{
    abstract.ReloadModels();
}

/*
 *  Patches the game streaming methods to load files from disk 
 */
void CThePlugin::StreamingPatch()
{
    typedef function_hooker<0x40CF34, int(int streamNum, void* buf, int sectorOffset, int sectorCount)> cdread_hook;
    typedef function_hooker<0x5B8E1B, size_t()> loadcd_hook;
    typedef function_hooker<0x5B915B, int(const char*, char)> datcd_hook;
    typedef function_hooker<0x40A106, char(int, int)> rqcloth_hook;
    typedef function_hooker_fastcall<0x5A6A01, void(CDirectory*, int, const char*)> clothdir_hook;
    typedef function_hooker<0x5B63E8, int(const char*)> addrrr_hook;
    typedef function_hooker<0x5B630B, int(const char*)> addcol_hook;
    typedef function_hooker_fastcall<0x5B6419, int(void*, int, const char*)> addscm_hook;
    typedef function_hooker_fastcall<0x409F76, char(void*, int, const char*, int*, int*)> findspecial_hook;
    typedef function_hooker<0x409FD9, char(int, int)> rqspecial_hook;
    
    static CAbstractStreaming::ModelFile* pSpecial = nullptr;
    
    // Setup some references
    ms_aInfoForModel   = memory_pointer(0x8E4CC0).get();
    pStreamCreateFlags = memory_pointer(0x8E3FE0).get();
    FindStreamedScript = memory_pointer(0x4706F0).get();
    images             = ReadMemory<CImgDescriptor*>(0x407613 + 1, true);
    CVehicleRecording__NumPlayBackFiles = ReadMemory<int*>(0x45A001 + 1, true);
    CVehicleRecording__StreamingArray   = ReadMemory<int*>(0x459FF0 + 2, true);
    CDirectory__CDirectory              = memory_pointer(0x5322F0).get();
    CDirectory__ReadDirFile             = memory_pointer(0x532350).get();
    playerImgDirectory                  = ReadMemory<CDirectory*>(0x5A69ED + 1, true);
    playerImgEntries                    = ReadMemory<CDirectoryEntry*>(0x5A69E3 + 1, true);
    
    
    // Making our our code for the stream thread would make things so much better
    MakeJMP(0x406560, raw_ptr(CdStreamThread));
    
    // We need to know the next model to be read before the CdStreamRead call happens
    MakeCALL(0x40CCA6, raw_ptr(HOOK_RegisterNextModelRead));
    MakeNOP(0x40CCA6 + 5, 2);
    
    // We need to return a new hFile if the file is on disk
    MakeCALL(0x406A5B, raw_ptr(HOOK_NewFile));
    MakeNOP(0x406A5B+5, 1);
    
    
    // Hook the CStreaming::AddImageToList from the CFileLoader::LoadLevel (gta.dat reader) to open custom images in modloader folder
    make_function_hook<datcd_hook>([](datcd_hook::func_type AddImageToList, const char*& path, char& bIsNotClothesImage)
    {
        std::string normalized = NormalizePath(path);
        
        // Try to find a custom image file that has almost the same path as @path
        for(auto& img : imgPlugin->imgFiles)
        {
            // Find part of the path similar to @normalized and see if it's equal to @normalized
            std::string properly = GetProperlyPath(img.path, normalized.c_str());
            if(properly == normalized)
            {
                // Yep, we found the replacement
                path = img.path.c_str();
                break;
            }
        }
        
        // Open the image file as before
        imgPlugin->Log("Opening level image file \"%s\"", path);
        return AddImageToList(path, bIsNotClothesImage);
    });
    
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
    
    
    
    
    // We need to reimport model files for clothes body-parts
    make_function_hook<rqcloth_hook>([](rqcloth_hook::func_type RequestModel, int& index, int& type)
    {
        if(type == 18)    // Clothes object
        {
            // Tell the abstract streaming that we're about to request a model that's a cloth object
            abstract.RequestClothes(index);
        }
        return RequestModel(index, type);
    });
    
    // We should fix the directory entries form player.img to have the same size as our clothes imports
    make_function_hook<clothdir_hook>([](clothdir_hook::func_type ReadDirFile, CDirectory*& dir, int&, const char*& filename)
    {
        ReadDirFile(dir, 0, filename);
        abstract.FixClothesDirectory(*dir);
    });
    
    // Before rqspecial_hook, we need to know which model is being loaded into the special model index.
    // To do it, we'll hook the CDirectory::FindItem call and set the static var pSpecial to contain a pointer to our local model.
    make_function_hook<findspecial_hook>([](findspecial_hook::func_type FindItem, void*& dir, int&, const char*& name, int*& pOff , int*& pCount)
    {
        char result; pSpecial = nullptr;
        
        if(result = FindItem(dir, 0, name, pOff, pCount))
        {
            if(*pOff == 0)  // If the offset is 0, it may be one of our local models...
            {
                // Find model with name and put it's pointer at pSpecial
                auto it = imgPlugin->models.find(modloader::hash(std::string(name) + ".dff", ::toupper));
                pSpecial = (it == imgPlugin->models.end()? nullptr : &it->second.get());
            }
        }
        return result;
    });

    // Hook call to CStreaming::RequestModel at CStreaming::RequestSpecialChar, so we can load the new file into the abstract streaming
    make_function_hook<rqspecial_hook>([](rqspecial_hook::func_type RequestModel, int& index, int& type)
    {
        // Imports the special model into the abstract streaming index, or remove previous import at index
        abstract.RemoveOrReimport(pSpecial, index);
        
        // Do the request
        return RequestModel(index, type);
    });
    

    
    // Let's make the function CVehicleRecording::RegisterRecordingFile not re-register paths that have been already registered.
    // That will let us have a rrr at the modloader folder and the other at the carrecs.img
    make_function_hook<addrrr_hook>([](addrrr_hook::func_type RegisterRecordingFile, const char*& r3name)
    {
        // Let's see if there's already any carrec path with the specified number
        if(true)
        {
            int r3number;
            
            // Extract recording number
            if(sscanf(r3name, "carrec%d", &r3number) || sscanf(r3name, "CARREC%d", &r3number))
            {
                // Check out if this number is already created
                for(int i = 0; i < *CVehicleRecording__NumPlayBackFiles; ++i)
                {
                    if(CVehicleRecording__StreamingArray[i * 4] == r3number)
                        return i;    // Returning previosly registered recording
                }
            }
        }
        
        // Return newly registered recording
        return RegisterRecordingFile(r3name);
    });
    

    // Let's make the function CStreamedScript::RegisterScript check if the script has been already registered.
    // That will let us have a scm at the modloader folder and the other at the script.img
    make_function_hook<addscm_hook>([](addscm_hook::func_type RegisterScript, void*& self, int&, const char*& name)
    {
        int script = FindStreamedScript(self, 0, name);
        if(script != -1) return script;
        return RegisterScript(self, 0, name);
    });
    
    // The CColStore class does not register the col names so it can't tell if the col was
    // registered before or not. Let's fix it, at CColStore::AddColSlot itself
    make_function_hook<addcol_hook>([](addcol_hook::func_type AddColSlot, const char*& name)
    {
        static std::map<std::string, int> store;
        
        // Find lower case name for comparision
        std::string lname = name;
        modloader::tolower(lname);
        
        // Check if col filename has been already registered
        auto it = store.find(lname);
        if(it != store.end()) return it->second;

        // Register col filename and tell the game to add it into CColStore
        return store.emplace(std::move(lname), AddColSlot(name)).first->second;
    });
    
    
        
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
        typedef function_hooker<0x5B6183, void*(const char*, const char*)>  ldcd_hook;
        
        size_t nopcount = injector::address_manager::singleton().IsSteam()? 0xC : 0xA;
        
        MakeNOP(0x406886, nopcount);
        MakeCALL(0x406886, raw_ptr(HOOK_SetStreamName));
        
        MakeNOP(0x407642, nopcount);
        MakeCALL(0x407642, raw_ptr(HOOK_SetImgDscName));
        
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


