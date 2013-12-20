/* 
 * std-img -- Standard IMG Loader Plugin for San Andreas Mod Loader
 * Copyright (C) 2013  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */

#include <cstdio>
#include "img.h"
#include "Injector.h"
#include <SImgGTAItemInfo.h>
#include <CImgDescriptor.h>

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
void HOOK_SetImgDscName();
void HOOK_SetStreamName();
void HOOK_ReadImgContent_Base();
void* pBeginStreamReadCoolReturn;


/* Those will point to game functions */
void* CStreaming__RequestObject;
void (*ReadImgContent)(void* imgEntry, int imgId) = memory_pointer(0x5B6170).get();
int (*CStreaming__OpenImgFile)(const char* filename, char notPlayerImg);

static void(*readImgFileFromDat)(const char* path, char notPlayerImg);
static int (*addPath)(const char* name);

static int (__fastcall *CExternalScripts__Allocate)(CExternalScripts* self, int dummy, const char* name);
static int (__fastcall *CExternalScripts__FindByName)(CExternalScripts* self, int dummy, const char* name);
static int (__fastcall *CDirectory__CDirectory)(void* pDirectory, int dummy, size_t count, void* pEntries);
static int (__fastcall *CDirectory__ReadImgEntries)(void* pDirectory, int dummy, const char* file);

/* Those will store pointer to game data */
void* imgDescriptors;   /* CImgDescriptor* */
char* StreamNames;      /* char[32][64] */

int *gta3ImgDescriptorNum, *gtaIntImgDescriptorNum;

DWORD* pStreamCreateFlags;          /* Pointer to games async file handle flags (for CreateFile) */
SImgGTAItemInfo* ms_aInfoForModel;  /* Pointer to array of game model info */
void* CStreaming__ReadImgContents;  /* Pointer to the function that reads img header from open img's */
void* playerImgDirectory;           /* files offsets, name, etc, from player.img */
void* playerImgEntries;

static char* playerImgPath;
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
    playerImgPath = ReadMemory<char*>(0x5A69F7 + 1, true);
    CDirectory__CDirectory(playerImgDirectory, 0, 550, playerImgEntries);
    CDirectory__ReadImgEntries(playerImgDirectory, 0, playerImgPath);
}


/*
 *      This hook adds the possibility to hook img loads that are loaded on default.dat and gta.dat
 *      It is good for TC's (but other situations too) that replaces the files loaded from gta.dat
 *      This will try to find a similar file in the mods folder and load it instead
 */
void HOOK_ReadImgFileFromDat(const char* path, char notPlayerImg)
{
    imgPlugin->Log("HOOK_ReadImgFileFromDat(\"%s\")", path);

    std::string normalizedPath = NormalizePath(path);
    size_t hash = modloader::hash(normalizedPath);
    
    auto it = std::find_if(imgPlugin->imgFiles.begin(), imgPlugin->imgFiles.end(),
        [&hash, &normalizedPath](const CThePlugin::ImgInfo& info)
        {
            /* If .img file found around the mods folders and was not registered as replacement to original img... */
            if(info.isCustomContent && !info.isOriginal)
            {
                /* check if paths are equal... */
                if(info.pathModHash == hash && info.pathMod == normalizedPath) 
                    return true;
            }
            return false;
        });
        
    /* If found replacement for this load, replace the 'path' pointer */
    if(it != imgPlugin->imgFiles.end())
    {
        path = it->path.data();
        imgPlugin->Log("Replacement img for dat img: %s", path);
    }
        
    /* ...continue the gta.dat img file loading */
    return readImgFileFromDat(path, notPlayerImg);
}

/*
 *  Allocates a string buffer with the lifetime of the program
 */
const char* AllocBufferForString(const char* inBuf)
{
    imgPlugin->Log("Allocating static buffer for string \"%s\"", inBuf);
    
    static std::list<std::string> bufList;
    std::string& buf = AddNewItemToContainer(bufList);
    buf = inBuf;
    return buf.data();
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
        pStreamCreateFlags = ReadMemory<DWORD*>(0x406BE5+1, true);
        
        StreamNames    = ReadMemory<char*>(0x406880 + 2, true);
        imgDescriptors = ReadMemory<void*>(0x407638 + 2, true);
        
        gta3ImgDescriptorNum    = ReadMemory<int*>(0x408402 + 2, true);
        gtaIntImgDescriptorNum  = ReadMemory<int*>(0x408423 + 1, true);

        playerImgDirectory = ReadMemory<void*>(0x5A69ED + 1, true);
        playerImgEntries = ReadMemory<void*>(0x5A69E3 + 1, true);
        
        paths = ReadMemory<int*>(0x45A001 + 1, true);
        pathCount = ReadMemory<int*>(0x459FF0 + 2, true);

        ReadImgContent = memory_pointer(0x5B6170).get();
        CExternalScripts__FindByName = memory_pointer(0x4706F0).get();
        CDirectory__CDirectory = memory_pointer(0x5322F0).get();
        CDirectory__ReadImgEntries = memory_pointer(0x532350).get();
        CStreaming__OpenImgFile = memory_pointer(0x407610).get();
    }
   
    /* Apply hooks */
    {
        /* Checks the path of the img files being loaded from gta.dat to see if any replacement, etc */
        readImgFileFromDat = MakeCALL(0x5B915B, (void*) HOOK_ReadImgFileFromDat).get();
        
        /* Replace calls to find/get entity index from img file name */
        addPath = MakeCALL(0x5B63E8, (void*)(HOOK_AllocateOrFindPath)).get();
        CExternalScripts__Allocate = MakeCALL(0x5B6419, (void*)(HOOK_AllocateOrFindExternalScript)).get();
        
        /* Replace ReadImgContents call and get it's original calling function pointer */
        CStreaming__ReadImgContents = MakeCALL(0x5B8E1B, (void*)HOOK_ReadImgContents).p;

        /* We need to know the next model to be read before the BeginStreamRead request happens */
        MakeCALL(0x40CCA6, (void*) HOOK_RegisterNextModelRead);
        MakeNOP(0x40CCA6 + 5, 2);

        /* for clothes we also need to know it's hashes and other info */
        if(isHoodlum)
        {
            CStreaming__RequestObject = MakeCALL(0x156644F, (void*) HOOK_RequestClothes).p; 
            
            // Rewrite the HOODLUM call
            MakeCALL(0x1566401, (void*) HOOK_RequestClothes).p;
            MakeJMP (0x1566406, 0x156641F);
            MakeNOP (0x156640B, 0x14);
        }
        else
        {
            CStreaming__RequestObject = MakeCALL(0x40A106, (void*) HOOK_RequestClothes).p;     
            MakeCALL(0x40A0D1, (void*) HOOK_RequestClothes).p;
        }
        
        
        /* Our hook to BeginStreamRead (our hook is HOOK_NewFile) will only
         * do something if it came from the call at 0x40CF34 */
        pBeginStreamReadCoolReturn = (void*)(0x40CF34 + 5);
        
        /* If registered a model that we've replaced, open our file and load from it */
        addr = isHoodlum? 0x156C2FB : 0x406A5B; // isHOODLUM? HOODLUM : ORIGINAL
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
        MakeNOP(addr, 10);
        MakeCALL(addr, (void*) HOOK_SetStreamName);
        
        addr = isHoodlum? 0x01567BC2 : 0x407642;        // imgDescriptor hook
        MakeNOP(addr, 10);
        MakeCALL(addr, (void*) HOOK_SetImgDscName);
        
        addr = 0x5B6170;                                // hook to read new field at imgDescriptor
        MakeCALL(addr, (void*) HOOK_ReadImgContent_Base);
        MakeNOP(addr + 5, 2);
    }
    
    
    /* 
     * HACK HACK
     * We're not supporting stream reads called at function 0x4076C0
     * and analyzing it's xref, it seems like this is never ever called, only when some stream fail to open, so we're safe... or not?
     * Anyway it looks to be safe, and if someone relates the code reaching there, I think it's easily fixed.
     * 
     */
#if 0 || !defined(NDEBUG)   // TODO FIX, THIS REALLY NEEDS A HOOK
                            // 0x4076C0 is actually CStreaming::RetryLoadFile
    static void (*trouble_4076C0)() = []()
    {
        imgPlugin->Log("FATAL ERROR: THIS IS A BUG! [0x4076C0 called]. Please report this bug!");
        // Let's crash the game in a hacky way
        *((int*)(0x26)) = 0;
    };
    MakeCALL(0x4076C0, (void*) trouble_4076C0);
#endif
}





