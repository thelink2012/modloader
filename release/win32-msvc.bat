::
::  May require to be opened from Visual Studio Command Prompt
::
@echo off

echo Building release builds...
cd ..

rm -rf build
rm -rf bin
rm -rf release/gamedir

mkdir build
cd build
cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release ../
nmake
nmake install "DESTDIR=../release/gamedir"
cd ../release/gamedir

:: Copy readme files into base
cp ./modloader/.data/Readme.txt     ./Readme.txt
cp ./modloader/.data/Leia-me.txt    ./Leia-me.txt

echo Cannot zip because Windows has no built-in zipping utility
