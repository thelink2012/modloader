#pragma once
#include <windows.h>
#include <cstdint>

struct LibF92LA
{
    // $fastman92limitAdjuster.asi
    HMODULE hLib = NULL;

    // Count of file IDs
    int32_t(*GetNumberOfFileIDs)();
    // Returns model info, prev file ID
    int32_t(*GetFileInfoPrevFileID)(int32_t fileID);
    // Returns model info, next file ID
    int32_t(*GetFileInfoNextFileID)(int32_t fileID);
    // Returns model info, next on CD file ID
    int32_t(*GetFileInfoNextOnCDfileID)(int32_t fileID);
    // Sets file info, Prev file ID 
    void(*SetFileInfoPrevFileID)(int32_t fileID, int32_t newValue);
    // Sets file info, Next file ID
    void(*SetFileInfoNextFileID)(int32_t fileID, int32_t newValue);
    // Sets file info, NextOnCd file ID
    void (*SetFileInfoNextOnCDfileID)(int32_t fileID, int32_t newValue);
};

inline LibF92LA Fastman92LimitAdjusterCreate()
{
	LibF92LA f92la;
    if(GetModuleHandleEx(0, "$fastman92limitAdjuster.asi", &f92la.hLib))
    {
        f92la.GetNumberOfFileIDs        = (decltype(f92la.GetNumberOfFileIDs)) GetProcAddress(f92la.hLib, "GetNumberOfFileIDs");
        f92la.GetFileInfoPrevFileID     = (decltype(f92la.GetFileInfoPrevFileID)) GetProcAddress(f92la.hLib, "GetFileInfoPrevFileID");
        f92la.GetFileInfoNextFileID     = (decltype(f92la.GetFileInfoNextFileID)) GetProcAddress(f92la.hLib, "GetFileInfoNextFileID");
        f92la.GetFileInfoNextOnCDfileID = (decltype(f92la.GetFileInfoNextOnCDfileID)) GetProcAddress(f92la.hLib, "GetFileInfoNextOnCDfileID");
        f92la.SetFileInfoPrevFileID     = (decltype(f92la.SetFileInfoPrevFileID)) GetProcAddress(f92la.hLib, "SetFileInfoPrevFileID");
        f92la.SetFileInfoNextFileID     = (decltype(f92la.SetFileInfoNextFileID)) GetProcAddress(f92la.hLib, "SetFileInfoNextFileID");
        f92la.SetFileInfoNextOnCDfileID = (decltype(f92la.SetFileInfoNextOnCDfileID)) GetProcAddress(f92la.hLib, "SetFileInfoNextOnCDfileID");
    }
	return f92la;
}

inline void Fastman92LimitAdjusterDestroy(const LibF92LA& f92la)
{
	if(f92la.hLib) FreeLibrary(f92la.hLib);
}
