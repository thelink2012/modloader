# Mod Loader

<p align="center">
<img alt="GitHub repo size" src="https://img.shields.io/github/repo-size/thelink2012/modloader">
<img alt="GitHub commit activity" src="https://img.shields.io/github/commit-activity/w/thelink2012/modloader">
<img alt="GitHub contributors" src="https://img.shields.io/github/contributors/thelink2012/modloader">
</p>

Mod Loader is a plugin for Grand Theft Auto III, Vice City and San Andreas that adds an easy and user-friendly way to install and uninstall modifications into the game, as if the game had official modding support. No changes are **ever** made to the original game files, everything is injected on the fly, at runtime!

The usage is as simple as inserting the mod files into the *modloader/* directory. Uninstalling is as easy as that too, delete the mod files and you are done. Hot swapping mods while the game is running? By using Mod Loader you can!

Still not sure? Check out [this](https://www.youtube.com/watch?v=TvRpQa8dJ7E) nice video from Ivey. For more, check out our [GTAForums](http://gtaforums.com/topic/669520-mod-loader/) thread and our [GTAGarage](http://www.gtagarage.com/mods/show.php?id=25377) page.

### Building and Installing

Requirements:

+ [Premake 5](http://industriousone.com/premake/download) *(pre-built executable available in this repository root)*
+ [Visual Studio](http://www.visualstudio.com/downloads) 2013 or greater.

Run the following command in the root of this directory to generate the project files:

    premake5 vs2013

You can install the generated binaries into your game directory by running:

    premake5 install "C:/Program Files (x86)/Rockstar Games/GTA San Andreas"

Or, you might want the files to be automatically installed everytime you build the solution:
 
    premake5 vs2013 "--idir=C:/Program Files (x86)/Rockstar Games/GTA San Andreas"

