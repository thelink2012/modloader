/*
 * San Andreas Mod Loader - Address Translation Between Game Versions
 * Copyright (C) 2013-2015  LINK/2012 <dma_2012@hotmail.com>
 * Copyright (C) 2014-2015  ThirteenAG <thirteenag@gmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 *
 */
#include <modloader/util/injector.hpp>
#include <map>

// GTA 3 10 table
static void III_10(std::map<memory_pointer_raw, memory_pointer_raw>& map)
{
    //
    // (Notice '->' means "exactly pointing to"!!!!!)
    //

    // Core
    if(true)
    {
        map[0x8246EC] = 0x5C1F39;   // call    _WinMain
        map[0x53ECBD] = 0x48E8FD;   // call    _Z4IdlePv
        map[0x53ECCB] = 0x48E90F;   // call    _Z12FrontendIdlePv
        map[0xC8D4C0] = 0x8F5838;   // int gGameState
    }

    // std.fx
    if(true)
    {
		map[0x5BF8B7] = 0x48BF5E;   // call    _ZN9CTxdStore7LoadTxdEiPKc ; "models/particle.txd"
        
		map[0x5BA850] = 0x5048F0;   // _ZN4CHud10InitialiseEv
		map[0x588850] = 0x504C50;   // _ZN4CHud8ShutdownEv
		map[0x5827D0] = 0x4A4030;   // _ZN6CRadar12LoadTexturesEv
		map[0x5BA865] = 0x504905;   // call    _ZN9CTxdStore7LoadTxdEiPKc ; "models/hud.txd"

		map[0x5BA690] = 0x500A40;   // _ZN5CFont10InitialiseEv
		map[0x7189B0] = 0x500BA0;   // _ZN5CFont8ShutdownEv
		map[0x5BA6A4] = 0x500A57;   // call    _ZN9CTxdStore7LoadTxdEiPKc ; "models/fonts.txd"

		map[0xBA6748] = 0x8F59D8;   // CMenuManager FrontEndMenuManager
		map[0x572EC0] = 0x47A230;   // _ZN12CMenuManager15LoadAllTexturesEv
		map[0x574630] = 0x47A440;   // _ZN12CMenuManager14UnloadTexturesEv
		map[0x572F1E] = 0x47A34F;   // call    _ZN9CTxdStore7LoadTxdEiPKc ; "models/frontend.txd"
		map[0x573040] = 0x47A3D3;   // call    _ZN9CTxdStore7LoadTxdEiPKc ; "models/menu.txd"

        // VC/III stuff
        map[xVc(0x570D8A)] = 0x51B394;  // call    _ZN11CFileLoader21LoadAtomicFile2ReturnEPKc ; "models/generic/zonecylb.dff"
        map[xVc(0x570D7A)] = 0x51B384;  // call    _ZN11CFileLoader21LoadAtomicFile2ReturnEPKc ; "models/generic/arrow.dff"
        map[xVc(0x4BBB30)] = 0x4A1700;  // _ZN11CPlayerInfo14LoadPlayerSkinEv
        map[xVc(0x94AD28)] = 0x9412F0;  // CPlayerInfo _ZN6CWorld7PlayersE[]
        map[xVc(0x627EB8)] = 0x59BA42;  // push    offset "models\generic\player.bmp"
        map[0x7F3820] = 0x5A7330;  // _RwTextureDestroy
        map[xVc(0x627E7F)] = 0x59BA0F;  // call    RwTextureRead    ; @CPlayerSkin::GetSkinTexture
    }

    // std.text
    if(true)
    {
		map[0x57A161] = 0x4813CD;   // call    _ZN5CText3GetEPKc                        ; fxtp
        map[0x748CFB] = 0x582E6C;   // call    _Z14InitialiseGamev                      ; fxtp
		map[0x6A0228] = 0x52C436;   // call    _ZN8CFileMgr8OpenFileEPKcS1_  ; @CText::Load
		map[0x57326E] = 0x47A4DE;   // jz      loc_573313                    ; Check for some menu stuff to avoid reloading text
		map[0xBA6748] = 0x8F59D8;   // CMenuManager FrontEndMenuManager
		map[0x573260] = 0x47A4D0;   // _ZN12CMenuManager33InitialiseChangedLanguageSettingsEv
        map[0x6A0050] = 0x52C5A0;   // _ZN5CText3GetEPKc
    }

    // std.movies
    if(true)
    {
		map[0x748B00] = 0x582A93;   //  call    _CreateVideoPlayer  ; "movies/Logo.mpg"
		map[0x748BF9] = 0x582C26;   //  call    _CreateVideoPlayer  ; "movies/GTAtitles.mpg"
    }

    // std.scm
    if(true)
    {
		map[0x468EC9] = 0x438869;   // call    _ZN8CFileMgr8OpenFileEPKcS1_  ; "main.scm" @CTheScripts
		map[0x489A4A] = 0x588E13;   // call    _ZN8CFileMgr8OpenFileEPKcS1_  ; "main.scm" @CRunningScript
    }

    // std.sprites
    if(true)
    {
		map[0x48418A] = 0x44D6E9;   // call    _ZN9CTxdStore7LoadTxdEiPKc
    }

    // std.asi
    if(true)
    {
		map[0x836F3B] = 0x5AB904;   // SetCurrentDirectory return pointer for _chdir
    }

#if 0
    // std.stream
    if(true)
    {
        map[0x5B8AE8] = ;   // -> offset ms_aInfoForModel
        map[0x5A419B] = ;   // -> offset clothesDirectory
        map[0x5B8AFC] = ;   // -> &ms_aInfoForModel[MAX_INFO_FOR_MODEL]

        map[0x8E3FE0] = ;   // DWORD StreamCreateFlags
        map[0x8E3FEC] = ;   // CdStreamInfo cdinfo
        map[0x8E4CAC] = ;   // void* CStreaming::ms_pStreamingBuffer[2]
        map[0x8E4CA8] = ;   // unsigned int CStreaming::ms_streamingBufferSize

        map[0x72F420] = ;   // _ZN10CMemoryMgr6MallocEj
        map[0x72F4C0] = ;   // _ZN10CMemoryMgr11MallocAlignEjj
        map[0x72F4F0] = ;   // _ZN10CMemoryMgr9FreeAlignEPv
        map[0x532310] = ;   // _ZN10CDirectory7AddItemERKNS_13DirectoryInfoE
        map[0x5324A0] = ;   // _ZN10CDirectory8FindItemEPKcRjS2_

        map[0x406560] = ;   // _Z14CdStreamThreadPv
        map[0x5B8E1B] = ;   // call    _ZN10CStreaming15LoadCdDirectoryEv       ; @CStreaming::Init
        map[0x40CF34] = ;   // call    _Z12CdStreamReadiPvjj                    ; @CStreaming::RequestModelStream
        map[0x40CCA6] = ;   // mov     edx, ms_aInfoForModel.iBlockCount[eax*4] ; @CStreaming::RequestModelStream
        map[0x406A5B] = ;   // and     esi, 0FFFFFFh    ; @CdStreamRead
        map[0x409F76] = ;   // call    _ZN10CDirectory8FindItemEPKcRjS2_        ; @CStreaming::RequestSpecialModel
        map[0x409FD9] = ;   // call    _ZN10CStreaming12RequestModelEii         ; @CStreaming::RequestSpecialModel
        map[0x5B6183] = ;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CStreaming::LoadCdDirectory
        map[0x532361] = ;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CDirectory::ReadDirFile
        map[0x5AFC9D] = ;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CCutsceneMgr::LoadCutsceneData_postload
        map[0x5AFBEF] = ;   // call    _RwStreamOpen                            ; @CCutsceneMgr::LoadCutsceneData_postload
        map[0x5B07E9] = ;   // call    _RwStreamOpen                            ; @CCutsceneMgr::LoadCutsceneData_preload
        map[0x40685E] = ;   // call    ds:__imp_CreateFileA                     ; @CdStreamOpen
        map[0x5B8310] = ;   // call    _ZN10CStreaming15LoadCdDirectoryEPKci    ; @CStreaming::LoadCdDirectory
        map[0x5B61B8] = ;   // call    _ZN8CFileMgr4ReadEiPci                   ; @CStreaming::LoadCdDirectory  ; read entry count
        map[0x5B61E1] = ;   // call    _ZN8CFileMgr4ReadEiPci                   ; @CStreaming::LoadCdDirectory  ; read entry
        map[0x5B627A] = ;   // call    _ZN10CDirectory7AddItemERKNS_13DirectoryInfoE ; @CStreaming::LoadCdDirectory
        map[0x5B6449] = ;   // call    _ZN14CStreamingInfo16GetCdPosnAndSizeERjS0_  ; @CStreaming::LoadCdDirectory
        map[0x5B630B] = ;   // call    _ZN9CColStore10AddColSlotEPKc
        map[0x5B63E8] = ;   // call    _ZN17CVehicleRecording21RegisterRecordingFileEPKc
        map[0x5B6419] = ;   // call    _ZN16CStreamedScripts14RegisterScriptEPKc
        map[0x4D565A] = ;   // call    _RwStreamOpen    ; @CAnimManager::LoadAnimFiles "anim/ped.ifp"
        map[0x40E2C5] = ;   // call    _ZN10CStreaming21ConvertBufferToObjectEPcii
        map[0x40E1BE] = ;   // call    _ZN10CStreaming22FinishLoadingLargeFileEPci

        map[0xB74490] = ;   // CPool<> *CPools::ms_pPedPool
        map[0xB74494] = ;   // CPool<> *CPools::ms_pVehiclePool
        map[0xB74498] = ;   // CPool<> *CPools::ms_pBuildingPool
        map[0xB7449C] = ;   // CPool<> *CPools::ms_pObjectPool
        map[0x4D6C30] = ;   // _Z32RpAnimBlendClumpGiveAssociationsP7RpClumpP21CAnimBlendAssociation
        map[0x4D6BE0] = ;   // _Z35RpAnimBlendClumpExtractAssociationsP7RpClump
        map[0x681810] = ;   // _ZNK12CTaskManager16GetTaskSecondaryEi
        map[0x561A00] = ;   // _ZN6CTimer6ResumeEv
        map[0x5619D0] = ;   // _ZN6CTimer7SuspendEv
        map[0x4087E0] = ;   // _ZN10CStreaming12RequestModelEii
        map[0x4089A0] = ;   // _ZN10CStreaming11RemoveModelEi
        map[0x40EA10] = ;   // _ZN10CStreaming22LoadAllRequestedModelsEb
        map[0x40E460] = ;   // _ZN10CStreaming13FlushChannelsEv
        map[0x40CF80] = ;   // _ZN10CStreaming21RemoveAllUnusedModelsEv
        map[0x40CFD0] = ;   // _ZN10CStreaming20RemoveLeastUsedModelEj

        // Those are related to player clothing
        map[0x40A106] = ;   // call    _ZN10CStreaming12RequestModelEii ; @CStreaming::RequestFile
        map[0x532341] = ;   // call    _printf                                  ; @CDirectory::AddItem
        map[0x5A6A01] = ;   // call    _ZN10CDirectory11ReadDirFileEPKc         ; @CClothesBuilder::CreateSkinnedClump
        map[0x5A4190] = ;   // _ZN15CClothesBuilder15LoadCdDirectoryEv
        map[0x56E210] = ;   // _Z13FindPlayerPedi 
        map[0x5A82C0] = ;   // _ZN8CClothes13RebuildPlayerEP10CPlayerPedb
        map[0x5A8346] = ;   // push    offset _PlayerClothes                    ; @CClothes::RebuildPlayer

        // Non streamed resources
        map[0x5B9188] = ;   // call    _ZN11CFileLoader17LoadCollisionFileEPKch
        map[0x5B91B0] = ;   // call    _ZN11CFileLoader14LoadAtomicFileEPKc
        map[0x5B91DB] = ;   // call    _ZN11CFileLoader13LoadClumpFileEPKc
        map[0x5B910A] = ;   // call    _ZN11CFileLoader17LoadTexDictionaryEPKc
    }
#endif

#if 0
    // std.data
    if(true)
    {
        map[0x464D50] = ;   // _ZN11CTheScripts18IsPlayerOnAMissionEv
        map[0x5B6890] = ;   // _ZN17CVehicleModelInfo18LoadVehicleColoursEv
        map[0x5B68AB] = ;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CVehicleModelInfo::LoadVehicleColours
        map[0x5B65A0] = ;   // _ZN17CVehicleModelInfo19LoadVehicleUpgradesEv
        map[0x5B65BE] = ;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CVehicleModelInfo::LoadVehicleUpgrades
        map[0xB4E6D8] = ;   // CLinkedUpgradeList CVehicleModelInfo::ms_linkedUpgradesE
        map[0x4C6770] = ;   // mov     eax, gsVehicleModels.m_nCount
        map[0x5B905E] = ;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CFileLoader::LoadLevel
        map[0x5BD830] = ;   // _ZN16cHandlingDataMgr16LoadHandlingDataEv
        map[0x5BD850] = ;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @cHandlingDataMgr::LoadHandlingData
        map[0xC2B9C8] = ;   // mod_HandlingManager CHandlingData
        map[0x5B8428] = ;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CFileLoader::LoadObjectTypes
        map[0x5B871A] = ;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CFileLoader::LoadScene
        map[0x5DD780] = ;   // _ZN9CPlantMgr12ReloadConfigEv
        map[0x5DD3D1] = ;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CPlantSurfPropMgr::LoadPlantsDat
        map[0x5BF750] = ;   // _ZN11CWeaponInfo10InitialiseEv
        map[0x5BE68A] = ;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CWeaponInfo::Initialise
        map[0x5BC090] = ;   // _ZN9CPopCycle10InitialiseEv
        map[0x5BC0AE] = ;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CPopCycle::Initialise
        map[0x5BBAC0] = ;   // _ZN10CTimeCycle10InitialiseEb 
        map[0x5BBADE] = ;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CTimeCycle::initialise
        map[0x6EAE80] = ;   // _ZN11CWaterLevel20WaterLevelInitialiseEv
        map[0x5A7B30] = ;   // _ZN8CClothes15LoadClothesFileEv
        map[0x5A7B54] = ;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CClothes::LoadClothesFile
        map[0x5C0297] = ;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @FurnitureManager_c::LoadFurniture
        map[0x6EAF4D] = ;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CWaterLevel::WaterLevelInitialise
        map[0x5BD1A0] = ;   // _ZN11CPopulation13LoadCarGroupsEv
        map[0x5BD1BB] = ;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CPopulation::LoadCarGroups
        map[0x5BCFE0] = ;   // _ZN11CPopulation13LoadPedGroupsEv
        map[0x5BCFFB] = ;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CPopulation::LoadPedGroups
        map[0x5BB890] = ;   // _ZN9CPedStats12LoadPedStatsEv
        map[0x5BB89F] = ;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CPedStats::LoadPedStats
        map[0x5B5360] = ;   // _ZN11CObjectData10InitialiseEPcb
        map[0x5B5444] = ;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CObjectData::Initialise
        map[0x7187C0] = ;   // _ZN5CFont14LoadFontValuesEv
        map[0x7187DB] = ;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CFont::LoadFontValues
        map[0x461100] = ;   // _ZN11CRoadBlocks4InitEv
        map[0x461125] = ;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CRoadBlocks::Init
        map[0x5599B0] = ;   // _ZN6CStats23LoadActionReactionStatsEv
        map[0x5599D8] = ;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CStats::LoadActionReactionStats
        map[0x55988F] = ;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CStats::LoadStatUpdateConditions
        map[0x608B45] = ;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CPedType::LoadPedData
        map[0x5BEDC0] = ;   // _ZN16CTaskSimpleFight13LoadMeleeDataEv
        map[0x5BEF47] = ;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CTaskSimpleFight::LoadMeleeData
        map[0x5BC92B] = ;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CAnimManager::ReadAnimAssociationDefinitions
        map[0x5BF400] = ;   //  _ZN29CDecisionMakerTypesFileLoader24LoadDefaultDecisionMakerEv
        map[0x6076CE] = ;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CDecisionMakerTypesFileLoader::LoadDecisionMaker
        map[0x5A3EA0] = ;   // _ZN15ProcObjectMan_c4InitEv
        map[0xBB7CB0] = ;   // ProcObjectMan g_procObjMan
        map[0x5A3154] = ;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @ProcObjectMan_c::LoadDataFile
        map[0x55F420] = ;   // _ZN14SurfaceInfos_c4InitEv
        map[0xB79538] = ;   // SurfaceInfos_c g_surfaceInfos
        map[0x55D100] = ;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @SurfaceInfos_c::LoadAdhesiveLimits
        map[0x55EBA4] = ;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @SurfaceInfos_c::LoadSurfaceInfos
        map[0x55F2C1] = ;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @SurfaceInfos_c::LoadSurfaceAudioInfos
        map[0x49AE30] = ;   // _ZN9CShopping16RemoveLoadedShopEv
        map[0x49BBE0] = ;   // _ZN9CShopping8LoadShopEPKc
        map[0xA9A7D8] = ;   // char _ZN9CShopping13ms_shopLoadedE[24]
        map[0x49B6AF] = ;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CShopping::LoadStats
        map[0x49B93F] = ;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CShopping::LoadPrices
        map[0x49BC98] = ;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CShopping::LoadShop
        map[0x6F7440] = ;   // _ZN6CTrain10InitTrainsEv
        map[0x6F7470] = ;   // call    _ZN6CTrain25ReadAndInterpretTrackFileEPcPP10CTrainNodePiPfi ; "tracks.dat"
        map[0x6F74BC] = ;   // call    _ZN6CTrain25ReadAndInterpretTrackFileEPcPP10CTrainNodePiPfi ; "tracks2.dat"
        map[0x6F7496] = ;   // call    _ZN6CTrain25ReadAndInterpretTrackFileEPcPP10CTrainNodePiPfi ; "tracks3.dat"
        map[0x6F74E2] = ;   // call    _ZN6CTrain25ReadAndInterpretTrackFileEPcPP10CTrainNodePiPfi ; "tracks4.dat"
        map[0x748CFB] = ;   // call    _Z14InitialiseGamev
        map[0x590D2A] = ;   // mov     eax, 8Ch                                 ; 8Ch = Loading Screen Max Progress
        map[0x590D67] = ;   // cmp     eax, 8Ch                                 ; 8Ch = Loading Screen Max Progress
        map[0x5B906A] = ;   // call    _ZN11CFileLoader8LoadLineEi              ; @CFileLoader::LoadLevel -- loop begin
        map[0x5B92E6] = ;   // call    _ZN11CFileLoader8LoadLineEi              ; @CFileLoader::LoadLevel -- loop end
        map[0x5B92F9] = ;   // call    _ZN8CFileMgr9CloseFileEi                 ; @CFileLoader::LoadLevel
        map[0x53BC95] = ;   // call    _ZN11CFileLoader9LoadLevelEPKc           ; @CGame::Initialise -- default.dat
        map[0x53BC9B] = ;   // call    _ZN11CFileLoader9LoadLevelEPKc           ; @CGame::Initialise -- gta.dat
        map[0x4C5940] = ;   // _ZN10CModelInfo12GetModelInfoEPKcPi
        map[0x5B922F] = ;   // call    _Z20MatchAllModelStringsv                ; @CFileLoader::LoadLevel
    }
#endif

    // traits
    if(true)
    {
        map[0x5B62CF] = 0x40702D;   // -> DWORD 4E20h   ; TXD Start Index+
        map[0x408897] = 0x83D408;   // -> offset _ZN10CModelInfo16ms_modelInfoPtrsE+
    }

    // AbstractFrontend | TheMenu
    if(true)
    {
        // Not available for III
    }
}
