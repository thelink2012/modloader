/* 
 * std-img -- Standard IMG Loader Plugin for San Andreas Mod Loader
 * Copyright (C) 2013  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
.intel_syntax noprefix

/* Hooks and sub calls */
.globl  _HOOK_AllocateOrFindExternalScript
.globl  _HOOK_ReadImgContents
.globl  _HOOK_RegisterModelIndex

.globl  _HOOK_RegisterNextModelRead
.globl  _CALL_RegisterNextModelRead

.globl  _HOOK_NewFile
.globl  _CALL_NewFile 

.globl  _HOOK_RemFile
.globl  _CALL_RemFile

.globl _HOOK_RequestClothes

.globl _HOOK_SetStreamName
.globl _HOOK_SetImgDscName
.globl _HOOK_ReadImgContent_Base

/* vars */
.globl  _ms_aInfoForModel
.globl  _playerImgDirectory
.globl  _iLoadingObjectIndex
.globl  _pBeginStreamReadCoolReturn
.globl  _CStreaming__RequestObject

/* funcs */
.globl __AllocateOrFindExternalScript
.globl _ImportImgContents
.globl _CALL_RegisterModelIndex
.globl _OnClothesRequest
.globl _FixSetImgName
.globl _AllocBufferForString

.text



/*
    int _thiscall HOOK_AllocateOrFindExternalScript(ecx = this, arg4 = name)
    Hooks because it needs to check if object already exist before allocating since our user img can have defined the thing
*/
_HOOK_AllocateOrFindExternalScript:
        push dword ptr [esp+4]
        push ecx
        call _AllocateOrFindExternalScript
        add esp, 8
        ret 4





/*
    void _cdecl HOOK_ReadImgContents()
    Hooks because we need to import our files data before importing the main game imgs contents (gta3.img, etc)
*/
_HOOK_ReadImgContents:
        call _ImportImgContents
        jmp dword ptr [_CStreaming__ReadImgContents]

/*
    void _nakedcall HOOK_RegisterModelIndex(eax = offsetAt_aInfoForModel)
        This is hooked temporarily during ImportImgContents to register imported indexes
*/
_HOOK_RegisterModelIndex:
        pushfd      # We need to save the EFLAGS because there's a check before our hook, and after the hook there's the branching
            pushad
                mov ecx, dword ptr [_ms_aInfoForModel]
                add eax, ecx
                push edx            # ucImgId 
                push eax            # &ms_aInfoForModel[index]
                call _CALL_RegisterModelIndex   # runs replaced code and stuff
                add esp, 8
            popad
        popfd
        ret

/*
    void _nakedcall HOOK_RegisterNextModelRead(esi = objectIndex, eax = someOffsetAt_aInfoForModel)
        Used to tell us which model will get loaded ahead, so we know if it's a imported one
*/
_HOOK_RegisterNextModelRead:
        mov dword ptr [_iLoadingObjectIndex], esi
        # Run replaced code:
        mov edx,  dword ptr [_ms_aInfoForModel]
        mov edx, [edx+0xC+eax*4]    # edx = ms_aInfoForModel[iLoadingModelIndex].iBlockCount
        ret

/*
    char _cdecl HOOK_RequestClothes(arg4 = index, arg8 = type)
        This hook replaces the call to CStreaming::RequestObject that loads clothes.
        We need it because we need to know clothes are being requested.
*/
_HOOK_RequestClothes:
        cmp dword ptr [esp+8], 0x12     # Is clothes?
        jnz _IsNotClothes
        
        push dword ptr [esp+4]      # index
        push _playerImgDirectory    # img data
        call _OnClothesRequest
        add esp, 8

    _IsNotClothes:
        jmp dword ptr [_CStreaming__RequestObject]





/*
    HANDLE _cdecl HOOK_NewFile(eax = hOriginalFile, esi = blockOffset)
        Returns the file handle to the file that will get read to get an object data
        Normally it will return it's original handle, but if a custom file (put in a modloader folder) it will return a new unique handle
*/
_HOOK_NewFile:

        # If this call is not coming from (pBeginStreamReadCoolReturn - 5), clean the loading object index
        # This may be being called from a audio streaming process
        mov edx, dword ptr [esp+0x14+0x4]       # return pointer from call to CStreaming::BeginStreamRead
        cmp dword ptr [_pBeginStreamReadCoolReturn], edx
        je _YesCoolFile
        mov dword ptr [_iLoadingObjectIndex], -1    # clean iLoadingObjectIndex

    _YesCoolFile:
        and esi, 0x00FFFFFF     # Original code
        push edx                # Caller + 5
        push eax                # hOriginalFile
        call _CALL_NewFile      # Returns hFile HANDLE
        add esp, 8
        ret

/*
    xbool _nakedcall HOOK_RemFile(esi = GTA_STREAM)
        Closes the file at GTA_STREAM if needed (that's, if it was created at NewFile above)
*/
_HOOK_RemFile:
        push esi
        call _CALL_RemFile
        add esp, 4

        # Original code
        mov dword ptr [esi+0x4], 0
        cmp byte ptr [esi+0xD], 0
        ret





/*
    void HOOK_SetStreamName(eax = szImgTempStr, edx = p2)
        StreamName copying hook
*/
_HOOK_SetStreamName:
        mov dword ptr [eax+edx], 0x0000003F  # StreamName[x] = "?\0\0\0"
        ret

/*
    void HOOK_SetImgDscName(eax = szImgTempStr, edx = p2)
        imgDescriptor.name copying hook.
        Added new fields into imgDescriptor, see the explanation at hooks.cpp
*/
_HOOK_SetImgDscName:
        push ebx
        lea ebx, [eax+edx]  # ebx = img.name

        push eax
        call _AllocBufferForString
        add esp, 4
        # eax = szImgNameBuffer

        mov dword ptr [ebx+0], 0x3F   # strncpy(CImgDescriptor.dummy, "?", 4)
        mov dword ptr [ebx+4], eax    # CImgDescriptor.customName = eax

        pop ebx
        ret

/*
    const char* ReadImgContent_Base(arg0 = retPointer, arg4 = upperRetPointer, arg8 = imgDescriptor, arg12 = imgId)
        We should return (in eax ofcourse) the pointer into the imgDescriptor name
        See hooks.cpp and HOOK_SetImgDscName
*/
_HOOK_ReadImgContent_Base:
        # First let's do the operations that we've replaced
        pop edx                     # Remove our return pointer, so we'll be at the caller scope
        mov eax, dword ptr [esp+4]  # eax = imgDescriptor   [restoring replaced code]
        sub esp, 0x3C               #                       [restoring replaced code]
        push edx                    # Restore our return pointer

        # Test if valid imgDescriptor (we have a hook that sends a null imgDescriptor...)
        test eax, eax
        jz _NotCustomName

        # If custom name, return pointer to custom name
        cmp word ptr [eax], 0x003F  # cmp(imgDescriptor.dummy, "?\0")
        jnz _NotCustomName
        mov eax, dword ptr [eax+4]  # We have a custom pointer at CImgDescriptor+4

    _NotCustomName:
        ret

.att_syntax prefix
