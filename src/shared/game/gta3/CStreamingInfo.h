#pragma once
#include <windows.h>
#include <modloader/util/injector.hpp>

// NOTE: ucFlags are different between game versions, don't set them like nothing.

#pragma pack(push, 1)
struct CStreamingInfoVC	// sizeof = 0x14
{
    CStreamingInfoVC* pNext;
    CStreamingInfoVC* pPrev;
    unsigned char  ucLoadState;
    unsigned char  ucFlags;
    unsigned short usNextOnCd;
    unsigned int   uiBlockOffset;
    unsigned int   uiBlockCount;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct CStreamingInfoSA	// sizeof = 0x14
{
    unsigned short usNext;
    unsigned short usPrev;
    unsigned short usNextOnCd;
    unsigned char  ucFlags;
    unsigned char  ucImgId;
    unsigned int   uiBlockOffset;
    unsigned int   uiBlockCount;
    unsigned char  ucLoadState;
    unsigned char  _pad[3];
};
#pragma pack(pop)

struct CStreamingInfo
{
    // Opaque pointer, cannot be constructed.
    CStreamingInfo() = delete;
    CStreamingInfo(const CStreamingInfo&) = delete;
    CStreamingInfo& operator=(const CStreamingInfo&) = delete;

    static size_t GetSizeof()
    {
        switch(modloader::gvm.GetGame())
        {
            case '3': break;
            case 'V': return sizeof(CStreamingInfoVC);
            case 'S': return sizeof(CStreamingInfoSA);
        }
        *(int*)(0xFF00F1D0) = 0; // unreachable code
        return 0;
    }

    void SetStreamData(uint32_t offset, uint32_t blocks)
    {
        switch(modloader::gvm.GetGame())
        {
            case '3': break;
            case 'V': this->AsVC().uiBlockOffset = offset; this->AsVC().uiBlockCount = blocks; return;
            case 'S': this->AsSA().uiBlockOffset = offset; this->AsSA().uiBlockCount = blocks; return;
        }
        *(int*)(0xFF00F1D0) = 0; // unreachable code
    }

    void SetNextOnCd(uint32_t nextOnCd)
    {
        switch(modloader::gvm.GetGame())
        {
            case '3': break;
            case 'V': this->AsVC().usNextOnCd = nextOnCd; return;
            case 'S': this->AsSA().usNextOnCd = nextOnCd; return;
        }
        *(int*)(0xFF00F1D0) = 0; // unreachable code
    }

    void ClearStreamFlags()
    {
        switch(modloader::gvm.GetGame())
        {
            case '3': break;
            case 'V': this->AsVC().ucFlags = 0; return;
            case 'S': this->AsSA().ucFlags = 0; return;
        }
        *(int*)(0xFF00F1D0) = 0; // unreachable code
    }

    uint8_t GetStreamFlags()
    {
        switch(modloader::gvm.GetGame())
        {
            case '3': break;
            case 'V': return this->AsVC().ucFlags;
            case 'S': return this->AsSA().ucFlags;
        }
        *(int*)(0xFF00F1D0) = 0; // unreachable code
        return 0;
    }

    uint8_t GetLoadStatus()
    {
        switch(modloader::gvm.GetGame())
        {
            case '3': break;
            case 'V': return this->AsVC().ucLoadState;
            case 'S': return this->AsSA().ucLoadState;
        }
        *(int*)(0xFF00F1D0) = 0; // unreachable code
        return 0;
    }

    uint32_t GetNextOnCd()
    {
        switch(modloader::gvm.GetGame())
        {
            case '3': break;
            case 'V': return this->AsVC().usNextOnCd;
            case 'S': return this->AsSA().usNextOnCd;
        }
        *(int*)(0xFF00F1D0) = 0; // unreachable code
        return 0;
    }

    uint32_t GetOffset()
    {
        switch(modloader::gvm.GetGame())
        {
            case '3': break;
            case 'V': return this->AsVC().uiBlockOffset;
            case 'S': return this->AsSA().uiBlockOffset;
        }
        *(int*)(0xFF00F1D0) = 0; // unreachable code
        return 0;
    }

    uint8_t GetImgId()
    {
        // TODO ensure VC has no img_id thing
        switch(modloader::gvm.GetGame())
        {
            case '3': break;
            case 'V': return 0;
            case 'S': return this->AsSA().ucImgId;
        }
        *(int*)(0xFF00F1D0) = 0; // unreachable code
        return 0;
    }

public:
    CStreamingInfoVC& AsVC()
    {
        return *(CStreamingInfoVC*)(this);
    }

    CStreamingInfoSA& AsSA()
    {
        return *(CStreamingInfoSA*)(this);
    }
};

static_assert(sizeof(CStreamingInfoSA) == 0x14, "Incorrect struct size: CStreamingInfoSA");
static_assert(sizeof(CStreamingInfoVC) == 0x14, "Incorrect struct size: CStreamingInfoVC");
