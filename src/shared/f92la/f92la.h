#pragma once
#include <windows.h>
#include <cstdint>

struct LibF92LA
{
    // $fastman92limitAdjuster.asi
    HMODULE hLib = nullptr;

    // Count of file IDs
    int32_t(*GetNumberOfFileIDs)() = nullptr;
    // Returns model info, prev file ID
    int32_t(*GetFileInfoPrevFileID)(int32_t fileID) = nullptr;
    // Returns model info, next file ID
    int32_t(*GetFileInfoNextFileID)(int32_t fileID) = nullptr;
    // Returns model info, next on CD file ID
    int32_t(*GetFileInfoNextOnCDfileID)(int32_t fileID) = nullptr;
    // Sets file info, Prev file ID 
    void(*SetFileInfoPrevFileID)(int32_t fileID, int32_t newValue) = nullptr;
    // Sets file info, Next file ID
    void(*SetFileInfoNextFileID)(int32_t fileID, int32_t newValue) = nullptr;
    // Sets file info, NextOnCd file ID
    void (*SetFileInfoNextOnCDfileID)(int32_t fileID, int32_t newValue) = nullptr;
};

inline LibF92LA Fastman92LimitAdjusterCreate()
{
	LibF92LA f92la;
    if(GetModuleHandleEx(0, TEXT("$fastman92limitAdjuster.asi"), &f92la.hLib) != FALSE || GetModuleHandleEx(0, TEXT("$fastman92limitAdjuster"), &f92la.hLib) != FALSE)
    {
        f92la.GetNumberOfFileIDs        = (decltype(f92la.GetNumberOfFileIDs)) GetProcAddress(f92la.hLib, "GetNumberOfFileIDs");
        f92la.GetFileInfoPrevFileID     = (decltype(f92la.GetFileInfoPrevFileID)) GetProcAddress(f92la.hLib, "GetFileInfoPrevFileID");
        f92la.GetFileInfoNextFileID     = (decltype(f92la.GetFileInfoNextFileID)) GetProcAddress(f92la.hLib, "GetFileInfoNextFileID");
        f92la.GetFileInfoNextOnCDfileID = (decltype(f92la.GetFileInfoNextOnCDfileID)) GetProcAddress(f92la.hLib, "GetFileInfoNextOnCDfileID");
        f92la.SetFileInfoPrevFileID     = (decltype(f92la.SetFileInfoPrevFileID)) GetProcAddress(f92la.hLib, "SetFileInfoPrevFileID");
        f92la.SetFileInfoNextFileID     = (decltype(f92la.SetFileInfoNextFileID)) GetProcAddress(f92la.hLib, "SetFileInfoNextFileID");
        f92la.SetFileInfoNextOnCDfileID = (decltype(f92la.SetFileInfoNextOnCDfileID)) GetProcAddress(f92la.hLib, "SetFileInfoNextOnCDfileID");

        // If, for some reason, FLA we're using lacks any of those imports, don't try to do anything with it
        if (f92la.GetNumberOfFileIDs == nullptr || f92la.GetFileInfoPrevFileID == nullptr || f92la.GetFileInfoNextFileID == nullptr || f92la.GetFileInfoNextOnCDfileID == nullptr
            || f92la.SetFileInfoPrevFileID == nullptr || f92la.SetFileInfoNextFileID == nullptr || f92la.SetFileInfoNextOnCDfileID == nullptr)
        {
            FreeLibrary(f92la.hLib);
            f92la.hLib = nullptr;
        }
    }
	return f92la;
}

inline void Fastman92LimitAdjusterDestroy(const LibF92LA& f92la)
{
	if(f92la.hLib) FreeLibrary(f92la.hLib);
}
