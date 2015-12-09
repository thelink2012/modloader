#pragma once
#include <windows.h>
#include "Queue.h"

#pragma pack(push, 1)
struct CdStream	// sizeof = 0x30
{
	DWORD nSectorOffset;
	DWORD nSectorsToRead;
	LPVOID lpBuffer;
	BYTE field_C;
	BYTE bLocked;
	BYTE bInUse;
	BYTE field_F;
	DWORD status;
	HANDLE semaphore;
	HANDLE hFile;
	OVERLAPPED overlapped;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct CdStreamInfoSA	// sizeof = 0x8C0
{
	Queue queue;
	CdStream* pStreams;
	DWORD thread_id;
	HANDLE semaphore;
	HANDLE thread;
	HANDLE streamHandles[32];
	DWORD streamCount;
	DWORD openStreamCount;
	CHAR  streamNames[2048];
	DWORD field_8A8;
	DWORD lastPosn;
	DWORD field_8B0;
	DWORD field_8B4;
	DWORD gtaint_id;
	DWORD gta3_id;
};
#pragma pack(pop)

static_assert(sizeof(CdStreamInfoSA) == 0x8C0, "Incorrect struct size: CdStreamInfoSA");
static_assert(sizeof(CdStream) == 0x30, "Incorrect struct size: CdStream");

