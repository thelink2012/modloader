/* 
 * std-img -- Standard IMG Loader Plugin for San Andreas Mod Loader
 * Copyright (C) 2013  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 * Streaming replacements for the game
 * 
 */
#include "img.h"
#include "Injector.h"
#include <GTA_STREAM.h>
#include <SImgGTAItemInfo.h>
#include <CDirectory.h>
#include <CDirectoryEntry.h>
#include <vector>


int iLoadingObjectIndex = -1;

extern "C" CDirectory* playerImgDirectory;
extern "C" void ReadPlayerImgEntries();
extern "C" void BuildPlayerFilesList(CDirectoryEntry* entries, size_t numEntries);
extern "C" void* CStreaming__ReadImgContents;
extern "C" SImgGTAItemInfo* ms_aInfoForModel;
extern "C" DWORD* pStreamCreateFlags;
extern "C" void HOOK_RegisterModelIndex();
extern "C" void (*ReadImgContent)(void* imgEntry, int imgId);



/* Critical Section object wrapper for RAII
 * We could use C++11 thread facilities, but hey, use it just for this simple task?
 */
struct CriticalSection
{
    CRITICAL_SECTION cs;
    
    /* Initialize on ctor, Delete on dtor */
    CriticalSection()
    { InitializeCriticalSection(&cs); }
    ~CriticalSection()
    { DeleteCriticalSection(&cs); }
};

struct CriticalLock
{
    CriticalSection* c;
    
    /* Enter on ctor, Leave on dtor */
    CriticalLock(CriticalSection& cs)
    { c = &cs; EnterCriticalSection(&c->cs); }
    ~CriticalLock()
    { LeaveCriticalSection(&c->cs); }
};

/* Util to replace and unreplace the calls */
static void (*ReplaceCall)(size_t, void*, void*, bool) = [](size_t addr, void* backup, void* newFunc, bool bReplace)
{
    if(!addr) return;
    
    /* If replace, puts a call (or NOPs if newFunc is nullptr */
    if(bReplace)
    {
        ReadMemoryRaw(addr, backup, 5, true);
        if(newFunc) MakeCALL(addr, newFunc); else MakeNOP(addr, 5);
    }
    /* It is restore */
    else
    {
        WriteMemoryRaw(addr, backup, 5, true);
    }
};
    
/* RAII to ReplaceCall function */
struct ReplacedCall
{
    char backup[5];
    size_t addr;
        
    /* Replace on ctor, restore on dtor */
    
    ReplacedCall(size_t addr, void* newFunc) :
        addr(addr)
    {
        ReplaceCall(addr, backup, newFunc, true);
    }
        
    ~ReplacedCall()
    {
        ReplaceCall(addr, backup, 0, false);
    }
};





/*
 * 
 */
static CriticalSection cs;                  /* use to lock on openFileList modifications */
static std::vector<HANDLE> openFileList;    /* list of open files */

/* Auxiliar iterator to ImportImgContents */
static CThePlugin::ImgInfo::imgFiles_t::iterator ImportImgContents_it;


/*
 *  ImportImgContents
 *      Injects all our handled files into game data structures (ms_aInfoForModel)
 *      NOTE: This function shall be called only by the main thread of the game!
 */
extern "C" void ImportImgContents()
{
    /* Before anything, build our player.img files list */
    ReadPlayerImgEntries();
    BuildPlayerFilesList((CDirectoryEntry*)(playerImgDirectory->m_pEntries),
                         playerImgDirectory->m_dwCount);
    
    typedef int (*fRead_t)(void* dummy, void* buf, size_t sz);
    imgPlugin->Log("\nImportImgContents()");
    
    /* Those are static because they will be accessed by other static (hooks) functions in this scope */
    static bool bFirstTime = true;
    static CThePlugin::ImgInfo* imgInfo;
    static CThePlugin::ImgInfo::imgFiles_t::iterator it;
   
    /* Don't call me twice */
    if(bFirstTime) bFirstTime = false;
    else return; 

    imgInfo = &imgPlugin->mainContent;
    if(!imgInfo->isReady) return; /* No replacement files */
    
    /* Initialize iterator */
    it = imgInfo->imgFiles.begin();
    
    /*
     *  Hooks ReadImgContents call to 'fopen' to return a valid (well, fake) handle
     */
    static void* (*OpenNoFile)() = []()
    {
        return (void*) 1;
    };
    
    /*
     *  Hooks one of the calls to 'fread', this one reads the number of files to read 
     */
    static fRead_t ReadCount = [](void*, void* buf, size_t sz)
    {
        if(sz != 4) { return 0; }
        *((int*)(buf)) = imgPlugin->mainContent.imgFiles.size() - imgPlugin->playerFiles.size();    /* don't read player entries */
        return 4;
    };
    
    /*
     *  Hooks another of the calls to 'fread', now it returns a img entry with the file information
     */
    static fRead_t ReadEntry = [](void*, void* buf, size_t sz)
    {
        char* bBuf = (char*)buf;
        if(sz != 0x20) { return 0; }
        
        /* Don't read player entries */
        while(it->second.bIsPlayerFile) ++it;
        
        /* Build entry buffer to return to the game code */
        memset(&bBuf[0], 0, 8);
        *((unsigned short*)&bBuf[4]) = it->second.GetSizeInBlocks();
        strncpy(&bBuf[8], it->first.c_str(), 24);

        /* Get current iterator then advance the iterator */
        ImportImgContents_it = it++;
        return 0x20;
    };
    
    /* Replace calls in ReadImgContent function to the hooks above (or NOPs) */
    imgPlugin->Log("Replacing ReadImgContent calls...");
    char c5B6468;
    ReplacedCall x1(0x5B6183, (void*) OpenNoFile);  /* call     CFileMgr__Open */
    ReplacedCall x2(0x5B61AB, 0);                   /* call    _CFileMgr__Read */
    ReplacedCall x3(0x5B61B8, (void*) ReadCount);   /* call    _CFileMgr__Read */
    ReplacedCall x4(0x5B61E1, (void*) ReadEntry);   /* call    _CFileMgr__Read */
    ReplacedCall x5(0x5B64CE, 0);                   /* call    _CFileMgr__CloseFile */

    /* This one has no RAII auxiliar at all */
    ReplacedCall x6(0x5B6469, (void*) HOOK_RegisterModelIndex); /* [-1] mov     ms_aInfoForModel.ucImgId[eax], dl */
    c5B6468 = ReadMemory<char>(0x5B6468, true);  
    MakeNOP(0x5B6468, 1);
    
    /* Call CStreaming::ReadImgContent with replaced calls */
    imgPlugin->Log("Calling ReadImgContent...");
    ReadImgContent(0, 0);
    
    /* Undo replaced calls and return */
    imgPlugin->Log("Restoring ReadImgContent calls...\n");
    WriteMemory<char>(0x5B6468, c5B6468, true);
    return;
}
/*
 * Imports the replacement for an object (object means: model, texture, animation, etc)
 */
extern "C" void ImportObject(int index, CThePlugin::FileInfo& file)
{
    auto& InfoForModel = ms_aInfoForModel[index];
    imgPlugin->importList[index] = &file;
    
    /* Setup new file offsets */
    InfoForModel.iBlockOffset = 0;
    InfoForModel.iBlockCount = file.GetSizeInBlocks();
    
    /* If importing for the first time, log it */
    if(!file.bImported)
    {
        imgPlugin->Log("Registering imported object index %d at \"%s\"", index, file.path.c_str());
        file.bImported = true;
    }
    
    if(InfoForModel.uiLoadFlag == 1)
        imgPlugin->Log("warning: Importing object data %d [\"%s\"] while it is still loaded!",
                       index, file.path.c_str());
}

/* Called from above, See HOOK_RegisterModelIndex too */
extern "C" void CALL_RegisterModelIndex(SImgGTAItemInfo* item, char imgId)
{
    int index = (item - ms_aInfoForModel);
    item->ucImgId = imgId;                    /* original operation */
    ImportObject(index, ImportImgContents_it->second);
}

/*
 *  Builds a list with the pointer to all files in the mods directory that have similar names to the files in player.img
 */
extern "C" void BuildPlayerFilesList(CDirectoryEntry* entries, size_t numEntries)
{
    auto& playerFiles = imgPlugin->playerFiles;
    auto& files = imgPlugin->mainContent.imgFilesSorted;
        
    playerFiles.clear();
    
    /* Iterate on the img entries */
    for(size_t i = 0; i < numEntries; ++i)
    {
        CDirectoryEntry& entry = entries[i];
        uint32_t entryHash = hash((char*)entry.filename, ::toupper);

        // Find in our files by the hash
        auto it = files.find(entryHash);
        if(it != files.end())
        {
            // Register it
            playerFiles[entry.fileOffset] = it->second;
            it->second->bIsPlayerFile = true;
            imgPlugin->Log("Adding to playerFiles: name: %s, fileOffset: %d;", entry.filename, entry.fileOffset);
        }
    }
}

/*
 *  This is called clothes get requested, so we can find a replacement 
 */
extern "C" void OnClothesRequest(CDirectory* imgData, int index)
{
#if 0 && !defined(NDEBUG)
    imgPlugin->Log("OnClothesRequest(%p, %d)", imgData, index);
#endif
    
    /* Is some texture or  is body model index? */
    if(index >= 20000 || (index >= 384 && index < 394))
    {
        /* Check if the requested clothes has a replacement... */
        auto& InfoForModel = ms_aInfoForModel[index];
        auto it = imgPlugin->playerFiles.find(InfoForModel.iBlockOffset);
        if(it != imgPlugin->playerFiles.end())
        {
            /* We have a replacement over here! */
            ImportObject(index, *((*it).second));
        }
    }
    else
        imgPlugin->Log("OnClothesRequest failed on index range");
}

/*
 * This is called to get a file handle to read from in the streaming thread
 */
extern "C" HANDLE CALL_NewFile(HANDLE hOriginal, void* from)
{
#if 0 && !defined(NDEBUG)
    imgPlugin->Log("CALL_NewFile from %p with original handle %p -- (modelIndex %d)", from, hOriginal, iLoadingObjectIndex);
#endif
    
    CriticalLock lock(cs);  /* We're in main thread, lock access from streaming thread */
    
    /* Check if we've imported the current loading object index, if not,
     * just continue with the original handle (probably gta3.img handle) */
    auto it = imgPlugin->importList.find(iLoadingObjectIndex);
    if(it == imgPlugin->importList.end())
        return hOriginal;
        
    /* Alright, this is a custom object, let's return the hFile to it... */
    const char* filepath = it->second->path.c_str();

    // Reprocess the file, maybe it has been changed
    if(false)
    {
        it->second->Process();
        ImportObject(it->first, *it->second);
    }
    
#if 0 && !defined(NDEBUG)
    imgPlugin->Log("\t...going on this call.", from, hOriginal, iLoadingObjectIndex);
#endif
    
    /* Open our custom file */
    HANDLE hResult = CreateFileA(filepath, GENERIC_READ, FILE_SHARE_READ, NULL,
                                 OPEN_EXISTING, *pStreamCreateFlags, NULL);
    if(hResult == INVALID_HANDLE_VALUE)
    {
        imgPlugin->Log("Failed to open file for streaming (GetLastError() = %d): \"%s\"", GetLastError(), filepath);
        hResult = NULL;
    }
    else
    {
        /* Push new file into open files list */
        openFileList.emplace_back(hResult);
    }
    
    return (hResult);
}


/*
 *  After reading the data (asynchonous) from the file returned above, this is called 
 */
extern "C" void CALL_RemFile(GTA_STREAM* stream)
{
    CriticalLock lock(cs);  /* We're in the streaming thread, lock any access to our data from main thread */
    
#if 0 && !defined(NDEBUG) 
    imgPlugin->Log("CALL_RemFile");
#endif
    
    /* Try to find the hFile in our open files list, if not found that means it is a game hFile
     * (for example gta3.img, it's a game hFile) */
    auto it = std::find(openFileList.begin(), openFileList.end(), (HANDLE) stream->hFile);
    if(it != openFileList.end())
    {
        /* Found it, let's close it, we're done with this file. */
        if(!CloseHandle(*it))
        {
            imgPlugin->Log("Failed to close streaming file, this shall never happen.");
        }

        /* Remove this file from the open files list */
        openFileList.erase(it);
    }
}
