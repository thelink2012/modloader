/* 
 * San Andreas modloader
 * Copyright (C) 2013  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 *  Modloader core hooks
 * 
 */
.intel_syntax noprefix      # Let me use Intel syntax, I don't like AT&T syntax

.globl _HOOK_Init
.globl _WinMainPtr
.globl _CALL_Startup

.data
    _WinMainPtr:
        .long 0x0

.text

    // Hooked at 0x824577 replacing a call to [__gtasa_try]
    _HOOK_Init:
        // Call modloader startup
        call _CALL_Startup
    /   // Get back to __gtasa_try
        mov eax, _WinMainPtr
        jmp eax


.att_syntax prefix      # Restore AT&T syntax
