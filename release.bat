@echo off
set CL=/MP
premake5 --file=release.lua prepare --toolset=vs2015
pause
goto:eof
