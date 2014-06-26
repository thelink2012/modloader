/*
 * San Andreas Mod Loader - Address Translation Between Game Versions
 * Copyright (C) 2013-2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 *
 */
#include <injector/injector.hpp>
#include <map>

// GTA SA 10US table
static void sa_10us(std::map<memory_pointer_raw, memory_pointer_raw>& map)
{
    bool isHoodlum = injector::address_manager::singleton().IsHoodlum();

    // Core
    if(true)
    {
        map[0x8246EC] = 0x8246EC;   // call    _WinMain
        map[0x6A0050] = 0x6A0050;   // _ZN5CText3GetEPKc
    }

    // std.fx
    if(true)
    {
        map[0x5B8F5E] = 0x5B8F5E;   // call    _ZN9CTxdStore7LoadTxdEiPKc ; "models/generic/vehicle.txd"
        map[0x5BF8B7] = 0x5BF8B7;   // call    _ZN9CTxdStore7LoadTxdEiPKc ; "models/particle.txd"
        map[0x5C248F] = 0x5C248F;   // call    _ZN9CTxdStore7LoadTxdEiPKc ; "models/effectsPC.txd"
        map[0x5C24B1] = 0x5C24B1;   // call    _Z8OpenFilePKcS0_          ; "models/effects.fxp"

        map[0x5BA850] = 0x5BA850;   // _ZN4CHud10InitialiseEv
        map[0x588850] = 0x588850;   // _ZN4CHud8ShutdownEv
        map[0x5827D0] = 0x5827D0;   // _ZN6CRadar12LoadTexturesEv
        map[0x5BA865] = 0x5BA865;   // call    _ZN9CTxdStore7LoadTxdEiPKc ; "models/hud.txd"

        map[0x5BA690] = 0x5BA690;   // _ZN5CFont10InitialiseEv
        map[0x7189B0] = 0x7189B0;   // _ZN5CFont8ShutdownEv
        map[0x5BA6A4] = 0x5BA6A4;   // call    _ZN9CTxdStore7LoadTxdEiPKc ; "models/fonts.txd"
        map[0x5BA7D4] = 0x5BA7D4;   // call    _ZN9CTxdStore7LoadTxdEiPKc ; "models/pcbtns.txd"

        map[0xBA6748] = 0xBA6748;   // CMenuManager FrontEndMenuManager
        map[0x572EC0] = 0x572EC0;   // _ZN12CMenuManager15LoadAllTexturesEv
        map[0x574630] = 0x574630;   // _ZN12CMenuManager14UnloadTexturesEv
        map[0x572F1E] = 0x572F1E;   // call    _ZN9CTxdStore7LoadTxdEiPKc ; "models/fronten1.txd"
        map[0x573040] = 0x573040;   // call    _ZN9CTxdStore7LoadTxdEiPKc ; "models/fronten2.txd"
        map[0x572FB5] = 0x572FB5;   // call    _ZN9CTxdStore7LoadTxdEiPKc ; "models/fronten_pc.txd"

        map[0x5DD95F] = 0x5DD95F;   // call    _ZN9CTxdStore7LoadTxdEiPKc ; "models/grass/plant1.txd"
        map[0x5DD272] = 0x5DD272;   // call    _RwStreamOpen              ; "models/grass/grass%d_%d.dff"
    }

    // std.text
    if(true)
    {
        map[0x57A161] = 0x57A161;   // call    _ZN5CText3GetEPKc
        map[0x6A0228] = 0x6A0228;   // call    _ZN8CFileMgr8OpenFileEPKcS1_  ; @CText::Load
        map[0x69FD5A] = 0x69FD5A;   // call    _ZN8CFileMgr8OpenFileEPKcS1_  ; @CText::LoadMissionText
        map[0x57326E] = 0x57326E;   // jz      loc_573313                    ; Check for some menu stuff to avoid reloading text
        map[0xBA6748] = 0xBA6748;   // CMenuManager FrontEndMenuManager
        map[0x573260] = 0x573260;   // _ZN12CMenuManager33InitialiseChangedLanguageSettingsEv
    }

    // std.movies
    if(true)
    {
        map[0x748B00] = 0x748B00;   //  call    _CreateVideoPlayer  ; "movies/Logo.mpg"
        map[0x748BF9] = 0x748BF9;   //  call    _CreateVideoPlayer  ; "movies/GTAtitles.mpg"
    }


    // AbstractFrontend | TheMenu
    if(true)
    {
        map[0xB72F20] = 0xB72F20;   // _ZN4CPad11OldKeyStateE CKeyState;
        map[0xB73190] = 0xB73190;   // _ZN4CPad11NewKeyStateE CKeyState;
        map[0x53ECBD] = 0x53ECBD;   // call    _Z4IdlePv
        map[0x506EA0] = 0x506EA0;   // _ZN12CAudioEngine24ReportFrontendAudioEventEiff
        map[0xB6BC90] = 0xB6BC90;   // CAudioEngine AudioEngine
        map[0x57E240] = 0x57E240;   // _ZN12CMenuManager17DisplayHelperTextEPc
        map[0x579330] = 0x579330;   // _ZN12CMenuManager13MessageScreenEPKcbb
    }

    // ---> All the following addresses are optional and depends only on the existence of 0x8CE008 (aScreens) on the map
    if(true)
    {
        // MenuExtender
        map[0x8CE008] = 0x8CE008;   // CMenuItem aScreens[]
        map[0xBA6748] = 0xBA6748;   // CMenuManager FrontEndMenuManager
        map[0x576FEF] = 0x576FEF;   // call    _ZN12CMenuManager20ProcessPCMenuOptionsEah
        map[0x57B9FD] = 0x57B9FD;   // call    _ZN9CSprite2d4DrawERK5CRectRK5CRGBA
        map[0x58061B] = 0x58061B;   // call    _ZN12CMenuManager16ProcessUserInputEhhhha
        map[0x57BA58] = 0x57BA58;   // call    _ZN12CMenuManager17DrawStandardMenusEh
        map[0x579D9D] = 0x579D9D;   // ja      loc_57A168      ; jumptable 00579DAA default case
        map[0x579DEE] = 0x579DEE;   // loc_579DEE:             ; jumptable 00579DAA case 12
        map[0x57A161] = 0x57A161;   // call    _ZN5CText3GetEPKc
        map[0x5794CB] = 0x5794CB;   // call    _ZN5CFont12SetAlignmentE14eFontAlignment ; before the loop
        map[0x579AC9] = 0x579AC9;   // call    _ZN5CFont12SetAlignmentE14eFontAlignment ; in loop
        // MenuExtender aScreen references
        map[0x57345A] = 0x57345A;   // Maybe we need to remount those, 
        map[0x57370A] = 0x57370A;   // since they point to the middle of the instruction instead of
        map[0x573713] = 0x573713;   // the beggining of it
        map[0x573728] = 0x573728;
        map[0x57373D] = 0x57373D;
        map[0x573752] = 0x573752;
        map[0x573772] = 0x573772;
        map[0x573EA9] = 0x573EA9;
        map[0x576B08] = 0x576B08;
        map[0x576B1E] = 0x576B1E;
        map[0x576B38] = 0x576B38;
        map[0x576B58] = 0x576B58;
        map[0x577017] = 0x577017;
        map[0x57723D] = 0x57723D;
        map[0x577280] = 0x577280;
        map[0x5772F2] = 0x5772F2;
        map[0x579568] = 0x579568;
        map[0x57967E] = 0x57967E;
        map[0x5796AF] = 0x5796AF;
        map[0x57981F] = 0x57981F;
        map[0x5798D6] = 0x5798D6;
        map[0x5798FC] = 0x5798FC;
        map[0x579AB2] = 0x579AB2;
        map[0x579AE4] = 0x579AE4;
        map[0x579AEE] = 0x579AEE;
        map[0x579AF7] = 0x579AF7;
        map[0x579B10] = 0x579B10;
        map[0x579B17] = 0x579B17;
        map[0x579B2E] = 0x579B2E;
        map[0x579B3A] = 0x579B3A;
        map[0x579B43] = 0x579B43;
        map[0x579B5A] = 0x579B5A;
        map[0x579B70] = 0x579B70;
        map[0x579B7A] = 0x579B7A;
        map[0x579B8B] = 0x579B8B;
        map[0x579B9F] = 0x579B9F;
        map[0x579BC3] = 0x579BC3;
        map[0x579C83] = 0x579C83;
        map[0x579D20] = 0x579D20;
        map[0x579D3F] = 0x579D3F;
        map[0x579D4A] = 0x579D4A;
        map[0x579D93] = 0x579D93;
        map[0x57A18A] = 0x57A18A;
        map[0x57A1BD] = 0x57A1BD;
        map[0x57A235] = 0x57A235;
        map[0x57A39F] = 0x57A39F;
        map[0x57A455] = 0x57A455;
        map[0x57A469] = 0x57A469;
        map[0x57A4B9] = 0x57A4B9;
        map[0x57A4DA] = 0x57A4DA;
        map[0x57A54F] = 0x57A54F;
        map[0x57A615] = 0x57A615;
        map[0x57A65D] = 0x57A65D;
        map[0x57A6A5] = 0x57A6A5;
        map[0x57A6E6] = 0x57A6E6;
        map[0x57A729] = 0x57A729;
        map[0x57A77C] = 0x57A77C;
        map[0x57A7BE] = 0x57A7BE;
        map[0x57B27E] = 0x57B27E;
        map[0x57B4F2] = 0x57B4F2;
        map[0x57B519] = 0x57B519;
        map[0x57B52A] = 0x57B52A;
        map[0x57B534] = 0x57B534;
        map[0x57B588] = 0x57B588;
        map[0x57B5A4] = 0x57B5A4;
        map[0x57B5C9] = 0x57B5C9;
        map[0x57B5E9] = 0x57B5E9;
        map[0x57B601] = 0x57B601;
        map[0x57B629] = 0x57B629;
        map[0x57B69C] = 0x57B69C;
        map[0x57B6F1] = 0x57B6F1;
        map[0x57C313] = 0x57C313;
        map[0x57CD6B] = 0x57CD6B;
        map[0x57D26C] = 0x57D26C;
        map[0x57D287] = 0x57D287;
        map[0x57D2D2] = 0x57D2D2;
        map[0x57D6D8] = 0x57D6D8;
        map[0x57D701] = 0x57D701;
        map[0x57E3F7] = 0x57E3F7;
        map[0x57FE0A] = 0x57FE0A;
        map[0x57FE25] = 0x57FE25;
        map[0x57FE57] = 0x57FE57;
        map[0x57FE96] = 0x57FE96;
        map[0x57FF5F] = 0x57FF5F;
        map[0x57FFAE] = 0x57FFAE;
        map[0x580316] = 0x580316;
        map[0x580496] = 0x580496;
        map[0x5804EB] = 0x5804EB;
        map[0x5805D3] = 0x5805D3;
        map[0x57FE57] = 0x57FE57;
        map[0x57FE96] = 0x57FE96;
    }
}
