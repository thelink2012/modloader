#pragma once
#include "CAEBankLoader.h"

#pragma pack(push, 1)
struct CAETrackLoader
{
  char m_bInitialized;
  char field_1[3];
  int m_usNumTracks;
  int m_usNumStreamPaks;
  CAEBankLookupItem *m_pTrackLookup;
  const char *m_pStreamPaks;
  char m_bUseDVD;
  char gap_15[3];
  const char *m_szDvdPath;
};
#pragma pack(pop)
