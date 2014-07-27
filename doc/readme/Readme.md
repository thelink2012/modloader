Mod Loader
==================

## What is it?

Mod Loader is an ASI Plugin for Rockstar's Grand Theft Auto San Andreas that adds an extremely user-friendly and easy way to install and uninstall your modifications, without even messing around anything in your game installation.

The usage is simple, you just have to create one or more folders inside modloader directory and then drop the mod contents there. It's done.
It is recommended to have one folder for each modification you have.

## Installing Mod Loader

Mod Loader depends on an [ASI Loader](http://www.gtagarage.com/mods/show.php?id=21709), make sure you have it!

Then just extract *modloader.asi* and *modloader* folder into your game directory.

## Installing Mods in Mod Loader

To install a mod in Mod Loader, it's extremely simple, just extract the contents of the mod **into a folder** inside *modloader* directory.

That means the following are valid installation methods:

 + modloader/nsx/infernus.dff
 + modloader/nsx/another folder/infernus.dff

But the following is **NOT** valid:

 - modloader/infernus.dff 
 - modloader/.data/infernus.dff


## Uninstalling Mods from Mod Loader

Even simpler, just delete the mod content from *modloader* directory.
If you just want to disable the mod for a while, go to the in-game menu and disable it or edit *modloader.ini* manually. 

## Highlights and Details

- Do not replace **ANY** original file, never, really.
- Let Mod Loader take care of everything
    + Data files mixing.
        * Therefore you can for example have 70 handling.cfg files at modloader and they'll all work perfectly fine.
    + Readme files reading
        * No need to care about taking the data line from the readme file and placing in the data file, Mod Loader does that for you too!
- Refreshable mods
    + Change or add the files while the game is running and see the results immediatelly*!!!!
- Command line support
    + See *modloader/.data/Command Line Arguments.md*
- In-Game menu for configurations
    + Go to *Options > Mod Loader Setup*
    + When the menu is not available, do it manually by editing *modloader/modloader.ini* and *modloader/.data/config.ini*

\* Currently you need to press F4 to refresh the mods, also not all kinds of files can be refreshed but most of them can.



## Under Progress

Mod Loader is still under progress, thus not everything is loadable yet.
That is, some *.dat* files are not loadable for now, but we're working on it.

### You have found a bug?

It's essential to report bugs, so the loader gets improved, to report a bug go to any of the following channels:

 * On GitHub, using [our issue tracker](https://github.com/thelink2012/sa-modloader/issues)
 * English support on [GTA Forums](http://gtaforums.com/topic/669520-sarel-mod-loader/)
 * Portuguese support on [Brazilian Modding Studio Forums](http://brmodstudio.forumeiros.com/t3591-mod-loader-topico-oficial)

When reporting a bug **PLEASE, FOR GOD'S SAKE** provide the *modloader/modloader.log* file created just after the crash and give some detailed information on how to reproduce the bug.

### Supported Executables

Not all executables are supported at the moment, the supported ones are:

 + 1.0 US *(Original, HOODLUM, Listener's Compact)*

## Download

You can download the lastest version of Mod Loader from:

 * [GTA Garage](http://www.gtagarage.com/mods/show.php?id=25377), for the lastest stable build
 * [GitHub](https://github.com/thelink2012/modloader/releases), for the lastest (including unstable) builds

## Source Code

Mod Loader is a open source project, feel free to learn and contribute.
The source code is licensed under the GPL v3 license, check it out on [GitHub](https://github.com/thelink2012/modloader/).

## Credits

Finally, let's go to the credits.

#### Developer
  * LINK/2012 (<dma_2012@hotmail.com>)

#### Special Thanks To
  * ArtututuVidor$, Andryo, Junior_Djjr, JNRois12 for alpha-testing
  * methodunderg and TheJamesGM for emotional support.
  * SilentPL for additional support
