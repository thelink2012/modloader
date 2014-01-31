#pragma once
#include <CDirectoryEntry.h>

#pragma pack(push, 1)
struct CDirectory	// sizeof = 0x10
{
	CDirectoryEntry* m_pEntries;
	uint32_t m_dwSize;
	uint32_t m_dwCount;
	uint8_t m_bDynamicAllocated;
	uint8_t __pad[3];
};
#pragma pack(pop)

static_assert(sizeof(CDirectory) == 0x10, "Incorrect struct size: CDirectory");
