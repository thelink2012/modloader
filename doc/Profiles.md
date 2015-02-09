Mod Loader Profiles
========================

Mod Loader comes with a nice feature to manage which modifications to get loaded under certain circustances called profiles.
Profiles let you select a group of mods to load, files to ignore and so on.

To make use a profile switch the current profile in *modloader.ini base config*, send a command line `-modprof` followed by the profile name to use, or have a `UseIfModule` condition on it.

To create or modify profiles go into `modloader.ini` or create a new ini in the *.profiles/* directory, such ini can have any name.

For a profile to be detected as existing, you need to have at least one ini section named after a profile.
A profile section starts with `Profile` followed by a *dot*, then the *profile name* followed by another *dot* and now what the section is supposed to do. Example:

    [Profiles.ProfileName.Config]

For the config section of the profile named ProfileName

__Note__: Profile names are not case-sensitive!

`[Profiles.ProfileName.Config] `
--------------------------------------

##### Parents = ProfileName|$Current|$None, ...

Defines one or more parent profiles to inherit all it's configuration from, that's priorities, files to ignore, and so on.

If two inherited profiles are mutual exclusive the behaviour is undefined.

Special values are:
* **$Current** -- Inherits from the currently selected profile in *modloader.ini base config*. This may be useful for conditional profiles.
* **$None**    -- No inheritance.

If **$None** is specified as **any** of the parents no inheritance will take place.

Default value is `$None`.

##### IgnoreAllMods = true|false

Ignores all mods in the modloader directory, essentially disabling any mod.
Default value is `false`.

__Note__: `IgnoreAllFiles` can also be used and it does the same as this.

##### ExcludeAllMods = true|false

Ignores all mods in the modloader directory except for the ones in the [Profiles.ProfileName.IncludeMods] list. 
Default value is `false`.

##### UseIfModule = ModuleName

This profile is forced to be used by Mod Loader if the specified executable/dll module is loaded.
Any changes on this entry while the game while the game is running won't make any effect until the next run.
This may be useful for a SA:MP profile or game-dependent profile.

Notice this profile is used as an *anonymous* profile when the condition is met thus it's a clone of the saved profile but not itself consequently any in-game changes to the profile will not be saved or loaded.

Using a profile if running from SAMP example:

    UseIfModule = SAMP

`[Profiles.ProfileName.IgnoreMods]`
--------------------------------------
Any modification listed on this section is going to be ignored while scanning for mods.

Supports wildcards.

`[Profiles.ProfileName.IncludeMods]`
--------------------------------------
All the mods in this list are going to be used even when `[Profiles.ProfileName.Config]:ExcludeAllMods` is set to `true`. 

Supports wildcards.

`[Profiles.ProfileName.ExclusiveMods]`
--------------------------------------
Any mod in this list will be loaded *ONLY* by this profile, any other profile will have this mod automatically ignored.
If another profile has a similar exclusive mod, both will have the mods as exclusive and use them.

Supports wildcards.

`[Profiles.ProfileName.IgnoreFiles]`
--------------------------------------
Any file in this list are ignored by the loader while scanning for files.
The wildcards can contain sub folders, for example:

    to_ignore/*.dff

This line would ignores all files in the *to_ignore* directory that have a dff extension.
Notice *to_ignore* must be **INSIDE a mod directory**, such as *modloader/my mod/to_ignore/*

Supports wildcards.

`[Profiles.ProfileName.Priority]`
--------------------------------------
Defines mods priorities, that's if two mods modify the same file which should have precedence over the other.
The key in this section is the mod name and the value the priority from 0 to 100.
