/*
 * San Andreas Mod Loader - Address Translation Between Game Versions
 * Copyright (C) 2013-2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 *
 */
#include <injector/injector.hpp>
#include <map>

// GTA 3 10 table
static void III_10(std::map<memory_pointer_raw, memory_pointer_raw>& map)
{
    bool isHoodlum = injector::address_manager::singleton().IsHoodlum();

    // Core
    if(true)
    {
		map[0x8246EC] = 0x5C1F39;   // call    _WinMain+
		map[0x6A0050] = 0x52C5A0;   // _ZN5CText3GetEPKc+
    }

    // std.fx
    if(true)
    {
		//map[0x5900D2] = nope;   // call    _ZN9CTxdStore7LoadTxdEiPKc ; "loadscs.txd"
        //map[0x5B8F5E] = nope;   // call    _ZN9CTxdStore7LoadTxdEiPKc ; "models/generic/vehicle.txd"
		map[0x5BF8B7] = 0x48BF5E;   // call    _ZN9CTxdStore7LoadTxdEiPKc ; "models/particle.txd"+
        //map[0x5C248F] = nope;   // call    _ZN9CTxdStore7LoadTxdEiPKc ; "models/effectsPC.txd"
        //map[0x5C24B1] = nope;   // call    _Z8OpenFilePKcS0_          ; "models/effects.fxp"
        
		map[0x5BA850] = 0x5048F0;   // _ZN4CHud10InitialiseEv+
		map[0x588850] = 0x504C50;   // _ZN4CHud8ShutdownEv+
		map[0x5827D0] = 0x4A4030;   // _ZN6CRadar12LoadTexturesEv+
		map[0x5BA865] = 0x504905;   // call    _ZN9CTxdStore7LoadTxdEiPKc ; "models/hud.txd"+

		map[0x5BA690] = 0x500A40;   // _ZN5CFont10InitialiseEv+
		map[0x7189B0] = 0x500BA0;   // _ZN5CFont8ShutdownEv+
		map[0x5BA6A4] = 0x500A57;   // call    _ZN9CTxdStore7LoadTxdEiPKc ; "models/fonts.txd"+
        //map[0x5BA7D4] = nope;   // call    _ZN9CTxdStore7LoadTxdEiPKc ; "models/pcbtns.txd"

		map[0xBA6748] = 0x8F59D8    // CMenuManager FrontEndMenuManager+
		map[0x572EC0] = 0x47A230;   // _ZN12CMenuManager15LoadAllTexturesEv+
		map[0x574630] = 0x47A440;   // _ZN12CMenuManager14UnloadTexturesEv+
		map[0x572F1E] = 0x47A34F;   // call    _ZN9CTxdStore7LoadTxdEiPKc ; "models/frontend.txd"+
		map[0x573040] = 0x47A3D3;   // call    _ZN9CTxdStore7LoadTxdEiPKc ; "models/menu.txd"+
    }
	
    // std.text
    if(true)
    {
		map[0x57A161] = 0x4813CD;   // call    _ZN5CText3GetEPKc+
		map[0x6A0228] = 0x52C436;   // call    _ZN8CFileMgr8OpenFileEPKcS1_  ; @CText::Load+
		//map[0x69FD5A] = nope;   // call    _ZN8CFileMgr8OpenFileEPKcS1_  ; @CText::LoadMissionText-
		map[0x57326E] = 0x47A4DE;   // jz      loc_573313                    ; Check for some menu stuff to avoid reloading text+
		map[0xBA6748] = 0x8F59D8;   // CMenuManager FrontEndMenuManager+
		map[0x573260] = 0x47A4D0;   // _ZN12CMenuManager33InitialiseChangedLanguageSettingsEv+
    }

    // std.movies
    if(true)
    {
		map[0x748B00] = 0x582A93;   //  call    _CreateVideoPlayer  ; "movies/Logo.mpg"+
		map[0x748BF9] = 0x582C26;   //  call    _CreateVideoPlayer  ; "movies/GTAtitles.mpg"+
    }

    // std.scm
    if(true)
    {
		map[0x468EC9] = 0x438869;   // call    _ZN8CFileMgr8OpenFileEPKcS1_  ; "main.scm" @CTheScripts+
		map[0x489A4A] = 0x588E13;   // call    _ZN8CFileMgr8OpenFileEPKcS1_  ; "main.scm" @CRunningScript+
    }

    // std.sprites
    if(true)
    {
		map[0x48418A] = 0x44D6E9;   // call    _ZN9CTxdStore7LoadTxdEiPKc+
    }

    // std.asi
    if(true)
    {
		map[0x836F3B] = 0x5AB904;   // SetCurrentDirectory return pointer for _chdir+
    }

    // std.stream
    if(true)
    {
        // !!!!! REMEMBER TO CHECK TRAITS
    }

    // AbstractFrontend | TheMenu
    if(true)
    {
		map[0xB72F20] = 0x7DDBA0;   // _ZN4CPad11OldKeyStateE CKeyState;-
		map[0xB73190] = 0x7D15C0;   // _ZN4CPad11NewKeyStateE CKeyState;-
		map[0x53ECBD] = 0x48E8FD;   // call    _Z4IdlePv+
		map[0x506EA0] = 0x57CC20;   // _ZN12CAudioEngine24ReportFrontendAudioEventEiff+
        //map[0xB6BC90] = 0xB6BC90; // CAudioEngine AudioEngine
        //map[0x57E240] = 0x57E240; // _ZN12CMenuManager17DisplayHelperTextEPc
        //map[0x579330] = 0x579330; // _ZN12CMenuManager13MessageScreenEPKcbb
    }
}
