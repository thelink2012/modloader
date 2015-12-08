#pragma once
#include <cstdint>

#pragma pack(push, 1)
struct DirectoryInfo	// sizeof = 0x20
{
	uint32_t m_dwFileOffset;
	uint16_t m_usSize;
	uint16_t m_usLightSize;
	char m_szFileName[24];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct CDirectory	// sizeof = 0x10
{
	DirectoryInfo* m_pEntries;
	uint32_t m_dwSize;
	uint32_t m_dwCount;
	uint8_t m_bDynamicAllocated;
	uint8_t __pad[3];

    CDirectory() = delete;
    CDirectory(const CDirectory&) = delete;
    CDirectory& operator=(const CDirectory&) = delete;
};
#pragma pack(pop)

static_assert(sizeof(CDirectory) == 0x10, "Incorrect struct size: CDirectory");
static_assert(sizeof(DirectoryInfo) == 0x20, "Incorrect struct size: CDirectory::DirectoryInfo");
