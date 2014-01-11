//----------------------------------------------------------
//
//   SA Limit Adjuster
//   Copyright 2008 Sacky
//
//   ...with some modifications to compile on modloader...
//
//
//----------------------------------------------------------
#include "sala.h"
#include "sacky.hpp"
#include <modloader_util_injector.hpp>

// Some defines to make sacky's code compatible with modloader
#define saVersion       VERSION_US_1
#define Log             salaPlugin->Log
#define patch(a,b)      WriteMemoryRaw(raw_ptr(a), (b), sizeof((b)), true)
#define patchFloat(a,b) WriteMemory<float>(raw_ptr(a), (b), true)
#define nop(a,b)        MakeNOP(raw_ptr(a), (b))
#define Patch(a,b)      patch(a,b)
#define Nop(a,b)        nop(a,b)
#define Patch_Float(a,b) patchFloat(a,b)

//
#define memadd(num,arr) dwAlloc+=num;cBytes=(BYTE*)&dwAlloc;for(int i=0;i<4;i++)arr[i]=cBytes[i]




SALimitAdjuster::SALimitAdjuster()
{
    bDebug = true;
    Log("Starting up SALimitAdjuster()");
}

SALimitAdjuster::~SALimitAdjuster()
{
}

void SALimitAdjuster::SetBuildings(int iBuildings)
{
	BYTE* cBytes = (BYTE*)&iBuildings;
	BYTE pushBuildings[] = { 0x68, cBytes[0], cBytes[1], cBytes[2], cBytes[3] };
	patch(0x55105E,pushBuildings);
	if(bDebug)
		Log("Buildings: %d",iBuildings);
}

void SALimitAdjuster::SetDummys(int iDummys)
{
	BYTE* cBytes = (BYTE*)&iDummys;
	BYTE pushDummys[] = { 0x68, cBytes[0], cBytes[1], cBytes[2], cBytes[3] };
	patch(0x5510CE,pushDummys);
	if(bDebug)
		Log("Dummys: %d",iDummys);
}

void SALimitAdjuster::SetPtrNodeDoubles(int iPtrNodeDoubles)
{
	BYTE* cBytes = (BYTE*)&iPtrNodeDoubles;
	BYTE pushPtrNodeDoubles[] = { 0x68, cBytes[0], cBytes[1], cBytes[2], cBytes[3] };
	patch(0x550F81,pushPtrNodeDoubles);
	if(bDebug)
		Log("PtrNodeDoubles: %d",iPtrNodeDoubles);
}

void SALimitAdjuster::SetIPLFiles(int iIPLFiles)
{
	BYTE* cBytes = (BYTE*)&iIPLFiles;
	BYTE pushIPLFiles[] = { 0x68, cBytes[0], cBytes[1], cBytes[2], cBytes[3] };
	patch(0x405F25,pushIPLFiles);
	if(bDebug)
		Log("IPL Files: %d",iIPLFiles);
}

void SALimitAdjuster::SetStuntJumps(int iStuntJumps)
{
	BYTE* cBytes = (BYTE*)&iStuntJumps;
	BYTE pushStuntJumps[] = { 0x68, cBytes[0], cBytes[1], cBytes[2], cBytes[3] };
	switch(saVersion)
	{
		case VERSION_US_1: patch(0x156DDE4,pushStuntJumps); break;
		case VERSION_EU_1: patch(0x156DE14,pushStuntJumps); break;
	}
	if(bDebug)
		Log("Stunt Jumps: %d",iStuntJumps);
}

void SALimitAdjuster::SetColModels(int iColModels)
{
	BYTE* cBytes = (BYTE*)&iColModels;
	BYTE pushColModels[] = { 0x68, cBytes[0], cBytes[1], cBytes[2], cBytes[3] };
	patch(0x551106,pushColModels);
	if(bDebug)
		Log("ColModels: %d",iColModels);
}

void SALimitAdjuster::SetCollisionFiles(int iCollisionFiles)
{
	BYTE* cBytes = (BYTE*)&iCollisionFiles;
	BYTE pushCollisionFiles[] = { 0x68, cBytes[0], cBytes[1], cBytes[2], cBytes[3] };
	patch(0x411457,pushCollisionFiles);
	if(bDebug)
		Log("Collision Files: %d",iCollisionFiles);
}

void SALimitAdjuster::SetQuadTreeNodes(int iQuadTreeNodes)
{
	BYTE* cBytes = (BYTE*)&iQuadTreeNodes;
	BYTE pushQuadTreeNodes[] = { 0x68, cBytes[0], cBytes[1], cBytes[2], cBytes[3] };
	patch(0x552C3E,pushQuadTreeNodes);
	if(bDebug)
		Log("QuadTreeNodes: %d",iQuadTreeNodes);
}

void SALimitAdjuster::SetQuadTreeNodes2(int iQuadTreeNodes2)
{
	BYTE* cBytes = (BYTE*)&iQuadTreeNodes2;
	BYTE pushQuadTreeNodes2[] = { 0x6A, cBytes[0]};
	patch(0x405F4C,pushQuadTreeNodes2);
	patch(0x41147E,pushQuadTreeNodes2);
	if(bDebug)
		Log("QuadTreeNodes2: %d",iQuadTreeNodes2);
}

void SALimitAdjuster::SetMatDataPool(int iMatDataPool)
{
	BYTE* cBytes = (BYTE*)&iMatDataPool;
	BYTE pushMatDataPool[] = { 0x68, cBytes[0], cBytes[1], cBytes[2], cBytes[3]};
	patch(0x5DA08D,pushMatDataPool);
	patch(0x5DA105,pushMatDataPool);
	if(bDebug)
		Log("MatDataPool: %d",iMatDataPool);
}

void SALimitAdjuster::SetAtmDataPool(int iAtmDataPool)
{
	BYTE* cBytes = (BYTE*)&iAtmDataPool;
	BYTE pushAtmDataPool[] = { 0x68, cBytes[0], cBytes[1], cBytes[2], cBytes[3]};
	patch(0x5DA0C9,pushAtmDataPool);
	if(bDebug)
		Log("AtmDataPool: %d",iAtmDataPool);
}

void SALimitAdjuster::SetVehicles(int iVehicles)
{
	BYTE* cBytes = (BYTE*)&iVehicles;
	BYTE pushVehicles[] = { 0x6A, cBytes[0]};
	patch(0x551029,pushVehicles);
	if(bDebug)
		Log("Vehicles: %d",iVehicles);
}

void SALimitAdjuster::SetVehicleStructs(int iVehicleStructs)
{
	BYTE* cBytes = (BYTE*)&iVehicleStructs;
	BYTE pushVehicleStructs[] = { 0x6A, cBytes[0]};
	patch(0x5B8FE3,pushVehicleStructs);
	if(bDebug)
		Log("Vehicle Structs: %d",iVehicleStructs);
}

void SALimitAdjuster::SetPeds(int iPeds)
{
	BYTE* cBytes = (BYTE*)&iPeds;
	BYTE pushPeds[] = { 0x68, cBytes[0], cBytes[1], cBytes[2], cBytes[3]};
	patch(0x550FF1,pushPeds);
	patch(0x551282,pushPeds);
	if(bDebug)
		Log("Peds: %d",iPeds);
}

void SALimitAdjuster::SetPolygons(int iPolygons)
{
	BYTE* cBytes = (BYTE*)&iPolygons;
	BYTE pushPolygons[] = { 0x68, cBytes[0], cBytes[1], cBytes[2], cBytes[3]};
	patch(0x731F5F,pushPolygons);
	if(bDebug)
		Log("Polygons: %d",iPolygons);
}

void SALimitAdjuster::SetPtrNodeSingles(int iPtrNodeSingles)
{
	BYTE* cBytes = (BYTE*)&iPtrNodeSingles;
	BYTE pushPtrNodeSingles[] = { 0x68, cBytes[0], cBytes[1], cBytes[2], cBytes[3]};
	patch(0x550F45,pushPtrNodeSingles);
	if(bDebug)
		Log("PtrNodeSingles: %d",iPtrNodeSingles);
}

void SALimitAdjuster::SetEntryInfoNodes(int iEntryInfoNodes)
{
	BYTE* cBytes = (BYTE*)&iEntryInfoNodes;
	BYTE pushEntryInfoNodes[] = { 0x68, cBytes[0], cBytes[1], cBytes[2], cBytes[3]};
	patch(0x550FB9,pushEntryInfoNodes);
	if(bDebug)
		Log("EntryInfoNodes: %d",iEntryInfoNodes);
}

void SALimitAdjuster::SetObjects(int iObjects)
{
	BYTE* cBytes = (BYTE*)&iObjects;
	BYTE pushObjects[] = { 0x68, cBytes[0], cBytes[1], cBytes[2], cBytes[3]};
	patch(0x551096,pushObjects);
	if(bDebug)
		Log("Objects: %d",iObjects);
}

void SALimitAdjuster::SetTasks(int iTasks)
{
	BYTE* cBytes = (BYTE*)&iTasks;
	BYTE pushTasks[] = { 0x68, cBytes[0], cBytes[1], cBytes[2], cBytes[3]};
	patch(0x55113E,pushTasks);
	if(bDebug)
		Log("Tasks: %d",iTasks);
}

void SALimitAdjuster::SetEvents(int iEvents)
{
	BYTE* cBytes = (BYTE*)&iEvents;
	BYTE pushEvents[] = { 0x68, cBytes[0], cBytes[1], cBytes[2], cBytes[3]};
	patch(0x551176,pushEvents);
	if(bDebug)
		Log("Events: %d",iEvents);
}

void SALimitAdjuster::SetPointRoutes(int iPointRoutes)
{
	BYTE* cBytes = (BYTE*)&iPointRoutes;
	BYTE pushPointRoutes[] = { 0x6A, cBytes[0]};
	patch(0x5511AE,pushPointRoutes);
	if(bDebug)
		Log("Point Routes: %d",iPointRoutes);
}

void SALimitAdjuster::SetPatrolRoutes(int iPatrolRoutes)
{
	BYTE* cBytes = (BYTE*)&iPatrolRoutes;
	BYTE pushPatrolRoutes[] = { 0x6A, cBytes[0]};
	patch(0x5511E3,pushPatrolRoutes);
	if(bDebug)
		Log("Point Routes: %d",iPatrolRoutes);
}

void SALimitAdjuster::SetNodeRoutes(int iNodeRoutes)
{
	BYTE* cBytes = (BYTE*)&iNodeRoutes;
	BYTE pushNodeRoutes[] = { 0x6A, cBytes[0]};
	patch(0x551218,pushNodeRoutes);
	if(bDebug)
		Log("Node Routes: %d",iNodeRoutes);
}

void SALimitAdjuster::SetTaskAllocators(int iTaskAllocators)
{
	BYTE* cBytes = (BYTE*)&iTaskAllocators;
	BYTE pushTaskAllocators[] = { 0x6A, cBytes[0]};
	patch(0x551218,pushTaskAllocators);
	if(bDebug)
		Log("Task Allocators: %d",iTaskAllocators);
}

void SALimitAdjuster::SetPedAttractors(int iPedAttractors)
{
	BYTE* cBytes = (BYTE*)&iPedAttractors;
	BYTE pushPedAttractors[] = { 0x6A, cBytes[0]};
	patch(0x5512BB,pushPedAttractors);
	if(bDebug)
		Log("Ped Attractors: %d",iPedAttractors);
}

void SALimitAdjuster::SetEntryExits(int iEntryExits)
{
	BYTE* cBytes = (BYTE*)&iEntryExits;
	BYTE pushEntryExits[] = { 0x68, cBytes[0], cBytes[1], cBytes[2], cBytes[3]};
	switch(saVersion)
	{
		case VERSION_US_1: patch(0x156A797,pushEntryExits); break;
		case VERSION_EU_1: patch(0x156A777,pushEntryExits); break;
	}
	if(bDebug)
		Log("Entry Exits: %d",iEntryExits);
}

void SALimitAdjuster::SetStreamingMemory(int iStreamingMemory)
{
	int iBytes = iStreamingMemory*1048576;
	BYTE* cBytes = (BYTE*)&iBytes;
	BYTE StreamingMemory[] = { cBytes[0], cBytes[1], cBytes[2], cBytes[3]};
	patch(0x8A5A80,StreamingMemory);
	nop(0x5B8E64,10);
	nop(0x5BCD50,5);
	nop(0x5BCD78,5);
	if(bDebug)
		Log("Streaming Memory: %d",iStreamingMemory);
}

void SALimitAdjuster::SetStreamingVehicles(int iStreamingVehicles)
{
	BYTE* cBytes = (BYTE*)&iStreamingVehicles;
	BYTE StreamingVehicles[] = { cBytes[0], cBytes[1], cBytes[2], cBytes[3]};
	BYTE cmpStreamingVehicles[] = { 0x83, 0xFA, (BYTE) iStreamingVehicles };
	patch(0x8A5A84,StreamingVehicles);
	patch(0x611C3D,cmpStreamingVehicles);
	nop(0x5BCD9C,5);
	nop(0x5B8E6E,10);
	if(bDebug)
		Log("Streaming Vehicles: %d",iStreamingVehicles);
}

void SALimitAdjuster::SetImgHeaders(int iImgHeaders)
{
	BYTE* cBytes = (BYTE*)&iImgHeaders;
	BYTE pushImgHeaders[] = { 0x68, cBytes[0], cBytes[1], cBytes[2], cBytes[3] };
	patch(0x4D5A79,pushImgHeaders);
	patch(0x5B8DDF,pushImgHeaders);
	patch(0x7315DA,pushImgHeaders);
	if(bDebug)
		Log("Img Headers: %d",iImgHeaders);
}

void SALimitAdjuster::SetCurrentMission(int iCurrentMission)
{
	DWORD dwAlloc = (DWORD) malloc(iCurrentMission);
	if(dwAlloc)
	{
		memset((LPVOID)dwAlloc,0x00,iCurrentMission);
		BYTE* cCurrentMission = (BYTE*)&dwAlloc;
		BYTE bCurrentMission[] = { cCurrentMission[0], cCurrentMission[1], cCurrentMission[2], cCurrentMission[3] };
		patch(0x4899D7,bCurrentMission);
		patch(0x4899F6,bCurrentMission);
		patch(0x489A0E,bCurrentMission);
		patch(0x489A60,bCurrentMission);
		patch(0x489A76,bCurrentMission);
		patch(0x489A8E,bCurrentMission);
		cCurrentMission = (BYTE*)&iCurrentMission;
		BYTE bCurrentMission2[] = { cCurrentMission[0], cCurrentMission[1], cCurrentMission[2], cCurrentMission[3] };
		patch(0x4899D2,bCurrentMission2);
		patch(0x489A5B,bCurrentMission2);
		if(bDebug)
			Log("Current Mission: %d",iCurrentMission);
	}
	else if(bDebug) Log("Current Mission Allocation Failed!");
}

void SALimitAdjuster::SetTimedObjects(int iTimedObjects)
{
	DWORD dwAlloc = (DWORD) malloc((0x24*iTimedObjects)+4);
	if(dwAlloc)
	{
		memset((LPVOID)dwAlloc,0x00,(0x24*iTimedObjects)+4);
		for(DWORD i=dwAlloc+4;i<(dwAlloc+4+(0x24*iTimedObjects));i+=0x24)
		{
			*(BYTE*) i = 0xB0;
			*(BYTE*) (i+1) = 0xBC;
			*(BYTE*) (i+2) = 0x85;
			*(BYTE*) (i+10) = 0xFF;
			*(BYTE*) (i+11) = 0xFF;
			*(BYTE*) (i+34) = 0xFF;
			*(BYTE*) (i+35) = 0xFF;
		}
		BYTE* cBytes = (BYTE*)&dwAlloc;
		BYTE bTimedObjects[] = { cBytes[0], cBytes[1], cBytes[2], cBytes[3] };
		patch(0x4C66B1,bTimedObjects);
		patch(0x4C66C2,bTimedObjects);
		patch(0x84BC51,bTimedObjects);
		patch(0x856261,bTimedObjects);
		patch(0x4C683B,bTimedObjects);
		memadd(4,bTimedObjects);
		patch(0x4C6464,bTimedObjects);
		patch(0x4C66BD,bTimedObjects);
		cBytes = (BYTE*)&iTimedObjects;
		BYTE pushTimedObjects[] = { 0x68, cBytes[0], cBytes[1], cBytes[2], cBytes[3] };
		patch(0x4C58A5,pushTimedObjects);
		if(bDebug)
			Log("Timed Objects: %d",iTimedObjects);
	}
	else if(bDebug) Log("Timed Object Allocation Failed!");
}

void SALimitAdjuster::SetVehicleModels(int iVehicleModels)
{
	DWORD dwAlloc = (DWORD) malloc((0x308*iVehicleModels)+4);
	if(dwAlloc)
	{
		memset((LPVOID)dwAlloc,0x00,(0x308*iVehicleModels)+4);
		for(DWORD i=dwAlloc+4;i<(dwAlloc+4+(0x308*iVehicleModels));i+=0x308)
		{
			*(BYTE*) i = 0xC8;
			*(BYTE*) (i+1) = 0xC5;
			*(BYTE*) (i+2) = 0x85;
		}
		BYTE* cBytes = (BYTE*)&dwAlloc;
		BYTE bVehicleModels[] = { cBytes[0], cBytes[1], cBytes[2], cBytes[3] };
		patch(0x4C6771,bVehicleModels);
		patch(0x4C6786,bVehicleModels);
		patch(0x4C64ED,bVehicleModels);
		patch(0x4C6508,bVehicleModels);
		patch(0x84BCD1,bVehicleModels);
		patch(0x8562A1,bVehicleModels);
		patch(0x4C6853,bVehicleModels);
		memadd(4,bVehicleModels);
		patch(0x4C64F8,bVehicleModels);
		patch(0x4C6780,bVehicleModels);
		cBytes = (BYTE*)&iVehicleModels;
		BYTE pushVehicleModels[] = { 0x68, cBytes[0], cBytes[1], cBytes[2], cBytes[3] };
		patch(0x4C6375,pushVehicleModels);
		patch(0x4C5F5B,pushVehicleModels);
		if(bDebug)
			Log("Vehicle Models: %d",iVehicleModels);
	}
	else if(bDebug) Log("Vehicle Model Allocation Failed!");
}

void SALimitAdjuster::SetWaterPlanes(int iWaterPlanes)
{
	DWORD dwAlloc = (DWORD) malloc((0x14*iWaterPlanes)+4);
	if(dwAlloc)
	{
		memset((LPVOID)dwAlloc,0x00,(0x14*iWaterPlanes)+4);
		BYTE* cBytes = (BYTE*)&dwAlloc;
		BYTE bWaterBlocks[] = { cBytes[0], cBytes[1], cBytes[2], cBytes[3] };
		patch(0x6E811C,bWaterBlocks);
		patch(0x6E8127,bWaterBlocks);
		patch(0x6E8143,bWaterBlocks);
		patch(0x6E814E,bWaterBlocks);
		patch(0x6E9E2F,bWaterBlocks);
		patch(0x6E9E63,bWaterBlocks);
		patch(0x6EA00A,bWaterBlocks);
		patch(0x6EA04C,bWaterBlocks);
		patch(0x6EA08E,bWaterBlocks);
		patch(0x6E7B9B,bWaterBlocks);
		patch(0x6E7BBC,bWaterBlocks);
		patch(0x6E7C51,bWaterBlocks);
		patch(0x6E7C73,bWaterBlocks);
		patch(0x6E5B6E,bWaterBlocks);
		patch(0x6E7E11,bWaterBlocks);
		patch(0x6E7E18,bWaterBlocks);
		patch(0x6E7E3A,bWaterBlocks);
		patch(0x6E7E41,bWaterBlocks);
		patch(0x6E7E5A,bWaterBlocks);
		patch(0x6E7E61,bWaterBlocks);
		patch(0x6E6487,bWaterBlocks);
		patch(0x6E64A7,bWaterBlocks);
		patch(0x6E65E4,bWaterBlocks);
		patch(0x6E6607,bWaterBlocks);
		patch(0x6E5EA3,bWaterBlocks);
		patch(0x6E5ED7,bWaterBlocks);
		patch(0x6E5F84,bWaterBlocks);
		patch(0x6E5F8B,bWaterBlocks);
		patch(0x6E5BC3,bWaterBlocks);
		patch(0x6E5BF7,bWaterBlocks);
		patch(0x6EFC1F,bWaterBlocks);
		patch(0x6EFC5E,bWaterBlocks);
		patch(0x6EFC95,bWaterBlocks);
		patch(0x6EFDF9,bWaterBlocks);
		patch(0x6EFE31,bWaterBlocks);
		memadd(2,bWaterBlocks);
		patch(0x6E5B36,bWaterBlocks);
		patch(0x6E5B75,bWaterBlocks);
		patch(0x6E5C15,bWaterBlocks);
		patch(0x6E5C45,bWaterBlocks);
		patch(0x6E5EFC,bWaterBlocks);
		patch(0x6E5F06,bWaterBlocks);
		patch(0x6E64BF,bWaterBlocks);
		patch(0x6E64E0,bWaterBlocks);
		patch(0x6E6624,bWaterBlocks);
		patch(0x6E6649,bWaterBlocks);
		patch(0x6E7BDC,bWaterBlocks);
		patch(0x6E7BF6,bWaterBlocks);
		patch(0x6E7C58,bWaterBlocks);
		patch(0x6E7C8E,bWaterBlocks);
		patch(0x6E7DF8,bWaterBlocks);
		patch(0x6E7E08,bWaterBlocks);
		patch(0x6E7E31,bWaterBlocks);
		patch(0x6E801C,bWaterBlocks);
		patch(0x6E8029,bWaterBlocks);
		patch(0x6E804B,bWaterBlocks);
		patch(0x6E8060,bWaterBlocks);
		patch(0x6E807B,bWaterBlocks);
		patch(0x6E8088,bWaterBlocks);
		patch(0x6E809A,bWaterBlocks);
		patch(0x6E80BF,bWaterBlocks);
		patch(0x6E80D8,bWaterBlocks);
		patch(0x6E80FD,bWaterBlocks);
		patch(0x6E9E8D,bWaterBlocks);
		patch(0x6E9EC1,bWaterBlocks);
		patch(0x6E9FFF,bWaterBlocks);
		patch(0x6EA03D,bWaterBlocks);
		patch(0x6EA07F,bWaterBlocks);
		patch(0x6EFC18,bWaterBlocks);
		patch(0x6EFC57,bWaterBlocks);
		patch(0x6EFC8E,bWaterBlocks);
		patch(0x6EFDCB,bWaterBlocks);
		patch(0x6EFE2A,bWaterBlocks);
		memadd(2,bWaterBlocks);
		patch(0x6E5863,bWaterBlocks);
		patch(0x6E58B7,bWaterBlocks);
		patch(0x6E5945,bWaterBlocks);
		patch(0x6E598F,bWaterBlocks);
		patch(0x6E5B7B,bWaterBlocks);
		patch(0x6E5CA9,bWaterBlocks);
		patch(0x6E5CB3,bWaterBlocks);
		patch(0x6E5CBB,bWaterBlocks);
		patch(0x6E5CC7,bWaterBlocks);
		patch(0x6E5CD3,bWaterBlocks);
		patch(0x6E5D83,bWaterBlocks);
		patch(0x6E5D8E,bWaterBlocks);
		patch(0x6E5D96,bWaterBlocks);
		patch(0x6E5DA3,bWaterBlocks);
		patch(0x6E5DAE,bWaterBlocks);
		patch(0x6E5FC4,bWaterBlocks);
		patch(0x6E5FD0,bWaterBlocks);
		patch(0x6E5FDE,bWaterBlocks);
		patch(0x6E5FE4,bWaterBlocks);
		patch(0x6E5FF0,bWaterBlocks);
		patch(0x6E60AE,bWaterBlocks);
		patch(0x6E60BA,bWaterBlocks);
		patch(0x6E60C4,bWaterBlocks);
		patch(0x6E60CA,bWaterBlocks);
		patch(0x6E60D4,bWaterBlocks);
		patch(0x6E9FC7,bWaterBlocks);
		patch(0x6EFBF6,bWaterBlocks);
		patch(0x6EFC35,bWaterBlocks);
		patch(0x6EFC6C,bWaterBlocks);
		patch(0x6EFD90,bWaterBlocks);
		patch(0x6EFDC2,bWaterBlocks);
		patch(0x6EFDF0,bWaterBlocks);
		patch(0x6EFE1E,bWaterBlocks);
		memadd(4,bWaterBlocks);
		patch(0x6E5CEF,bWaterBlocks);
		patch(0x6E5CFD,bWaterBlocks);
		patch(0x6E5D06,bWaterBlocks);
		patch(0x6E5D0D,bWaterBlocks);
		patch(0x6E5D1A,bWaterBlocks);
		patch(0x6E5DCB,bWaterBlocks);
		patch(0x6E5DD9,bWaterBlocks);
		patch(0x6E5DE2,bWaterBlocks);
		patch(0x6E5DE9,bWaterBlocks);
		patch(0x6E5DF4,bWaterBlocks);
		patch(0x6E600C,bWaterBlocks);
		patch(0x6E601A,bWaterBlocks);
		patch(0x6E6025,bWaterBlocks);
		patch(0x6E602C,bWaterBlocks);
		patch(0x6E6039,bWaterBlocks);
		patch(0x6E60F0,bWaterBlocks);
		patch(0x6E60FE,bWaterBlocks);
		patch(0x6E6107,bWaterBlocks);
		patch(0x6E610E,bWaterBlocks);
		patch(0x6E611B,bWaterBlocks);
		patch(0x6E9F09,bWaterBlocks);
		patch(0x6E9F41,bWaterBlocks);
		patch(0x6E9F71,bWaterBlocks);
		patch(0x6E9FA1,bWaterBlocks);
		memadd(4,bWaterBlocks);
		patch(0x6E5D30,bWaterBlocks);
		patch(0x6E5D3E,bWaterBlocks);
		patch(0x6E5D47,bWaterBlocks);
		patch(0x6E5D4E,bWaterBlocks);
		patch(0x6E5D5B,bWaterBlocks);
		patch(0x6E5E0B,bWaterBlocks);
		patch(0x6E5E19,bWaterBlocks);
		patch(0x6E5E22,bWaterBlocks);
		patch(0x6E5E29,bWaterBlocks);
		patch(0x6E5E34,bWaterBlocks);
		patch(0x6E604F,bWaterBlocks);
		patch(0x6E605D,bWaterBlocks);
		patch(0x6E6068,bWaterBlocks);
		patch(0x6E606F,bWaterBlocks);
		patch(0x6E607C,bWaterBlocks);
		patch(0x6E6131,bWaterBlocks);
		patch(0x6E613F,bWaterBlocks);
		patch(0x6E6148,bWaterBlocks);
		patch(0x6E614F,bWaterBlocks);
		patch(0x6E615C,bWaterBlocks);
		patch(0x6E9F20,bWaterBlocks);
		patch(0x6E9F54,bWaterBlocks);
		patch(0x6E9F84,bWaterBlocks);
		patch(0x6E9FB4,bWaterBlocks);
		memadd(4,bWaterBlocks);
		patch(0x6EA0EE,bWaterBlocks);
		patch(0x6EA149,bWaterBlocks);
		patch(0x6EA193,bWaterBlocks);
		patch(0x6EA1CB,bWaterBlocks);
		memadd(1,bWaterBlocks);
		patch(0x6EA0F5,bWaterBlocks);
		patch(0x6EA150,bWaterBlocks);
		patch(0x6EA19A,bWaterBlocks);
		patch(0x6EA1D2,bWaterBlocks);
		if(bDebug)
			Log("Water Planes: %d",iWaterPlanes);
	}
	else if(bDebug) Log("Water Plane Allocation Failed!");
}

void SALimitAdjuster::SetPickups(int iPickups)
{
	DWORD dwAlloc = (DWORD) malloc((0x20*iPickups)+4);
	if(dwAlloc)
	{
		memset((LPVOID)dwAlloc,0x00,(0x20*iPickups)+4);
		BYTE* cBytes = (BYTE*)&dwAlloc;
		BYTE bPickups[] = { cBytes[0], cBytes[1], cBytes[2], cBytes[3] };
		patch(0x4020BC,bPickups);
		patch(0x456BF5,bPickups);
		patch(0x456BFE,bPickups);
		patch(0x4571A1,bPickups);
		patch(0x457216,bPickups);
		patch(0x4590CC,bPickups);
		patch(0x47AC92,bPickups);
		patch(0x494727,bPickups);
		patch(0x585F0A,bPickups);
		patch(0x587111,bPickups);
		patch(0x5D3543,bPickups);
		patch(0x5D35A3,bPickups);
		memadd(4,bPickups);
		patch(0x455472,bPickups);
		patch(0x456E6D,bPickups);
		patch(0x457315,bPickups);
		patch(0x458E2C,bPickups);
		memadd(4,bPickups);
		patch(0x4571EA,bPickups);
		memadd(4,bPickups);
		patch(0x455205,bPickups);
		patch(0x457228,bPickups);
		patch(0x45726F,bPickups);
		patch(0x45728C,bPickups);
		memadd(4,bPickups);
		patch(0x456D33,bPickups);
		memadd(2,bPickups);
		patch(0x4563AA,bPickups);
		patch(0x456454,bPickups);
		patch(0x456A84,bPickups);
		memadd(4,bPickups);
		patch(0x4571F6,bPickups);
		memadd(2,bPickups);
		patch(0x45729D,bPickups);
		memadd(2,bPickups);
		patch(0x4556D5,bPickups);
		patch(0x456A3B,bPickups);
		patch(0x456C0D,bPickups);
		patch(0x456E03,bPickups);
		patch(0x456E85,bPickups);
		patch(0x456ED7,bPickups);
		patch(0x457347,bPickups);
		patch(0x457357,bPickups);
		patch(0x457369,bPickups);
		patch(0x457372,bPickups);
		memadd(2,bPickups);
		patch(0x456F52,bPickups);
		patch(0x45721C,bPickups);
		patch(0x457268,bPickups);
		patch(0x45727F,bPickups);
		patch(0x458FE6,bPickups);
		memadd(1,bPickups);
		patch(0x4571FC,bPickups);
		patch(0x45722E,bPickups);
		patch(0x4572AC,bPickups);
		patch(0x4572C4,bPickups);
		patch(0x457300,bPickups);
		patch(0x45731B,bPickups);
		memadd(31,bPickups);
		patch(0x456F65,bPickups);
		patch(0x457003,bPickups);
		patch(0x45709A,bPickups);
		memadd((iPickups*0x20)-61,bPickups);
		patch(0x456F44,bPickups);
		int iPickups2 = iPickups-1;
		cBytes = (BYTE*)&iPickups2;
		BYTE movPickups[] = { 0xBB, cBytes[0], cBytes[1], cBytes[2], cBytes[3] };
		patch(0x456F3E,movPickups);
		cBytes = (BYTE*)&iPickups;
		BYTE cmpPickups[] = { 0x81, 0xFB, cBytes[0], cBytes[1], cBytes[2], cBytes[3] };
		patch(0x45718E,cmpPickups);
		patch(0x4571D1,cmpPickups);
		if(bDebug)
			Log("Pickups: %d",iPickups);
	}
	else if(bDebug) Log("Pickup Allocation Failed!");
}

void SALimitAdjuster::SetMarkers(int iMarkers)
{
	DWORD dwAlloc = (DWORD) malloc((0x28*iMarkers)+4);
	if(dwAlloc)
	{
		memset((LPVOID)dwAlloc,0x00,(0x28*iMarkers)+4);
		BYTE* cBytes = (BYTE*)&dwAlloc;
		BYTE bMarkers[] = { cBytes[0], cBytes[1], cBytes[2], cBytes[3] };
		patch(0x405B05,bMarkers);
		patch(0x5838B2,bMarkers);
		patch(0x583A07,bMarkers);
		patch(0x583A13,bMarkers);
		patch(0x585AA7,bMarkers);
		patch(0x586D77,bMarkers);
		memadd(4,bMarkers);
		patch(0x5838C6,bMarkers);
		patch(0x58703E,bMarkers);
		patch(0x587071,bMarkers);
		patch(0x58709E,bMarkers);
		patch(0x5870B2,bMarkers);
		patch(0x5870F8,bMarkers);
		patch(0x587C88,bMarkers);
		memadd(4,bMarkers);
		patch(0x58386B,bMarkers);
		patch(0x583B1D,bMarkers);
		patch(0x5874D8,bMarkers);
		patch(0x58795B,bMarkers);
		memadd(4,bMarkers);
		patch(0x583B17,bMarkers);
		memadd(8,bMarkers);
		patch(0x58282D,bMarkers);
		patch(0x58283D,bMarkers);
		patch(0x58284E,bMarkers);
		patch(0x582858,bMarkers);
		patch(0x58288C,bMarkers);
		patch(0x5838A2,bMarkers);
		patch(0x5838EA,bMarkers);
		patch(0x5838FD,bMarkers);
		patch(0x583975,bMarkers);
		patch(0x583A37,bMarkers);
		patch(0x583A78,bMarkers);
		patch(0x583A89,bMarkers);
		patch(0x583A92,bMarkers);
		patch(0x583ACC,bMarkers);
		patch(0x583C8D,bMarkers);
		patch(0x583CDC,bMarkers);
		patch(0x583D3C,bMarkers);
		patch(0x583D8C,bMarkers);
		patch(0x583DCC,bMarkers);
		patch(0x583E1C,bMarkers);
		patch(0x583E6D,bMarkers);
		patch(0x583ECC,bMarkers);
		patch(0x583F1C,bMarkers);
		patch(0x587CFC,bMarkers);
		patch(0x587F81,bMarkers);
		memadd(4,bMarkers);
		patch(0x5838BC,bMarkers);
		patch(0x583A42,bMarkers);
		patch(0x587185,bMarkers);
		patch(0x5871AB,bMarkers);
		patch(0x5871BA,bMarkers);
		patch(0x5871CD,bMarkers);
		patch(0x587CA0,bMarkers);
		memadd(4,bMarkers);
		patch(0x5838CD,bMarkers);
		patch(0x583A4D,bMarkers);
		patch(0x583D0D,bMarkers);
		patch(0x587349,bMarkers);
		patch(0x58764C,bMarkers);
		patch(0x587B5C,bMarkers);
		patch(0x587CAB,bMarkers);
		patch(0x587FC3,bMarkers);
		patch(0x587FD2,bMarkers);
		memadd(4,bMarkers);
		patch(0x5838E0,bMarkers);
		patch(0x583A69,bMarkers);
		patch(0x583F36,bMarkers);
		patch(0x5871D7,bMarkers);
		patch(0x587CB1,bMarkers);
		patch(0x5D53C9,bMarkers);
		patch(0x5D5868,bMarkers);
		memadd(4,bMarkers);
		patch(0x5838D3,bMarkers);
		patch(0x583A5B,bMarkers);
		patch(0x583DA6,bMarkers);
		patch(0x587798,bMarkers);
		patch(0x587BC5,bMarkers);
		patch(0x587CB7,bMarkers);
		patch(0x588307,bMarkers);
		patch(0x5883A5,bMarkers);
		memadd(1,bMarkers);
		patch(0x582894,bMarkers);
		patch(0x583821,bMarkers);
		patch(0x5838D9,bMarkers);
		patch(0x58397D,bMarkers);
		patch(0x58398D,bMarkers);
		patch(0x583998,bMarkers);
		patch(0x5839A1,bMarkers);
		patch(0x583A25,bMarkers);
		patch(0x583A62,bMarkers);
		patch(0x583AD4,bMarkers);
		patch(0x583AE6,bMarkers);
		patch(0x583B05,bMarkers);
		patch(0x583C95,bMarkers);
		patch(0x583CB0,bMarkers);
		patch(0x583CB8,bMarkers);
		patch(0x583CE4,bMarkers);
		patch(0x583D44,bMarkers);
		patch(0x583D94,bMarkers);
		patch(0x583DD4,bMarkers);
		patch(0x583DF1,bMarkers);
		patch(0x583E24,bMarkers);
		patch(0x583E41,bMarkers);
		patch(0x583E75,bMarkers);
		patch(0x583EA4,bMarkers);
		patch(0x583ED4,bMarkers);
		patch(0x583EF1,bMarkers);
		patch(0x583F24,bMarkers);
		patch(0x585A33,bMarkers);
		patch(0x585AA1,bMarkers);
		patch(0x587147,bMarkers);
		patch(0x58723B,bMarkers);
		patch(0x5872AE,bMarkers);
		patch(0x587CBD,bMarkers);
		patch(0x587D04,bMarkers);
		patch(0x587F89,bMarkers);
		memadd(1,bMarkers);
		patch(0x583865,bMarkers);
		patch(0x583899,bMarkers);
		patch(0x5839E3,bMarkers);
		patch(0x5839F8,bMarkers);
		patch(0x583A55,bMarkers);
		patch(0x583D52,bMarkers);
		patch(0x583D5C,bMarkers);
		patch(0x583D66,bMarkers);
		patch(0x583E85,bMarkers);
		patch(0x585A18,bMarkers);
		patch(0x585BFE,bMarkers);
		patch(0x586D70,bMarkers);
		patch(0x587015,bMarkers);
		patch(0x587169,bMarkers);
		patch(0x5871EB,bMarkers);
		patch(0x587287,bMarkers);
		patch(0x587C74,bMarkers);
		patch(0x587CC3,bMarkers);
		memadd(39,bMarkers);
		patch(0x583840,bMarkers);
		patch(0x5839BD,bMarkers);
		if(bDebug)
			Log("Markers: %d",iMarkers);
	}
	else if(bDebug) Log("Marker Allocation Failed!");
}

void SALimitAdjuster::SetGarages(int iGarages)
{
	DWORD dwAlloc = (DWORD) malloc((0x216*iGarages)+4);
	if(dwAlloc)
	{
		memset((LPVOID)dwAlloc,0x00,(0x216*iGarages)+4);
		BYTE* cBytes = (BYTE*)&dwAlloc;
		BYTE bGarages[] = { cBytes[0], cBytes[1], cBytes[2], cBytes[3] };
		patch(0x401089,bGarages);
		patch(0x447493,bGarages);
		patch(0x449754,bGarages);
		patch(0x449BAF,bGarages);
		patch(0x44A025,bGarages);
		patch(0x44A26E,bGarages);
		patch(0x44A3E6,bGarages);
		patch(0x44C902,bGarages);
		patch(0x44CA1F,bGarages);
		patch(0x44CA2A,bGarages);
		patch(0x5D3224,bGarages);
		patch(0x5D3335,bGarages);
		patch(0x849EA2,bGarages);
		patch(0x85613D,bGarages);
		memadd(4,bGarages);
		patch(0x44749D,bGarages);
		memadd(4,bGarages);
		patch(0x4474A3,bGarages);
		memadd(4,bGarages);
		patch(0x4474A9,bGarages);
		patch(0x4474F4,bGarages);
		patch(0x44750C,bGarages);
		patch(0x447514,bGarages);
		memadd(4,bGarages);
		patch(0x4474C2,bGarages);
		patch(0x4474EA,bGarages);
		patch(0x44751C,bGarages);
		patch(0x447528,bGarages);
		memadd(4,bGarages);
		patch(0x4474D0,bGarages);
		patch(0x447538,bGarages);
		patch(0x447550,bGarages);
		patch(0x447558,bGarages);
		memadd(4,bGarages);
		patch(0x4474DE,bGarages);
		patch(0x44752E,bGarages);
		patch(0x447560,bGarages);
		patch(0x44756C,bGarages);
		memadd(4,bGarages);
		patch(0x4474E4,bGarages);
		memadd(4,bGarages);
		patch(0x447506,bGarages);
		patch(0x447522,bGarages);
		memadd(4,bGarages);
		patch(0x44754A,bGarages);
		patch(0x447566,bGarages);
		memadd(4,bGarages);
		patch(0x447296,bGarages);
		patch(0x44C97C,bGarages);
		patch(0x44C99D,bGarages);
		patch(0x44C9B2,bGarages);
		memadd(4,bGarages);
		patch(0x447333,bGarages);
		memadd(4,bGarages);
		patch(0x4473E0,bGarages);
		patch(0x44C9D4,bGarages);
		patch(0x44C9F6,bGarages);
		patch(0x44CA0C,bGarages);
		memadd(4,bGarages);
		patch(0x44747D,bGarages);
		memadd(4,bGarages);
		patch(0x44A554,bGarages);
		patch(0x44A577,bGarages);
		patch(0x44A5AF,bGarages);
		patch(0x44A5CB,bGarages);
		patch(0x44A5E2,bGarages);
		memadd(12,bGarages);
		patch(0x447583,bGarages);
		memadd(8,bGarages);
		patch(0x447572,bGarages);
		patch(0x448B6E,bGarages);
		patch(0x44A174,bGarages);
		patch(0x44A459,bGarages);
		patch(0x44A5A4,bGarages);
		patch(0x44C95A,bGarages);
		memadd(2,bGarages);
		patch(0x4475A1,bGarages);
		patch(0x4475AD,bGarages);
		patch(0x447E1D,bGarages);
		patch(0x447E23,bGarages);
		patch(0x44A54B,bGarages);
		patch(0x44A581,bGarages);
		patch(0x44A604,bGarages);
		memadd(1,bGarages);
		patch(0x447578,bGarages);
		if(bDebug)
			Log("Garages: %d",iGarages);
	}
	else if(bDebug) Log("Garage Allocation Failed!");
}

void SALimitAdjuster::SetCarCols(int iCarCols)
{
	DWORD dwAlloc = (DWORD) malloc((0x4*iCarCols)+4);
	if(dwAlloc)
	{
		memset((LPVOID)dwAlloc,0x00,(0x4*iCarCols)+4);
		BYTE* cBytes = (BYTE*)&dwAlloc;
		BYTE bCarCols[] = { cBytes[0], cBytes[1], cBytes[2], cBytes[3] };
		patch(0x6A6FFA,bCarCols);
		patch(0x44B1C1,bCarCols);
		patch(0x4C8390,bCarCols);
		patch(0x5817CC,bCarCols);
		patch(0x582176,bCarCols);
		memadd(1,bCarCols);
		patch(0x4C8399,bCarCols);
		patch(0x5B68D8,bCarCols);
		memadd(1,bCarCols);
		patch(0x4C83A3,bCarCols);
		if(bDebug)
			Log("CarCols: %d",iCarCols);
	}
	else if(bDebug) Log("CarCol Allocation Failed!");
}


void SALimitAdjuster::SetIDEs(int iIDEs)
{
	DWORD dwAlloc = (DWORD) malloc((0x20*iIDEs)+4);
	if(dwAlloc)
	{
		memset((LPVOID)dwAlloc,0x00,(0x20*iIDEs)+4);
		for(DWORD i=dwAlloc+4;i<(dwAlloc+4+(iIDEs*0x20));i+=0x20)
		{
			*(BYTE*) i = 0xF0;
			*(BYTE*) (i+1) = 0xBB;
			*(BYTE*) (i+2) = 0x85;
			*(BYTE*) (i+10) = 0xFF;
			*(BYTE*) (i+11) = 0xFF;
		}
		BYTE* cBytes = (BYTE*)&dwAlloc;
		BYTE bIDEs[] = { cBytes[0], cBytes[1], cBytes[2], cBytes[3] };
		patch(0x4C6621,bIDEs);
		patch(0x4C6633,bIDEs);
		patch(0x4C63E1,bIDEs);
		patch(0x4C63FE,bIDEs);
		patch(0x84BBF1,bIDEs);
		patch(0x856231,bIDEs);
		patch(0x4C6865,bIDEs);
		patch(0x4C689A,bIDEs);
		patch(0x4C68B5,bIDEs);
		patch(0x4C68E8,bIDEs);
		patch(0x4C68F9,bIDEs);
		patch(0x4C6927,bIDEs);
		patch(0x4C6938,bIDEs);
		patch(0x4C6966,bIDEs);
		patch(0x4C6977,bIDEs);
		patch(0x4C69A5,bIDEs);
		patch(0x4C69B6,bIDEs);
		patch(0x4C69E4,bIDEs);
		patch(0x4C69F5,bIDEs);
		patch(0x4C6A23,bIDEs);
		patch(0x4C6A34,bIDEs);
		patch(0x4C6865,bIDEs);
		memadd(4,bIDEs);
		patch(0x4C662D,bIDEs);
		patch(0x4C6822,bIDEs);
		patch(0x4C6829,bIDEs);
		patch(0x4C6877,bIDEs);
		patch(0x4C6881,bIDEs);
		patch(0x4C6890,bIDEs);
		patch(0x4C68A5,bIDEs);
		patch(0x4C68F3,bIDEs);
		patch(0x4C6932,bIDEs);
		patch(0x4C6971,bIDEs);
		patch(0x4C69B0,bIDEs);
		patch(0x4C69EF,bIDEs);
		patch(0x4C6A2E,bIDEs);
		patch(0x4C63F2,bIDEs);
		memadd(24,bIDEs);
		patch(0x4C68AC,bIDEs);
		cBytes = (BYTE*)&iIDEs;
		BYTE pushIDEs[] = { 0x68, cBytes[0], cBytes[1], cBytes[2], cBytes[3] };
		patch(0x4C5845,pushIDEs);
		patch(0x4C5CBB,pushIDEs);
		if(bDebug)
			Log("IDEs: %d",iIDEs);
	}
	else if(bDebug) Log("IDE Allocation Failed!");
}

void SALimitAdjuster::SetWeapons(int iWeapons)
{
	char** cWeaponNames = (char**)malloc((iWeapons*sizeof(char*))+4);
	if(cWeaponNames)
	{
		memset((LPVOID)cWeaponNames,0x00,(iWeapons*sizeof(char*))+4);
		char cBuffer[100];
		FILE* fWeapons = fopen("saweapons.txt","r");
		if(fWeapons)
		{
			for(int i=0;!feof(fWeapons)&&i<=iWeapons;i++)
			{
				fscanf(fWeapons,"%s",cBuffer);
				char* cWeapon = (char*) malloc((strlen(cBuffer)*sizeof(char))+1);
				memset((LPVOID)cWeapon,0x00,(strlen(cBuffer)*sizeof(char))+1);
				strcpy(cWeapon,cBuffer);
				cWeaponNames[i] = cWeapon;
			}
			DWORD dwAlloc = (DWORD) cWeaponNames;
			BYTE* cBytes = (BYTE*)&dwAlloc;
			BYTE bWeaponTable[] = { 0x8B, 0x04, 0xB5, cBytes[0], cBytes[1], cBytes[2], cBytes[3] };
			patch(0x743D20,bWeaponTable);
			int iWeapons2 = iWeapons-1;
			cBytes = (BYTE*)&iWeapons2;
			BYTE cmpWeapons[] = { 0x83, 0xFE, cBytes[0] };
			patch(0x743D36,cmpWeapons);
			dwAlloc = (DWORD) malloc((0x70*iWeapons)+4);
			if(dwAlloc)
			{
				memset((LPVOID)dwAlloc,0x00,(0x70*iWeapons)+4);
				DWORD dwAlloc2 = dwAlloc;
				cBytes = (BYTE*)&dwAlloc;
				BYTE bWeapons[] = { cBytes[0], cBytes[1], cBytes[2], cBytes[3] };
				patch(0x5BE779,bWeapons);
				patch(0x5BE9E5,bWeapons);
				patch(0x743C7B,bWeapons);
				patch(0x743C90,bWeapons);
				patch(0x743CA8,bWeapons);
				patch(0x743CC0,bWeapons);
				patch(0x855E1F,bWeapons);
				patch(0x856C6A,bWeapons);
				memadd(4,bWeapons);
				patch(0x5BE76F,bWeapons);
				patch(0x5BE9F5,bWeapons);
				patch(0x5BF77E,bWeapons);
				memadd(4,bWeapons);
				patch(0x5BE789,bWeapons);
				patch(0x5BE9FF,bWeapons);
				memadd(4,bWeapons);
				patch(0x5BE793,bWeapons);
				patch(0x5BEA16,bWeapons);
				memadd(4,bWeapons);
				patch(0x5BE783,bWeapons);
				patch(0x5BEA24,bWeapons);
				memadd(4,bWeapons);
				patch(0x5BE7AB,bWeapons);
				patch(0x5BEA35,bWeapons);
				memadd(4,bWeapons);
				patch(0x5BE7D5,bWeapons);
				patch(0x5BEACE,bWeapons);
				memadd(4,bWeapons);
				patch(0x5BE84E,bWeapons);
				patch(0x5BEB8F,bWeapons);
				patch(0x5BEB95,bWeapons);
				memadd(4,bWeapons);
				patch(0x5BEA3C,bWeapons);
				memadd(2,bWeapons);
				patch(0x5BEA51,bWeapons);
				memadd(2,bWeapons);
				patch(0x5BEA5B,bWeapons);
				memadd(12,bWeapons);
				patch(0x5BEB02,bWeapons);
				memadd(4,bWeapons);
				patch(0x5BEAA2,bWeapons);
				memadd(4,bWeapons);
				patch(0x5BEAB6,bWeapons);
				memadd(4,bWeapons);
				patch(0x5BEA98,bWeapons);
				memadd(4,bWeapons);
				patch(0x5BEA05,bWeapons);
				patch(0x5BEBCB,bWeapons);
				patch(0x5BEBF1,bWeapons);
				memadd(4,bWeapons);
				patch(0x5BEA42,bWeapons);
				patch(0x5BEBC2,bWeapons);
				patch(0x5BEBF7,bWeapons);
				memadd(4,bWeapons);
				patch(0x5BEA70,bWeapons);
				memadd(4,bWeapons);
				patch(0x5BEAAC,bWeapons);
				patch(0x5BEC03,bWeapons);
				patch(0x5BEC2E,bWeapons);
				memadd(4,bWeapons);
				patch(0x5BEAD8,bWeapons);
				patch(0x5BEBFD,bWeapons);
				patch(0x5BEC34,bWeapons);
				memadd(4,bWeapons);
				patch(0x5BEB0E,bWeapons);
				memadd(4,bWeapons);
				patch(0x5BEB27,bWeapons);
				memadd(4,bWeapons);
				patch(0x5BEADE,bWeapons);
				memadd(4,bWeapons);
				patch(0x5BEAC8,bWeapons);
				memadd(4,bWeapons);
				patch(0x5BEB08,bWeapons);
				memadd(4,bWeapons);
				patch(0x5BEB14,bWeapons);
				memadd(4,bWeapons);
				patch(0x5BEBAA,bWeapons);
				memadd(2,bWeapons);
				patch(0x5BE7BE,bWeapons);
				memadd(1,bWeapons);
				patch(0x5BE7B1,bWeapons);
				memadd(4930,bWeapons);
				patch(0x97A6F4,bWeapons);
				memadd(2710,bWeapons);
				patch(0x729106,bWeapons);
				dwAlloc2 += (0x70*(iWeapons));
				cBytes = (BYTE*)&dwAlloc2;
				for(int i=0;i<4;i++) bWeapons[i] = cBytes[i];
				patch(0x5BF7F8,bWeapons);
				if(bDebug)
					Log("Weapon IDs: %d",iWeapons);
			}
			else if(bDebug) Log("Weapon ID Allocation Failed!");
		}
		else if(bDebug) Log("Failed To Open saweapons.txt");
		fclose(fWeapons);
	}
	else if(bDebug) Log("Weapon Names Allocation Failed!");
}

void SALimitAdjuster::SetCarMods(int iCarMods)
{
	DWORD dwAlloc = (DWORD) malloc((0x24*iCarMods)+4);
	if(dwAlloc)
	{
		memset((LPVOID)dwAlloc,0x00,(0x24*iCarMods)+4);
		for(DWORD i=dwAlloc+4;i<(dwAlloc+4+(0x24*iCarMods));i+=0x24)
		{
			*(BYTE*) i = 0x30;
			*(BYTE*) (i+1) = 0xBC;
			*(BYTE*) (i+2) = 0x85;
			*(BYTE*) (i+10) = 0xFF;
			*(BYTE*) (i+11) = 0xFF;
		}
		BYTE* cBytes = (BYTE*)&dwAlloc;
		BYTE bCarMods[] = { cBytes[0], cBytes[1], cBytes[2], cBytes[3] };
		patch(0x4C640B,bCarMods);
		patch(0x4C6428,bCarMods);
		patch(0x4C65E0,bCarMods);
		patch(0x4C6651,bCarMods);
		patch(0x4C6662,bCarMods);
		patch(0x4C682F,bCarMods);
		patch(0x84BC11,bCarMods);
		patch(0x856241,bCarMods);
		memadd(4,bCarMods);
		patch(0x4C6416,bCarMods);
		patch(0x4C665D,bCarMods);
		cBytes = (BYTE*)&iCarMods;
		BYTE pushCarMods[] = { 0x6A, cBytes[0] };
		patch(0x4C5D1B,bCarMods);
		patch(0x4C5865,bCarMods);
		if(bDebug)
			Log("Car Mods: %d",iCarMods);
	}
	else if(bDebug) Log("Car Mods Allocation Failed!");
}

void SALimitAdjuster::SetSCMBlock(int iSCMBlock)
{
	DWORD dwAlloc = (DWORD) malloc(iSCMBlock);
	if(dwAlloc)
	{
		memset((LPVOID)dwAlloc,0x00,iSCMBlock);
		BYTE* cBytes = (BYTE*)&dwAlloc;
		BYTE bSCMBlock[] = { cBytes[0], cBytes[1], cBytes[2], cBytes[3] };
		patch(0x44CA44,bSCMBlock);
		patch(0x44CA94,bSCMBlock);
		patch(0x44CAB3,bSCMBlock);
		patch(0x44CB38,bSCMBlock);
		patch(0x44CB5A,bSCMBlock);
		patch(0x44CB62,bSCMBlock);
		patch(0x44CB6D,bSCMBlock);
		patch(0x44CBAF,bSCMBlock);
		patch(0x463DAE,bSCMBlock);
		patch(0x463E03,bSCMBlock);
		patch(0x463E3B,bSCMBlock);
		patch(0x463E54,bSCMBlock);
		patch(0x463FC9,bSCMBlock);
		patch(0x463FEA,bSCMBlock);
		patch(0x4640E0,bSCMBlock);
		patch(0x46415B,bSCMBlock);
		patch(0x464168,bSCMBlock);
		patch(0x464194,bSCMBlock);
		patch(0x4641BC,bSCMBlock);
		patch(0x46429C,bSCMBlock);
		patch(0x464313,bSCMBlock);
		patch(0x4643C2,bSCMBlock);
		patch(0x464414,bSCMBlock);
		patch(0x464423,bSCMBlock);
		patch(0x46444F,bSCMBlock);
		patch(0x464475,bSCMBlock);
		patch(0x464557,bSCMBlock);
		patch(0x4645D2,bSCMBlock);
		patch(0x4645DF,bSCMBlock);
		patch(0x46460B,bSCMBlock);
		patch(0x464633,bSCMBlock);
		patch(0x4647CB,bSCMBlock);
		patch(0x46482D,bSCMBlock);
		patch(0x464849,bSCMBlock);
		patch(0x464860,bSCMBlock);
		patch(0x466836,bSCMBlock);
		patch(0x468D5E,bSCMBlock);
		patch(0x468E7A,bSCMBlock);
		patch(0x468ED6,bSCMBlock);
		patch(0x46944A,bSCMBlock);
		patch(0x469F34,bSCMBlock);
		patch(0x5D4C94,bSCMBlock);
		patch(0x5D4CB1,bSCMBlock);
		patch(0x5D4F6B,bSCMBlock);
		patch(0x5D4F93,bSCMBlock);
		patch(0x5D5022,bSCMBlock);
		patch(0x5D503F,bSCMBlock);
		patch(0x5D5351,bSCMBlock);
		patch(0x5D5380,bSCMBlock);
		patch(0x11A36C5,bSCMBlock);
		patch(0x11A6520,bSCMBlock);
		patch(0x11A65B4,bSCMBlock);
		patch(0x11A6C11,bSCMBlock);
		patch(0x11AB5F4,bSCMBlock);
		patch(0x11AB684,bSCMBlock);
		patch(0x11AB6A1,bSCMBlock);
		patch(0x11AC01B,bSCMBlock);
		patch(0x11AF977,bSCMBlock);
		patch(0x11B097A,bSCMBlock);
		memadd(3,bSCMBlock);
		patch(0x5D4C44,bSCMBlock);
		patch(0x11A7421,bSCMBlock);
		patch(0x11A7427,bSCMBlock);
		patch(0x11B0A54,bSCMBlock);
		patch(0x11B0A5E,bSCMBlock);
		patch(0x11B0A64,bSCMBlock);
		patch(0x11B0BA2,bSCMBlock);
		memadd(5,bSCMBlock);
		patch(0x11A742C,bSCMBlock);
		patch(0x11B0A6A,bSCMBlock);
		patch(0x11B0BA9,bSCMBlock);
		patch(0x11B0BAF,bSCMBlock);
		cBytes = (BYTE*)&iSCMBlock;
		BYTE pushSCMBlock[] = { cBytes[0], cBytes[1], cBytes[2], cBytes[3] };
		patch(0x468E75,pushSCMBlock);
		patch(0x468ECF,pushSCMBlock);
		if(bDebug)
			Log("SCM Block: %d",iSCMBlock);
	}
	else if(bDebug) Log("SCM Block Allocation Failed!");
}

void SALimitAdjuster::SetSCMLocals(int iSCMLocals)
{
	DWORD dwAlloc = (DWORD) malloc((iSCMLocals*4));
	if(dwAlloc)
	{
		memset((LPVOID)dwAlloc,0x00,(iSCMLocals*4));
		BYTE* cBytes = (BYTE*)&dwAlloc;
		BYTE bSCMLocals[] = { cBytes[0], cBytes[1], cBytes[2], cBytes[3] };
		patch(0x463CDE,bSCMLocals);
		patch(0x46410B,bSCMLocals);
		patch(0x46417F,bSCMLocals);
		patch(0x4641DC,bSCMLocals);
		patch(0x4643EB,bSCMLocals);
		patch(0x464438,bSCMLocals);
		patch(0x464495,bSCMLocals);
		patch(0x464582,bSCMLocals);
		patch(0x4645F6,bSCMLocals);
		patch(0x464653,bSCMLocals);
		patch(0x4647F2,bSCMLocals);
		patch(0x465B08,bSCMLocals);
		patch(0x465BA6,bSCMLocals);
		patch(0x465C9A,bSCMLocals);
		patch(0x468D6A,bSCMLocals);
		patch(0x11AB519,bSCMLocals);
		patch(0x11AF8C1,bSCMLocals);
		patch(0x11AF98C,bSCMLocals);
		patch(0x11B0997,bSCMLocals);
		cBytes = (BYTE*)&iSCMLocals;
		BYTE movSCMLocals[] = { 0xB9, cBytes[0], cBytes[1], cBytes[2], cBytes[3] };
		patch(0x468D64,movSCMLocals);
		patch(0x1569F11,movSCMLocals);
		if(bDebug)
			Log("SCM Locals: %d",iSCMLocals);
	}
	else if(bDebug) Log("SCM Locals Allocation Failed!");
}

void SALimitAdjuster::SetSCMThreads(int iSCMThreads)
{
	DWORD dwAlloc = (DWORD) malloc(iSCMThreads*0xE0);
	if(dwAlloc)
	{
		memset((LPVOID)dwAlloc,0x00,iSCMThreads*0xE0);
		BYTE* cBytes = (BYTE*)&dwAlloc;
		BYTE bSCMThreads[] = { cBytes[0], cBytes[1], cBytes[2], cBytes[3] };
		patch(0x468D7B,bSCMThreads);
		patch(0x11A3636,bSCMThreads);
		patch(0x11AF823,bSCMThreads);
		memadd(iSCMThreads*0xE0,bSCMThreads);
		patch(0x468D9D,bSCMThreads);
		cBytes = (BYTE*)&iSCMThreads;
		BYTE asmSCMThreads[] = { cBytes[0], cBytes[1], cBytes[2], cBytes[3] };
		patch(0x468D97,asmSCMThreads);
		patch(0x156E217,asmSCMThreads);
		if(bDebug)
			Log("SCM Threads: %d",iSCMThreads);
	}
	else if(bDebug) Log("SCM Thread Allocation Failed!");
}

void SALimitAdjuster::SetBounds(float fBounds)
{
	float* fBounds2 = (float*) malloc(sizeof(float));
	*fBounds2 = -fBounds;
	DWORD dwAlloc = (DWORD) fBounds2;
	BYTE* cBytes = (BYTE*)&dwAlloc;
	BYTE bBoundsFloat_1[] = { cBytes[0], cBytes[1], cBytes[2], cBytes[3] };
	patch(0x5347ED,bBoundsFloat_1);
	patch(0x534834,bBoundsFloat_1);
	patch(0x534AFF,bBoundsFloat_1);
	patch(0x534B2C,bBoundsFloat_1);
	patch(0x534B55,bBoundsFloat_1);
	patch(0x578843,bBoundsFloat_1);
	patch(0x578869,bBoundsFloat_1);
	patchFloat(0x534811,-fBounds);
	patchFloat(0x534843,-fBounds);
	patchFloat(0x534B64,-fBounds);
	patchFloat(0x411413,-fBounds);
	patchFloat(0x41141B,-fBounds);
	patchFloat(0x405EE6,-fBounds);
	patchFloat(0x405EEE,-fBounds);
	//patchFloat(0x57883E,-fBounds);

	float* fBounds3 = (float*) malloc(sizeof(float));
	*fBounds3 = fBounds;
	dwAlloc = (DWORD) fBounds3;
	cBytes = (BYTE*)&dwAlloc;
	BYTE bBoundsFloat_0[] = { cBytes[0], cBytes[1], cBytes[2], cBytes[3] };
	patch(0x53481B,bBoundsFloat_0);
	patch(0x53484D,bBoundsFloat_0);
	patch(0x534B3C,bBoundsFloat_0);
	patch(0x534B6E,bBoundsFloat_0);
	patch(0x57882B,bBoundsFloat_0);
	patch(0x578856,bBoundsFloat_0);
	patchFloat(0x53482A,fBounds);
	patchFloat(0x53485C,fBounds);
	patchFloat(0x534B4B,fBounds);
	patchFloat(0x534B7D,fBounds);
	patchFloat(0x411423,fBounds);
	patchFloat(0x41142B,fBounds);
	patchFloat(0x405EF6,fBounds);
	patchFloat(0x405EFE,fBounds);
	//patchFloat(0x50E6DE,fBounds);
	//patchFloat(0x578825,fBounds);

	if(bDebug)
		Log("Map Bounds: %f",fBounds);
}

void SALimitAdjuster::SetAimingOffsets(int iAimingOffsets)
{
	DWORD dwAlloc = (DWORD) malloc((iAimingOffsets*0x18)+4);
	if(dwAlloc)
	{
		memset((LPVOID)dwAlloc,0x00,(iAimingOffsets*0x18)+4);
		BYTE* cBytes = (BYTE*)&dwAlloc;
		BYTE bAiming[] = { cBytes[0], cBytes[1], cBytes[2], cBytes[3] };
		patch(0x5BED30,bAiming);
		patch(0x5FD87F,bAiming);
		patch(0x5FDE93,bAiming);
		memadd(4,bAiming);
		patch(0x5BED3A,bAiming);
		patch(0x5BF802,bAiming);
		patch(0x5FD8AF,bAiming);
		patch(0x5FDED3,bAiming);
		memadd(4,bAiming);
		patch(0x5BED44,bAiming);
		patch(0x5FD872,bAiming);
		patch(0x5FDE8B,bAiming);
		memadd(4,bAiming);
		patch(0x5BED4F,bAiming);
		patch(0x5FD8A2,bAiming);
		patch(0x5FDECB,bAiming);
		memadd(4,bAiming);
		patch(0x5BED5B,bAiming);
		patch(0x73A2A1,bAiming);
		patch(0x743DAB,bAiming);
		memadd(2,bAiming);
		patch(0x5BED67,bAiming);
		patch(0x73A2D1,bAiming);
		patch(0x743DCC,bAiming);
		memadd(2,bAiming);
		patch(0x4039E5,bAiming);
		patch(0x5BED73,bAiming);
		patch(0x743DBE,bAiming);
		memadd(2,bAiming);
		patch(0x5BED7A,bAiming);
		patch(0x73A2C3,bAiming);
		patch(0x743DDD,bAiming);
		memadd(482,bAiming);
		patch(0x855D5E,bAiming);
		memadd((iAimingOffsets*0x18)-504,bAiming);
		patch(0x5BF826,bAiming);
		patch(0x855D7E,bAiming);
		if(bDebug)
			Log("Aiming Offsets: %d",iAimingOffsets);
	}
	else if(bDebug) Log("Aiming Offset Allocation Failed!");
}

void SALimitAdjuster::SetWeaponObjects(int iWeaponObjects)
{
	DWORD dwAlloc = (DWORD) malloc((iWeaponObjects*0x28)+4);
	if(dwAlloc)
	{
		memset((LPVOID)dwAlloc,0x00,(iWeaponObjects*0x28)+4);
		for(DWORD i=dwAlloc+4;i<(dwAlloc+4+(iWeaponObjects*0x28));i+=0x28)
		{
			*(BYTE*) i = 0x78;
			*(BYTE*) (i+1) = 0xBD;
			*(BYTE*) (i+2) = 0x85;
			*(BYTE*) (i+10) = 0xFF;
			*(BYTE*) (i+11) = 0xFF;
		}
		BYTE* cBytes = (BYTE*)&dwAlloc;
		BYTE bWeaponObject[] = { cBytes[0], cBytes[1], cBytes[2], cBytes[3] };
		patch(0x4C64A5,bWeaponObject);
		patch(0x4C64BC,bWeaponObject);
		patch(0x4C65F8,bWeaponObject);
		patch(0x4C6711,bWeaponObject);
		patch(0x4C6722,bWeaponObject);
		patch(0x4C6847,bWeaponObject);
		patch(0x84BC91,bWeaponObject);
		patch(0x856281,bWeaponObject);
		memadd(4,bWeaponObject);
		patch(0x4C64B0,bWeaponObject);
		patch(0x4C671D,bWeaponObject);
		if(bDebug)
			Log("Weapon Objects: %d",iWeaponObjects);
	}
	else if(bDebug) Log("Weapon Objects Allocation Failed!");
}

void SALimitAdjuster::SetPlayers(int iPlayers)
{
	DWORD dwAlloc = (DWORD) malloc(iPlayers*0x2D4);
	if(dwAlloc)
	{
		memset((LPVOID)dwAlloc,0x00,iPlayers*0x2D4);
		memcpy((LPVOID)dwAlloc,(void*)0xC09920,0x2D4);
		for(int i=1;i<iPlayers;i++)
			memcpy((LPVOID)(dwAlloc+(i*0x2D4)),(void*)0xC09BE8,0x2D4);
		BYTE* cBytes = (BYTE*)&dwAlloc;
		BYTE bPlayers[] = { cBytes[0], cBytes[1], cBytes[2], cBytes[3] };
		patch(0x444069,bPlayers);
		patch(0x4923AA,bPlayers);
		patch(0x4924A6,bPlayers);
		patch(0x49252E,bPlayers);
		patch(0x5684D4,bPlayers);
		patch(0x571B5C,bPlayers);
		patch(0x5F7ED2,bPlayers);
		patch(0x5F7EE8,bPlayers);
		patch(0x5FB305,bPlayers);
		patch(0x5FB3E4,bPlayers);
		patch(0x5FB824,bPlayers);
		patch(0x5FB88D,bPlayers);
		patch(0x5FB8B6,bPlayers);
		patch(0x5FCB3D,bPlayers);
		patch(0x5FD215,bPlayers);
		patch(0x5FD6C9,bPlayers);
		patch(0x60A29A,bPlayers);
		patch(0x60A392,bPlayers);
		patch(0x60CA87,bPlayers);
		patch(0x60CD0C,bPlayers);
		patch(0x60CF1F,bPlayers);
		patch(0x60D6BF,bPlayers);
		patch(0x60F770,bPlayers);
		patch(0x60F79D,bPlayers);
		patch(0x69CCB9,bPlayers);
		patch(0x69D1EE,bPlayers);
		patch(0x69D4E6,bPlayers);
		patch(0x851342,bPlayers);
		patch(0x85686D,bPlayers);
		memadd(4,bPlayers);
		patch(0x477360,bPlayers);
		patch(0x60A16B,bPlayers);
		patch(0x60A24B,bPlayers);
		patch(0x60CC87,bPlayers);
		patch(0x60CED8,bPlayers);
		patch(0x60F733,bPlayers);
		memadd(4,bPlayers);
		patch(0x401882,bPlayers);
		patch(0x43AD61,bPlayers);
		patch(0x43D23F,bPlayers);
		patch(0x43DD60,bPlayers);
		patch(0x43F214,bPlayers);
		patch(0x46FDEF,bPlayers);
		patch(0x472F83,bPlayers);
		patch(0x475FE4,bPlayers);
		patch(0x476E33,bPlayers);
		patch(0x47AD88,bPlayers);
		patch(0x49239F,bPlayers);
		patch(0x492466,bPlayers);
		patch(0x492722,bPlayers);
		patch(0x496D4F,bPlayers);
		patch(0x49878B,bPlayers);
		patch(0x4987DC,bPlayers);
		patch(0x4AFA10,bPlayers);
		patch(0x4B5CD2,bPlayers);
		patch(0x4B7D82,bPlayers);
		patch(0x4BE934,bPlayers);
		patch(0x4E5433,bPlayers);
		patch(0x4E546B,bPlayers);
		patch(0x571B2D,bPlayers);
		patch(0x58BDB2,bPlayers);
		patch(0x5DF12A,bPlayers);
		patch(0x5E90F6,bPlayers);
		patch(0x5F6AFD,bPlayers);
		patch(0x5F7F57,bPlayers);
		patch(0x5FB32C,bPlayers);
		patch(0x5FB40B,bPlayers);
		patch(0x5FB8C7,bPlayers);
		patch(0x5FB935,bPlayers);
		patch(0x60A0B4,bPlayers);
		patch(0x60A0F9,bPlayers);
		patch(0x60A14C,bPlayers);
		patch(0x60A229,bPlayers);
		patch(0x60A36E,bPlayers);
		patch(0x60A462,bPlayers);
		patch(0x60A4D2,bPlayers);
		patch(0x60C250,bPlayers);
		patch(0x60C6BB,bPlayers);
		patch(0x60C8E7,bPlayers);
		patch(0x60C98E,bPlayers);
		patch(0x60C9AD,bPlayers);
		patch(0x60C9CF,bPlayers);
		patch(0x60CA6D,bPlayers);
		patch(0x60CBB2,bPlayers);
		patch(0x60CC64,bPlayers);
		patch(0x60D6A5,bPlayers);
		patch(0x60F74A,bPlayers);
		patch(0x60F802,bPlayers);
		patch(0x622469,bPlayers);
		patch(0x622CD7,bPlayers);
		patch(0x622D0C,bPlayers);
		patch(0x65E490,bPlayers);
		patch(0x665D4F,bPlayers);
		patch(0x673B2F,bPlayers);
		patch(0x69C36F,bPlayers);
		memadd(4,bPlayers);
		patch(0x5F6B08,bPlayers);
		patch(0x5F7F1F,bPlayers);
		memadd(28,bPlayers);
		patch(0x5F6B18,bPlayers);
		patch(0x5F7E47,bPlayers);
		patch(0x5F7F2B,bPlayers);
		patch(0x5F7F33,bPlayers);
		memadd(8,bPlayers);
		patch(0x468815,bPlayers);
		patch(0x46C4F3,bPlayers);
		patch(0x49228B,bPlayers);
		patch(0x49259D,bPlayers);
		patch(0x4925E2,bPlayers);
		patch(0x4972D2,bPlayers);
		patch(0x5FC808,bPlayers);
		patch(0x60443D,bPlayers);
		patch(0x60A183,bPlayers);
		patch(0x60A263,bPlayers);
		patch(0x60A31C,bPlayers);
		patch(0x60CAE0,bPlayers);
		patch(0x60CC96,bPlayers);
		patch(0x60CEF0,bPlayers);
		patch(0x60CFA1,bPlayers);
		patch(0x60D674,bPlayers);
		patch(0x632ECE,bPlayers);
		patch(0x632F58,bPlayers);
		patch(0x632FD0,bPlayers);
		patch(0x633028,bPlayers);
		patch(0x6330C6,bPlayers);
		memadd(664,bPlayers);
		patch(0x5FC8F9,bPlayers);
		memadd(4,bPlayers);
		patch(0x471B44,bPlayers);
		memadd(4,bPlayers);
		patch(0x492299,bPlayers);
		patch(0x5FC8F2,bPlayers);
		patch(0x60D68E,bPlayers);
		memadd(716,bPlayers);
		patch(0x5FC90E,bPlayers);
		memadd(8,bPlayers);
		patch(0x5FC907,bPlayers);
		memadd(716,bPlayers);
		patch(0x5FC923,bPlayers);
		memadd(8,bPlayers);
		patch(0x5FC91C,bPlayers);
		memadd(716,bPlayers);
		patch(0x5FC938,bPlayers);
		memadd(8,bPlayers);
		patch(0x5FC931,bPlayers);
		memadd(716,bPlayers);
		patch(0x5FC94F,bPlayers);
		memadd(8,bPlayers);
		patch(0x5FC948,bPlayers);
		memadd(716,bPlayers);
		patch(0x5FC960,bPlayers);
		memadd(8,bPlayers);
		patch(0x5FC959,bPlayers);
		memadd(716,bPlayers);
		patch(0x5FC978,bPlayers);
		memadd(8,bPlayers);
		patch(0x5FC971,bPlayers);
		dwAlloc = (DWORD) malloc(iPlayers);
		if(dwAlloc)
		{
			memset((LPVOID)dwAlloc,0x00,iPlayers);
			cBytes = (BYTE*)&dwAlloc;
			BYTE bPlayerSlots[] = { cBytes[0], cBytes[1], cBytes[2], cBytes[3] };
			patch(0x43F204,bPlayerSlots);
			patch(0x483AF3,bPlayerSlots);
			patch(0x492710,bPlayerSlots);
			patch(0x498778,bPlayerSlots);
			patch(0x4B2A45,bPlayerSlots);
			patch(0x5684E2,bPlayerSlots);
			patch(0x5F7E52,bPlayerSlots);
			patch(0x5F7E92,bPlayerSlots);
			patch(0x5F7F62,bPlayerSlots);
			patch(0x5FB808,bPlayerSlots);
			patch(0x5FB82B,bPlayerSlots);
			patch(0x5FB876,bPlayerSlots);
			patch(0x5FB880,bPlayerSlots);
			patch(0x5FB8A6,bPlayerSlots);
			patch(0x5FB8D2,bPlayerSlots);
			patch(0x5FB8DC,bPlayerSlots);
			patch(0x5FC812,bPlayerSlots);
			patch(0x5FC865,bPlayerSlots);
			patch(0x5FC870,bPlayerSlots);
			patch(0x5FC8EA,bPlayerSlots);
			patch(0x604443,bPlayerSlots);
			memadd(1,bPlayerSlots);
			patch(0x5FC8FF,bPlayerSlots);
			memadd(1,bPlayerSlots);
			patch(0x5FC914,bPlayerSlots);
			memadd(1,bPlayerSlots);
			patch(0x5FC929,bPlayerSlots);
			memadd(1,bPlayerSlots);
			patch(0x5FC93E,bPlayerSlots);
			memadd(4,bPlayerSlots);
			patch(0x5F7E31,bPlayerSlots);
			patch(0x5FC893,bPlayerSlots);
			patch(0x5FC8AC,bPlayerSlots);
			patch(0x5FC99A,bPlayerSlots);
			nop(0x5FB811,3);
			BYTE jmp[] = { 0xEB };
			patch(0x5FB814,jmp);
			//BYTE jnz[] = { 0xE9, 0x9D, 0x00, 0x00, 0x00, 0x90 };
			//patch(0x60D64D,jnz);
			if(bDebug)
				Log("Players: %d",iPlayers);
		}
		else if(bDebug) Log("Player Slot Allocation Failed!");
	}
	else if(bDebug) Log("Players Allocation Failed!");
}

void SALimitAdjuster::SetPedModels(int iPedModels)
{
	DWORD dwAlloc = (DWORD) malloc((0x44*iPedModels)+4);
	if(dwAlloc)
	{
		memset((LPVOID)dwAlloc,0x00,(0x44*iPedModels)+4);
		for(DWORD i=dwAlloc+4;i<dwAlloc+4+(0x44*iPedModels);i+=0x44)
			memcpy((LPVOID)i,(LPVOID)0xB478FC,0x44);
		BYTE* cBytes = (BYTE*)&dwAlloc;
		BYTE bPeds[] = { cBytes[0], cBytes[1], cBytes[2], cBytes[3] };
		patch(0x4C6518,bPeds);
		patch(0x4C652F,bPeds);
		patch(0x4C660A,bPeds);
		patch(0x4C67A1,bPeds);
		patch(0x4C67B3,bPeds);
		patch(0x4C6859,bPeds);
		patch(0x84BCF1,bPeds);
		patch(0x8562B1,bPeds);
		memadd(4,bPeds);
		patch(0x4C6523,bPeds);
		patch(0x4C67AD,bPeds);
		if(bDebug)
			Log("Ped Models: %d",iPedModels);
	}
	else if(bDebug) Log("Ped Models Allocation Failed!");
}

void SALimitAdjuster::SetIPLs(int iIPLs)
{
	DWORD dwAlloc = (DWORD) malloc(iIPLs+4);
	if(dwAlloc)
	{
		memset((void*)dwAlloc,0x0,iIPLs+4);
		BYTE* cBytes = (BYTE*)&dwAlloc;
		BYTE bIPLs[] = { cBytes[0], cBytes[1], cBytes[2], cBytes[3] };
		if(saVersion == VERSION_US_1)
		{
			patch(0x1569777,bIPLs);
			patch(0x15649FA,bIPLs);
			patch(0x1561160,bIPLs);
		}
		else
		{
			patch(0x1569717,bIPLs);
			patch(0x156495A,bIPLs);
			patch(0x156115C,bIPLs);
		}
		patch(0x40619B,bIPLs);
		patch(0x405C3D,bIPLs);
		if(bDebug)
			Log("IPLs: %d",iIPLs);
	}
	else if(bDebug) Log("IPL Allocation Failed!");
}

void SALimitAdjuster::SetLOD(float fLod)
{
	patchFloat(0x858FD8,fLod);
	if(bDebug)
		Log("LOD Distance: %f",fLod);
}

void SALimitAdjuster::SetGangs(int iGangs)
{

}

void SALimitAdjuster::FillSALACfg(SALACfg& s)
{
    s.cDynamicLimits = "DYNAMIC LIMITS";
    s.cStaticLimits = "STATIC_LIMITS";
    s.cColours = "COLOURS";
    s.cCredits = "CREDITS";
    s.cAI = "AI";
    s.cGraphics = "GRAPHICS";
    s.cMisc = "MISC";
    s.cModels = "MODELS";
    s.cPerformance = "PERFORMANCE";    
}

void SALimitAdjuster::ReadIni(const char* cPath)
{
    SALACfg s;
    FillSALACfg(s);
    return ReadConfigFile(cPath, s);
}

void SALimitAdjuster::ReadSalaScript(const char* cPath)
{
    SALACfg s;
    FillSALACfg(s);
    s.cStaticLimits = "STATIC";
    s.cDynamicLimits = "DYNAMIC";
    s.cColours = "COLORS";
    s.cCredits = "MISC";
    return ReadConfigFile(cPath, s);  
}

void SALimitAdjuster::ReadConfigFile(const char* cPath, SALACfg& s)
{
	if(true)
	{
		char cTmp[100];
        int iVar = 0;
        DWORD dwVar = 0;
        float fVar = 0;
        BYTE cBytesTempBuf[4];
        BYTE* cBytes = cBytesTempBuf; //(char*) malloc(4*sizeof(char));
        // TEST SPACE
        //Nop(0x740DA7,5);
        // ----------
        // Check Credits
        GetPrivateProfileString(s.cCredits,"Made By",NULL,cTmp,sizeof(cTmp),cPath);
        if(cTmp[0] != ('T'-1)
        || cTmp[1] != ('b'-1)
        || cTmp[2] != ('d'-1)
        || cTmp[3] != ('l'-1)
        || cTmp[4] != ('z'-1))
        {
            MessageBox(0,"Screwing around with Credits isn't cool...","SA Limit Adjuster",0);
            ExitProcess(0);
        }
        GetPrivateProfileString(s.cMisc,"Debug",NULL,cTmp,sizeof(cTmp),cPath);
        // Patch Static Limits
        DWORD dwAlloc = 0;
        // Colours
        // Health Bar ==
        GetPrivateProfileString(s.cColours,"Health Bar",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%x",&dwVar);
            cBytes = (BYTE*)&dwVar;
            BYTE bColour[] = { cBytes[3], cBytes[2], cBytes[1], cBytes[0] };
            Patch(0xBAB22C,bColour);
        }
        // Money Font ==
        GetPrivateProfileString(s.cColours,"Money Font",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%x",&dwVar);
            cBytes = (BYTE*)&dwVar;
            BYTE bColour[] = { cBytes[3], cBytes[2], cBytes[1], cBytes[0] };
            Patch(0xBAB230,bColour);
        }
        // White ==
        GetPrivateProfileString(s.cColours,"White",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%x",&dwVar);
            cBytes = (BYTE*)&dwVar;
            BYTE bColour[] = { cBytes[3], cBytes[2], cBytes[1], cBytes[0] };
            Patch(0xBAB23C,bColour);
        }
        // Title Menu ==
        GetPrivateProfileString(s.cColours,"Title Menu",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%x",&dwVar);
            cBytes = (BYTE*)&dwVar;
            BYTE bColour[] = { cBytes[3], cBytes[2], cBytes[1], cBytes[0] };
            Patch(0xBAB238,bColour);
        }
        // Title Menu Border ==
        GetPrivateProfileString(s.cColours,"Title Menu Border",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%x",&dwVar);
            cBytes = (BYTE*)&dwVar;
            BYTE bColour[] = { cBytes[3], cBytes[2], cBytes[1], cBytes[0] };
            Patch(0xBAB240,bColour);
        }
        // Wanted Level ==
        GetPrivateProfileString(s.cColours,"Wanted Level",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%x",&dwVar);
            cBytes = (BYTE*)&dwVar;
            BYTE bColour[] = { cBytes[3], cBytes[2], cBytes[1], cBytes[0] };
            Patch(0xBAB244,bColour);
        }
        // Radio Station ==
        GetPrivateProfileString(s.cColours,"Radio Station",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%x",&dwVar);
            cBytes = (BYTE*)&dwVar;
            BYTE bColour[] = { cBytes[3], cBytes[2], cBytes[1], cBytes[0] };
            Patch(0xBAB24C,bColour);
        }
        // Models
        // Cop Cars
        for(int i=1;i<7;i++)
        {
            char cIni[34];
            if(i<6)
            {
                if(i<4)
                {
                    sprintf(cIni,"Ambulance %i",i);
                    GetPrivateProfileString(s.cModels,cIni,NULL,cTmp,sizeof(cTmp),cPath);
                    if(strlen(cTmp))
                    {
                        sscanf(cTmp,"%i",&iVar);
                        *(DWORD*) (0x8A5AB8+(0x4*(i-1))) = (DWORD) iVar;
                    }
                    sprintf(cIni,"Medic %i",i);
                    GetPrivateProfileString(s.cModels,cIni,NULL,cTmp,sizeof(cTmp),cPath);
                    if(strlen(cTmp))
                    {
                        sscanf(cTmp,"%i",&iVar);
                        *(DWORD*) (0x8A5AC8+(0x4*(i-1))) = (DWORD) iVar;
                    }
                    sprintf(cIni,"Firetruck %i",i);
                    GetPrivateProfileString(s.cModels,cIni,NULL,cTmp,sizeof(cTmp),cPath);
                    if(strlen(cTmp))
                    {
                        sscanf(cTmp,"%i",&iVar);
                        *(DWORD*) (0x8A5AD8+(0x4*(i-1))) = (DWORD) iVar;
                    }
                    sprintf(cIni,"Fireman %i",i);
                    GetPrivateProfileString(s.cModels,cIni,NULL,cTmp,sizeof(cTmp),cPath);
                    if(strlen(cTmp))
                    {
                        sscanf(cTmp,"%i",&iVar);
                        *(DWORD*) (0x8A5AE8+(0x4*(i-1))) = (DWORD) iVar;
                    }
                }
                sprintf(cIni,"CopCar %i",i);
                GetPrivateProfileString(s.cModels,cIni,NULL,cTmp,sizeof(cTmp),cPath);
                if(strlen(cTmp))
                {
                    sscanf(cTmp,"%i",&iVar);
                    *(DWORD*) (0x8A5A8C+(0x4*(i-1))) = (DWORD) iVar;
                }
                sprintf(cIni,"CopPed %i",i);
                GetPrivateProfileString(s.cModels,cIni,NULL,cTmp,sizeof(cTmp),cPath);
                if(strlen(cTmp))
                {
                    sscanf(cTmp,"%i",&iVar);
                    *(DWORD*) (0x8A5AA0+(0x4*(i-1))) = (DWORD) iVar;
                }
            }
            sprintf(cIni,"TaxiDriver %i",i);
            GetPrivateProfileString(s.cModels,cIni,NULL,cTmp,sizeof(cTmp),cPath);
            if(strlen(cTmp))
            {
                sscanf(cTmp,"%i",&iVar);
                *(DWORD*) (0x8A5AF4+(0x4*(i-1))) = (DWORD) iVar;
            }
        }
        GetPrivateProfileString(s.cModels,"Enforcer Replacement",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%i",&iVar);
            cBytes = (BYTE*)&iVar;
            BYTE bpush[] = { 0x68, cBytes[0], cBytes[1], cBytes[2], cBytes[3] };
            Patch(0x40B722,bpush);
            Patch(0x40B73D,bpush);
        }
        GetPrivateProfileString(s.cModels,"SWAT Ped Replacement",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%i",&iVar);
            cBytes = (BYTE*)&iVar;
            BYTE bpush[] = { 0x68, cBytes[0], cBytes[1], cBytes[2], cBytes[3] };
            Patch(0x40B733,bpush);
            Patch(0x40B753,bpush);
            Patch(0x5DDD8F,bpush);
        }
        GetPrivateProfileString(s.cModels,"FBI Rancher Replacement",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%i",&iVar);
            cBytes = (BYTE*)&iVar;
            BYTE bpush[] = { 0x68, cBytes[0], cBytes[1], cBytes[2], cBytes[3] };
            Patch(0x40B777,bpush);
            Patch(0x40B792,bpush);
        }
        GetPrivateProfileString(s.cModels,"FBI Ped Replacement",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%i",&iVar);
            cBytes = (BYTE*)&iVar;
            BYTE bpush[] = { 0x68, cBytes[0], cBytes[1], cBytes[2], cBytes[3] };
            Patch(0x40B783,bpush);
            Patch(0x40B7A8,bpush);
            Patch(0x5DDDCF,bpush);
        }
        GetPrivateProfileString(s.cModels,"Rhino Replacement",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%i",&iVar);
            cBytes = (BYTE*)&iVar;
            BYTE bpush[] = { 0x68, cBytes[0], cBytes[1], cBytes[2], cBytes[3] };
            Patch(0x40B7CC,bpush);
            Patch(0x40B7FD,bpush);
        }
        GetPrivateProfileString(s.cModels,"Barracks Replacement",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%i",&iVar);
            cBytes = (BYTE*)&iVar;
            BYTE bpush[] = { 0x68, cBytes[0], cBytes[1], cBytes[2], cBytes[3] };
            Patch(0x40B7D8,bpush);
            Patch(0x40B7F3,bpush);
        }
        GetPrivateProfileString(s.cModels,"Army Ped Replacement",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%i",&iVar);
            cBytes = (BYTE*)&iVar;
            BYTE bpush[] = { 0x68, cBytes[0], cBytes[1], cBytes[2], cBytes[3] };
            Patch(0x40B7E4,bpush);
            Patch(0x40B81C,bpush);
            Patch(0x5DDE0F,bpush);
        }
        GetPrivateProfileString(s.cModels,"Police Maverick Replacement",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%i",&iVar);
            cBytes = (BYTE*)&iVar;
            BYTE bpush[] = { 0x68, cBytes[0], cBytes[1], cBytes[2], cBytes[3] };
            Patch(0x40B840,bpush);
            Patch(0x40B890,bpush);
        }
        GetPrivateProfileString(s.cModels,"SAN Maverick Replacement",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%i",&iVar);
            cBytes = (BYTE*)&iVar;
            BYTE bpush[] = { 0x68, cBytes[0], cBytes[1], cBytes[2], cBytes[3] };
            Patch(0x40B877,bpush);
            Patch(0x40B886,bpush);
        }
        GetPrivateProfileString(s.cModels,"Police Weapon 1",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%i",&iVar);
            BYTE bpush[] = { 0x6A, (BYTE) iVar };
            Patch(0x5DDCCC,bpush);
        }
        GetPrivateProfileString(s.cModels,"Police Weapon 2",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%i",&iVar);
            BYTE bpush[] = { 0x6A, (BYTE) iVar };
            Patch(0x5DDCBE,bpush);
        }
        GetPrivateProfileString(s.cModels,"SWAT Weapon",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%i",&iVar);
            BYTE bpush[] = { 0x6A, (BYTE) iVar };
            Patch(0x5DDDA0,bpush);
            Patch(0x5DDDA9,bpush);
        }
        GetPrivateProfileString(s.cModels,"FBI Weapon",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%i",&iVar);
            BYTE bpush[] = { 0x6A, (BYTE) iVar };
            Patch(0x5DDDE0,bpush);
            Patch(0x5DDDE9,bpush);
        }
        GetPrivateProfileString(s.cModels,"Army Weapon",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%i",&iVar);
            BYTE bpush[] = { 0x6A, (BYTE) iVar };
            Patch(0x5DDE20,bpush);
            Patch(0x5DDE20,bpush);
        }
        // Sirens
        GetPrivateProfileString(s.cModels,"Siren 1",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%f",&fVar);
            cBytes = (BYTE*)&fVar;
            BYTE bColour[] = { cBytes[3], cBytes[2], cBytes[1], cBytes[0] };
            Patch(0x6AB3AA,bColour);
            Patch(0x6AB414,bColour);
        }
        GetPrivateProfileString(s.cModels,"Siren 2",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%f",&fVar);
            cBytes = (BYTE*)&fVar;
            BYTE bColour[] = { cBytes[3], cBytes[2], cBytes[1], cBytes[0] };
            Patch(0x6AB3B6,bColour);
            Patch(0x6AB3E6,bColour);
        }
        GetPrivateProfileString(s.cModels,"Siren 3",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%x",&dwVar);
            cBytes = (BYTE*)&dwVar;
            BYTE bColour[] = { cBytes[3], cBytes[2], cBytes[1], cBytes[0] };
            Patch(0x6AB3C6,bColour);
            Patch(0x6AB3F6,bColour);
            Patch(0x6AB500,bColour);
            Patch(0x6AB530,bColour);
            Patch(0x6ABBD5,bColour);
        }
        GetPrivateProfileString(s.cModels,"Siren 4",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%f",&fVar);
            cBytes = (BYTE*)&fVar;
            BYTE bColour[] = { cBytes[3], cBytes[2], cBytes[1], cBytes[0] };
            Patch(0x6AB3D6,bColour);
            Patch(0x6AB440,bColour);
        }
        GetPrivateProfileString(s.cModels,"Siren 5",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%f",&fVar);
            cBytes = (BYTE*)&fVar;
            BYTE bColour[] = { cBytes[3], cBytes[2], cBytes[1], cBytes[0] };
            Patch(0x6AB420,bColour);
            Patch(0x6AB450,bColour);
            Patch(0x6ABA95,bColour);
        }
        GetPrivateProfileString(s.cModels,"Siren 6",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%f",&fVar);
            cBytes = (BYTE*)&fVar;
            BYTE bColour[] = { cBytes[3], cBytes[2], cBytes[1], cBytes[0] };
            Patch(0x6AB430,bColour);
            Patch(0x6AB460,bColour);
        }
        GetPrivateProfileString(s.cModels,"Siren 7",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%f",&fVar);
            cBytes = (BYTE*)&fVar;
            BYTE bColour[] = { cBytes[3], cBytes[2], cBytes[1], cBytes[0] };
            Patch(0x6AB47E,bColour);
        }
        GetPrivateProfileString(s.cModels,"Siren 8",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%f",&fVar);
            cBytes = (BYTE*)&fVar;
            BYTE bColour[] = { cBytes[3], cBytes[2], cBytes[1], cBytes[0] };
            Patch(0x6AB48A,bColour);
            Patch(0x6AB4BA,bColour);
        }
        GetPrivateProfileString(s.cModels,"Siren 9",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%f",&fVar);
            cBytes = (BYTE*)&fVar;
            BYTE bColour[] = { cBytes[3], cBytes[2], cBytes[1], cBytes[0] };
            Patch(0x6AB49A,bColour);
            Patch(0x6AB4CA,bColour);
        }
        GetPrivateProfileString(s.cModels,"Siren 10",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%f",&fVar);
            cBytes = (BYTE*)&fVar;
            BYTE bColour[] = { cBytes[3], cBytes[2], cBytes[1], cBytes[0] };
            Patch(0x6AB4AA,bColour);
        }
        GetPrivateProfileString(s.cModels,"Siren 11",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%f",&fVar);
            cBytes = (BYTE*)&fVar;
            BYTE bColour[] = { cBytes[3], cBytes[2], cBytes[1], cBytes[0] };
            Patch(0x6AB4E8,bColour);
        }
        GetPrivateProfileString(s.cModels,"Siren 12",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%f",&fVar);
            cBytes = (BYTE*)&fVar;
            BYTE bColour[] = { cBytes[3], cBytes[2], cBytes[1], cBytes[0] };
            Patch(0x6AB4F4,bColour);
            Patch(0x6AB524,bColour);
            Patch(0x6AB552,bColour);
        }
        GetPrivateProfileString(s.cModels,"Siren 13",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%f",&fVar);
            cBytes = (BYTE*)&fVar;
            BYTE bColour[] = { cBytes[3], cBytes[2], cBytes[1], cBytes[0] };
            Patch(0x6AB514,bColour);
        }
        GetPrivateProfileString(s.cModels,"Siren 14",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%f",&fVar);
            cBytes = (BYTE*)&fVar;
            BYTE bColour[] = { cBytes[3], cBytes[2], cBytes[1], cBytes[0] };
            Patch(0x6AB55E,bColour);
            Patch(0x6AB58E,bColour);
        }
        GetPrivateProfileString(s.cModels,"Siren 15",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%f",&fVar);
            cBytes = (BYTE*)&fVar;
            BYTE bColour[] = { cBytes[3], cBytes[2], cBytes[1], cBytes[0] };
            Patch(0x6AB56A,bColour);
            Patch(0x6AB59A,bColour);
        }
        GetPrivateProfileString(s.cModels,"Siren 16",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%f",&fVar);
            cBytes = (BYTE*)&fVar;
            BYTE bColour[] = { cBytes[3], cBytes[2], cBytes[1], cBytes[0] };
            Patch(0x6AB582,bColour);
        }
        GetPrivateProfileString(s.cModels,"Siren 17",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%f",&fVar);
            cBytes = (BYTE*)&fVar;
            BYTE bColour[] = { cBytes[3], cBytes[2], cBytes[1], cBytes[0] };
            Patch(0x6ABA9D,bColour);
        }
        GetPrivateProfileString(s.cModels,"Siren 18",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%f",&fVar);
            cBytes = (BYTE*)&fVar;
            BYTE bColour[] = { cBytes[3], cBytes[2], cBytes[1], cBytes[0] };
            Patch(0x6ABABC,bColour);
        }
        GetPrivateProfileString(s.cModels,"Siren 19",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%f",&fVar);
            cBytes = (BYTE*)&fVar;
            BYTE bColour[] = { cBytes[3], cBytes[2], cBytes[1], cBytes[0] };
            Patch(0x6ABBD5,bColour);
        }
        GetPrivateProfileString(s.cModels,"Siren 20",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%f",&fVar);
            cBytes = (BYTE*)&fVar;
            BYTE bColour[] = { cBytes[3], cBytes[2], cBytes[1], cBytes[0] };
            Patch(0x6ABBDD,bColour);
        }
        // Graphics
        // Ped Dynamic Shadows
        GetPrivateProfileString(s.cGraphics,"Ped Dynamic Shadows",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%i",&iVar);
            if(!iVar) Nop(0x5E6766,2);
        }
        // Map Dynamic Shadows
        GetPrivateProfileString(s.cGraphics,"Map Dynamic Shadows",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%i",&iVar);
            if(!iVar)
            {
                BYTE bjmp[] = { 0xEB };
                Patch(0x7113C0,bjmp);
            }
        }
        // License Plate Texture Filtering
        GetPrivateProfileString(s.cGraphics,"License Plate Texture Filtering",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%i",&iVar);
            BYTE bPatch[] = { (BYTE) iVar };
            Patch(0x884958,bPatch);
        }
        // Texture Filter 1
        GetPrivateProfileString(s.cGraphics,"Texture Filter 1",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%i",&iVar);
            BYTE bPatch[] = { (BYTE) iVar };
            Patch(0x884960,bPatch);
        }
        // Texture Filter Min
        GetPrivateProfileString(s.cGraphics,"Texture Filter Min",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%i",&iVar);
            BYTE bPatch[] = { (BYTE) iVar };
            Patch(0x884988,bPatch);
        }
        // Texture Filter Max
        GetPrivateProfileString(s.cGraphics,"Texture Filter Max",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%i",&iVar);
            BYTE bPatch[] = { (BYTE) iVar };
            Patch(0x88498C,bPatch);
        }
        // Shadow Wrapper 1
        GetPrivateProfileString(s.cGraphics,"Shadow Wrapper 1",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%i",&iVar);
            BYTE bPatch[] = { (BYTE) iVar };
            Patch(0x884948,bPatch);
        }
        // Shadow Wrapper 2
        GetPrivateProfileString(s.cGraphics,"Shadow Wrapper 2",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%i",&iVar);
            BYTE bPatch[] = { (BYTE) iVar };
            Patch(0x884934,bPatch);
        }
        // Diffuse Texture Wrap
        GetPrivateProfileString(s.cGraphics,"Diffuse Texture Wrap",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%i",&iVar);
            BYTE bPatch[] = { (BYTE) iVar };
            Patch(0x884940,bPatch);
        }
        // Alpha Blend
        GetPrivateProfileString(s.cGraphics,"Alpha Blend",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%i",&iVar);
            BYTE bPatch[] = { (BYTE) iVar };
            Patch(0x884924,bPatch);
        }
        // Object/Vehicle Shadow Cull
        GetPrivateProfileString(s.cGraphics,"Object/Vehicle Shadow Cull",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%i",&iVar);
            BYTE bPatch[] = { (BYTE) iVar };
            Patch(0x8849D4,bPatch);
        }
        // Character Shadow Max Size
        GetPrivateProfileString(s.cGraphics,"Character Shadow Max Size",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%i",&iVar);
            BYTE bPatch[] = { (BYTE) iVar };
            Patch(0x8D5218,bPatch);
        }
        // Character Shadow Min Size
        GetPrivateProfileString(s.cGraphics,"Character Shadow Min Size",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%i",&iVar);
            BYTE bPatch[] = { (BYTE) iVar };
            Patch(0x8D521C,bPatch);
        }
        // Vehicle Vertex Alpha
        GetPrivateProfileString(s.cGraphics,"Vehicle Vertex Alpha",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%i",&iVar);
            BYTE bPatch[] = { 0x6A, (BYTE) iVar };
            Patch(0x5D9E21,bPatch);
        }
        // Vehicle Vertex Alpha
        GetPrivateProfileString(s.cGraphics,"Vehicle Vertex Alpha",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%i",&iVar);
            BYTE bPatch[] = { 0x6A, (BYTE) iVar };
            Patch(0x5D9E21,bPatch);
        }
        // Vehicle UV Ref Flags
        GetPrivateProfileString(s.cGraphics,"Vehicle UV Ref Flags",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%i",&iVar);
            BYTE bPatch[] = { 0x68, (BYTE) iVar, 0x00, 0x01, 0x00 };
            Patch(0x5D9C82,bPatch);
        }
        // Vehicle TCI
        GetPrivateProfileString(s.cGraphics,"Vehicle TCI",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%i",&iVar);
            BYTE bPatch[] = { 0x6A, (BYTE) iVar };
            Patch(0x5D9D28,bPatch);
        }
        // Vehicle TCI
        GetPrivateProfileString(s.cGraphics,"Vehicle TCI",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%i",&iVar);
            BYTE bPatch[] = { 0x6A, (BYTE) iVar };
            Patch(0x5D9D28,bPatch);
        }
        // Vehicle Texture Trans Flags
        GetPrivateProfileString(s.cGraphics,"Vehicle Texture Trans Flags",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%i",&iVar);
            cBytes = (BYTE*)&iVar;
            BYTE bPatch[] = { 0x68, cBytes[0], cBytes[1], cBytes[2], cBytes[3] };
            Patch(0x5D9C90,bPatch);
        }
        // Vehicle Lighting
        GetPrivateProfileString(s.cGraphics,"Vehicle Lighting",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%f",&fVar);
            cBytes = (BYTE*)&fVar;
            BYTE bPatch[] = { 0xC7, 0x44, 0x24, 0x14, cBytes[0], cBytes[1], cBytes[2], cBytes[3] };
            Patch(0x6FFD41,bPatch);
        }
        // Stencil Shaded Opacity
        GetPrivateProfileString(s.cGraphics,"Stencil Shaded Opacity",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%i",&iVar);
            BYTE bPatch[] = { 0x6A, (BYTE) iVar };
            Patch(0x7115B4,bPatch);
        }
        // Performance
        // Cloud Height
        GetPrivateProfileString(s.cPerformance,"Cloud Height",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%f",&fVar);
            Patch_Float(0x716642,fVar);
            Patch_Float(0x716655,fVar);
        }
        // Zone Name Text
        GetPrivateProfileString(s.cPerformance,"Zone Text",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%i",&iVar);
            if(iVar == 0) Nop(0x58AA6F,2);
        }
        // Vertgio
        GetPrivateProfileString(s.cPerformance,"Vertigo",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%i",&iVar);
            if(iVar == 0)
            {
                BYTE bjmp[] = { 0xEB };
                Patch(0x524B3E,bjmp);
            }
        }
        // Speed Limiter
        GetPrivateProfileString(s.cPerformance,"Speed Limiter",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%i",&iVar);
            if(iVar == 0) Nop(0x72DF08,5);
        }
        // Heat Haze
        GetPrivateProfileString(s.cPerformance,"Heat Haze",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%i",&iVar);
            if(iVar == 0)
            {
                BYTE bjmp[] = { 0xEB };
                Patch(0x72C1B7,bjmp);
            }
        }
        // AI
        // Max AI Vehicles
        GetPrivateProfileString(s.cAI,"Max AI Vehicles",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%i",&iVar);
            BYTE bcmpMax[] = { 0x83, 0xF8, (BYTE) iVar };
            Patch(0x434222,bcmpMax);
        }
        // Max AI Peds
        GetPrivateProfileString(s.cAI,"Max AI Peds",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%i",&iVar);
            *(DWORD*) 0x8D2538 = (DWORD) iVar;
        }
        // Misc
        // Enable Ads == 
        GetPrivateProfileString(s.cMisc,"Enable Ads",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%i",&iVar);
            if(!iVar)
            {
                *(PBYTE)0xC8D4C0 = 5;
                Nop(0x747483,6); // Disable Ads
            }
        }
        // Window Name
        GetPrivateProfileString(s.cMisc,"Window Name",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp)) SetWindowText(FindWindow(NULL,"GTA: San Andreas"),cTmp);
        // Windowed
        GetPrivateProfileString(s.cMisc,"Windowed",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
        {
            sscanf(cTmp,"%i",&iVar);
            //if(iVar == 1) bWindowed = true;
        }
        // Flight Height
        BYTE bcmpFlight[] = { 0xC2, 0x08, 0x00 };
        Patch(0x6D2600,bcmpFlight);
        // Unlimited Stream
        //Nop(0x40E144,15);
        //BYTE bStream[] = { 0xEB, 0xE1 };
        //Patch(0x40E153,bStream);
        
        
		// Dynamic Limits
		GetPrivateProfileString(s.cDynamicLimits,"Buildings",NULL,cTmp,sizeof(cTmp),cPath);
		if(strlen(cTmp))
			SALA->SetBuildings(atoi(cTmp));
		GetPrivateProfileString(s.cDynamicLimits,"Dummys",NULL,cTmp,sizeof(cTmp),cPath);
		if(strlen(cTmp))
			SALA->SetDummys(atoi(cTmp));
		GetPrivateProfileString(s.cDynamicLimits,"PtrNode Doubles",NULL,cTmp,sizeof(cTmp),cPath);
		if(strlen(cTmp))
			SALA->SetPtrNodeDoubles(atoi(cTmp));
		GetPrivateProfileString(s.cDynamicLimits,"IPL Files",NULL,cTmp,sizeof(cTmp),cPath);
		if(strlen(cTmp))
			SALA->SetIPLFiles(atoi(cTmp));
		GetPrivateProfileString(s.cDynamicLimits,"Stunt Jumps",NULL,cTmp,sizeof(cTmp),cPath);
		if(strlen(cTmp))
			SALA->SetStuntJumps(atoi(cTmp));
		GetPrivateProfileString(s.cDynamicLimits,"ColModels",NULL,cTmp,sizeof(cTmp),cPath);
		if(strlen(cTmp))
			SALA->SetColModels(atoi(cTmp));
		GetPrivateProfileString(s.cDynamicLimits,"Collision Files",NULL,cTmp,sizeof(cTmp),cPath);
		if(strlen(cTmp))
			SALA->SetCollisionFiles(atoi(cTmp));
		GetPrivateProfileString(s.cDynamicLimits,"QuadTreeNodes",NULL,cTmp,sizeof(cTmp),cPath);
		if(strlen(cTmp))
			SALA->SetQuadTreeNodes(atoi(cTmp));
		GetPrivateProfileString(s.cDynamicLimits,"QuadTreeNodes2",NULL,cTmp,sizeof(cTmp),cPath);
		if(strlen(cTmp))
			SALA->SetQuadTreeNodes2(atoi(cTmp));
		GetPrivateProfileString(s.cDynamicLimits,"MatDataPool",NULL,cTmp,sizeof(cTmp),cPath);
		if(strlen(cTmp))
			SALA->SetMatDataPool(atoi(cTmp));
		GetPrivateProfileString(s.cDynamicLimits,"AtmDataPool",NULL,cTmp,sizeof(cTmp),cPath);
		if(strlen(cTmp))
			SALA->SetAtmDataPool(atoi(cTmp));
		GetPrivateProfileString(s.cDynamicLimits,"Vehicles",NULL,cTmp,sizeof(cTmp),cPath);
		if(strlen(cTmp))
			SALA->SetVehicles(atoi(cTmp));
		GetPrivateProfileString(s.cDynamicLimits,"Vehicle Structs",NULL,cTmp,sizeof(cTmp),cPath);
		if(strlen(cTmp))
			SALA->SetVehicleStructs(atoi(cTmp));
		GetPrivateProfileString(s.cDynamicLimits,"Peds",NULL,cTmp,sizeof(cTmp),cPath);
		if(strlen(cTmp))
			SALA->SetPeds(atoi(cTmp));
		GetPrivateProfileString(s.cDynamicLimits,"Polygons",NULL,cTmp,sizeof(cTmp),cPath);
		if(strlen(cTmp))
			SALA->SetPolygons(atoi(cTmp));
		GetPrivateProfileString(s.cDynamicLimits,"PtrNode Singles",NULL,cTmp,sizeof(cTmp),cPath);
		if(strlen(cTmp))
			SALA->SetPtrNodeSingles(atoi(cTmp));
		GetPrivateProfileString(s.cDynamicLimits,"EntryInfoNodes",NULL,cTmp,sizeof(cTmp),cPath);
		if(strlen(cTmp))
			SALA->SetEntryInfoNodes(atoi(cTmp));
		GetPrivateProfileString(s.cDynamicLimits,"Objects",NULL,cTmp,sizeof(cTmp),cPath);
		if(strlen(cTmp))
			SALA->SetObjects(atoi(cTmp));
		GetPrivateProfileString(s.cDynamicLimits,"Tasks",NULL,cTmp,sizeof(cTmp),cPath);
		if(strlen(cTmp))
			SALA->SetTasks(atoi(cTmp));
		GetPrivateProfileString(s.cDynamicLimits,"Events",NULL,cTmp,sizeof(cTmp),cPath);
		if(strlen(cTmp))
			SALA->SetEvents(atoi(cTmp));
		GetPrivateProfileString(s.cDynamicLimits,"Point Routes",NULL,cTmp,sizeof(cTmp),cPath);
		if(strlen(cTmp))
			SALA->SetPointRoutes(atoi(cTmp));
		GetPrivateProfileString(s.cDynamicLimits,"Patrol Routes",NULL,cTmp,sizeof(cTmp),cPath);
		if(strlen(cTmp))
			SALA->SetPatrolRoutes(atoi(cTmp));
		GetPrivateProfileString(s.cDynamicLimits,"Node Routes",NULL,cTmp,sizeof(cTmp),cPath);
		if(strlen(cTmp))
			SALA->SetNodeRoutes(atoi(cTmp));
		GetPrivateProfileString(s.cDynamicLimits,"Task Allocators",NULL,cTmp,sizeof(cTmp),cPath);
		if(strlen(cTmp))
			SALA->SetTaskAllocators(atoi(cTmp));
		GetPrivateProfileString(s.cDynamicLimits,"Ped Attractors",NULL,cTmp,sizeof(cTmp),cPath);
		if(strlen(cTmp))
			SALA->SetPedAttractors(atoi(cTmp));
		GetPrivateProfileString(s.cDynamicLimits,"Entry Exits",NULL,cTmp,sizeof(cTmp),cPath);
		if(strlen(cTmp))
			SALA->SetEntryExits(atoi(cTmp));
		GetPrivateProfileString(s.cDynamicLimits,"Streaming Memory",NULL,cTmp,sizeof(cTmp),cPath);
		if(strlen(cTmp))
			SALA->SetStreamingMemory(atoi(cTmp));
		GetPrivateProfileString(s.cDynamicLimits,"Streaming Vehicles",NULL,cTmp,sizeof(cTmp),cPath);
		if(strlen(cTmp))
			SALA->SetStreamingVehicles(atoi(cTmp));
		GetPrivateProfileString(s.cDynamicLimits,"IMG Headers",NULL,cTmp,sizeof(cTmp),cPath);
		if(strlen(cTmp))
			SALA->SetImgHeaders(atoi(cTmp));
		// Static Limits
        GetPrivateProfileString(s.cStaticLimits,"Water Planes",NULL,cTmp,sizeof(cTmp),cPath);
        if(strlen(cTmp))
            SALA->SetWaterPlanes(atoi(cTmp));
		GetPrivateProfileString(s.cStaticLimits,"Current Mission",NULL,cTmp,sizeof(cTmp),cPath);
		if(strlen(cTmp))
			SALA->SetCurrentMission(atoi(cTmp));
		GetPrivateProfileString(s.cStaticLimits,"Timed Objects",NULL,cTmp,sizeof(cTmp),cPath);
		if(strlen(cTmp))
			SALA->SetTimedObjects(atoi(cTmp));
		GetPrivateProfileString(s.cStaticLimits,"Vehicle Models",NULL,cTmp,sizeof(cTmp),cPath);
		if(strlen(cTmp))
			SALA->SetVehicleModels(atoi(cTmp));
		GetPrivateProfileString(s.cStaticLimits,"Pickups",NULL,cTmp,sizeof(cTmp),cPath);
		if(strlen(cTmp))
			SALA->SetPickups(atoi(cTmp));
		GetPrivateProfileString(s.cStaticLimits,"Markers",NULL,cTmp,sizeof(cTmp),cPath);
		if(strlen(cTmp))
			SALA->SetMarkers(atoi(cTmp));
		GetPrivateProfileString(s.cStaticLimits,"Garages",NULL,cTmp,sizeof(cTmp),cPath);
		if(strlen(cTmp))
			SALA->SetGarages(atoi(cTmp));
		GetPrivateProfileString(s.cStaticLimits,"CarCols",NULL,cTmp,sizeof(cTmp),cPath);
		if(strlen(cTmp))
			SALA->SetCarCols(atoi(cTmp));
		GetPrivateProfileString(s.cStaticLimits,"IDEs",NULL,cTmp,sizeof(cTmp),cPath);
		if(strlen(cTmp))
			SALA->SetIDEs(atoi(cTmp));
		GetPrivateProfileString(s.cStaticLimits,"Weapon IDs",NULL,cTmp,sizeof(cTmp),cPath);
		if(strlen(cTmp))
			SALA->SetWeapons(atoi(cTmp));
		GetPrivateProfileString(s.cStaticLimits,"CarMods",NULL,cTmp,sizeof(cTmp),cPath);
		if(strlen(cTmp))
			SALA->SetCarMods(atoi(cTmp));
		GetPrivateProfileString(s.cStaticLimits,"SCM Block",NULL,cTmp,sizeof(cTmp),cPath);
		if(strlen(cTmp))
			SALA->SetSCMBlock(atoi(cTmp));
		GetPrivateProfileString(s.cStaticLimits,"SCM Locals",NULL,cTmp,sizeof(cTmp),cPath);
		if(strlen(cTmp))
			SALA->SetSCMLocals(atoi(cTmp));
		GetPrivateProfileString(s.cStaticLimits,"SCM Threads",NULL,cTmp,sizeof(cTmp),cPath);
		if(strlen(cTmp))
			SALA->SetSCMThreads(atoi(cTmp));
		GetPrivateProfileString(s.cStaticLimits,"Map Bounds",NULL,cTmp,sizeof(cTmp),cPath);
		if(strlen(cTmp))
			SALA->SetBounds((float)atof(cTmp));
		GetPrivateProfileString(s.cStaticLimits,"Aiming Offsets",NULL,cTmp,sizeof(cTmp),cPath);
		if(strlen(cTmp))
			SALA->SetAimingOffsets(atoi(cTmp));
		GetPrivateProfileString(s.cStaticLimits,"Weapon Objects",NULL,cTmp,sizeof(cTmp),cPath);
		if(strlen(cTmp))
			SALA->SetWeaponObjects(atoi(cTmp));
		GetPrivateProfileString(s.cStaticLimits,"Players",NULL,cTmp,sizeof(cTmp),cPath);
		if(strlen(cTmp))
			SALA->SetPlayers(atoi(cTmp));
		GetPrivateProfileString(s.cStaticLimits,"Ped Models",NULL,cTmp,sizeof(cTmp),cPath);
		if(strlen(cTmp))
			SALA->SetPedModels(atoi(cTmp));
		GetPrivateProfileString(s.cStaticLimits,"IPLs",NULL,cTmp,sizeof(cTmp),cPath);
		if(strlen(cTmp))
			SALA->SetIPLs(atoi(cTmp));
		GetPrivateProfileString(s.cStaticLimits,"LOD Distance",NULL,cTmp,sizeof(cTmp),cPath);
		if(strlen(cTmp))
			SALA->SetLOD((float)atof(cTmp));
	}
}
