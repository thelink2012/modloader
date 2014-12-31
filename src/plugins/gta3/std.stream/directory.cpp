/* 
 * Standard Streamer Plugin for Mod Loader
 * Copyright (C) 2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#include <stdinc.hpp>
#include "streaming.hpp"
using namespace modloader;

// Hooks and other util stuff
extern "C"
{
    auto pStreamingBuffer           = memory_pointer(0x8E4CAC).get<void*>();
    auto& streamingBufferSize       = *memory_pointer(0x8E4CA8).get<uint32_t>();
    auto LoadCdDirectory2           = ReadRelativeOffset(0x5B8310 + 1).get<void(const char*, int)>();
    CDirectory* clothesDirectory    = ReadMemory<CDirectory*>(lazy_ptr<0x5A419B>(), true);
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
    std::function<bool(DirectoryInfo&)> ReadEntry,
    std::function<void(DirectoryInfo&)> RegisterSpecialEntry  = nullptr,
    std::function<bool(CStreamingInfo&, bool)> RegisterEntry    = nullptr
    )
{
    using nf_hook = function_hooker<0x5B6183, void*(const char*, const char*)>;
    using cf_hook = function_hooker<0x5B61B8, size_t(void*, void*, size_t)>;
    using rf_hook = function_hooker<0x5B61E1, size_t(void*, void*, size_t)>;
    using sf_hook = function_hooker_thiscall<0x5B627A, void(CDirectory*, DirectoryInfo* entry)>;
    using gf_hook = function_hooker_thiscall<0x5B6449, char(CStreamingInfo*, int*, int*)>;

    nf_hook nf; // Open Null File
    cf_hook cf; // Entry Count
    rf_hook rf; // Read Entry
    sf_hook sf; // Register Special Entry
    gf_hook gf; // Register Entry

    // Open a null but valid file handle, so it can be closed (fclose) without any problem
    nf = make_function_hook<nf_hook>([](nf_hook::func_type OpenFile, const char*&, const char*& mode)
    {
        return OpenFile(modloader::szNullFile, mode);
    });

    if(gvm.IsSA())  // Only SA needs such thing as a counter, III/VC uses EOF as end of entries indicator
    {
        cf = make_function_hook<cf_hook>([&size](cf_hook::func_type, void*&, void*& buf, size_t&)
        {
            *reinterpret_cast<size_t*>(buf) = size;
            return sizeof(size_t);
        });
    }

    // Fills a directory entry
    rf = make_function_hook<rf_hook>([&ReadEntry](rf_hook::func_type, void*&, void*& buf_, size_t&)
    {
        auto& buf = *reinterpret_cast<DirectoryInfo*>(buf_);
        if(!ReadEntry(buf))
        {
            // Avoid this getting read by the game
            buf.m_szFileName[0] = 'A'; buf.m_szFileName[1] = '.'; buf.m_szFileName[2] = '\0';
            return size_t(0);
        }
        return sizeof(buf);
    });

    // Registers a special entry
    sf = make_function_hook<sf_hook>([&RegisterSpecialEntry](sf_hook::func_type AddItem, CDirectory*& dir, DirectoryInfo*& entry)
    {
        // Tell the callback about this registering
        if(RegisterSpecialEntry) RegisterSpecialEntry(*entry);

        // Add entry to directory ONLY if it doesn't exist yet
        if(injector::thiscall<DirectoryInfo*(void*, const char*)>::call<0x532450>(dir, entry->m_szFileName) == nullptr)  // CDirectory::FindItem
            AddItem(dir, entry);
    });

    // Registers a normal entry
    gf = make_function_hook<gf_hook>([&RegisterEntry](gf_hook::func_type GetCdPosnAndSize, CStreamingInfo*& model, int*& pOffset, int*& pSize)
    {
        char r = GetCdPosnAndSize(model, pOffset, pSize);
        if(RegisterEntry) r = (char) RegisterEntry(*model, !!r);    // Override function result
        return r;
    });

    // Run the cd directory reader
    Run();
}


/*
 *  CAbstractStreaming::FetchCdDirectories
 *      Fetches (but do not load) the cd directories into @cd_dir
 */
void CAbstractStreaming::FetchCdDirectories(TempCdDir_t& cd_dir, std::function<void()> LoadCdDirectories)
{
    using namespace std::placeholders;
    typedef function_hooker<0x5B8310, void(const char*, int)> fetchcd_hook;
    auto fetcher = make_function_hook<fetchcd_hook>(std::bind(&CAbstractStreaming::FetchCdDirectory, this, std::ref(cd_dir), _2, _3));
    return LoadCdDirectories();
}


/*
 *  CAbstractStreaming::FetchCdDirectory
 *      Fetches (but do not load) the cd directory file at @descriptor=>name into @cd_dir
 */
void CAbstractStreaming::FetchCdDirectory(TempCdDir_t& cd_dir, const char*& filename_, int id)
{
    static bool isSAMP = gvm.IsSA() && GetModuleHandleA("samp");
    auto filename = GetCdStreamPath(filename_);
    auto fopen  = std::fopen;
    auto fread  = std::fread;
    auto fclose = std::fclose;

    // SAMP does something in the backs of fopen/fread/fclose, for example it opens some dummy script.img when requested
    // to open it, so it reads dummy scripts. Well, let's follow SAMP, use original game funcs.
    if(isSAMP)
    {
        // SAMP works in 1.0US only, so we don't care about address translations here
        fopen  = raw_ptr(0x8232D8).get();   // _fopen
        fread  = raw_ptr(0x823521).get();   // _fread
        fclose = raw_ptr(0x82318B).get();   // _fclose
    }

    // Do actual work
    if(FILE* f = fopen(filename, "rb"))
    {
        uint32_t count = -1;
        DirectoryInfo entry;      // Works for III/VC and SA!!!!

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
            if(entry.m_usLightSize != 0)
            {
                entry.m_usSize = entry.m_usLightSize;
                entry.m_usLightSize = 0;
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
        DirectoryInfo* entry = nullptr;
        
        // Fills the entry buffer with the current iterating entry (also sets ^entry with the current entry)
        auto ReadEntry = [&](DirectoryInfo& buf)
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

        // Register standard entries
        auto RegisterEntry = [&](CStreamingInfo& model, bool hadModel)
        {
            // If it's going to register the model (newly!), let's register on our abstract streaming too
            if(!hadModel && entry)  
            {
                auto index = InfoForModelIndex(model);
                RegisterModelIndex(curr->m_szFileName, index);
                RegisterStockEntry(curr->m_szFileName, *entry, index, id);
            }
            return hadModel;
        };

        // Read this cd directory using the above callbacks
        PerformDirectoryRead(cd.second.size(), std::bind(LoadCdDirectory2, nullptr, id), ReadEntry, nullptr, RegisterEntry);
    }
}


/*
 *  CAbstractStreaming::LoadAbstractCdDirectory
 *      Loads a abstract (fake) custom cd directories related to our disk files @files
 */
void CAbstractStreaming::LoadAbstractCdDirectory(ref_list<const modloader::file*> files)
{
    plugin_ptr->Log("Loading abstract cd directory...");

    // Setup iterators
    auto it = files.begin();
    auto curr = it;
    
    // Updates the streaming buffer size after the addition of new streaming items
    StreamingBufferUpdater bufup;

    // Streaming bus must be empty before this operation
    if(this->bHasInitializedStreaming)
        this->FlushChannels();

    // Fill entry buffer and advance iterator
    auto ReadEntry = [&](DirectoryInfo& entry)
    {
        if(it != files.end())
        {
            // Fill the entry with this file information
            FillDirectoryEntry(entry, it->get()->filename(), 0, it->get()->size);

            // Check out if we're in the bounds of the streaming buffer
            bufup.AddItem(entry.m_usSize);

            // Advance........
            curr = it++;
            return true;
        }
        return false;
    };

    // Be aware of existence of special entries here on the abstract directory
    auto RegisterSpecialEntry = [&](DirectoryInfo& entry)
    {
        special.emplace(mhash(entry.m_szFileName), curr->get());
    };

    // Be aware of existence of standard entries here
    auto RegisterEntry = [&](CStreamingInfo& model, bool hasModel)
    {
        this->QuickImport(InfoForModelIndex(model), curr->get());
        return true;
    };

    // Read the custom cd directory
    PerformDirectoryRead(files.size(), std::bind(LoadCdDirectory2, nullptr, 0), ReadEntry, RegisterSpecialEntry, RegisterEntry);
    plugin_ptr->Log("Abstract cd directory has been loaded.");

    // StreamingBufferUpdater will reallocating the streaming on destruction
}






/*
 *  CAbstractStreaming::BuildClothesMap
 *      Builds a reference map of models that are clothing items
 *      This should be called while the streaming hasn't initialized 
 */
void CAbstractStreaming::BuildClothesMap()
{
    if(gvm.IsSA())  // Only San Andreas has clothing items
    {
        // Load clothes cd directory to perform search on it
        this->cloth_dir_sparse_start = 0;
        this->clothes_map.clear();
        injector::cstd<void()>::call<0x5A4190>();       // CClothesBuilder::LoadCdDirectory

        // Build clothes map based on clothes directory
        for(auto i = 0u; i < clothesDirectory->m_dwCount; ++i)
        {
            auto& entry = clothesDirectory->m_pEntries[i];
            this->RegisterClothingItem(entry.m_szFileName, i);

            // Find highest file offset in the clothes directory
            if(entry.m_dwFileOffset > cloth_dir_sparse_start)
                cloth_dir_sparse_start = entry.m_dwFileOffset;
        }

        // Setup vars for sparse entries
        ++this->cloth_dir_sparse_start;
        this->cloth_dir_sparse_curr = this->cloth_dir_sparse_start;

        // Remove items from raw_models into clothes
        if(!this->bHasInitializedStreaming) // Should always evaluate to TRUE!!!!
        {
            StreamingBufferUpdater bufup;
            for(auto it = this->raw_models.begin(); it != raw_models.end(); )
            {
                if(this->IsClothes(it->second))
                {
                    // Take from raw_models and put into clothes
                    if(this->ImportCloth(it->second))
                        bufup.AddItem(GetSizeInBlocks(it->second->size));
                    it = raw_models.erase(it);
                }
                else
                    ++it;
            }
        }
        else
        {
            plugin_ptr->Log("Warning: CAbstractStreaming::BuildClothesMap called after streaming initialization");
        }
    }
}

/*
 *  CAbstractStreaming::FindClothEntry
 *      Finds clothing item entry from hash
 */
DirectoryInfo* CAbstractStreaming::FindClothEntry(hash_t hash)
{
    auto it = this->clothes_map.find(hash);
    if(it != clothes_map.end()) return &clothesDirectory->m_pEntries[it->second];
    return nullptr;
}

/*
 *  CAbstractStreaming::FixClothesDirectory
 *      Fixes the clothing directory to contain proper references to our custom clothings
 */
void CAbstractStreaming::FixClothesDirectory()
{
    // Add our sparse entries back to the directory
    for(auto& entry : this->sparse_dir_entries)
        injector::thiscall<void(CDirectory*, DirectoryInfo*)>::call<0x532310>(clothesDirectory, &entry);  // CDirectory::AddItem
}




/*
 *  CAbstractStreaming::StreamingBufferUpdater::StreamingBufferUpdater
 *      Setups the streaming buffer updater
 */
CAbstractStreaming::StreamingBufferUpdater::StreamingBufferUpdater()
{
    // We need those temp vars, don't even dare to remove them
    this->realStreamingBufferSize = streaming.bHasInitializedStreaming? streamingBufferSize * 2 : streamingBufferSize;
    this->tempStreamingBufSize    = realStreamingBufferSize;
}

/*
 *  CAbstractStreaming::StreamingBufferUpdater::~StreamingBufferUpdater
 *      Updates the streaming buffer
 */
CAbstractStreaming::StreamingBufferUpdater::~StreamingBufferUpdater()
{
    // Right, so, if the streaming has already initialized we may need to resize the streaming buffer because of the recently inserted entries!
    if(streaming.bHasInitializedStreaming)
    {
        static const uint32_t align = 2048;

        // Make sure the buffer size is even ('cuz we'll divide it by two)
        if(tempStreamingBufSize & 1) ++tempStreamingBufSize;

        // If the necessary streaming buffer to load that file is bigger than the current buffer size, let's resize the buffer
        if(tempStreamingBufSize > realStreamingBufferSize)
        {
            // Streaming bus must be empty before reallocating
            streaming.FlushChannels();

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
    else
    {
        // Streaming not initialized, we can just set the streamingBufferSize variable and let the game take care of the rest
        if(tempStreamingBufSize > streamingBufferSize)
            streamingBufferSize = tempStreamingBufSize;
    }
}
