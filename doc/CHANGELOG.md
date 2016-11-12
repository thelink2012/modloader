v0.3.5 (Nov 12 2016)
------------------------
 * Fixed III/VC streaming corrupting on 2nd/3rd new game.
 * Support for path translation in ASIs compiled in VS2015+.

v0.3.4 (Jun 22 2016)
------------------------
 * Support for GTA III, Vice City and San Andreas.
 * Support for Fastman92 Limit Adjuster 32 bit id limits.

v0.3.3 (Jun 19 2016)
------------------------
 * Support for III/VC splash sprites.
 * Support for III/VC script sprites (but uses SA directory structure, not III/VC one).
 * Support for overloading and loading cdstreams on III/VC.
 * Fixed a problem with the CLEO injector.
 * Fixed III crashing with collision mods.
 * Fixed Vice City crashing with map mods containing collision left overs.

v0.3.2 (Jun 17 2016)
------------------------
 * Support for III/VC data files
    + gta.dat, IDEs, IPLs
	+ carcols.dat
	+ cullzones.dat
	+ fistfite.dat
	+ handling.cfg
	+ object.dat
	+ particle.cfg
	+ ped.dat
	+ pedgrp.dat
	+ pedstats.dat
	+ surface.dat
	+ timecyc.dat
	+ waterpro.dat
	+ weapon.dat
 * Support for loading III/VC CLEO scripts inside the modloader directory.

v0.3.1 (Jun 13 2016)
------------------------
 * First GTA III public release.

v0.3.0 (Jun 12 2016)
------------------------
 * First GTA Vice City public release.

v0.2.5 (Aug 15 2015)
------------------------
 * Support for UTF-8, UTF-16 and UTF-32 readme files.
 * Support for 32GB img archives.
 * Readme files are now limited to 60KB instead of 10KB.
 * Fixed *GTA San Andreas 1.0 EU* support crashing on SAMP.

v0.2.4 (Feb 09 2015)
------------------------
 * Added IDE refresher.
 * Added *AUDIO/STREAMS/*, TrakLkUp.dat and StrmPaks.dat overriders.
 * Added *stream.ini* merger.
 * Added profiles, a feature to let you have different mod setups with easy switching between them or even automatic *(See modloader/.data/Profiles.md for details)*. 
 * Wildcards support has been expanded plus not only the IgnoreFiles list support wildcards but the others too.
 * IPL and IDE files given to modloader **NEED** to be registered in *gta.dat* like before, the *not needing* approach was error-prone.
 * Restored the `mod` command line and also added the `-modprof` command line to load a profile. *(See modloader/.data/Command Line Arguments.md for details)*. 
 * The `mod` command line allows the user to specify a priority now.
 * Fixed non-GENRL wave files not being installed properly.
 * Other minor fixes.

 * __Hotfix 1 (Feb 10 2015)__:
    + Fixed readme lines for gta.dat not handled properly.

 * __Hotfix 2 (Feb 11 2015)__:
    + carcols.dat lines with only a single color are allowed now.
    + Fixed profiles inheritance.
    + Fixed last menu page being empty when number of mods is multiple of 9.  

v0.2.3 (Feb 02 2015)
------------------------
 * The In-Game menu is ready to be used _(go into Options > Mod Configuration)_.
 * Automatic refreshing implemented, meaning changes on *modloader/* directory are detected automatically while you play.
 * The refresh key _(by default F4)_ has been removed in consequence of the addition of the menu and automatic refreshing.
 * Implemented *weapon.dat* merger and readme reader.
 * Implemented *ar_stats.dat, animgrp.dat, ped.dat, pedstats.dat, pedgrp.dat, cargrp.dat, melee.dat, statdisp.dat, shopping.dat, surface.dat, surfinfo.dat, surfaud.dat, object.dat, procobj.dat and decision makers* merger.
 * Implemented *clothes.dat, furnitur.dat, fonts.dat, roadblox.dat, tracks.dat, tracks2.dat, tracks3.dat and tracks4.dat* overrider.
 * Removed the `-mod` command-line, something better than that will come up later (`-nomods` is still available!).
 * Fixed no date-time in the load game menu screen.
 * Other minor fixes

v0.2.2 (Jan 13 2015)
------------------------
 * The game version *GTA San Andreas 1.0 EU* is now supported.
 * The data merger and overriden plugin *std.data* is back up and running but even better.
 * Just like before, *gta.dat, .ipl, .ide, handling.cfg, carmods.dat plants.dat, water.dat* mergers and *timecyc.dat & popcycle.dat* overriders are supported...
 * Just like before, again, reading data lines from readme files are suppported *(gta.dat, handling.cfg, carmods.dat, carcols.dat, veh_mods.ide, vehicles.ide, peds.ide)*.
 * Seeking our current goals, all those data files and readmes are refreshable in game except for *gta.dat* and it's friends *.ipl and .ide*.
 * Implemented *carcols.dat* merger and readme reader.
 * Implemented caching for mergeable data files so it does not need to merge every time the game loads.
 * Any custom timecyc is now loaded properly on SAMP.
 * Lazy loading of *default.dat* & *gta.dat* which allows a dynamic loading screen progress, fixing it flickering when there's few entries in gta.dat.
 * IPL and IDE files given to modloader don't need to be registered in *gta.dat* to work nor they need to be in the *correct path*.
 * Fixed the loader not creating the default *plugin.ini* file.
 * Fixed Mod Loader FXT hooks mistakenly relying on CLEO hooks.
 * Fixed the fixed COLFILE command which fixes the game's COLFILE gta.dat command being problematic with zero-sized collision files.

 * __Hotfix 1 (Jan 14 2014)__:
    + Fixed some textures being detected as sprites because of it's path.

v0.2.1 (Nov 23 2014)
------------------------
 * The COLFILE command for gta.dat works completly fine now *(SA bug fix)*
 * The size of the streaming buffer gets affected by clothing items now
 * Possible to change the refresh key from *modloader/.data/config.ini*, not from the menu though
 * Effective way to distinguish between *coach.dff* (vehicle) and *coach.dff* (clothing item) implemented
 * The priority system has been reversed, now *100 overrides 1* instead of *1 overrides 100*
 * Mod Loader automatically updates an old ini *(from 0.1.15)* to the new ini format *(as from 0.2.0/0.2.1)*
 * Fixed problems with SAMP introduced by v0.2.0
 * Fixed script sprites not loading properly
 * Fixed ini entries beggining with square brackets not being readen properly
 * Fixed an issues with the std.asi path translator
 * Fixed command line argument `-nomods` not working properly
 
v0.2.0 (Aug 02 2014)
------------------------
 * Rewritten from scratch to be more flexible and powerful
 * Most mods can be refreshed without getting out of the game _(press F4 to refresh, auto-refresh soon)_
 * The `-nomods` command line has been added
 * Extremely descriptive unhandled exception filter
 * Nested *modloader* folders has been removed
 * Mod Loader config files changed a lot **(please delete your current modloader.ini and let Mod Loader create another)**
 * The std.data plugin has been removed, that means .dat/.cfg files aren't handled anymore _(it'll be back soon)_
 * CLEO scripts inside cleo sub folders aren't loaded anymore (this is a fix)
 * Custom missions aren't loaded at startup anymore (this is a fix)
 * So many fixes and changes that I don't even remember about them
 
 * __Hotfix 1 (Aug 03 2014)__:
    + Fixed streamed ifp files not being handled
    + Fixed streamed col/rrr/scm files not being re-registered properly
 
v0.1.15 (Mar 05 2014)
------------------------
 * CLEO scripts injection works with CLEO 4.1 and below
 * CLEO folder don't need to exist on the game base path for CLEO 4.3 to work

v0.1.14 (Mar 04 2014)
------------------------
 * Binary IPLs are automatically detected
 * CLEO files aren't copied and pasted anymore, they're injected into CLEO.asi searches
 * Fixed binary IPLs being handled wrongly
 * Fixed crash on exit if playing SA:MP
 * Fixed new clothing models not working properly
 * Other minor fixes

v0.1.12 (Feb 01 2014)
------------------------
 * Random crashes caused by std-bank fixed
 * The wave loader accepts the extended wave format header now

v0.1.11 (Jan 31 2014)
------------------------
 * Fixed std-img plugin being auto-deleted

v0.1.10 (Jan 31 2014
------------------------
 * The very well known GTA San Andreas bug on modded sound effects has been fixed
 * Implemented sound effects loader (.wav)
 * ASI files linked using the Borland linker won't crash Mod Loader anymore (Thanks to SilentPL).

v0.1.8 (Jan 23 2014)
------------------------
 * Implemented overrider for fonts.dat
 * Text files (.txt) greater than 10KB will not be detect as readme files
 * Anti-flooding for logging the failure on CaptureStackBackTrace

v0.1.7 (Jan 21 2014)
------------------------
 * ASI plugins that rely on external files on run-time works fine now, except for a few exceptions.
 * The loader can handle D3D9 hooks
 * Implemented file mixer for water.dat
 * Implemented file overrider for roadblox.dat and tracks%d.dat
 * Sacky's Limit Adjuster removed from Mod Loader
 * The basic IDE files (vehicles.ide, peds.ide, default.ide) do not need to be inside a data folder anymore to be detected
 * Fixed a immediate crash that happens when a .img files gets overridden
 * Fixed wheels line for veh_mods.ide not getting read properly on a readme file
 * Fixed the conflict between the clothing file COACH and the vehicle file COACH
 * Other minor fixes

v0.1.6 (Jan 12 2014)
------------------------
 * Implemented file mixer for plants.dat
 * Clothes models now works properly... Really...

v0.1.5 (Jan 11 2014)
------------------------
 * The plugin std-cleo now handles .cleo plugins by itself
 * The plugin std-cleo now handles .fxt files by itself
 * Data mixing is working fine now
 * Files are now read properly when there's a UTF-8 BOM in it
 * Special character models now loads properly
 * Clothes models now works properly
 * Collision files now works properly
 * Other minor fixes

v0.1.0 (Dec 30 2013)
------------------------
 * First public release

