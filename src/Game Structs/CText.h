#pragma once
#include <windows.h>

#pragma pack(push, 1)
struct TablEntry	// sizeof = 0xC
{
	BYTE name[8];
	DWORD offset;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct CText__Tabl	// sizeof = 0x964
{
	TablEntry data[200];
	WORD size;
	WORD __pad;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct CText__TKey	// sizeof = 0x8
{
	DWORD data;
	DWORD size;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct CText__TDat	// sizeof = 0x8
{
	DWORD data;
	DWORD size;
};
#pragma pack(pop)


#pragma pack(push, 1)
struct CText	// sizeof = 0xA90
{
	CText__TKey tkeyMain;
	CText__TDat tdatMain;
	CText__TKey tkeyMission;
	CText__TDat tdatMission;
	BYTE field_20;
	BYTE haveTabl;
	BYTE cderrorInitialized;
	BYTE missionLoaded;
	BYTE missionName[8];
	BYTE cderrorText[256];
	CText__Tabl tabl;
};
#pragma pack(pop)

static_assert(sizeof(CText) == 0xA90, "Incorrect struct size: CText");
