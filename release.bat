@echo off
echo Are you sure this is a final release?
echo This will not be tagged in the logs as a development build.
choice /C YN /M "Continue"
if errorlevel 2 exit /b 1

set CL=/MP
set "PATH=%ProgramFiles(x86)%\Windows Kits\10\Debuggers\x86;%PATH%"
premake5 --file=release.lua prepare --toolset=vs2022 --final-release

pause