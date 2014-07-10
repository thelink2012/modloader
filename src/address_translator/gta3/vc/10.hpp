/*
 * San Andreas Mod Loader - Address Translation Between Game Versions
 * Copyright (C) 2013-2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 *
 */
#include <injector/injector.hpp>
#include <map>

// GTA VC 10 table
static void vc_10(std::map<memory_pointer_raw, memory_pointer_raw>& map)
{
    bool isHoodlum = injector::address_manager::singleton().IsHoodlum();

    // Core
    if(true)
    {
		map[0x8246EC] = 0x667CB9;   // call    _WinMain
		map[0x6A0050] = 0x584F30;   // _ZN5CText3GetEPKc
    }

    // std.fx
    if(true)
    {
		//map[0x5900D2] = nope;   // call    _ZN9CTxdStore7LoadTxdEiPKc ; "loadscs.txd"
        //map[0x5B8F5E] = nope;   // call    _ZN9CTxdStore7LoadTxdEiPKc ; "models/generic/vehicle.txd"
		map[0x5BF8B7] = 0x4A4BB0;   // call    _ZN9CTxdStore7LoadTxdEiPKc ; "models/particle.txd"
        //map[0x5C248F] = nope;   // call    _ZN9CTxdStore7LoadTxdEiPKc ; "models/effectsPC.txd"
        //map[0x5C24B1] = nope;   // call    _Z8OpenFilePKcS0_          ; "models/effects.fxp"
        
		map[0x5BA850] = 0x55C8A0;   // _ZN4CHud10InitialiseEv
		map[0x588850] = 0x55C7F0;   // _ZN4CHud8ShutdownEv
		map[0x5827D0] = 0x4C5DC0;   // _ZN6CRadar12LoadTexturesEv
		map[0x5BA865] = 0x55C8B5;   // call    _ZN9CTxdStore7LoadTxdEiPKc ; "models/hud.txd"

		map[0x5BA690] = 0x552310;   // _ZN5CFont10InitialiseEv
		map[0x7189B0] = 0x5522E0;   // _ZN5CFont8ShutdownEv
		map[0x5BA6A4] = 0x552327;   // call    _ZN9CTxdStore7LoadTxdEiPKc ; "models/fonts.txd"
        //map[0x5BA7D4] = nope;   // call    _ZN9CTxdStore7LoadTxdEiPKc ; "models/pcbtns.txd"

		map[0xBA6748] = 0x869630 //??// CMenuManager FrontEndMenuManager
		map[0x572EC0] = 0x4A3A13;   // _ZN12CMenuManager15LoadAllTexturesEv
		map[0x574630] = 0x4A394D;   // _ZN12CMenuManager14UnloadTexturesEv
		map[0x572F1E] = 0x4A3AC3;   // call    _ZN9CTxdStore7LoadTxdEiPKc ; "models/fronten1.txd"
		map[0x573040] = 0x4A3B5D;   // call    _ZN9CTxdStore7LoadTxdEiPKc ; "models/fronten2.txd"
        //map[0x572FB5] = nope;   // call    _ZN9CTxdStore7LoadTxdEiPKc ; "models/fronten_pc.txd"

		//map[0x5DD95F] = nope;   // call    _ZN9CTxdStore7LoadTxdEiPKc ; "models/grass/plant1.txd"
		//map[0x5DD272] = nope;   // call    _RwStreamOpen              ; "models/grass/grass%d_%d.dff"
    }

    // std.text
    if(true)
    {
		map[0x57A161] = 0x49EE23;   // call    _ZN5CText3GetEPKc
		map[0x6A0228] = 0x5855F8;   // call    _ZN8CFileMgr8OpenFileEPKcS1_  ; @CText::Load
		map[0x69FD5A] = 0x5852A8;   // call    _ZN8CFileMgr8OpenFileEPKcS1_  ; @CText::LoadMissionText
		map[0x57326E] = 0x4A388C;   // jz      loc_573313                    ; Check for some menu stuff to avoid reloading text
		map[0xBA6748] = 0x869630;   //??// CMenuManager FrontEndMenuManager
		map[0x573260] = 0x4A3882;   // _ZN12CMenuManager33InitialiseChangedLanguageSettingsEv
    }

    // std.movies
    if(true)
    {
		map[0x748B00] = 0x5FFFC3;   //  call    _CreateVideoPlayer  ; "movies/Logo.mpg"
		map[0x748BF9] = 0x600179;   //  call    _CreateVideoPlayer  ; "movies/GTAtitles.mpg"
    }

    // std.scm
    if(true)
    {
		map[0x468EC9] = 0x4506E6;   // call    _ZN8CFileMgr8OpenFileEPKcS1_  ; "main.scm" @CTheScripts
		map[0x489A4A] = 0x608C86;   // call    _ZN8CFileMgr8OpenFileEPKcS1_  ; "main.scm" @CRunningScript
    }

    // std.sprites
    if(true)
    {
		map[0x48418A] = 0x45944D;   // call    _ZN9CTxdStore7LoadTxdEiPKc
    }

    // std.asi
    if(true)
    {
		map[0x836F3B] = 0x6534A4;   // SetCurrentDirectory return pointer for _chdir
    }

    // std.stream
    if(true)
    {
        // !!!!! REMEMBER TO CHECK TRAITS
    }

    // AbstractFrontend | TheMenu
    if(true)
    {
		map[0xB72F20] = 0x7DDBA0;   // _ZN4CPad11OldKeyStateE CKeyState;
		map[0xB73190] = 0x7D15C0;   // _ZN4CPad11NewKeyStateE CKeyState;
		map[0x53ECBD] = 0x4A5BE0;   // call    _Z4IdlePv
		map[0x506EA0] = 0x5F9960;   // _ZN12CAudioEngine24ReportFrontendAudioEventEiff
        //map[0xB6BC90] = 0xB6BC90; // CAudioEngine AudioEngine
        //map[0x57E240] = 0x57E240; // _ZN12CMenuManager17DisplayHelperTextEPc
        //map[0x579330] = 0x579330; // _ZN12CMenuManager13MessageScreenEPKcbb
    }
}
