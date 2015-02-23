/* 
 * std-img -- Standard IMG Loader Plugin for San Andreas Mod Loader
 * Copyright (C) 2013-2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under the MIT License, see LICENSE at top level directory.
 * 
 * Low-level stuff for the GNU Assembler (GAS)
 *
 */
.intel_syntax noprefix

/* Hooks and sub calls */
.globl  _HOOK_RegisterNextModelRead
.globl  _HOOK_NewFile
.globl  _HOOK_LoadColFileFix
.globl _HOOK_FixBikeSuspLines
.globl  _CallGetAbstractHandle

/* vars */
.globl  _ms_aInfoForModel
.globl  _ColModelPool_new

/* funcs */
.globl _AllocBufferForString
.globl _RegisterNextModelRead
.globl _FixBikeSuspPtr

.text

/*
    void* _nakedcall _cdecl HOOK_LoadColFileFix(arg0 = size)
        Fixes the CFileLoader::LoadCollisionFile method to work properly
*/
_HOOK_LoadColFileFix:

        /* Perform the original operation (new ColModel) */
        push [esp+4]
        mov eax, _ColModelPool_new
        call eax
        add esp, 4

        /* Now, the fix is here, edi should contain the ColModel pointer, but it doesn't! 
           Let's fix it     */
        mov edi, eax
        ret


/*
    void _nakedcall HOOK_RegisterNextModelRead(esi = objectIndex, eax = someOffsetAt_aInfoForModel)
        Used to tell us which model will get loaded ahead, so we know if it's a imported one
*/
_HOOK_RegisterNextModelRead:
        pushad
        push esi
        call _RegisterNextModelRead
        add esp, 4
        popad

        /* Run replaced code: */
        mov edx,  dword ptr [_ms_aInfoForModel]
        mov edx, [edx+0xC+eax*4]    /* edx = ms_aInfoForModel[iLoadingModelIndex].iBlockCount */
        ret

/*
    HANDLE _nakedcall HOOK_NewFile(eax = hOriginalFile, esi = blockOffset)
        Returns the file handle to the file that will get read to get an object data
        Normally it will return it's original handle, but if a custom file (put in a modloader folder) it will return a new unique handle
*/
_HOOK_NewFile:
        and esi, 0x00FFFFFF     /* Original code */
        push eax
        call _CallGetAbstractHandle
        add esp, 4
        ret


/*
    void _nakedcall HOOK_FixBikeSuspLines(edi = v2->clump.base.m_pColModel->m_pColData, ...)
        Fixes broken pointer on CBike::SetupSuspensionLine after a refresh
*/
_HOOK_FixBikeSuspLines:
    _BikeSuspTry:
        mov eax, [edi+0x10]  /* Original Code */
        test eax, eax
        jz _BikeSuspFix
        mov edx, [eax+0x28]  /* Original Code */
        ret

    _BikeSuspFix:
        pushad
        push edi
        call _FixBikeSuspPtr
        add esp, 4
        popad
        jmp _BikeSuspTry


.att_syntax prefix
