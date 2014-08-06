Mod Loader
=============================

Mod Loader is an ASI Plugin for Rockstar's Grand Theft Auto San Andreas that adds an extremely user-friendly and easy way to install and uninstall your modifications, without even messing around anything in your game installation.

### Introduction

Modifications are very popular in the Grand Theft Auto community, specially in Grand Theft Auto San Andreas, but everything is just too difficult to install since modding is not officially supported. People gets scared of how hard it is to install modifications.

This modification aims to provide a extremely simple way to install modifications as seen in games with official modding support, you just place the modification in the modloader folder and look, it is installed! Easier than that is impossible. Uninstalling is that simple too, just remove the modification from the modloader folder.

A important note is that modloader **DO NOT** touch **ANY** file from your game installation, so no risks.

It may be very helpful for people with two or more game installations for different kind of modifications (e.g. one install clean, one install to play multiplayer, one install for mods, one install for a total conversion...).

It certainlly is very helpful for developers, they don't have to be messing with *.img* files, rebuilding them everytime. And even better, you can reload the mods without getting out of the game!

This is a open source project, so, feel free to learn and contribute!


### Compiling

If you are building from the source code, it is very simple to compile. You'll need the following:

+ [CMake](http://www.cmake.org/) 2.8 or greater
+ An C++11 compiler, tested with:
    - [Visual Studio](http://www.visualstudio.com/downloads) 2013 or greater
    - [MinGW](http://mingw-w64.sourceforge.net/download.php) 4.8.2 or greater *(32 bits, SJLJ exception handling is prefered)*


Then, in a terminal _(cmd.exe on Windows)_ go into the base source directory and run the commands:

    mkdir build
    cd build
    cmake ../
    
This will generate a project or make file for your target at the *build* folder.
    
To build and install you should do the following

 + __For Visual Studio__: Open the generated solution file (*.sln*), setup it and build
 + __For MinGW__:
 
        cd build
        mingw32-make
        mingw32-make install "DESTDIR=C:/Program Files (x86)/Rockstar Games/GTA San Andreas"

    ...replacing the path after `DESTDIR=` with your game directory.


### License

The source code is licensed under GNU GPL v3, giving you the freedom to modify, create derivated works and more. See the LICENSE file for details.

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Grand Theft Auto and all related trademarks are © Rockstar North 1997-2014.