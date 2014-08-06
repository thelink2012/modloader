Mod Loader Command Line
==============================

Mod Loader allows certain command line arguments to get passed to the game executable

* -mod _modname_

   This command line option makes Mod Loader load only _modname_ from the *modloader* directory.
   That's essentially the same as going into *modloader.ini*, enabling *ExcludeAllMods*, and adding _modname_ into *[IncludeMods]* section
            
   __Usage__: `gta_sa.exe -mod grandcarma`
   
   This will make Mod Loader load *modloader/grandcarma* with priority 80 and other mods at *[IncludeMods]* section of *modloader.ini*, but no other mod gets loaded.

* -nomods

   No modification from *modloader* directory gets loaded when this argument is used.
   
   __Usage__: `gta_sa.exe -nomods`
