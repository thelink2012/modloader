@echo off
set CL=/MP
set "PATH=%ProgramFiles(x86)%\Windows Kits\10\Debuggers\x86;%PATH%"
premake5 --file=release.lua prepare --toolset=vs2022
pause
goto:eof