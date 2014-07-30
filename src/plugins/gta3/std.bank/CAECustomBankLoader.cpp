/*
 * Copyright (C) 2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 *  CAECustomBankLoader
 *  This is a custom bank loader for GTA San Andreas. The differences between the original and this are:
 *      [*] Dynamically allocated sound buffers instead of BankSlot.dat pre-allocation.
 *      [*] Dedicated thread to load banks
 *      [*] Capable of reading bank dumps split on the diskse
 *      [*] Capable of reading wave files on the disk
 * 
 *  Before you ask if BankSlot.dat pre-allocation isn't faster...
 *  No, it isn't because the game ends up allocating a temporary buffer of memory when reading the bank from the SFXPak.
 * 
 */
#include "CWavePCM.hpp"
#include "CAEBankLoader.h"
#include "Queue.h"
#include "modloader/util/path.hpp"
#include <modloader/util/injector.hpp>
#include <string>
#include <list>
#include <map>
using namespace modloader;

#include "CAECustomBankLoader.hpp"

// Request status
enum
{
    REQUEST_STATUS_NULL         = 0,
    REQUEST_STATUS_BEGIN        = 1,
    REQUEST_STATUS_CUSTOM       = 100,
    REQUEST_STATUS_IN_PROGRESS,
    REQUEST_STATUS_DONE
};

CAbstractBankLoader banker;

// Bank loading thread
static Queue queue;                                 // Request queue
static HANDLE hSemaphore;                           // Thread semaphore
static HANDLE hThread;                              // Thread handle
static DWORD __stdcall BankLoadingThread(void*);    // Thread body

// Bank information for lookup so there's no need to peek the SFXPak for the bank header
static class CAEBankInfo* pBankInfo;

static HANDLE OpenForReading(LPCSTR lpFilename, DWORD dwFlags)
{
    return CreateFileA(lpFilename, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, dwFlags, nullptr);
}

static bool IsValidHandle(HANDLE hFile)
{
    return hFile != 0 && hFile != INVALID_HANDLE_VALUE;
}


struct CAEBankHeader
{
    public:
        friend class CAEBankInfo;
        BankHeader          m_Header;
        CAEBankLookupItem*  m_pLookup;

    public:
        unsigned short GetNumSounds()
        {
            return m_Header.m_nSounds;
        }
        
        // Gets the bank header offset in the file
        unsigned int GetBankOffset()
        {
            return m_pLookup->m_dwOffset;
        }
        
        // Gets the bank sound buffer offset in the file
        unsigned int GetSoundOffset()
        {
            return GetBankOffset() + sizeof(BankHeader);
        }
        
        unsigned int GetSoundOffsetRaw(unsigned short usSound)
        {
            return m_Header.m_aSounds[usSound].m_dwOffset;
        }
        
        // Gets the bank sound buffer offset for the sound id @usSound in the file
        unsigned int GetSoundOffset(unsigned short usSound)
        {
            return GetSoundOffset() + GetSoundOffsetRaw(usSound);
        }
        
        // Gets the bank sound buffer size
        unsigned int GetSoundSize()
        {
            return m_pLookup->m_dwSize;
        }
        
        // Gets the sound buffer size for the sound in @usSound
        unsigned int GetSoundSize(unsigned short usSound)
        {
            short nSoundsOnBank = this->m_Header.m_nSounds;
            
            // If it's the last sound on the bank we should use the bank size to find the 'diff'
            if(usSound >= nSoundsOnBank - 1)    // Diff between bank size and this sound offset
                return m_pLookup->m_dwSize - m_Header.m_aSounds[usSound].m_dwOffset;
            else                                // Diff between next sound offset and this sound offset
                return m_Header.m_aSounds[(usSound+1) % 400].m_dwOffset - m_Header.m_aSounds[usSound].m_dwOffset;
        }
        
        void* AllocateBankSlot(CAEBankSlot& b, CAESoundRequest& r, unsigned int& dwOffset, unsigned int& dwSize)
        {
            unsigned short usSound = r.m_usSound;
            bool bSingleSound = usSound != 0xFFFF;
            
            // Setup output data
            void* pBuffer = r.m_pBuffer;
            dwOffset = bSingleSound? GetSoundOffset(usSound) : GetSoundOffset();
            dwSize   = bSingleSound? GetSoundSize(usSound)   : GetSoundSize();

            // Cleanup the old buffer
            if(pBuffer) operator delete(pBuffer);
            // Allocate new sound buffer
            pBuffer = operator new(dwSize);
            
            // Setup the bankslot data
            b.m_dwSlotBufferSize = dwSize;
            b.m_nSoundsOnBank    = GetNumSounds();
            r.m_pBuffer          = pBuffer;
            r.m_pBufferData      = m_Header.m_aSounds;
            
            return pBuffer;
        }
};


/*
 *  CAEBankInfo
 *      Stores information from a bank in memory, including it's filepath.
 */
class CAEBankInfo
{
    protected:
        friend DWORD __stdcall BankLoadingThread(void*);
        friend class CAECustomBankLoader;

        std::string           m_szPakName;
        std::string           m_szFilepath; // BANK file
        bool                  m_bHasBank;   // Structure has been initialized?
        short                 m_BankId;     // Global bank index
        short                 m_LocalBankId; // Local bank index (relative to SFX Pak first bank index -- 1 based)
        CAEBankLookupItem     m_OriginalLookup;
        CAEBankHeader         m_OriginalHeader;
        CAEBankHeader         m_VirtualHeader;
        
    public:
        CAEBankInfo() : m_BankId(-1)
        {}

        // Loads header and other information from the BANK file
        bool FetchBankFile(CAEBankLookupItem* pLookup, short usBankId, short usLocalBankId,
                         std::string szPakName, size_t dwOffset, size_t dwSize);
        
        // Calculates the offsets for m_VirtualHeader
        void ProcessVirtualBank();
};

















































/*
 *  CAECustomBankLoader
 *      Custom bank loader for the game
 */
class CAECustomBankLoader : public CAEBankLoader
{
    public:
        struct SPlan {
            char*           pBuffer;
            unsigned int    dwOffset;
            unsigned int    dwSize;
            const modloader::file*     pFile;
            HANDLE          hFile;
        };
        
        static void Patch();        // Patch the game code to use our custom bank loader
        
        bool PostInitialise();      // Initialise stuff
        void Finalize();            // Finalizes stuff
        void Service();             // Custom bank loading methods

        void InitialiseBankInfo();  // Initialise bank information pre-allocation
        
        void LoadRequest(int i);
        void LoadRequestLinear(int i);
        void LoadRequestSplit(int i);
        
        SPlan** GetRequestPlan(CAESoundRequest&);
        

        bool InitialiseThread();

        
        // Get SFXPak filename from it's index
        const char* GetPakName(unsigned char i)
        {
            return &this->m_pPakFiles[52 * i];
        }
        
        int FindSFXPak(const char* filename)
        {
            for(int i = 0; i < this->m_iNumPakFiles; ++i)
            {
                if(!modloader::strcmp(filename, GetPakName(i), false))
                    return i;
            }
            return -1;
        }
        
};

static_assert(sizeof(CAECustomBankLoader) == sizeof(CAEBankLoader), "Invalid size of CAECustomBankLoader");

void CAbstractBankLoader::Initialise(CAECustomBankLoader& AEBankLoader)
{
    this->m_pBankLoader = &AEBankLoader;
    this->m_bHasInitialized = true;
    this->m_GENRL = modloader::hash("genrl");

    std::vector<size_t> hashes;
    for(int i = 0; i < AEBankLoader.m_iNumPakFiles; ++i)
    {
        auto hash = modloader::hash(AEBankLoader.GetPakName(i), ::tolower);
        hashes.emplace_back(hash);
        this->m_Waves[hash];                // Create map key with this pak file
    }

    if(std::count(hashes.begin(), hashes.end(), m_GENRL) == 0)
    {
        // TODO WARNING
    }

    auto& GENRL = m_Waves[m_GENRL];

    for(auto it = m_Waves.begin(); it != m_Waves.end(); )
    {
        if(std::count(hashes.begin(), hashes.end(), it->first) == 0)
        {
            std::move(it->second.begin(), it->second.end(), std::inserter(GENRL, GENRL.begin()));
            it = m_Waves.erase(it);
            // TODO WARNING PAK DOESNT EXIST
        }
        else ++it;
    }
}



/*
 *  CAECustomBankLoader::PostInitialise
 *      Initialises the custom bank loader....
 *      This is called after the standard bank loader gets initialized.
 */
bool CAECustomBankLoader::PostInitialise()
{
    if(this->InitialiseThread())
    {
        // Our custom bank loader do not perform pre-allocation.....
        // We use CAEBankSlot::m_dwOffsetOnBuffer with the actual allocated memory pointer and things will work!
        this->m_iSoundBuffersSize = 0;
        this->m_pSoundBuffers = nullptr;

        // Cleanup CAEBankSlot pre-allocation information
        for(int i = 0; i < this->m_usNumBankSlots; ++i)
        {
            // Those need to be set when you allocate your own buffer memory
            this->m_pBankSlots[i].m_dwOffsetOnBuffer = 0;
            this->m_pBankSlots[i].m_dwSlotBufferSize = 0;
        }

        // Pre-allocate / store bank information, so we don't have to fetch it every time we need to load a bank or sound
        pBankInfo = new CAEBankInfo[this->m_usNumBanks];
        this->InitialiseBankInfo();

        banker.Initialise(*this);
        return true;
    }
    return false;
}

/*
 *  CAECustomBankLoader::InitialiseThread
 *      Initialises the custom bank worker thread....
 *      By default the game uses CdStream thread to process banks, we're separating it right now.
 */
bool CAECustomBankLoader::InitialiseThread()
{
    // Initialise bank loading thread
    if(hSemaphore = CreateSemaphoreA(nullptr, 0, 50 + 1, "BankLoaderSem"))
    {
        hThread = CreateThread(nullptr, 0, BankLoadingThread, this, CREATE_SUSPENDED, nullptr);
        if(hThread)
        {
            // Make the loading thread have the same priority as the main thread
            SetThreadPriority(hThread, GetThreadPriority(GetCurrentThread()));
            ResumeThread(hThread);
        }
        else
        {
            CloseHandle(hSemaphore);
            return false;
        }
    }
    else
        return false;

    // Initialise bank loading queue
    InitialiseQueue(&queue, 50 + 1);
    return true;
}

/*
 *  CAECustomBankLoader::InitialiseBankInfo
 *      Fetches all bank files information into CAEBankInfo structure
 */
void CAECustomBankLoader::InitialiseBankInfo()
{
    std::map<std::string, uint32_t> localBankId;
    std::string pakname;

    for(int i = 0; i < this->m_usNumBanks; ++i)
    {
        auto* lookup = &this->m_pBankLookup[i];
        pakname = GetPakName(lookup->m_iPak);
        tolower(pakname);

        // Read the bank header from the SFXPak file
        if(pBankInfo[i].FetchBankFile(lookup, i, ++localBankId[pakname],
            pakname.data(),
            lookup->m_dwOffset, lookup->m_dwSize) == false)
        {
            plugin_ptr->Log("Warning: Failed to fetch bank file %s", GetPakName(lookup->m_iPak));
        }
    }
}

/*
 *  CAECustomBankLoader::Finalize
 *      Finalizes all resources that the custom loader owns
 *      This is called before the standard bank loader gets destroyed
 */
void CAECustomBankLoader::Finalize()
{
    // Cleanup resources
    CloseHandle(hThread);
    CloseHandle(hSemaphore);
    FinalizeQueue(&queue);
    delete[] pBankInfo;

    // Destroy any sound buffer still allocated
    for(int i = 0; i < this->m_usNumBankSlots; ++i)
    {
        operator delete((void*)(this->m_pBankSlots[i].m_dwOffsetOnBuffer));
        this->m_pBankSlots[i].m_dwOffsetOnBuffer = 0;
    }
}



/*
 *  CAEBankInfo::FetchBankFile
 *      Fetches information about a specific bank, from a bank header in a bank file.
 */
bool CAEBankInfo::FetchBankFile(CAEBankLookupItem* pLookup, short usBankId, short usLocalBankId,
                                std::string szPakName, size_t dwOffset, size_t dwSize)
{
    bool result = false;
    std::string szFullPath = banker.GetSfxPakFullPath(szPakName.c_str());

    // Open the file to check if it exists and to read the bank header
    HANDLE hFile = CreateFileA(szFullPath.data(), GENERIC_READ, FILE_SHARE_READ, nullptr,
                               OPEN_EXISTING, 0, nullptr);

    if(hFile != INVALID_HANDLE_VALUE)
    {
        OVERLAPPED ov = {0};
        ov.Offset     = dwOffset;
     
        // Read the bank header
        if(ReadFile(hFile, &this->m_OriginalHeader.m_Header, sizeof(BankHeader), 0, &ov))
        {
            result = true;

            // MiniBanks (custom format) size is the entire file size
            if(dwSize == -1) dwSize = GetFileSize(hFile, 0) - sizeof(BankHeader);

            // Setup information about this bank......
            this->m_szFilepath  = std::move(szFullPath);
            this->m_szPakName   = szPakName;
            pLookup->m_dwOffset = dwOffset;
            pLookup->m_dwSize   = dwSize;

            // Setup custom lookups for this bank.......
            this->m_BankId                  = usBankId;
            this->m_LocalBankId             = usLocalBankId;    // 1 based
            this->m_OriginalLookup          = *pLookup;
            this->m_OriginalHeader.m_pLookup= &this->m_OriginalLookup;
            this->m_VirtualHeader.m_pLookup = pLookup;
            this->m_VirtualHeader.m_Header   = this->m_OriginalHeader.m_Header;

            // TODO move virtual setting to the virtual builder
        }
        
        CloseHandle(hFile);
    }
    
    return result;
}


/*
 *  CAEBankInfo::ProcessVirtualBank 
 *      Calculates information for the virtual bank header
 *      The virtual bank header a bank header modified to contain offsets and sizes from wave files on the disk.
 */
void CAEBankInfo::ProcessVirtualBank()
{
    static std::string fbuffer;

    // Headers
    CAEBankHeader& oh = this->m_OriginalHeader;
    CAEBankHeader& h = this->m_VirtualHeader;

    // Offset accumulator
    int accumulator = 0;
    int accumulator_bef = 0;

    // Copy original information into the virtual header
    memcpy(h.m_pLookup, oh.m_pLookup, sizeof(*h.m_pLookup));
    memcpy(&h.m_Header, &oh.m_Header, sizeof(h.m_Header));

    banker.SetWorkingBank(nullptr);

    // Do offset customization only if there's any wave on this bank
    if(auto pSounds = banker.GetSoundMap(this->m_szPakName, this->m_LocalBankId))
    {
        banker.SetWorkingBank(pSounds);

        // Iterate on the sounds information array to modify the offsets
        for(int i = 0; i < h.m_Header.m_nSounds; ++i)
        {
            auto& v = h.m_Header.m_aSounds[i];
            accumulator_bef = accumulator;

            // Check if there's a wav for this sound id
            if(auto* pSound = banker.FindSound(*pSounds, i))
            {
                if(GetFileAttributesA(pSound->file->fullpath(fbuffer).data()) != INVALID_FILE_ATTRIBUTES)
                {
                    // Accumulate the difference in size between the wave and the original sound
                    accumulator += int(pSound->sound_size - h.GetSoundSize(i));

                    // Setup the wave information on the virtual header
                    v.m_usSampleRate = pSound->sample_rate;
                }
            }

            // Add accumulator into offset
            v.m_dwOffset += accumulator_bef;
        }

        // Add accumulator into the bank size
        h.m_pLookup->m_dwSize += accumulator;
    }
}













/*
 *  CAECustomBankLoader::Service
 *      Processes the bank loading system
 */
void CAECustomBankLoader::Service()
{
    // Process sound requests
    for(int i = 0; i < 50 && this->m_nRequestsToLoad; ++i)
    {
        auto& r = this->m_aSoundRequests[i];
        unsigned short bankslot = r.m_usBankSlot;
        
        switch(r.m_iLoadingStatus)
        {
            // The request has just been sent
            case REQUEST_STATUS_BEGIN:
            {
                auto& b = this->m_pBankSlots[bankslot];
                
                // Mark the bankslot as free, we don't want anyone using it while we're touching it!
                r.m_pBuffer = (void*) b.m_dwOffsetOnBuffer;
                this->m_pBankSlots[bankslot].m_dwOffsetOnBuffer = 0;
                memset(this->m_pBankSlots[bankslot].m_aBankItems, 0, sizeof(this->m_pBankSlots[bankslot].m_aBankItems));
                this->m_pBankSlots[bankslot].m_usBankNum = 0xFFFF;
                this->m_aBankSlotSound[bankslot] = 0xFFFF;
                
                // Request the sound to the bank loading thread
                r.m_iLoadingStatus = REQUEST_STATUS_IN_PROGRESS;
                AddToQueue(&queue, i);
                ReleaseSemaphore(hSemaphore, 1, nullptr);
                
                break;
            }
            
            // The request has been completed, finish it
            case REQUEST_STATUS_DONE:
            {
                auto& b = this->m_pBankSlots[bankslot];
                
                // Mark the bankslot with the loaded bank/sound
                this->m_pBankSlots[bankslot].m_usBankNum = r.m_usBank;
                this->m_aBankSlotSound[bankslot] = r.m_usSound;
                b.m_dwOffsetOnBuffer = (uintptr_t)(r.m_pBuffer);
                memcpy(b.m_aBankItems, r.m_pBufferData, sizeof(b.m_aBankItems));
                
                // Special setup for single sounds
                if(r.m_usSound != 0xFFFF)
                {
                    b.m_aBankItems[r.m_usSound].m_dwOffset = 0;
                    b.m_aBankItems[(r.m_usSound + 1) % 400].m_dwOffset = b.m_dwSlotBufferSize;
                }
                
                // Cleanup request object
                r.m_iLoadingStatus = REQUEST_STATUS_NULL;
                r.m_usBankSlot = 0xFFFF;
                r.m_usBank = 0xFFFF;
                r.m_usSound = 0xFFFF;
                r.m_pBuffer = r.m_pBufferData = nullptr;
                --this->m_nRequestsToLoad;
                
                break;
            }
        }
    }
}






/*
 *  BankLoadingThread 
 *      Oh well, this is the hard worker that will load the bank files
 */
DWORD __stdcall BankLoadingThread(void* arg)
{
    CAECustomBankLoader& AEBankLoader = *(CAECustomBankLoader*)(arg);
    
    while(true)
    {
        // Wait for a request...
        WaitForSingleObject(hSemaphore, INFINITE);
        int i = GetFirstInQueue(&queue);
        
        // Load the request
        AEBankLoader.LoadRequest(i);

        // Done!
        RemoveFirstInQueue(&queue);
        AEBankLoader.m_aSoundRequests[i].m_iLoadingStatus = REQUEST_STATUS_DONE;
    }
    return 0;
}

/*
 *  CAECustomBankLoader::LoadRequest
 *      Load sound request at index i from the request array
 */
void CAECustomBankLoader::LoadRequest(int i)
{
    auto& r = this->m_aSoundRequests[i];
    auto& b = this->m_pBankSlots[r.m_usBankSlot];

    this->LoadRequestSplit(i);
    
    // On single sound request some data must be changed...
    if(r.m_usSound != 0xFFFF)
        b.m_nSoundsOnBank = 0xFFFF;
}


/*
 *  CAECustomBankLoader::LoadRequestSplit
 *      This request reads many files to load banks or sounds.
 *      This loading method is used when there's a custom wave in the bank.
 */
void CAECustomBankLoader::LoadRequestSplit(int i)
{
    HANDLE hFile = INVALID_HANDLE_VALUE, hBank = INVALID_HANDLE_VALUE;
    static std::string fbuffer;

    // Reads planned 'snd' information from file hFile
    auto Read = [](HANDLE hFile, const SPlan& snd)
    {
        if(IsValidHandle(hFile))
        {
            OVERLAPPED ov = {0};
            ov.Offset = snd.dwOffset;
            if(ReadFile(hFile, snd.pBuffer, snd.dwSize, 0, &ov))
                return true;
        }
        return false;
    };
    
    // Setup references for helping us, neh
    auto& r = this->m_aSoundRequests[i];
    auto& f = pBankInfo[r.m_usBank];                // The bank information

    // Process virtual wave files
    f.ProcessVirtualBank();

    // Allocate the sound buffer and get a request plan
    SPlan** pPlan = this->GetRequestPlan(r);
    
    // Execute the request plan
    for(SPlan** x = pPlan; *x; ++x)
    {
        auto& block = **x;                  // The block requested
        if(block.dwSize == 0) continue;     // Ignore this block
        
        if(block.pFile == nullptr) // Requesting from the big bank file?
        {
            if(!IsValidHandle(hBank))   // Bank not opened yet? Open it!
                hBank = OpenForReading(f.m_szFilepath.c_str(), 0);
            
            // Read from bank
            Read(hBank, block);
         }
        else
        {
            // Read from another buffer (probably a wave file)
            if((hFile = OpenForReading(block.pFile->fullpath(fbuffer).data(), 0)) != INVALID_HANDLE_VALUE)
            {
                Read(hFile, block);
                CloseHandle(hFile);
            }
            else
            {
                auto i = 0;
                i++;
                i--; // TODO REM
            }
        }
    }
 
    // Close bank file handle
    if(IsValidHandle(hBank)) CloseHandle(hBank);
}

/*
 *  CAECustomBankLoader::GetRequestPlan 
 *      Plans the best loading method for a bank or sound file
 */
auto CAECustomBankLoader::GetRequestPlan(CAESoundRequest& r) -> SPlan**
{
    // Planning arrays
    static SPlan  aPlan[400];
    static SPlan* aSortedPlan[400];
    
    unsigned short usBank       = r.m_usBank;
    unsigned short usBankSlot   = r.m_usBankSlot;
    unsigned short usSound      = r.m_usSound;
    
    unsigned int nRequests = 0;
    bool bSingleSound = usSound != 0xFFFF;

    // Adds a new plan into the planning list
    auto NewPlan = [&nRequests]
    {
        SPlan& item = aPlan[nRequests];
        aSortedPlan[nRequests++] = &item;
        return &item;
    };
    
    // Refs
    auto& f = pBankInfo[usBank];
    auto& b = this->m_pBankSlots[usBankSlot];
    auto& vh = f.m_VirtualHeader;
    auto& oh = f.m_OriginalHeader;
    auto& h  = vh;
    
    // Allocate bank slot...
    unsigned int dwOffset, dwSize;
    char* pBuffer = (char*) h.AllocateBankSlot(b, r, dwOffset, dwSize);
    
    auto pSounds = banker.GetWorkingBank();
    if(pSounds == nullptr)
    {
        // If there's no custom wave file on this bank, just read the bank normally
        SPlan& item = *NewPlan();
        item.pFile   = 0;
        item.dwOffset= dwOffset;
        item.dwSize  = dwSize;
        item.pBuffer = pBuffer;
    }
    else
    {
        // Build the basic request plan array
        for(int i = 0; i < h.GetNumSounds(); ++i)
        {
            // Add to the request plan the requested sounds
            if(!bSingleSound || i == usSound)
            {
                SPlan& item = *NewPlan();

                // Setup the buffer position and the amount of bytes to read
                item.pBuffer    = &pBuffer[bSingleSound? 0 : vh.GetSoundOffsetRaw(i)];
                item.dwSize     = vh.GetSoundSize(i);

                if(auto pSound = banker.FindSound(*pSounds, i)) // Has wave for this sound?
                {
                    // Start reading after the wave header
                    item.dwOffset   = pSound->sound_offset;
                    item.pFile      = pSound->file;
                }
                else
                {
                    // Start reading at the sound offset
                    item.dwOffset = oh.GetSoundOffset(i);
                    item.pFile = 0;
                }
            }
        }

        // If there's any request after all, do some smart processing
        if(nRequests > 0)
        {
            // Sort the request plan by file and offset
            std::sort(&aSortedPlan[0], &aSortedPlan[nRequests], [](const SPlan* a, const SPlan* b)
            {
                // The big bank file should come first and sorted by offset!
                if(a->pFile == b->pFile) return a->dwOffset < b->dwOffset;
                return(a->pFile == 0);
            });
            
            // Make all continuous block a single block
            for(int i = 0, k = nRequests - 1; i < k; ++i)
            {
                // Get pointer to the first and next block to read
                auto& curr = aSortedPlan[i];
                auto& next = aSortedPlan[i+1];

                // The next block isn't on the standard bank file? We're done
                if(next->pFile != 0) break;

                // If the next block is at the end of this block just make both a single block
                if((curr->pBuffer + curr->dwSize) == next->pBuffer
                && (curr->dwOffset + curr->dwSize) == next->dwOffset)
                {
                    // Make next the single block so the information can propagate to the next loop iterations
                    next->pBuffer  = curr->pBuffer;
                    next->dwOffset = curr->dwOffset;
                    next->dwSize  += curr->dwSize;
                    curr->dwSize = 0;               // Mark this block as ignored
                }
            }
        }
    }

    // Finish the plan array and return it
    aSortedPlan[nRequests] = 0;
    return aSortedPlan;
}






/*
 *  CAECustomBankLoader::Patch
 *      Patches the game to use this custom bank loader 
 */

// I'm going to use this little wrapper to avoid compiler warnings
static void __fastcall ServiceCaller(CAEBankLoader* loader)
{
    return static_cast<CAECustomBankLoader*>(loader)->Service();
}

// Return a null pointer
static void* ReturnNull()
{
    return 0;
}

// Patcher
void CAECustomBankLoader::Patch()
{
    typedef function_hooker_fastcall<0x4D99B3, char(CAEBankLoader*)> ihook;
    typedef function_hooker_fastcall<0x4D9800, void(CAEBankLoader*)> dhook;

    //
    MakeJMP (0x4DFE30, raw_ptr(ServiceCaller));
    MakeCALL(0x4E065B, raw_ptr(ReturnNull));    // Return null bankslot pre-allocated memory
    MakeCALL(0x4DFD9D, raw_ptr(ReturnNull));    // Return null streaming handle for SFXPak
    MakeNOP (0x4DFDC3, 5);                      // Don't free PakFiles.dat buffer
    MakeNOP (0x4DFDCE, 7);                      // ^
    
    
    // After the standard bank loader initialization, initialise our custom bank loader
    make_static_hook<ihook>([](ihook::func_type Initialise, CAEBankLoader*& loader)
    {
        char result = 0;
        if(Initialise(loader))      // Initialise CAEBankLoader
        {
            // Initialise CAECustomBankLoader
            if(static_cast<CAECustomBankLoader*>(loader)->PostInitialise()) 
            {
                result = 1;
            }
        }
        return result;
    });
    
    // Finalizes the custom bank loader
    make_static_hook<dhook>([](dhook::func_type dtor, CAEBankLoader*& loader)
    {
        static_cast<CAECustomBankLoader*>(loader)->Finalize();
        return dtor(loader);
    });
}

void DoPatch()
{
    CAECustomBankLoader::Patch();
}
