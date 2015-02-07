Mod Loader Command Line
==============================

Mod Loader allows certain command line arguments to get passed to the game executable

* -nomods

   No modification from *modloader* directory gets loaded when this argument is used.
   
   __Usage__: `gta_sa.exe -nomods`
   

* -mod _modname_

   This command line option makes Mod Loader load only _modname_ from the *modloader* directory.
   That's essentially the same as creating a profile with *ExcludeAllMods* enabled plus the mod _modname_ in the *[IncludeMods]* section

   __Usage__: `gta_sa.exe -mod modname`
   
   This will make Mod Loader load *modloader/modname* with priority 20.

   You can send more than one command of this type to load many mods.

* -mod _modname_=_priority_

   Does the same as the `-mod _modname_` except giving the specified mod a priority.

* -modprof _profilename_

   Loads the specified profile.


Notes
---------------------

+ The `-nomods`, `-mod` and `-modprof` command lines are mutually exclusive.
+ When `-mod` or a `-modprof` command line are used a anonymous profile is created so any changes to the related profiles aren't saved/loaded. 
