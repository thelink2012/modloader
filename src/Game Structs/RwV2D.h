#pragma once
#include <windows.h>

#pragma pack(push, 1)
struct RwV2D	// sizeof = 0x8
{
	float x;
	float y;
};
#pragma pack(pop)

static_assert(sizeof(RwV2D) == 0x8, "Incorrect struct size: RwV2D");