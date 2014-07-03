/* 
 * std-img -- Standard IMG Loader Plugin for San Andreas Mod Loader
 * Copyright (C) 2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 * Low-level stuff for Microsoft Visual Studio
 *
 */

/* Hooks and sub calls */
extern void* CallGetAbstractHandle(void*);

/* vars */
extern void* ms_aInfoForModel;
extern int iNextModelBeingLoaded;

/* funcs */
extern char* AllocBufferForString(const char*);


/*
    void _nakedcall HOOK_RegisterNextModelRead(esi = objectIndex, eax = someOffsetAt_aInfoForModel)
        Used to tell us which model will get loaded ahead, so we know if it's a imported one
*/
void __declspec(naked) HOOK_RegisterNextModelRead()
{
    _asm
    {
        mov dword ptr[iNextModelBeingLoaded], esi
        /* Run replaced code: */
        mov edx, dword ptr[ms_aInfoForModel]
        mov edx, [edx + 0xC + eax * 4]    /* edx = ms_aInfoForModel[iLoadingModelIndex].iBlockCount */
        ret
    }
}

/*
    HANDLE _nakedcall HOOK_NewFile(eax = hOriginalFile, esi = blockOffset)
        Returns the file handle to the file that will get read to get an object data
        Normally it will return it's original handle, but if a custom file (put in a modloader folder) it will return a new unique handle
*/
void __declspec(naked) HOOK_NewFile()
{
    _asm
    {
        and esi, 0x00FFFFFF     /* Original code */
        push eax
        call CallGetAbstractHandle
        add esp, 4
        ret
    }
}



///////////////////////////////////////////////////////////////////////
// ------------------- Stream Name Size Fix ------------------------ //
///////////////////////////////////////////////////////////////////////


/*
    void _nakedcall HOOK_SetStreamName(eax = szImgTempStr, edx = p2)
        StreamName copying hook
*/
void __declspec(naked) HOOK_SetStreamName()
{
    _asm
    {
        mov dword ptr [eax + edx], 0x0000003F  /* StreamName[x] = "?\0\0\0" */
        ret
    }
}

/*
    void _nakedcall HOOK_SetImgDscName(eax = szImgTempStr, edx = p2)
        imgDescriptor.name copying hook.
        Added new fields into imgDescriptor, see the explanation at hooks.cpp
*/
void __declspec(naked) HOOK_SetImgDscName()
{
    _asm
    {
        push ebx
        lea ebx, [eax + edx]  /* ebx = img.name */

        push eax
        call AllocBufferForString
        add esp, 4
        /* eax = szImgNameBuffer */

        mov dword ptr [ebx + 0], 0x3F   /* strncpy(CImgDescriptor.dummy, "?", 4) */
        mov dword ptr [ebx + 4], eax    /* CImgDescriptor.customName = eax */

        pop ebx
        ret
    }
}

