#!/bin/sh

echo "Building release builds..."
cd ..

rm -rf build
rm -rf bin
rm -rf release/gamedir

mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-linux-i686-w64-mingw32-toolchain.cmake -DCMAKE_BUILD_TYPE=Release ../
make #VERBOSE=1
make install "DESTDIR=/home/link2012/Projects/modloader/release/gamedir"

cd ../release/gamedir

echo "Stripping binaries..."
strip "modloader.asi"
cd modloader/.data/plugins
for f in *.dll
do
	strip "$f"
done
cd ../../..

# Copy readme files into base
cp ./modloader/.data/Readme.txt     ./Readme.txt
cp ./modloader/.data/Leia-me.txt    ./Leia-me.txt

echo "Zipping"
cd gamedir
zip modloader.zip -r -9 .

