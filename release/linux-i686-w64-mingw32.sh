#!/bin/sh
#
#
#

echo Building release builds...
cd ..

rm -rf build
rm -rf bin
rm -rf release/gamedir

mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-linux-i686-w64-mingw32-toolchain.cmake -DCMAKE_BUILD_TYPE=Release ../
make
make install "DESTDIR=../release/gamedir"
cd ../release/gamedir

# Copy readme files into base
cp ./modloader/.data/Readme.md     ./Readme.txt
cp ./modloader/.data/Leia-me.md    ./Leia-me.txt

# Strip GCC symbols out of the binary
echo Stripping binaries...
strip "modloader.asi"
cd modloader/.data/plugins
for f in *.dll
do
	strip "$f"
done
cd ../../..

echo Zipping
cd gamedir
zip modloader.zip -r -9 .
