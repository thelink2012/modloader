/* 
 * std-img -- Standard IMG Loader Plugin for San Andreas Mod Loader
 * Copyright (C) 2013  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */

/*
 *  TODO
 *    ** Needs testing on RRR and SCM replacement to see if it is working well
 */

#include "img.h"
#include "Injector.h"
#include <SImgGTAItemInfo.h>
#include <cstdio>

extern "C"      /* The content must have C name mangling, so our ASM code can access it easily! */
{
    
typedef int CExternalScripts;
  
/* The following are hooks implemented in hooks.s file */
void HOOK_AllocateOrFindExternalScript();
int  HOOK_AllocateOrFindPath(const char*);
void HOOK_ReadImgContents();
void HOOK_RegisterModelIndex();
void HOOK_RegisterNextModelRead();
void HOOK_RequestClothes();
void HOOK_NewFile();
void HOOK_RemFile();
void* pBeginStreamReadCoolReturn;

void* CStreaming__RequestObject;
void (*ReadImgContent)(void* imgEntry, int imgId) = memory_pointer_a(0x5B6170);

uint32_t (*crc32FromUpcaseString)(const char*);

DWORD* pStreamCreateFlags;          /* Pointer to games async file handle flags (for CreateFile) */
SImgGTAItemInfo* ms_aInfoForModel;  /* Pointer to array of game model info */
void* CStreaming__ReadImgContents;  /* Pointer to the function that reads img header from open img's */
void* playerImgDirectory;           /* files offsets, name, etc, from player.img */
void* playerImgEntries;

/* Those will point to game functions */
static int (__cdecl *addPath)(const char* name);
static int (__fastcall *CExternalScripts__Allocate)(CExternalScripts* self, int dummy, const char* name);
static int (__fastcall *CExternalScripts__FindByName)(CExternalScripts* self, int dummy, const char* name);
static int (__fastcall *CDirectory__CDirectory)(void* pDirectory, int dummy, size_t count, void* pEntries);
static int (__fastcall *CDirectory__ReadImgEntries)(void* pDirectory, int dummy, const char* file);

/* Those (again) will store pointer to game data */
static int* paths;
static int* pathCount;



/*
 *      We're going to replace the calls that registers a script or R3 path when reading from a img
 *      By default the game always create a script or a r3 path, different from the other files format,
 *      that does a (exist? existingIndex : allocIndex).
 * 
 */


/*
 *  AllocateOrFindExternalScript
 *      for streamed SCM files
 *      Tries to find a external script, if not found, alloc it
 */
int AllocateOrFindExternalScript(CExternalScripts* obj, const char* name)
{
   int result = CExternalScripts__FindByName(obj, 0, name);
   if(result == -1) result = CExternalScripts__Allocate(obj, 0, name);
   return result;
}

/*
 *  getPath
 *      Gets a R3 path number from name, but only if it exists
 */
static int getPath(const char* r3name)
{
    int r3number = 850;
    
    /* Extract R3 path number */
    if(sscanf(r3name, "carrec%d", &r3number) || sscanf(r3name, "CARREC%d", &r3number))
    {
        /* Checkout if this number is already created */
        for(int i = 0; i < *pathCount; ++i)
        {
            if(paths[i * 4] == r3number)
                return r3number;
        }
        
        /* ...Nope... */
    }
    return -1;
}

/*
 *  HOOK_AllocateOrFindPath
 *      for R3 paths
 * 
 */
int HOOK_AllocateOrFindPath(const char* name)
{
   int result = getPath(name);
   if(result == -1) result = addPath(name);
   return result;
}




/*
 * Read entries from player.img into it's CDirectory
 */
void ReadPlayerImgEntries()
{
    CDirectory__CDirectory(playerImgDirectory, 0, 550, playerImgEntries);
    CDirectory__ReadImgEntries(playerImgDirectory, 0, "MODELS\\PLAYER.IMG");
}




/*
 *  We're going to reimplement SA's ReadFile/GetOverlappedResult (well, kinda) so it ignores the error ERROR_HANDLE_EOF.
 *  This will permit ours files on disk to not have a real size aligned to 2KiB
 * 
 */

/*
 *  This will fix the return from ReadFile/GetOverlappedResult in the case it returns false for an End-Of-File
 *  Note that ReadFile for synchonous operation do not need this fix, it returns true on EOF anyway.
 */
static BOOL WINAPI FixReadReturn(BOOL bResult)
{
    if(bResult == FALSE && GetLastError() == ERROR_HANDLE_EOF)
    {
        SetLastError(0);
        bResult = TRUE;
    }
    return bResult;
}

/*
 *  Finally, the following's will be the replacement for the original functions.
 *  It justs call's it's WinAPI equivalent and fixes the return value with FixReadReturn
 * 
 */
static BOOL WINAPI NoEOF_ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead,
                                  LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped)
{
    return FixReadReturn(ReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped));
}

static BOOL WINAPI NoEOF_GetOverlappedResult(HANDLE hFile, LPOVERLAPPED lpOverlapped,
                                             LPDWORD lpNumberOfBytesTransferred, BOOL bWait)
{
    return FixReadReturn(GetOverlappedResult(hFile, lpOverlapped, lpNumberOfBytesTransferred, bWait));
}




}


/*
 *  Apply hooks into the game code
 */
void ApplyPatches()
{
    size_t addr;
    bool isHoodlum = ReadMemory<uint8_t>(0x406A20, true) == 0xE9;
    
    /* Read some pointers */
    {
        ms_aInfoForModel = ReadMemory<SImgGTAItemInfo*>(0x408835 + 2, true);
        playerImgDirectory = ReadMemory<void*>(0x5A69ED + 1, true);
        playerImgEntries = ReadMemory<void*>(0x5A69E3 + 1, true);
        paths = ReadMemory<int*>(0x45A001 + 1, true);
        pathCount = ReadMemory<int*>(0x459FF0 + 2, true);
        CExternalScripts__FindByName = memory_pointer_a(0x4706F0);
        crc32FromUpcaseString = memory_pointer_a(ReadRelativeOffset(0x471ADD + 1).p); 
        CDirectory__CDirectory = memory_pointer_a(ReadRelativeOffset(0x5A69F2 + 1).p);
        CDirectory__ReadImgEntries = memory_pointer_a(ReadRelativeOffset(0x5A6A01 + 1).p);

        /* for CreateFile flags... */
        pStreamCreateFlags = ReadMemory<DWORD*>(0x406BE5+1, true);
    }
   
    /* Apply hooks */
    {
        /* Replace calls to find/get entity index from img file name */
        addPath = memory_pointer_a(MakeCALL(0x5B63E8, (void*)(HOOK_AllocateOrFindPath)).p);
        CExternalScripts__Allocate = memory_pointer_a(MakeCALL(0x5B6419, (void*)(HOOK_AllocateOrFindExternalScript)).p);
        
        /* Replace ReadImgContents call and get it's original calling function pointer */
        CStreaming__ReadImgContents = MakeCALL(0x5B8E1B, (void*)HOOK_ReadImgContents).p;

        /* Our hook to BeginStreamRead (our hook is HOOK_NewFile) will only
         * do something if it came from the call at 0x40CF34 */
        pBeginStreamReadCoolReturn = (void*)(0x40CF34 + 5);

        /* We need to know the next model to be read before the BeginStreamRead request happens */
        MakeCALL(0x40CCA6, (void*) HOOK_RegisterNextModelRead);
        MakeNOP(0x40CCA6 + 5, 2);

        /* for clothes we also need to know it's hashes and other info */
        if(isHoodlum)
        {
            CStreaming__RequestObject = MakeCALL(0x156644F, (void*) HOOK_RequestClothes).p; 
            
            /* Rewrite the HOODLUM call */
            MakeCALL(0x1566401, (void*) HOOK_RequestClothes).p;
            MakeJMP (0x1566406, 0x156641F);
            MakeNOP (0x156640B, 0x14);
        }
        else
        {
            CStreaming__RequestObject = MakeCALL(0x40A106, (void*) HOOK_RequestClothes).p;     
            MakeCALL(0x40A0D1, (void*) HOOK_RequestClothes).p;
        }

        /* If registered a model that we've replaced, open our file and load from it */
        addr = isHoodlum? 0x156C2FB : 0x406A5B; /* isHOODLUM? HOODLUM : ORIGINAL */
        MakeCALL(addr, (void*) HOOK_NewFile);
        MakeNOP(addr+5, 1);

        /* ...We need to close our file after reading... */    
        MakeCALL(0x406669, (void*) HOOK_RemFile);
        MakeNOP(0x406669+5, 1);

        /* Replace CStreaming::StreamReaderThread functions to async read and not fail in case of EOF,
         * this allows non 2KiB aligned files in the disk */
        WriteMemory<char>(0x406565, 0xBD, true);    /* mov ebp, ? */
        WriteMemory<void*>(0x406565 + 1, (void*) NoEOF_ReadFile, true);
        MakeNOP(0x406565 + 5, 1);
        MakeCALL(0x4065FE, (void*) NoEOF_GetOverlappedResult);
        MakeNOP(0x4065FE + 5, 1);
    }
    
    /* 
     * HACK HACK
     * We're not supporting stream reads called at function 0x4076C0
     * and analyzing it's xref, it seems like this is never ever called, so we're safe... or not?
     * Anyway it looks to be safe, and if someone relates the code reaching there, I think it's easily fixed.
     * 
     */
    static void (*trouble_4076C0)() = []()
    {
        imgPlugin->Log("FATAL ERROR: THIS IS A BUG! [0x4076C0 called]. Please report this bug!");
        // Let's crash the game in a hacky way
        *((int*)(0x26)) = 0;
    };
    MakeCALL(0x4076C0, (void*) trouble_4076C0);
}





