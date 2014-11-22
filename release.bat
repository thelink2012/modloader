@echo off
premake5 --file=release.lua prepare --debug-symbols --toolset=vs2013
pause
goto:eof
