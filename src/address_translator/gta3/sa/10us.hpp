/*
 * Mod Loader - Address Translation Between Game Versions
 * Copyright (C) 2013-2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 *
 */
#include <modloader/util/injector.hpp>
#include <map>

// GTA SA 10US table
static void sa_10us(std::map<memory_pointer_raw, memory_pointer_raw>& map)
{
    using namespace modloader;
    bool isHoodlum = injector::address_manager::singleton().IsHoodlum();

    //
    // (Notice '->' means "exactly pointing to"!!!!!)
    //

    // Core
    if(true)
    {
        map[0x8246EC] = 0x8246EC;   // call    _WinMain
        map[0x53ECBD] = 0x53ECBD;   // call    _Z4IdlePv
        map[0x53ECCB] = 0x53ECCB;   // call    _Z12FrontendIdlePv
    }

    // std.fx
    if(true)
    {
        map[0x5900D2] = 0x5900D2;   // call    _ZN9CTxdStore7LoadTxdEiPKc ; "loadscs.txd"
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

    // std.scm
    if(true)
    {
        map[0x468EC9] = 0x468EC9;   // call    _ZN8CFileMgr8OpenFileEPKcS1_  ; "main.scm" @CTheScripts
        map[0x489A4A] = 0x489A4A;   // call    _ZN8CFileMgr8OpenFileEPKcS1_  ; "main.scm" @CRunningScript
    }

    // std.sprites
    if(true)
    {
        map[0x48418A] = 0x48418A;   // call    _ZN9CTxdStore7LoadTxdEiPKc
    }

    // std.asi
    if(true)
    {
        map[0x836F3B] = 0x836F3B;   // SetCurrentDirectory return pointer for _chdir
    }

    // std.stream
    if(true)
    {
        map[0x5B8AE8] = 0x5B8AE8;   // -> offset ms_aInfoForModel
        map[0x5A419B] = 0x5A419B;   // -> offset clothesDirectory

        map[0x8E3FE0] = 0x8E3FE0;   // DWORD StreamCreateFlags
        map[0x8E3FEC] = 0x8E3FEC;   // CdStreamInfo cdinfo
        map[0x8E4CAC] = 0x8E4CAC;   // void* CStreaming::ms_pStreamingBuffer[2]
        map[0x8E4CA8] = 0x8E4CA8;   // unsigned int CStreaming::ms_streamingBufferSize

        map[0x72F4C0] = 0x72F4C0;   // _ZN10CMemoryMgr11MallocAlignEjj
        map[0x72F4F0] = 0x72F4F0;   // _ZN10CMemoryMgr9FreeAlignEPv
        map[0x532310] = 0x532310;   // _ZN10CDirectory7AddItemERKNS_13DirectoryInfoE
        map[0x532450] = 0x532450;   // _ZN10CDirectory8FindItemEPKc

        map[0x406560] = 0x406560;   // _Z14CdStreamThreadPv
        map[0x5B8E1B] = 0x5B8E1B;   // call    _ZN10CStreaming15LoadCdDirectoryEv       ; @CStreaming::Init
        map[0x40CF34] = 0x40CF34;   // call    _Z12CdStreamReadiPvjj                    ; @CStreaming::RequestModelStream
        map[0x40CCA6] = 0x40CCA6;   // mov     edx, ms_aInfoForModel.iBlockCount[eax*4] ; @CStreaming::RequestModelStream
        map[0x406A5B] = isHoodlum? 0x0156C2FB : 0x406A5B;   // and     esi, 0FFFFFFh    ; @CdStreamRead
        map[0x409F76] = 0x409F76;   // call    _ZN10CDirectory8FindItemEPKcRjS2_        ; @CStreaming::RequestSpecialModel
        map[0x409FD9] = 0x409FD9;   // call    _ZN10CStreaming12RequestModelEii         ; @CStreaming::RequestSpecialModel
        map[0x5B6183] = 0x5B6183;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CStreaming::LoadCdDirectory
        map[0x532361] = 0x532361;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CDirectory::ReadDirFile
        map[0x5AFC9D] = 0x5AFC9D;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CCutsceneMgr::LoadCutsceneData_postload
        map[0x5AFBEF] = 0x5AFBEF;   // call    _RwStreamOpen                            ; @CCutsceneMgr::LoadCutsceneData_postload
        map[0x5B07E9] = 0x5B07E9;   // call    _RwStreamOpen                            ; @CCutsceneMgr::LoadCutsceneData_preload
        map[0x40685E] = 0x40685E;   // call    ds:__imp_CreateFileA                     ; @CdStreamOpen
        map[0x5B8310] = 0x5B8310;   // call    _ZN10CStreaming15LoadCdDirectoryEPKci    ; @CStreaming::LoadCdDirectory
        map[0x5B61B8] = 0x5B61B8;   // call    _ZN8CFileMgr4ReadEiPci                   ; @CStreaming::LoadCdDirectory  ; read entry count
        map[0x5B61E1] = 0x5B61E1;   // call    _ZN8CFileMgr4ReadEiPci                   ; @CStreaming::LoadCdDirectory  ; read entry
        map[0x5B627A] = 0x5B627A;   // call    _ZN10CDirectory7AddItemERKNS_13DirectoryInfoE ; @CStreaming::LoadCdDirectory
        map[0x5B6449] = 0x5B6449;   // call    _ZN14CStreamingInfo16GetCdPosnAndSizeERjS0_  ; @CStreaming::LoadCdDirectory
        map[0x5B630B] = 0x5B630B;   // call    _ZN9CColStore10AddColSlotEPKc
        map[0x5B63E8] = 0x5B63E8;   // call    _ZN17CVehicleRecording21RegisterRecordingFileEPKc
        map[0x5B6419] = 0x5B6419;   // call    _ZN16CStreamedScripts14RegisterScriptEPKc
        map[0x4D565A] = 0x4D565A;   // call    _RwStreamOpen    ; @CAnimManager::LoadAnimFiles "anim/ped.ifp"

        map[0xB74490] = 0xB74490;   // CPool<> *CPools::ms_pPedPool
        map[0xB74494] = 0xB74494;   // CPool<> *CPools::ms_pVehiclePool
        map[0xB74498] = 0xB74498;   // CPool<> *CPools::ms_pBuildingPool
        map[0xB7449C] = 0xB7449C;   // CPool<> *CPools::ms_pObjectPool
        map[0x4D6C30] = 0x4D6C30;   // _Z32RpAnimBlendClumpGiveAssociationsP7RpClumpP21CAnimBlendAssociation
        map[0x4D6BE0] = 0x4D6BE0;   // _Z35RpAnimBlendClumpExtractAssociationsP7RpClump
        map[0x681810] = 0x681810;   // _ZNK12CTaskManager16GetTaskSecondaryEi
        map[0x561A00] = 0x561A00;   // _ZN6CTimer6ResumeEv
        map[0x5619D0] = 0x5619D0;   // _ZN6CTimer7SuspendEv
        map[0x4087E0] = 0x4087E0;   // _ZN10CStreaming12RequestModelEii
        map[0x4089A0] = 0x4089A0;   // _ZN10CStreaming11RemoveModelEi
        map[0x40EA10] = 0x40EA10;   // _ZN10CStreaming22LoadAllRequestedModelsEb
        map[0x40E460] = 0x40E460;   // _ZN10CStreaming13FlushChannelsEv
        map[0x40CF80] = 0x40CF80;   // _ZN10CStreaming21RemoveAllUnusedModelsEv
        map[0x40CFD0] = 0x40CFD0;   // _ZN10CStreaming20RemoveLeastUsedModelEj

        // Those are related to player clothing
        map[0x40A106] = isHoodlum? 0x0156644F : 0x40A106;   // call    _ZN10CStreaming12RequestModelEii ; @CStreaming::RequestFile
        map[0x532341] = 0x532341;   // call    _printf                                  ; @CDirectory::AddItem
        map[0x5A6A01] = 0x5A6A01;   // call    _ZN10CDirectory11ReadDirFileEPKc         ; @CClothesBuilder::CreateSkinnedClump
        map[0x5A4190] = 0x5A4190;   // _ZN15CClothesBuilder15LoadCdDirectoryEv
        map[0x56E210] = 0x56E210;   // _Z13FindPlayerPedi 
        map[0x5A82C0] = 0x5A82C0;   // _ZN8CClothes13RebuildPlayerEP10CPlayerPedb
        map[0x5A8346] = 0x5A8346;   // push    offset _PlayerClothes                    ; @CClothes::RebuildPlayer
    }

    // traits
    if(true)
    {
        map[0x5B62CF] = 0x5B62CF;   // -> 4E20h ; TXD Start Index
        map[0x408897] = 0x408897;   // -> offset _ZN10CModelInfo16ms_modelInfoPtrsE

        /*
        TODO VTBL

        Find all "::vtbl", Subfolders, Find Results 1, "Entire Solution ( Including External Items )", ""
          C:\Projects\modloader\src\traits\gta3\sa.hpp(84):        return ModelType(injector::thiscall<uint8_t(void*)>::vtbl<4>(m));
          C:\Projects\modloader\src\plugins\gta3\std.stream\refresh.cpp(382):        injector::thiscall<void(void*, int)>::vtbl<5>(entity, GetEntityModel(entity));  // CEntity::SetModelIndex
          C:\Projects\modloader\src\plugins\gta3\std.stream\refresh.cpp(388):        injector::thiscall<void(void*)>::vtbl<8>(entity);   // CEntity::DeleteRwObject
          C:\Projects\modloader\src\plugins\gta3\std.stream\refresh.cpp(420):                injector::thiscall<void(CTask*, void*, int, int)>::vtbl<6>(task, entity, 2, 0); // CTask::MakeAbortable
          C:\Projects\modloader\src\plugins\gta3\std.stream\refresh.cpp(433):            injector::thiscall<void(void*)>::vtbl<48>(entity);  // CVehicle::SetupSuspensionLines
          Matching lines: 5    Matching files: 2    Total files searched: 342

        */
    }


    // ---> All the following addresses are optional and depends only on the existence of 0x8CE008 (aScreens) on the map
    // AbstractFrontend | TheMenu
    if(true)
    {
        // MenuExtender
        map[0x8CE008] = 0x8CE008;   // CMenuItem aScreens[]
        map[0xBA6748] = 0xBA6748;   // CMenuManager FrontEndMenuManager
        map[0xB6BC90] = 0xB6BC90;   // CAudioEngine AudioEngine
        map[0xB72F20] = 0xB72F20;   // _ZN4CPad11OldKeyStateE CKeyState;
        map[0xB73190] = 0xB73190;   // _ZN4CPad11NewKeyStateE CKeyState;
        map[0x506EA0] = 0x506EA0;   // _ZN12CAudioEngine24ReportFrontendAudioEventEiff
        map[0x57E240] = 0x57E240;   // _ZN12CMenuManager17DisplayHelperTextEPc
        map[0x579330] = 0x579330;   // _ZN12CMenuManager13MessageScreenEPKcbb
        map[0x576FEF] = 0x576FEF;   // call    _ZN12CMenuManager20ProcessPCMenuOptionsEah
        map[0x57B9FD] = 0x57B9FD;   // call    _ZN9CSprite2d4DrawERK5CRectRK5CRGBA
        map[0x58061B] = 0x58061B;   // call    _ZN12CMenuManager16ProcessUserInputEhhhha
        map[0x57BA58] = 0x57BA58;   // call    _ZN12CMenuManager17DrawStandardMenusEh
        map[0x579D9D] = 0x579D9D;   // ja      loc_57A168      ; jumptable 00579DAA default case
        map[0x579DEE] = 0x579DEE;   // loc_579DEE:             ; jumptable 00579DAA case 12
        map[0x6A0050] = 0x6A0050;   // _ZN5CText3GetEPKc
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
