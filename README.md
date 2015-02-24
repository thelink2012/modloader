Mod Loader
=============================

Mod Loader is an ASI Plugin for Rockstar's Grand Theft Auto San Andreas that adds an extremely user-friendly and easy way to install and uninstall your modifications, without even messing around anything in your game installation.

### Introduction

Modifications are very popular in the Grand Theft Auto community, specially in Grand Theft Auto San Andreas, but everything is just too difficult to install since modding is not officially supported. People gets scared of how hard it is to install modifications.

This modification aims to provide a extremely simple way to install modifications as seen in games with official modding support, you just place the modification in the modloader folder and look, it is installed! Easier than that is impossible. Uninstalling is that simple too, just remove the modification from the modloader folder.

An important note is that modloader **DO NOT** touch **ANY** file from your game installation, so no risks.

It may be very helpful for people with two or more game installations for different kind of modifications (e.g. one install clean, one install to play multiplayer, one install for mods, one install for a total conversion...).

It certainlly is very helpful for developers, they don't have to be messing with *.img* files, rebuilding them everytime. And even better, you can reload the mods without getting out of the game!

This is a open source project, so, feel free to learn and contribute!


### Compiling and Installing

If you are building from the source code, it is very simple to compile. You'll need the following:

+ [Premake](http://industriousone.com/premake/download) 5 *(pre-built executable available in this repository root)*
+ An C++11 compiler, tested under:
    - [Visual Studio](http://www.visualstudio.com/downloads) 2013 or greater
    - [MinGW](http://mingw-w64.sourceforge.net/download.php) 4.9.0 or greater _(currently have build errors with gta3.std.data)_


Then, in a command-line shell go into the repository root directory and run the commands:

 + __For Visual Studio__:
 
        premake5 vs2013

    then you can compile the generated project in the build directory

 + __For MinGW__:
 
        premake5 gmake
        cd build
        mingw32-make CC=gcc
        cd ..


After such, you can install the generated binaries into your game directory by running

    premake5 install "C:/Program Files (x86)/Rockstar Games/GTA San Andreas"

  ...replacing the path with your game directory.

If you are up to work with the project files you might want the files to be automatically installed everytime you build the solution, to accomplish that you should specify the *--idir=DESTDIR* option to premake5.
For example:
 
    premake5 vs2013 "--idir=C:/Program Files (x86)/Rockstar Games/GTA San Andreas"

Use *premake5 --help* for more command line options.

### License

The source code is licensed under the MIT License, giving you all the freedom you possibly want, see LICENSE for details.

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Grand Theft Auto and all related trademarks are © Rockstar North 1997-2014.
