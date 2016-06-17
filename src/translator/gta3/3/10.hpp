/*
 * San Andreas Mod Loader - Address Translation Between Game Versions
 * Copyright (C) 2013-2015  LINK/2012 <dma_2012@hotmail.com>
 * Copyright (C) 2014-2015  ThirteenAG <thirteenag@gmail.com>
 * Licensed under the MIT License, see LICENSE at top level directory.
 *
 */
#include <modloader/util/injector.hpp>
#include <map>

 // Emulating SA's CDirectory::FindItem(ecx, name) for III
static void* __fastcall gta3emu_CDirectory_FindItem2(void* self, int, const char* name)
{
    // The functions (and thus the patches) are identical between III and VC!
    return vcemu_CDirectory_FindItem2(self, 0, name);
}

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

    // std.stream
    if(true)
    {
        static void* gta3emu_pMaxInfoForModel;

        map[0x5B8AE8] = 0x40A4AD;               // -> offset ms_aInfoForModel //doublecheck
        map[xVc(0x4102B2)] = 0x40664B;          // -> DWORD MAX_RES ; max resources
        map[0x5B8AFC] = &gta3emu_pMaxInfoForModel;// -> &ms_aInfoForModel[MAX_INFO_FOR_MODEL]
        // DONT CHANGE THE FOLLOWING LINE, CHANGE THE MAPPING ABOVE INSTEAD!!! Maybe change the 0x14 sizeof /////////////
        gta3emu_pMaxInfoForModel = *mem_ptr(0x5B8AE8).get<char*>() + (0x14 * *mem_ptr(xVc(0x4102B2)).get<uint32_t>());
        ///////////////////////////////////

        map[0x8E3FE0] = 0x6212C4;   // DWORD StreamCreateFlags
        map[xVc(0x6F76F4)] = 0x62129C; // HANDLE hCdSemaphore
        map[xVc(0x6F7700)] = 0x6212A8; // Queue CdQueue;
        map[xVc(0x6F76FC)] = 0x6212A4; // CdStream* channelFile
        map[0x8E4CAC] = 0x87F818;   // void* CStreaming::ms_pStreamingBuffer[2]
        map[0x8E4CA8] = 0x942FB0;   // unsigned int CStreaming::ms_streamingBufferSize

        map[0x72F420] = nullptr;    // _ZN10CMemoryMgr6MallocEj // Doesn't exist!!? Doesn't matter. Used by us only in SA.
        map[0x72F4C0] = 0x526FD0;   // _ZN10CMemoryMgr11MallocAlignEjj
        map[0x72F4F0] = 0x527000;   // _ZN10CMemoryMgr9FreeAlignEPv
        map[0x532310] = 0x473600;   // _ZN10CDirectory7AddItemERKNS_13DirectoryInfoE
        map[0x532450] = gta3emu_CDirectory_FindItem2; // _ZN10CDirectory8FindItemEPKc
        map[0x5324A0] = 0x4736E0;   // _ZN10CDirectory8FindItemEPKcRjS2_

        map[0x406560] = 0x406140;   // _Z14CdStreamThreadPv
        map[0x5B8E1B] = 0x406A13;   // call    _ZN10CStreaming15LoadCdDirectoryEv       ; @CStreaming::Init //not found, check 405C80 
        map[0x40CF34] = 0x40A321;   // call    _Z12CdStreamReadiPvjj                    ; @CStreaming::RequestModelStream
        map[xVc(0x40B76A)] = 0x40A512;// call    _Z12CdStreamReadiPvjj                    ; @CStreaming::LoadAllRequestedModels
        map[xVc(0x40B780)] = 0x40A536;// call    _Z12CdStreamReadiPvjj                    ; @CStreaming::LoadAllRequestedModels
        map[xIII(0x40A128)] = 0x40A128; // call    _ZN14CStreamingInfo16GetCdPosnAndSizeERjS0_ ; @CStreaming::RequestModelStream
        map[xIII(0x40A4F3)] = 0x40A4F3; // call    _ZN14CStreamingInfo16GetCdPosnAndSizeERjS0_ ; @CStreaming::LoadAllRequestedModels
        map[xVc(0x408521)] = 0x405E71;// add     esi, edi    ; @CdStreamRead
        map[0x409F76] = 0x40A997;   // call    _ZN10CDirectory8FindItemEPKcRjS2_        ; @CStreaming::RequestSpecialModel
        map[0x409FD9] = 0x40A9E5;   // call    _ZN10CStreaming12RequestModelEii         ; @CStreaming::RequestSpecialModel
        map[0x5B6183] = 0x406DB3;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CStreaming::LoadCdDirectory
        // TODO X backend.cpp map[0x532361] = 0x473641;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CDirectory::ReadDirFile
        // TODO X backend.cpp map[0x5AFC9D] = 0x4046BF;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CCutsceneMgr::LoadCutsceneData_postload
        // TODO X backend.cpp map[0x5AFBEF] = 0x4BA6F6;   // call    _RwStreamOpen                            ; @CCutsceneMgr::LoadCutsceneData_postload
        // TODO X backend.cpp map[0x5B07E9] = ;   // call    _RwStreamOpen                            ; @CCutsceneMgr::LoadCutsceneData_preload //doesnt exist?
        // TODO X backend.cpp map[0x40685E] = ;   // call    ds:__imp_CreateFileA                     ; @CdStreamOpen  //doesnt exist?
        map[0x5B8310] = 0x406D6A;   // call    _ZN10CStreaming15LoadCdDirectoryEPKci    ; @CStreaming::LoadCdDirectory
        map[0x5B61E1] = 0x406DC9;   // call    _ZN8CFileMgr4ReadEiPci                   ; @CStreaming::LoadCdDirectory  ; read entry @up
        map[xVc(0x40FDD9)] = 0x407043;// call    _ZN8CFileMgr4ReadEiPci             ; @CStreaming::LoadCdDirectory ; read entry @up
        map[0x5B627A] = 0x406F2B;   // call    _ZN10CDirectory7AddItemERKNS_13DirectoryInfoE ; @CStreaming::LoadCdDirectory
        map[xIII(0x406EB0)] = 0x406EB0; // call    _ZN14CStreamingInfo16GetCdPosnAndSizeERjS0_  ; dff
        map[xIII(0x406FD9)] = 0x406FD9; // call    _ZN14CStreamingInfo16GetCdPosnAndSizeERjS0_  ; txd
        map[xIII(0x4038FC)] = 0x4038FC;   // call    __loadIfp    ; @CAnimManager::LoadAnimFiles "anim/ped.ifp"
        map[0x40E2C5] = 0x40A571;   // call    _ZN10CStreaming21ConvertBufferToObjectEPcii
        map[0x40E1BE] = 0x40A585;   // call    _ZN10CStreaming22FinishLoadingLargeFileEPci

        map[0xB74490] = 0x8F2C60;   // CPool<> *CPools::ms_pPedPool
        map[0xB74494] = 0x9430DC;   // CPool<> *CPools::ms_pVehiclePool
        map[0xB74498] = 0x8F2C04;   // CPool<> *CPools::ms_pBuildingPool
        map[0xB7449C] = 0x880E28;   // CPool<> *CPools::ms_pObjectPool
        // TODO X refresh.cpp map[0x4D6C30] = ;   // _Z32RpAnimBlendClumpGiveAssociationsP7RpClumpP21CAnimBlendAssociation //doesn't exist?
        // TODO X refresh.cpp map[0x4D6BE0] = ;   // _Z35RpAnimBlendClumpExtractAssociationsP7RpClump //doesn't exist?
        // TODO X refresh.cpp map[0x681810] = ;   // _ZNK12CTaskManager16GetTaskSecondaryEi //doesn't exist?
        map[0x561A00] = 0x4AD370;   // _ZN6CTimer6ResumeEv
        map[0x5619D0] = 0x4AD310;   // _ZN6CTimer7SuspendEv
        map[0x4087E0] = 0x407EA0;   // _ZN10CStreaming12RequestModelEii
        map[0x4089A0] = 0x408830;   // _ZN10CStreaming11RemoveModelEi
        map[0x40EA10] = 0x40A440;   // _ZN10CStreaming22LoadAllRequestedModelsEb
        map[0x40E460] = 0x40A610;   // _ZN10CStreaming13FlushChannelsEv
        map[0x40CF80] = 0x408940;   // _ZN10CStreaming21RemoveAllUnusedModelsEv
        map[0x40CFD0] = 0x4089B0;   // _ZN10CStreaming20RemoveLeastUsedModelEj
        map[xVc(0x59E2B0)] = 0x52D210; // _ZN11CAutomobile20SetupSuspensionLinesEv

        // Non streamed resources
        map[0x5B9188] = 0x4763B2 ;   // call    _ZN11CFileLoader17LoadCollisionFileEPKch
        map[0x5B91B0] = 0x4763E2;   // call    _ZN11CFileLoader14LoadAtomicFileEPKc
        map[0x5B91DB] = 0x47640F;   // call    _ZN11CFileLoader13LoadClumpFileEPKc
        map[0x5B910A] = 0x476358;   // call    _ZN11CFileLoader17LoadTexDictionaryEPKc

        // Removal of txd.img / txd.dir
        map[xVc(0x410814)] = 0x48C108; // call    _ZN8CFileMgr8OpenFileEPKcS1_; "models/txd.img"
        map[xVc(0x41083A)] = 0x48C12E; // call    __CreateCacheTxdImage
    }

    // std.data
    if(true)
    {
        map[0x464D50] = 0x439410;   // _ZN11CTheScripts18IsPlayerOnAMissionEv
        map[0x5B6890] = 0x521260;   // _ZN17CVehicleModelInfo18LoadVehicleColoursEv
        map[0x5B68AB] = 0x521283;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CVehicleModelInfo::LoadVehicleColours
        //map[0x5B65A0] = ;   // _ZN17CVehicleModelInfo19LoadVehicleUpgradesEv //doesn't exist
        //map[0x5B65BE] = ;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CVehicleModelInfo::LoadVehicleUpgrades //doesn't exist
        //map[0xB4E6D8] = ;   // CLinkedUpgradeList CVehicleModelInfo::ms_linkedUpgradesE //doesn't exist
        map[0x4C6770] = 0x50BA60;   // mov     eax, gsVehicleModels.m_nCount //=> cmp     ds:dword_8E2DE0, 78h doublecheck!
        map[0x5B905E] = 0x4762C2;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CFileLoader::LoadLevel
        map[0x5BD830] = 0x546DB0;   // _ZN16cHandlingDataMgr16LoadHandlingDataEv //doublecheck
        //map[0x5BD850] = 0x;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @cHandlingDataMgr::LoadHandlingData //check 546DDE 
        map[0x5BD850] = 0x546DDE; // call    _ZN8CFileMgr12LoadTextFileEPKcPhi ; "handling.cfg"
        map[0xC2B9C8] = 0x728060;   // mod_HandlingManager CHandlingData
        map[0x5B8428] = 0x476AEB;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CFileLoader::LoadObjectTypes
        map[0x5B871A] = 0x478393;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CFileLoader::LoadScene
        //map[0x5DD780] = 0x;   // _ZN9CPlantMgr12ReloadConfigEv //doesn't exist?
        //map[0x5DD3D1] = 0x;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CPlantSurfPropMgr::LoadPlantsDat //doesn't exist?
        map[0x5BF750] = 0x564EA0;   // _ZN11CWeaponInfo10InitialiseEv
        //map[0x5BE68A] = 0x;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CWeaponInfo::Initialise // check 5D527B
        map[0x5BE68A] = 0x56500B; // call    _ZN8CFileMgr12LoadTextFileEPKcPhi             ; @CWeaponInfo::Initialise
        //map[0x5BC090] = 0x;   // _ZN9CPopCycle10InitialiseEv //doesn't exist?
        //map[0x5BC0AE] = 0x;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CPopCycle::Initialise //doesn't exist?
        map[0x5BBAC0] = 0x4ABAE0;   // _ZN10CTimeCycle10InitialiseEb 
        //map[0x5BBADE] = 0x;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CTimeCycle::initialise //check 4ABB14
        map[0x6EAE80] = 0x554EA0;   // _ZN11CWaterLevel20WaterLevelInitialiseEv
        //map[0x5A7B30] = 0x;   // _ZN8CClothes15LoadClothesFileEv  //doesn't exist?
        //map[0x5A7B54] = 0x;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CClothes::LoadClothesFile   //doesn't exist?
       // map[0x5C0297] = 0x;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @FurnitureManager_c::LoadFurniture   //doesn't exist?
        map[0x6EAF4D] = 0x554ED0;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CWaterLevel::WaterLevelInitialise
        //map[0x5BD1A0] = 0x;   // _ZN11CPopulation13LoadCarGroupsEv //doesn't exist?
        //map[0x5BD1BB] = 0x;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CPopulation::LoadCarGroups //doesn't exist?
        map[0x5BCFE0] = 0x4F3870;   // _ZN11CPopulation13LoadPedGroupsEv 
        map[0x5BCFFB] = 0x4F388F;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CPopulation::LoadPedGroups
        map[0x5BB890] = 0x4EF580;   // _ZN9CPedStats12LoadPedStatsEv
        //map[0x5BB89F] = 0x;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CPedStats::LoadPedStats //check 4EF5B4
        map[0x5B5360] = 0x4BC0E0;   // _ZN11CObjectData10InitialiseEPcb
        //map[0x5B5444] = 0x;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CObjectData::Initialise // check 4BC10C
        map[0x5B5444] = 0x4BC10C;   // call    _ZN8CFileMgr12LoadTextFileEPKcPhi          ; @CObjectData::Initialise
        //map[0x7187C0] = 0x;   // _ZN5CFont14LoadFontValuesEv //doesn't exist?
        //map[0x7187DB] = 0x;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CFont::LoadFontValues //doesn't exist?
        map[0x461100] = 0x436F50;   // _ZN11CRoadBlocks4InitEv
        //map[0x461125] = 0x;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CRoadBlocks::Init //doesn't exist
        //map[0x5599B0] = 0x;   // _ZN6CStats23LoadActionReactionStatsEv //doesn't exist
        //map[0x5599D8] = 0x;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CStats::LoadActionReactionStats //doesn't exist?
        //map[0x55988F] = 0x;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CStats::LoadStatUpdateConditions //doesn't exist?
        //map[0x608B45] = 0x;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CPedType::LoadPedData //check 4EE917
        map[0x608B45] = 0x4EE917; // call    _ZN8CFileMgr12LoadTextFileEPKcPhi ; @CPedType::LoadPedData
        //map[0x5BEDC0] = 0x;   // _ZN16CTaskSimpleFight13LoadMeleeDataEv //doesn't exist
        //map[0x5BEF47] = 0x;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CTaskSimpleFight::LoadMeleeData //doesn't exist
        //map[0x5BC92B] = 0x;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CAnimManager::ReadAnimAssociationDefinitions //doesn't exist?
        //map[0x5BF400] = 0x;   //  _ZN29CDecisionMakerTypesFileLoader24LoadDefaultDecisionMakerEv //doesn't exist?
        //map[0x6076CE] = 0x;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CDecisionMakerTypesFileLoader::LoadDecisionMaker //doesn't exist?
        //map[0x5A3EA0] = 0x;   // _ZN15ProcObjectMan_c4InitEv //doesn't exist
        //map[0xBB7CB0] = 0x;   // ProcObjectMan g_procObjMan //doesn't exist
        //map[0x5A3154] = 0x;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @ProcObjectMan_c::LoadDataFile //doesn't exist
        //map[0x55F420] = 0x;   // _ZN14SurfaceInfos_c4InitEv //not sure, check 4CE8A0 (VC)
        //map[0xB79538] = 0x;   // SurfaceInfos_c g_surfaceInfos //^
        //map[0x55D100] = 0x;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @SurfaceInfos_c::LoadAdhesiveLimits//^
        //map[0x55EBA4] = 0x;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @SurfaceInfos_c::LoadSurfaceInfos//^
        //map[0x55F2C1] = 0x;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @SurfaceInfos_c::LoadSurfaceAudioInfos//^
        //map[0x49AE30] = 0x;   // _ZN9CShopping16RemoveLoadedShopEv //doesn't exist
        //map[0x49BBE0] = 0x;   // _ZN9CShopping8LoadShopEPKc //doesn't exist
        //map[0xA9A7D8] = 0x;   // char _ZN9CShopping13ms_shopLoadedE[24] //doesn't exist
        //map[0x49B6AF] = 0x;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CShopping::LoadStats //doesn't exist
        //map[0x49B93F] = 0x;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CShopping::LoadPrices //doesn't exist
        //map[0x49BC98] = 0x;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CShopping::LoadShop //doesn't exist
        map[0x6F7440] = 0x54F000;   // _ZN6CTrain10InitTrainsEv
        map[0x6F7470] = 0x54F035;   // call    _ZN6CTrain25ReadAndInterpretTrackFileEPcPP10CTrainNodePiPfi ; "tracks.dat" 
        map[0x6F74BC] = 0x54F06D;   // call    _ZN6CTrain25ReadAndInterpretTrackFileEPcPP10CTrainNodePiPfi ; "tracks2.dat"
        //map[0x6F7496] = 0x;   // call    _ZN6CTrain25ReadAndInterpretTrackFileEPcPP10CTrainNodePiPfi ; "tracks3.dat" //doesn't exist
        //map[0x6F74E2] = 0x;   // call    _ZN6CTrain25ReadAndInterpretTrackFileEPcPP10CTrainNodePiPfi ; "tracks4.dat" //doesn't exist
        map[0x748CFB] = 0x582E6C;   // call    _Z14InitialiseGamev
        //map[0x590D2A] = 0x;   // mov     eax, 8Ch                                 ; 8Ch = Loading Screen Max Progress //doesn't exist
        //map[0x590D67] = 0x;   // cmp     eax, 8Ch                                 ; 8Ch = Loading Screen Max Progress //doesn't exist
        //map[0x5B906A] = 0x;   // call    _ZN11CFileLoader8LoadLineEi              ; @CFileLoader::LoadLevel -- loop begin //check 4764F8
        //map[0x5B92E6] = 0x;   // call    _ZN11CFileLoader8LoadLineEi              ; @CFileLoader::LoadLevel -- loop end //check 4764F8
        map[0x5B92F9] = 0x47650B;   // call    _ZN8CFileMgr9CloseFileEi                 ; @CFileLoader::LoadLevel
        map[0x53BC95] = 0x48C038;   // call    _ZN11CFileLoader9LoadLevelEPKc           ; @CGame::Initialise -- default.dat
        map[0x53BC9B] = 0x48C03F;   // call    _ZN11CFileLoader9LoadLevelEPKc           ; @CGame::Initialise -- gta.dat
        map[0x4C5940] = 0x50B860;   // _ZN10CModelInfo12GetModelInfoEPKcPi
        map[0x5B925F] = 0x476472;   // call    _ZN11CObjectData10InitialiseEPc          ; @CFileLoader::LoadLevel

        map[xIII(0x524EDA)] = 0x524EDA;  // call    _ZN8CFileMgr8OpenFileEPKcS1_ ; "data/cullzone.dat"
        map[xVc(0x5C395A)] = 0x554ED0;   // call    _ZN8CFileMgr8OpenFileEPKcS1_ ; "data/waterpro.dat"
        map[xVc(0x4D0614)] = 0x4ABB14;   // call    _ZN8CFileMgr12LoadTextFileEPKcPhi ; "timecyc.dat"
        map[xVc(0x4CE8A0)] = 0x4AB8F0;   // _ZN13CSurfaceTable10InitialiseEPc
        map[0x55D100] = 0x4AB91C;   // call    _ZN8CFileMgr12LoadTextFileEPKcPhi
    }

    // traits
    if(true)
    {
        map[0x5B62CF] = 0x40702D;   // -> DWORD 4E20h   ; TXD Start Index+
        map[0x408897] = 0x407F11;   // -> offset _ZN10CModelInfo16ms_modelInfoPtrsE+
    }

    // AbstractFrontend | TheMenu
    if(true)
    {
        // Not available for III
    }
}
