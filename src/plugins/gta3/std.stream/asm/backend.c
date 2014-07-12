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

/* funcs */
extern char* AllocBufferForString(const char*);
extern void RegisterNextModelRead(int id);

/*
    void _nakedcall HOOK_RegisterNextModelRead(esi = objectIndex, eax = someOffsetAt_aInfoForModel)
        Used to tell us which model will get loaded ahead, so we know if it's a imported one
*/
void __declspec(naked) HOOK_RegisterNextModelRead()
{
    _asm
    {
        pushad
        push esi
        call RegisterNextModelRead
        add esp, 4
        popad

        /* Run replaced code: */
        mov edx, dword ptr[ms_aInfoForModel]
        mov edx, [edx + 0xC + eax * 4]    /* edx = ms_aInfoForModel[iLoadingModelIndex].blocks */
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
