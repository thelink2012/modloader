/* 
 * Copyright (C) 2013-2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 *      Cd Directory management
 */
#include <windows.h>
#include "streaming.hpp"
#include "CDirectory.h"
#include "CDirectoryEntry.h"
#include "CdStreamInfo.h"
#include "CImgDescriptor.h"
#include <modloader/util/path.hpp>
using namespace modloader;

// Hooks and other util stuff
extern "C"
{
    DWORD *pStreamCreateFlags       = memory_pointer(0x8E3FE0).get();
    auto pStreamingBuffer           = memory_pointer(0x008E4CAC).get<void*>();
    auto& streamingBufferSize       = *memory_pointer(0x8E4CA8).get<uint32_t>();
    auto LoadCdDirectory2           = ReadRelativeOffset(0x5B8310 + 1).get<void(CImgDescriptor*, int)>();

    // Next model read registers. It's important to have those two vars! Don't leave only one!
    int  iNextModelBeingLoaded = -1;            // Set by RegisterNextModelRead, will then be sent to iModelBeingLoaded
    int  iModelBeingLoaded = -1;                // Model currently passing throught CdStreamRead

    // Returns the file handle to be used for iModelBeingLoaded (called from Assembly)
    HANDLE CallGetAbstractHandle(HANDLE hFile)
    {
        if(iModelBeingLoaded == -1) return hFile;
        return streaming.TryOpenAbstractHandle(iModelBeingLoaded, hFile);
    }

    // Registers the next model to be loaded (called from Assembly)
    void RegisterNextModelRead(int id)
    {
        iNextModelBeingLoaded = id;
        streaming.MakeSureModelIsOnDisk(id);
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
    using nf_hook = function_hooker<0x5B6183, void*(const char*, const char*)>;
    using cf_hook = function_hooker<0x5B61B8, size_t(void*, void*, size_t)>;
    using rf_hook = function_hooker<0x5B61E1, size_t(void*, void*, size_t)>;
    using sf_hook = function_hooker_thiscall<0x5B627A, void(CDirectory*, CDirectoryEntry* entry)>;
    using gf_hook = function_hooker_thiscall<0x5B6449, char(CStreamingInfo*, int*, int*)>;

    nf_hook nf; // Open Null File
    cf_hook cf; // Entry Count
    rf_hook rf; // Read Entry
    sf_hook sf; // Register Special Entry
    gf_hook gf; // Register Entry

    // Open a null but valid file handle, so it can be closed (fclose) without any problem
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
    sf = make_function_hook2<sf_hook>([&RegisterSpecialEntry](sf_hook::func_type AddItem, CDirectory*& dir, CDirectoryEntry*& entry)
    {
        // Tell the callback about this registering
        if(RegisterSpecialEntry) RegisterSpecialEntry(*entry);

        // Add entry to directory ONLY if it doesn't exist yet
        if(injector::thiscall<CDirectoryEntry*(void*, const char*)>::call<0x532450>(dir, entry->filename) == nullptr)  // CDirectory::FindItem
            AddItem(dir, entry);
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
        auto filename = &filepath[GetLastPathComponent(filepath)];

        // Check twice, the first time check for path equality, the second time for filename equality
        for(int i = 0; i < 2 && !bBreak; ++i)
        {
            for(auto& file : this->imgFiles)
            {
                auto& cdpath    = (i == 0? filepath : filename);                    // Sent filepath/filename
                fpath           = (i == 0? file->FilePath() : file->FileName());    // Custom filepath/filename

                // Check if the ending of cdpath is same as fpath
                if(fpath.length() >= cdpath.length() && std::equal(cdpath.rbegin(), cdpath.rend(), fpath.rbegin()))
                {
                    // Yeah, let's override!!!
                    filepath_ = file->FileBuffer();
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
    auto it = streaming.imports.find(index);
    if(it != streaming.imports.end())
    {
         // Don't use our custom model if we're falling back to the original file because of an error
        if(it->second.isFallingBack == false)
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
void CAbstractStreaming::LoadCdDirectories(TempCdDir_t& cd_dir)
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
            // TODO (rmber fileoffset!)
        };

        // Register standard entries
        auto RegisterEntry = [&](CStreamingInfo& model, bool hadModel)
        {
            // If it's going to register the model (newly!), let's register on abstract streaming too
            if(!hadModel && entry)  
            {
                auto index = InfoForModelIndex(model);
                RegisterModelIndex(curr->filename, index);
                RegisterStockEntry(curr->filename, *entry, index, id);
            }
            return hadModel;
        };

        // Read this cd directory using the above callbacks
        PerformDirectoryRead(cd.second.size(), std::bind(LoadCdDirectory2, nullptr, id), ReadEntry, RegisterSpecialEntry, RegisterEntry);
    }
}


/*
 *  CAbstractStreaming::LoadAbstractCdDirectory
 *      Loads a abstract (fake) custom cd directories related to our disk files @files
 */
void CAbstractStreaming::LoadAbstractCdDirectory(ref_list<const modloader::file*> files)
{
    // TODO LOG Loading abstract cd directory... / Abstract cd directory has been loaded

    // Setup iterators
    auto it = files.begin();
    auto curr = it;
    
    // We need those temp vars, don't even dare to remove them
    auto realStreamingBufferSize = bHasInitializedStreaming? streamingBufferSize * 2 : streamingBufferSize;
    this->tempStreamingBufSize = realStreamingBufferSize;

    // Streaming bus must be empty before this operation
    if(this->bHasInitializedStreaming)
        this->FlushChannels();


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
    auto RegisterSpecialEntry = [&](CDirectoryEntry& entry)
    {
        // TODO (rmber fileoffset!)
        // TODO REGISTER INSTEAD OF RIGHT EMPLACING
        special_dir.emplace(modloader::hash(entry.filename, ::tolower), curr->get());
    };

    // Be aware of existence of standard entries here
    auto RegisterEntry = [&](CStreamingInfo& model, bool hasModel)
    {
        return GenericRegisterEntry(model, hasModel, curr->get());
    };

    // Read the custom cd directory
    PerformDirectoryRead(files.size(), std::bind(LoadCdDirectory2, nullptr, AbstractImgId), ReadEntry, RegisterSpecialEntry, RegisterEntry);

    // Right, so, if the streaming has already initialized we may need to resize the streaming buffer because of the recently
    // inserted entries!
    if(this->bHasInitializedStreaming)
    {
        static const uint32_t align = 2048;

        // Make sure the buffer size is even (we'll divide it by two)
        if(this->tempStreamingBufSize & 1) ++this->tempStreamingBufSize;

        // If the necessary streaming buffer to load that file is bigger than the current buffer size, let's resize the buffer
        if(tempStreamingBufSize > realStreamingBufferSize)
        {
            plugin_ptr->Log("Reallocating streaming buffer from %u bytes to %u bytes.",
                realStreamingBufferSize * align,
                tempStreamingBufSize * align);

            // Reallocate the buffer
            injector::cstd<void(void*)>::call<0x72F4F0>(pStreamingBuffer[0]);                                       // CMemoryMgr::FreeAlign
            auto* mem = injector::cstd<void*(size_t, size_t)>::call<0x72F4C0>(tempStreamingBufSize * align, align); // CMemoryMgr::MallocAlign

            // Reassign buffer variables
            streamingBufferSize = tempStreamingBufSize / 2;
            pStreamingBuffer[0] = mem;
            pStreamingBuffer[1] = (raw_ptr(mem) + (align * streamingBufferSize)).get();
        }
        else
        {
            // Fix the streaming buffer size variable, it was previosly broken by the LoadCdDirectory we just used
            streamingBufferSize = realStreamingBufferSize / 2;
        }
    }
}


void CAbstractStreaming::GenericReadEntry(CDirectoryEntry& entry, const modloader::file* file)
{
    // TODO

    strncpy(entry.filename, file->FileName(), sizeof(entry.filename));
    entry.fileOffset = 0;
    entry.sizePriority2 = GetSizeInBlocks(file->Size());
    entry.sizePriority1 = 0;

    if(entry.sizePriority2 > this->tempStreamingBufSize)
        tempStreamingBufSize = entry.sizePriority2;
}

bool CAbstractStreaming::GenericRegisterEntry(CStreamingInfo& model, bool hasModel, const modloader::file* file)
{
    this->QuickImport(InfoForModelIndex(model), file);
    return true;
}
