// http://stackoverflow.com/questions/20337870/what-is-the-equivalent-of-system-diagnostics-debugger-launch-in-unmanaged-code
#ifndef __LAUNCH_WIN32_DEBUGGER__
#define __LAUNCH_WIN32_DEBUGGER__
#pragma once

#include <windows.h>
#include <string>
#include <sstream>

inline bool LaunchDebugger()
{
    // Get System directory, typically c:\windows\system32
    std::wstring systemDir(MAX_PATH+1, '\0');
    UINT nChars = GetSystemDirectoryW(&systemDir[0], systemDir.length());
    if (nChars == 0) return false; // failed to get system directory
    systemDir.resize(nChars);

    // Get process ID and create the command line
    DWORD pid = GetCurrentProcessId();
    std::wostringstream s;
    s << systemDir << L"\\vsjitdebugger.exe -p " << pid;
    std::wstring cmdLine = s.str();

    // Start debugger process
    STARTUPINFOW si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);

    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));

    if (!CreateProcessW(NULL, &cmdLine[0], NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) return false;

    // Close debugger process handles to eliminate resource leak
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);

    // Wait for the debugger to attach
    while (!IsDebuggerPresent()) Sleep(100);

    // Stop execution so the debugger can take over
    //DebugBreak();
    return true;
}

#ifdef NDEBUG
#error Using this in non debug mode?
#endif

#endif