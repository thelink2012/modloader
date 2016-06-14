/*
 * San Andreas Mod Loader - Address Translation Between Game Versions
 * Copyright (C) 2013-2015  LINK/2012 <dma_2012@hotmail.com>
 * Copyright (C) 2014-2015  ThirteenAG <thirteenag@gmail.com>
 * Licensed under the MIT License, see LICENSE at top level directory.
 *
 */
#include <modloader/util/injector.hpp>
#include <map>
 
// Emulating SA's CStreaming::RemoveAllUnusedModels for VC
static void vcemu_CStreaming__RemoveAllUnusedModels()
{
    void* ptr_begin = injector::lazy_pointer<xVc(0x40FF65)>::get<void>();
    void* ptr_end   = injector::lazy_pointer<xVc(0x40FFB1)>::get<void>();

    injector::scoped_write<5> c3;
    c3.write<uint8_t>(raw_ptr(ptr_end), 0xC3, true); // ret

    _asm
    {
        pushad       // jumps into the middle of a game function, save regs
        sub esp, 0xC // local space in the middle of the function
        call dword ptr [ptr_begin]
        add esp, 0xC
        popad
    }
}

// Emulating SA's CDirectory::FindItem(ecx, name) for VC
static void* __fastcall vcemu_CDirectory_FindItem2(void* self, int, const char* name)
{
    // NOTE: This function is also used for on gta3emu, so mind it when touching.

    int dummy;
    void* ptr_FindItem = injector::lazy_pointer<0x5324A0>::get<void>(); // CDirectory::FindItem(ecx, name, out_offset, out_size)
    void* ptr_dummy = &dummy;
    void* result = nullptr;
    
    _asm
    {
        push ptr_dummy
        push ptr_dummy
        push name
        mov ecx, self
        call dword ptr [ptr_FindItem]
        test al, al
        jz NotFound

        mov eax, self
        mov eax, [eax+0] // m_pEntries
        add eax, [esp-0x30] // HACKHACKHACK take offset in the local scope of FindItem
        mov result, eax

        NotFound:
    }

    return result;
}


// GTA VC 10 table
static void vc_10(std::map<memory_pointer_raw, memory_pointer_raw>& map)
{
    //
    // (Notice '->' means "exactly pointing to"!!!!!)
    //
 
    // Core
    if(true)
    {
        map[0x8246EC] = 0x667CB9;   // call    _WinMain
        map[0x53ECBD] = 0x4A5BE0;   // call    _Z4IdlePv
        map[0x53ECCB] = 0x4A5BF2;   // call    _Z12FrontendIdlePv
        map[0xC8D4C0] = 0x9B5F08;   // int gGameState
    }
 
    // std.fx
    if(true)
    {
        map[0x5BF8B7] = 0x4A4BB0;   // call    _ZN9CTxdStore7LoadTxdEiPKc ; "models/particle.txd"
       
        map[0x5BA850] = 0x55C8A0;   // _ZN4CHud10InitialiseEv
        map[0x588850] = 0x55C7F0;   // _ZN4CHud8ShutdownEv
        map[0x5827D0] = 0x4C5DC0;   // _ZN6CRadar12LoadTexturesEv
        map[0x5BA865] = 0x55C8B5;   // call    _ZN9CTxdStore7LoadTxdEiPKc ; "models/hud.txd"
 
        map[0x5BA690] = 0x552310;   // _ZN5CFont10InitialiseEv
        map[0x7189B0] = 0x5522E0;   // _ZN5CFont8ShutdownEv
        map[0x5BA6A4] = 0x552327;   // call    _ZN9CTxdStore7LoadTxdEiPKc ; "models/fonts.txd"
 
        map[0xBA6748] = 0x869630;   // CMenuManager FrontEndMenuManager
        map[0x572EC0] = 0x4A3A13;   // _ZN12CMenuManager15LoadAllTexturesEv
        map[0x574630] = 0x4A394D;   // _ZN12CMenuManager14UnloadTexturesEv
        map[0x572F1E] = 0x4A3AC3;   // call    _ZN9CTxdStore7LoadTxdEiPKc ; "models/fronten1.txd"
        map[0x573040] = 0x4A3B5D;   // call    _ZN9CTxdStore7LoadTxdEiPKc ; "models/fronten2.txd"

        // VC stuff
        map[xVc(0x570D8A)] = 0x570D8A;  // call    _ZN11CFileLoader21LoadAtomicFile2ReturnEPKc ; "models/generic/zonecylb.dff"
        map[xVc(0x570D7A)] = 0x570D7A;  // call    _ZN11CFileLoader21LoadAtomicFile2ReturnEPKc ; "models/generic/arrow.dff"
        map[xVc(0x4BBB30)] = 0x4BBB30;  // _ZN11CPlayerInfo14LoadPlayerSkinEv
        map[xVc(0x94AD28)] = 0x94AD28;  // CPlayerInfo _ZN6CWorld7PlayersE[]
        map[xVc(0x627EB8)] = 0x627EB8;  // push    offset "models\generic\player.bmp"
        map[0x7F3820]      = 0x64DEC0;  // _RwTextureDestroy
        map[xVc(0x627E7F)] = 0x627E7F;  // call    RwTextureRead    ; @CPlayerSkin::GetSkinTexture
    }
 
    // std.text
    if(true)
    {
        map[0x57A161] = 0x49EE23;   // call    _ZN5CText3GetEPKc+                       ; fxtp
        map[0x748CFB] = 0x600411;   // call    _Z14InitialiseGamev                      ; fxtp
        map[0x6A0228] = 0x5855F8;   // call    _ZN8CFileMgr8OpenFileEPKcS1_  ; @CText::Load
        map[0x69FD5A] = 0x5852A8;   // call    _ZN8CFileMgr8OpenFileEPKcS1_  ; @CText::LoadMissionText
        map[0x57326E] = 0x4A388C;   // jz      loc_573313                    ; Check for some menu stuff to avoid reloading text
        map[0xBA6748] = 0x869630;   // CMenuManager FrontEndMenuManager
        map[0x573260] = 0x4A3882;   // _ZN12CMenuManager33InitialiseChangedLanguageSettingsEv
        map[0x6A0050] = 0x584F30;   // _ZN5CText3GetEPKc
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
        static void* vcemu_pMaxInfoForModel;

        map[0x5B8AE8] = 0x40B6E3;               // -> offset ms_aInfoForModel
        map[xVc(0x4102B2)] = 0x4102B2;          // -> DWORD 1F07h ; max resources
        map[0x5B8AFC] = &vcemu_pMaxInfoForModel;// -> &ms_aInfoForModel[MAX_INFO_FOR_MODEL]
        // DONT CHANGE THE FOLLOWING LINE, CHANGE THE MAPPING ABOVE INSTEAD!!! Maybe change the 0x14 sizeof /////////////
        vcemu_pMaxInfoForModel = *mem_ptr(0x5B8AE8).get<char*>() + (0x14 * *mem_ptr(xVc(0x4102B2)).get<uint32_t>());
        ///////////////////////////////////

        map[0x8E3FE0] = 0x6F771C;   // DWORD StreamCreateFlags
        map[xVc(0x6F76F4)] = 0x6F76F4; // HANDLE hCdSemaphore
        map[xVc(0x6F7700)] = 0x6F7700; // Queue CdQueue;
        map[xVc(0x6F76FC)] = 0x6F76FC; // CdStream* channelFile
        map[0x8E4CAC] = 0x94B840;   // void* CStreaming::ms_pStreamingBuffer[2]
        map[0x8E4CA8] = 0xA0FC90;   // unsigned int CStreaming::ms_streamingBufferSize
 

        map[0x72F420] = nullptr;    // _ZN10CMemoryMgr6MallocEj         // Doesn't exist!!? Doesn't matter. Used by us only in SA.
        map[0x72F4C0] = 0x5805D0;   // _ZN10CMemoryMgr11MallocAlignEjj
        map[0x72F4F0] = 0x5805C0;   // _ZN10CMemoryMgr9FreeAlignEPv
        map[0x532310] = 0x4873F0;   // _ZN10CDirectory7AddItemERKNS_13DirectoryInfoE
        map[0x532450] = vcemu_CDirectory_FindItem2;   // _ZN10CDirectory8FindItemEPKc
        map[0x5324A0] = 0x487220;   // _ZN10CDirectory8FindItemEPKcRjS2_
 
        map[0x406560] = 0x408260;   // _Z14CdStreamThreadPv
        map[0x5B8E1B] = 0x410723;   // call    _ZN10CStreaming15LoadCdDirectoryEv       ; @CStreaming::Init
        map[0x40CF34] = 0x40BBB9;   // call    _Z12CdStreamReadiPvjj                    ; @CStreaming::RequestModelStream
        map[xVc(0x40B76A)] = 0x40B76A;// call    _Z12CdStreamReadiPvjj                    ; @CStreaming::LoadAllRequestedModels
        map[xVc(0x40B780)] = 0x40B780;// call    _Z12CdStreamReadiPvjj                    ; @CStreaming::LoadAllRequestedModels
        map[0x40CCA6] = 0x40B97D;   // mov     edx, ms_aInfoForModel.iBlockCount[eax*4] ; @CStreaming::RequestModelStream
        map[xVc(0x40B738)] = 0x40B738;// mov     edx, ds:_ZN10CStreaming16ms_aInfoForModelE.uArchiveSize[ecx] ; @CStreaming::LoadAllRequestedModels
        map[xVc(0x408521)] = 0x408521;// add     esi, edi    ; @CdStreamRead
        map[0x409F76] = 0x40ACEE;   // call    _ZN10CDirectory8FindItemEPKcRjS2_        ; @CStreaming::RequestSpecialModel
        map[0x409FD9] = 0x40AD47;   // call    _ZN10CStreaming12RequestModelEii         ; @CStreaming::RequestSpecialModel
        map[0x5B6183] = 0x40FBD3;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CStreaming::LoadCdDirectory
        // TODO X backend.cpp map[0x532361] = 0x487383;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CDirectory::ReadDirFile
        // TODO X backend.cpp map[0x5AFC9D] = 0x406E42;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CCutsceneMgr::LoadCutsceneData_postload
        // TODO X backend.cpp map[0x5AFBEF] = 0x406D9A;   // call    _RwStreamOpen                            ; @CCutsceneMgr::LoadCutsceneData_postload
        // TODO X backend.cpp map[0x5B07E9] = ;    // call    _RwStreamOpen                            ; @CCutsceneMgr::LoadCutsceneData_preload //doesnt exist?
        // TODO X backend.cpp map[0x40685E] = ;    // call    ds:__imp_CreateFileA                     ; @CdStreamOpen  //doesnt exist?
        map[0x5B8310] = 0x40FE91;   // call    _ZN10CStreaming15LoadCdDirectoryEPKci    ; @CStreaming::LoadCdDirectory
        map[0x5B61E1] = 0x40FBE9;   // call    _ZN8CFileMgr4ReadEiPci                   ; @CStreaming::LoadCdDirectory  ; read entry @up
        map[xVc(0x40FDD9)] = 0x40FDD9;// call    _ZN8CFileMgr4ReadEiPci             ; @CStreaming::LoadCdDirectory ; read entry @up
        map[0x5B627A] = 0x40FC9D;   // call    _ZN10CDirectory7AddItemERKNS_13DirectoryInfoE ; @CStreaming::LoadCdDirectory
        map[xVc(0x40FD82)] = 0x40FD82;  // TODO ASM
        map[xVc(0x40FD9A)] = 0x40FD9A;  // TODO ASM
        map[xVc(0x40FDA0)] = 0x40FDA0;  // TODO ASM
        map[0x5B630B] = 0x40FD0D;   // call    _ZN9CColStore10AddColSlotEPKc
        map[0x4D565A] = 0x4055EA;   // call    _RwStreamOpen    ; @CAnimManager::LoadAnimFiles "anim/ped.ifp"
        map[0x40E2C5] = 0x40C086;   // call    _ZN10CStreaming21ConvertBufferToObjectEPcii
        map[0x40E1BE] = 0x40BF0B;   // call    _ZN10CStreaming22FinishLoadingLargeFileEPci
 
        map[0xB74490] = 0x97F2AC;   // CPool<> *CPools::ms_pPedPool
        map[0xB74494] = 0xA0FDE4;   // CPool<> *CPools::ms_pVehiclePool
        map[0xB74498] = 0x97F240;   // CPool<> *CPools::ms_pBuildingPool
        map[0xB7449C] = 0x94DBE0;   // CPool<> *CPools::ms_pObjectPool
        // TODO X refresh.cpp map[0x4D6C30] = ;   // _Z32RpAnimBlendClumpGiveAssociationsP7RpClumpP21CAnimBlendAssociation //doesn't exist?
        // TODO X refresh.cpp map[0x4D6BE0] = ;   // _Z35RpAnimBlendClumpExtractAssociationsP7RpClump //doesn't exist?
        // TODO X refresh.cpp map[0x681810] = ;   // _ZNK12CTaskManager16GetTaskSecondaryEi //doesn't exist?
        map[0x561A00] = 0x4D0E50;   // _ZN6CTimer6ResumeEv
        map[0x5619D0] = 0x4D0ED0;   // _ZN6CTimer7SuspendEv
        map[0x4087E0] = 0x40E310;   // _ZN10CStreaming12RequestModelEii
        map[0x4089A0] = 0x40D6E0;   // _ZN10CStreaming11RemoveModelEi
        map[0x40EA10] = 0x40B5F0;   // _ZN10CStreaming22LoadAllRequestedModelsEb
        map[0x40E460] = 0x40B580;   // _ZN10CStreaming13FlushChannelsEv
        map[0x40CF80] = vcemu_CStreaming__RemoveAllUnusedModels; // _ZN10CStreaming21RemoveAllUnusedModelsEv
        map[0x40CFD0] = 0x40D5A0;   // _ZN10CStreaming20RemoveLeastUsedModelEj
        map[xVc(0x59E2B0)] = 0x59E2B0; // _ZN11CAutomobile20SetupSuspensionLinesEv
        map[xVc(0x615080)] = 0x615080; // _ZN5CBike20SetupSuspensionLinesEv
 
        // Non streamed resources
        map[0x5B9188] = 0x48DB04 ;   // call    _ZN11CFileLoader17LoadCollisionFileEPKch
        map[0x5B91DB] = 0x48DC22;   // call    _ZN11CFileLoader13LoadClumpFileEPKc
        map[xVc(0x48DB7B)] = 0x48DB7B; // call    RwStreamOpen ; inlined _ZN11CFileLoader14LoadAtomicFileEPKc
        map[xVc(0x48DA46)] = 0x48DA46; // call    RwStreamOpen ; inlined _ZN11CFileLoader17LoadTexDictionaryEPKc

        // Removal of txd.img / txd.dir
        map[xVc(0x410801)] = 0x410801; // call    sub_61E310
        map[xVc(0x410814)] = 0x410814; // call    _ZN8CFileMgr8OpenFileEPKcS1_; "models/txd.img"
        map[xVc(0x41083A)] = 0x41083A; // call    __CreateCacheTxdImage
    }

    // std.data
    if(true)
    {
        map[0x464D50] = 0x44FE30;   // _ZN11CTheScripts18IsPlayerOnAMissionEv
        map[0x5B6890] = 0x578CC0;   // _ZN17CVehicleModelInfo18LoadVehicleColoursEv
        map[0x5B68AB] = 0x578CE3;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CVehicleModelInfo::LoadVehicleColours
        //map[0x5B65A0] = ;   // _ZN17CVehicleModelInfo19LoadVehicleUpgradesEv //doesn't exist
        //map[0x5B65BE] = ;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CVehicleModelInfo::LoadVehicleUpgrades //doesn't exist
        //map[0xB4E6D8] = ;   // CLinkedUpgradeList CVehicleModelInfo::ms_linkedUpgradesE //doesn't exist
        map[0x4C6770] = 0x55F5D0;   // mov     eax, gsVehicleModels.m_nCount //=> cmp     ds:dwLoadedVehicleModels, 6Eh doublecheck!
        map[0x5B905E] = 0x48D97C;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CFileLoader::LoadLevel
        map[0x5BD830] = 0x5AAE20;   // _ZN16cHandlingDataMgr16LoadHandlingDataEv
        //map[0x5BD850] = 0x;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @cHandlingDataMgr::LoadHandlingData //check 5AAE4E
        map[0xC2B9C8] = 0x978E58;   // mod_HandlingManager CHandlingData
        map[0x5B8428] = 0x48C846;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CFileLoader::LoadObjectTypes
        map[0x5B871A] = 0x48B079;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CFileLoader::LoadScene
        //map[0x5DD780] = 0x;   // _ZN9CPlantMgr12ReloadConfigEv //doesn't exist?
        //map[0x5DD3D1] = 0x;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CPlantSurfPropMgr::LoadPlantsDat //doesn't exist?
        map[0x5BF750] = 0x5D5750;   // _ZN11CWeaponInfo10InitialiseEv
        //map[0x5BE68A] = 0x;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CWeaponInfo::Initialise // check 5D527B
        //map[0x5BC090] = 0x;   // _ZN9CPopCycle10InitialiseEv //doesn't exist?
        //map[0x5BC0AE] = 0x;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CPopCycle::Initialise //doesn't exist?
        map[0x5BBAC0] = 0x4D05E0;   // _ZN10CTimeCycle10InitialiseEb
        //map[0x5BBADE] = 0x;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CTimeCycle::initialise //check 4D0614
        map[0x6EAE80] = 0x5C3940;   // _ZN11CWaterLevel20WaterLevelInitialiseEv
        //map[0x5A7B30] = 0x;   // _ZN8CClothes15LoadClothesFileEv  //doesn't exist?
        //map[0x5A7B54] = 0x;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CClothes::LoadClothesFile   //doesn't exist?
        //map[0x5C0297] = 0x;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @FurnitureManager_c::LoadFurniture   //doesn't exist?
        map[0x6EAF4D] = 0x5C395A;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CWaterLevel::WaterLevelInitialise
        //map[0x5BD1A0] = 0x;   // _ZN11CPopulation13LoadCarGroupsEv //doesn't exist?
        //map[0x5BD1BB] = 0x;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CPopulation::LoadCarGroups //doesn't exist?
        map[0x5BCFE0] = 0x53E9C0;   // _ZN11CPopulation13LoadPedGroupsEv
        map[0x5BCFFB] = 0x53E9DF;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CPopulation::LoadPedGroups
        map[0x5BB890] = 0x530020;   // _ZN9CPedStats12LoadPedStatsEv
        //map[0x5BB89F] = 0x;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CPedStats::LoadPedStats //check 530054
        map[0x5B5360] = 0x4E4560;   // _ZN11CObjectData10InitialiseEPcb
        //map[0x5B5444] = 0x;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CObjectData::Initialise // check 4E476D
        //map[0x7187C0] = 0x;   // _ZN5CFont14LoadFontValuesEv //doesn't exist?
        //map[0x7187DB] = 0x;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CFont::LoadFontValues //doesn't exist?
        map[0x461100] = 0x444AA0;   // _ZN11CRoadBlocks4InitEv
        //map[0x461125] = 0x;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CRoadBlocks::Init //doesn't exist
        //map[0x5599B0] = 0x;   // _ZN6CStats23LoadActionReactionStatsEv //doesn't exist
        //map[0x5599D8] = 0x;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CStats::LoadActionReactionStats //doesn't exist?
        //map[0x55988F] = 0x;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CStats::LoadStatUpdateConditions //doesn't exist?
        //map[0x608B45] = 0x;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CPedType::LoadPedData //check 530BD7
        //map[0x5BEDC0] = 0x;   // _ZN16CTaskSimpleFight13LoadMeleeDataEv //doesn't exist
        //map[0x5BEF47] = 0x;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CTaskSimpleFight::LoadMeleeData //doesn't exist
        //map[0x5BC92B] = 0x;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CAnimManager::ReadAnimAssociationDefinitions //doesn't exist?
        //map[0x5BF400] = 0x;   //  _ZN29CDecisionMakerTypesFileLoader24LoadDefaultDecisionMakerEv //doesn't exist?
        //map[0x6076CE] = 0x;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @CDecisionMakerTypesFileLoader::LoadDecisionMaker //doesn't exist?
        //map[0x5A3EA0] = 0x;   // _ZN15ProcObjectMan_c4InitEv //doesn't exist
        //map[0xBB7CB0] = 0x;   // ProcObjectMan g_procObjMan //doesn't exist
        //map[0x5A3154] = 0x;   // call    _ZN8CFileMgr8OpenFileEPKcS1_             ; @ProcObjectMan_c::LoadDataFile //doesn't exist
        //map[0x55F420] = 0x;   // _ZN14SurfaceInfos_c4InitEv //not sure, check 4CE8A0
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
        map[0x6F7440] = 0x5B2CA0;   // _ZN6CTrain10InitTrainsEv
        //map[0x6F7470] = 0x;   // call    _ZN6CTrain25ReadAndInterpretTrackFileEPcPP10CTrainNodePiPfi ; "tracks.dat" //doesn't exist
        //map[0x6F74BC] = 0x;   // call    _ZN6CTrain25ReadAndInterpretTrackFileEPcPP10CTrainNodePiPfi ; "tracks2.dat" //doesn't exist
        //map[0x6F7496] = 0x;   // call    _ZN6CTrain25ReadAndInterpretTrackFileEPcPP10CTrainNodePiPfi ; "tracks3.dat" //doesn't exist
        //map[0x6F74E2] = 0x;   // call    _ZN6CTrain25ReadAndInterpretTrackFileEPcPP10CTrainNodePiPfi ; "tracks4.dat" //doesn't exist
        map[0x748CFB] = 0x600411;   // call    _Z14InitialiseGamev
        //map[0x590D2A] = 0x;   // mov     eax, 8Ch                                 ; 8Ch = Loading Screen Max Progress //doesn't exist
        //map[0x590D67] = 0x;   // cmp     eax, 8Ch                                 ; 8Ch = Loading Screen Max Progress //doesn't exist
        //map[0x5B906A] = 0x;   // call    _ZN11CFileLoader8LoadLineEi              ; @CFileLoader::LoadLevel -- loop begin //check 48DDB8
        //map[0x5B92E6] = 0x;   // call    _ZN11CFileLoader8LoadLineEi              ; @CFileLoader::LoadLevel -- loop end //check 48DDB8
        map[0x5B92F9] = 0x48DDFD;   // call    _ZN8CFileMgr9CloseFileEi                 ; @CFileLoader::LoadLevel
        map[0x53BC95] = 0x4A4C99;   // call    _ZN11CFileLoader9LoadLevelEPKc           ; @CGame::Initialise -- default.dat
        map[0x53BC9B] = 0x4A4CA0;   // call    _ZN11CFileLoader9LoadLevelEPKc           ; @CGame::Initialise -- gta.dat
        map[0x4C5940] = 0x55F7D0;   // _ZN10CModelInfo12GetModelInfoEPKcPi
        map[0x5B925F] = 0x48DCF2;   // call    _ZN11CObjectData10InitialiseEPc          ; @CFileLoader::LoadLevel

        map[xVc(0x5C395A)] = 0x5C395A; // call    _ZN8CFileMgr8OpenFileEPKcS1_ ; "data/waterpro.dat"
        map[xVc(0x4D0614)] = 0x4D0614; // call    _ZN8CFileMgr12LoadTextFileEPKcPhi ; "timecyc.dat"
        map[xVc(0x4CE8A0)] = 0x4CE8A0; // _ZN13CSurfaceTable10InitialiseEPc
        map[xVc(0x4CE8CC)] = 0x4CE8CC; // call    _ZN8CFileMgr12LoadTextFileEPKcPhi
    }

    // traits
    if(true)
    {
        map[0x5B62CF] = 0x40FCDD;   // -> DWORD 4E20h   ; TXD Start Index
        map[0x5B6314] = 0x40FD1B;   // -> DWORD 61A8h   ; COL Start Index
        map[0x5B63C5] = 0x40FD48;   // -> DWORD 63E7h   ; IFP Start Index
        map[0x408897] = 0x4014EF+1; // -> offset _ZN10CModelInfo16ms_modelInfoPtrsE


        // TODO FIXME LATER
        static uint32_t minusone32 = -1;
        map[0x5B63F1] = (uintptr_t)&minusone32;
        map[0x5B641F] = (uintptr_t)&minusone32;
    }
 
    // helpers for VC emulating SA
    if(true)
    {
        map[xVc(0x40FF65)] = 0x40FF65; // begin of inlined CStreaming::RemoveAllUnusedModels
        map[xVc(0x40FFB1)] = 0x40FFB1; // end of inlined CStreaming::RemoveAllUnusedModels
    }

    // AbstractFrontend | TheMenu
    if(true)
    {
        // Not available for VC
    }
}
