#pragma once
#include <cstdint>

#pragma pack(push, 1)
struct DirectoryInfo	// sizeof = 0x20
{
	uint32_t m_dwFileOffset;
	uint16_t m_usSize;              // on III/VC this is only a single
	uint16_t m_usCompressedSize__;  // uint32_t m_dwSize
	char m_szFileName[24];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct CDirectorySA     // sizeof = 0x10
{
    DirectoryInfo* m_pEntries;
    uint32_t m_dwSize;
    uint32_t m_dwCount;
    uint8_t m_bDynamicAllocated;
    uint8_t __pad[3];
};
#pragma pack(pop)

struct CDirectory;

static_assert(sizeof(CDirectorySA) == 0x10, "Incorrect struct size: CDirectorySA");
//static_assert(sizeof(CDirectoryVC) == 0x0C, "Incorrect struct size: CDirectoryVC");
static_assert(sizeof(DirectoryInfo) == 0x20, "Incorrect struct size: CDirectory::DirectoryInfo");
