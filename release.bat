@echo off
set CL=/MP
premake5 --file=release.lua prepare --toolset=vs2013
pause
goto:eof
