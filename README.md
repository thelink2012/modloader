# San Andreas Mod Loader

San Andreas Mod Loader is an ASI Plugin for Rockstar's Grand Theft Auto San Andreas that adds an extremely easy way to install and uninstall your modifications, without even messing around anything in your game installation.

### Motivation

Modifications are very popular in the Grand Theft Auto community, specially in Grand Theft Auto San Andreas, but everything is just too difficult to install since modding is not officially supported. People starting get scared of how hard it is to install modifications (well, hyperbole makes marketing better).

This modification aims to provide a extremely simple way to install modifications as seen in games with official modding support, you just place the modification in the modloader folder and look, it is installed! Easier than that is impossible. Uninstalling is that simple too, just remove the modification from the modloader folder.

A important note is that modloader **DO NOT** touch **ANY** file from your game installation, so no risks. And it performs fast!

It may be very helpful for people with two or more game installations for different kind of modifications (e.g. one install clean, one install to play multiplayer, one install for mods, one install for a total conversion...).

It's certainlly is very helpful for developers too, you don't have to be messing with .img files, rebuilding them everytime. Many installable modifications in modloader permit an reloading without closing the game!

This is a open source project, so, feel free to do whatever you want (well, see the LICENSE at the base directory, whatever is not the right word here), contribute, report and fix bugs...


### Compiling

If you are building from the source code, it is very simple to compile. You'll need the following:

+ [CMake](http://www.cmake.org/)
+ [MinGW](http://mingw-w64.sourceforge.net/download.php) *(32 bits, SJLJ exception handling is prefered)*

Then, in a terminal *(cmd.exe on Windows)* go into the base source directory and run the commands:

    mkdir build
    cd build
    cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release ../ && make
    
On **Linux**, using MinGW for cross-compiling you might replace the last line with:
    cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-linux-i486-mingw32-toolchain.cmake -DCMAKE_BUILD_TYPE=Release ../ && make
    
This will build the mod loader, if everything went fine, there will be a folder /bin at the source base directory with the binaries.
To install those binaries (and other stuff) run the commands:
    
    cd build
    make install "DESTDIR=C:/Program Files (x86)/Rockstar Games/GTA San Andreas"
    
...replacing the path after `DESTDIR=` with your game directory.
    

### Installing Modifications

To install your modifications will probably just need to extract the modifications content into a new folder inside *modloader/* directory

**Mod Loader is still in development**


### License

The source code is licensed under GNU GPL v3, giving you the freedom to modify, create derivated works and more. See the LICENSE file for more details.

- - -
Grand Theft Auto and all related trademarks are © Rockstar North 1997-2013.

