@echo off

echo Building release builds...
cd ..

rm -rf build
rm -rf bin
rm -rf release/gamedir

mkdir build
cd build
cmake -G "MSYS Makefiles" -DCMAKE_BUILD_TYPE=RelMinSize ../
make
make install "DESTDIR=../release/gamedir"
cd ../release/gamedir

:: Strip GCC symbols out of the binary
echo Stripping binaries...
strip "modloader.asi"
cd modloader/.data/plugins
for /r %%f in (*.dll) do strip %%f
cd ../../..

:: Copy readme files into base
cp ./modloader/.data/Readme.md     ./Readme.txt
cp ./modloader/.data/Leia-me.md    ./Leia-me.txt

echo Cannot zip because Windows has no built-in zipping utility
