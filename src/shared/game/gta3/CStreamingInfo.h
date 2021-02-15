#pragma once
#include <windows.h>
#include <modloader/modloader.hpp> // TODO agh stop including this here
#include <modloader/util/injector.hpp>
#include <f92la/f92la.h>

// NOTE: ucFlags are different between game versions, don't set them like nothing.

#pragma pack(push, 1)
struct CStreamingInfoIII // sizeof = 0x14
{
    CStreamingInfoIII* pNext;
    CStreamingInfoIII* pPrev;
    unsigned char  ucLoadState;
    unsigned char  ucFlags;
    unsigned short usNextOnCd;
    unsigned int   uiBlockOffset;
    unsigned int   uiBlockCount;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct CStreamingInfoVC // sizeof = 0x14
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
struct CStreamingInfoSA // sizeof = 0x14
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

#pragma pack(push, 1)
class CStreamingInfoSA_F92LA // sizeof=0x14
{
public:
    unsigned short unused_usNext;
    unsigned short unused_usPrev;
    unsigned short unused_usNextOnCd;
    unsigned char  ucFlags;
    unsigned char  ucImgId;
    unsigned int   uiBlockOffset;
    unsigned int   uiBlockCount;
    unsigned char  ucLoadFlag;
    unsigned char  _pad3[3];
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
        if(modloader::plugin_ptr->loader->game_id == MODLOADER_GAME_RE3)
            return sizeof(CStreamingInfoIII);
        switch(modloader::gvm.GetGame())
        {
            case '3': return sizeof(CStreamingInfoIII);
            case 'V': return sizeof(CStreamingInfoVC);
            case 'S': return sizeof(CStreamingInfoSA);
        }
        *(int*)(0xFF00F1D0) = 0; // unreachable code
        return 0;
    }

    void SetStreamData(uint32_t offset, uint32_t blocks)
    {
        if(modloader::plugin_ptr->loader->game_id == MODLOADER_GAME_RE3)
        {
            this->AsIII().uiBlockOffset = offset; this->AsIII().uiBlockCount = blocks; return;
        }
        switch(modloader::gvm.GetGame())
        {
            case '3': this->AsIII().uiBlockOffset = offset; this->AsIII().uiBlockCount = blocks; return;
            case 'V': this->AsVC().uiBlockOffset = offset; this->AsVC().uiBlockCount = blocks; return;
            case 'S': this->AsSA().uiBlockOffset = offset; this->AsSA().uiBlockCount = blocks; return;
        }
        *(int*)(0xFF00F1D0) = 0; // unreachable code
    }

    void SetNextOnCd(uint32_t nextOnCd)
    {
        if(auto f92la = this->GetF92LA())
        {
            auto i = this->AsF92LA();
            f92la->SetFileInfoNextOnCDfileID(this->AsF92LA(), nextOnCd);
            return;
        }
        else
        {
            if(modloader::plugin_ptr->loader->game_id == MODLOADER_GAME_RE3)
            {
                this->AsIII().usNextOnCd = nextOnCd; return;
            }
            switch(modloader::gvm.GetGame())
            {
                case '3': this->AsIII().usNextOnCd = nextOnCd; return;
                case 'V': this->AsVC().usNextOnCd = nextOnCd; return;
                case 'S': this->AsSA().usNextOnCd = nextOnCd; return;
            }
        }
        *(int*)(0xFF00F1D0) = 0; // unreachable code
    }

    void ClearStreamFlags()
    {
        if(modloader::plugin_ptr->loader->game_id == MODLOADER_GAME_RE3)
        {
            this->AsIII().ucFlags = 0; return;
        }
        switch(modloader::gvm.GetGame())
        {
            case '3': this->AsIII().ucFlags = 0; return;
            case 'V': this->AsVC().ucFlags = 0; return;
            case 'S': this->AsSA().ucFlags = 0; return;
        }
        *(int*)(0xFF00F1D0) = 0; // unreachable code
    }

    uint8_t GetStreamFlags()
    {
        if(modloader::plugin_ptr->loader->game_id == MODLOADER_GAME_RE3)
        {
            return this->AsIII().ucFlags;
        }
        switch(modloader::gvm.GetGame())
        {
            case '3': return this->AsIII().ucFlags;
            case 'V': return this->AsVC().ucFlags;
            case 'S': return this->AsSA().ucFlags;
        }
        *(int*)(0xFF00F1D0) = 0; // unreachable code
        return 0;
    }

    uint8_t GetLoadStatus()
    {
        if(modloader::plugin_ptr->loader->game_id == MODLOADER_GAME_RE3)
        {
            return this->AsIII().ucLoadState;
        }
        switch(modloader::gvm.GetGame())
        {
            case '3': return this->AsIII().ucLoadState;
            case 'V': return this->AsVC().ucLoadState;
            case 'S': return this->AsSA().ucLoadState;
        }
        *(int*)(0xFF00F1D0) = 0; // unreachable code
        return 0;
    }

    uint32_t GetNextOnCd()
    {
        if(auto f92la = this->GetF92LA())
        {
            return f92la->GetFileInfoNextOnCDfileID(this->AsF92LA());
        }
        else
        {
            if(modloader::plugin_ptr->loader->game_id == MODLOADER_GAME_RE3)
            {
                return this->AsIII().usNextOnCd;
            }
            switch(modloader::gvm.GetGame())
            {
                case '3': return this->AsIII().usNextOnCd;
                case 'V': return this->AsVC().usNextOnCd;
                case 'S': return this->AsSA().usNextOnCd;
            }
        }
        *(int*)(0xFF00F1D0) = 0; // unreachable code
        return 0;
    }

    uint32_t GetOffset()
    {
        if(modloader::plugin_ptr->loader->game_id == MODLOADER_GAME_RE3)
        {
            return this->AsIII().uiBlockOffset;
        }
        switch(modloader::gvm.GetGame())
        {
            case '3': return this->AsIII().uiBlockOffset;
            case 'V': return this->AsVC().uiBlockOffset;
            case 'S': return this->AsSA().uiBlockOffset;
        }
        *(int*)(0xFF00F1D0) = 0; // unreachable code
        return 0;
    }

    uint8_t GetImgId()
    {
        if(modloader::plugin_ptr->loader->game_id == MODLOADER_GAME_RE3)
        {
            return 0;
        }
        switch(modloader::gvm.GetGame())
        {
            case '3': return 0;
            case 'V': return 0;
            case 'S': return this->AsSA().ucImgId;
        }
        *(int*)(0xFF00F1D0) = 0; // unreachable code
        return 0;
    }

private:
    CStreamingInfoIII& AsIII()
    {
        return *(CStreamingInfoIII*)(this);
    }

    CStreamingInfoVC& AsVC()
    {
        return *(CStreamingInfoVC*)(this);
    }

    CStreamingInfoSA& AsSA()
    {
        return *(CStreamingInfoSA*)(this);
    }

    int32_t AsF92LA();
    static const LibF92LA* GetF92LA();
};

static_assert(sizeof(CStreamingInfoSA) == 0x14, "Incorrect struct size: CStreamingInfoSA");
static_assert(sizeof(CStreamingInfoVC) == 0x14, "Incorrect struct size: CStreamingInfoVC");
static_assert(sizeof(CStreamingInfoIII) == 0x14, "Incorrect struct size: CStreamingInfoIII");
