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
 * Fixed command line argument -nomods not working properly
 
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

v0.1 (Dec 30 2013)
------------------------
 * First public release

