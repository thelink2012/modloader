#pragma once

#pragma pack(push, 1)
struct CAEBankLookupItem
{
  char m_iPak;
  char _pad[3];
  unsigned int m_dwOffset;
  unsigned int m_dwSize;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct CAEBankSlotItem
{
  unsigned int m_dwOffset;
  unsigned int m_dwLoopOffset;
  unsigned short m_usSampleRate;
  short m_usSoundHeadroom;
};
#pragma pack(pop)


#pragma pack(push, 1)
struct CAEBankSlot
{
  unsigned int m_dwOffsetOnBuffer;
  unsigned int m_dwSlotBufferSize;
  int _unused1;
  int _unused2;
  unsigned short m_usBankNum;
  unsigned short m_nSoundsOnBank;
  CAEBankSlotItem m_aBankItems[400];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct CAESoundRequest
{
  CAEBankSlot *m_pBankSlot;
  unsigned int m_dwOffset;
  unsigned int m_dwSize;
  void *m_pBufferData;
  void *m_pBuffer;
  int m_iLoadingStatus;
  unsigned short m_usBank;
  unsigned short m_usBankSlot;
  unsigned short m_usSound;
  char m_iPak;
  char field_1F;
};
#pragma pack(pop)




#pragma pack(push, 1)
struct CAEBankLoader
{
  CAEBankSlot *m_pBankSlots;
  CAEBankLookupItem *m_pBankLookup;
  char *m_pPakFiles;
  short m_usNumBankSlots;
  short m_usNumBanks;
  short m_iNumPakFiles;
  short _unused0;
  char m_bInitialized;
  char gap_15[3];
  unsigned int m_iSoundBuffersSize;
  char *m_pSoundBuffers;
  int *m_StreamHandles;
  CAESoundRequest m_aSoundRequests[50];
  short _unk0;
  short m_nRequestsToLoad;
  short m_iRequestListNext;
  short m_iStreamingChannel;
  unsigned short m_aBankSlotSound[45];
  char _unused1[30];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BankHeader
{
    unsigned short        m_nSounds;
    short                 _pad;
    CAEBankSlotItem       m_aSounds[400];
};
#pragma pack(pop)

static_assert(sizeof(CAEBankLoader)     == 0x6E4,   "Incorrect struct size: CAEBankLoader");
static_assert(sizeof(CAESoundRequest)   == 0x20,    "Incorrect struct size: CAESoundRequest");
static_assert(sizeof(CAEBankSlot)       == 0x12D4,  "Incorrect struct size: CAEBankSlot");
static_assert(sizeof(CAEBankLookupItem) == 0x0C,    "Incorrect struct size: CAEBankLookupItem");
