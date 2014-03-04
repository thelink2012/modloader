========= San Andreas Mod Loader ==========

---> What is it?

San Andreas Mod Loader is an ASI Plugin for Rockstar's Grand Theft Auto San Andreas that adds an extremely
user-friendly and easy way to install and uninstall your modifications, without even messing around anything
in your game installation.

The usage is simple, you just have to create one or more folders inside modloader folder and drop the mod contents there.
It is recommended to have one folder for each modification you have.

---> Installing modloader

Before installing modloader you need a ASI Loader (http://www.gtagarage.com/mods/show.php?id=21709).
Then, just extract modloader.asi and modloader folder into your game directory.

---> Installing mods at modloader

As I've already told you, any mod installation is simple, just extract the contents of the mod into a folder inside modloader folder.

That means for example the following are valid installations for a vehicle mod:
    modloader/ferrari/cheetah.dff & cheetah.txd & handling.cfg
    modloader/ferrari/gta3.img/cheetah.dff & cheetah.txd
But the following is invalid:
    modloader/cheetah.dff & cheetah.txd     (WRONG!!)

---> Uninstalling mods on modloader
    Just delete the folder you extracted the mods content to. wow.
    If you just want to disable the mod for a while, you can just set its priority to 0 in the file modloader/modloader.ini
    Like that:
        [PRIORITY]
        Folder from the mod I want to disable=0

---> Things you MUST know

    [*] Do not replace any original file
        You already know that, but I'd like to reinforce the idea.
        Modloader do not replace anything in the original game files, so you can rest in peace.

    [*] File mixing
        When it comes to data files (.dat, .cfg,. ide, .ipl) modloader will work in a interesting way.
        It won't simply load the file overriding the original (however it may happen in some cases for optimization).
        It will mix the files! That's right! Therefore you can feel free to for example have many handling.cfg files on modloader
        'cuz there's no problem, the differences will be solved.
        
    [*] Readme files reading
        Yet with data files, modloader can detect lines that would go to such files in a readme text file.
        Everything to make your life easier!

    [*] Reloadable models
        Whenever you reload the game (clicking new game or load game) most models will get reloaded.
        That's perfect for modders that replace models or texture files.
        
        I said "most" because there is models that the game nevers unloads, but they're minority.
        PS: It's not possible to ADD MORE models while the game do not get closed and opened again, only replacing of the already identified ones.

    [*] Priority system
        Modloader has a priority system, take a look at modloader.ini
        
        One thing to note is the "overriding rule" where files that got loaded late will override mods loaded first.
        For example, if you have mod A with a priority 100 and mod B with priority 1 and both replace LOADSCS.txd file,
        the LOADSCS that will get loaded is the one form B because it replaced what A did.

    [*] Command line support
        Modloader support command line passed throught the game executable.
        The command line is -mod ModFolder
        This forces modloader to load only the files at ModFolder and the mods taht on the inclusion list from modloader.ini
        You can send more than one mod, like that: -mod Mod1 -mod Mod2
        Any mod sent by command line will get a priority of 80!

    [*] Configuration
        You can configure many things related to mods that will get reload and other stuff.
        Take a look in the file modloader.ini

    [*] Recursivity
        Modloader has a recursivity system where you can create another modloader folder inside a mod folder and so on.
        For example: modloader/pack 1/modloader/...
        Useful if you'd like to enable of disable many mods.
        The configurations from modloader.ini are inherited and you can create another in the new folder.

---> Under progress

    Modloader is still under progress, thus not everything is loadable yet.
    Some data files are not loadable yet, take a look at the list of supported data files at modloader/.data/plugins/std-data.txt
    
---> You've found a bug?

    Perfect, don't forget to report!
    When reporting a bug, don't forget to include the file modloader/modloader.log and detail how to reproduce the bug.
    To report a bug, you can go in any of those places:
    
        [Recommended] Our issue tracker on GitHub: https://github.com/thelink2012/sa-modloader/issues
        [EN] On GTA Forums: http://gtaforums.com/topic/669520-sarel-san-andreas-mod-loader/
        [BR] on BMS Forum : http://brmodstudio.forumeiros.com/t3591-san-andreas-mod-loader-topico-oficial

---> Supported executables

    Modloader supports the following executables for GTA San Andreas:
        1.0 US (Original, HOODLUM, Listener's)

    If the requiriment is not fulfilled, a error message will show up to warn the user.
    Support for other executables are on the way, support for 3.0 (Steam) is a priority.


---> Download

    The place I'll surely let modloader always updated is on GTAGarage.
    So I'd really recommend that you redirect the user there instead of uploading modloader elsewhere.
        http://www.gtagarage.com/mods/show.php?id=25377

---> Source code

    Modloader's source code is available under GPL license.
        https://github.com/thelink2012/sa-modloader/

---> Credits

    Main programming: LINK/2012 (dma_2012@hotmail.com)
    
    Special thanks to:
        ArtututuVidor$
        Andryo
        Junior_Djjr
        JNRois12
        methodunderg
        R4GN0R0K
        SilentPL
        TheJAMESGM

