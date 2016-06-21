gta3.std.data
=========================================================================
 + __Author__:   LINK/2012 (<dma_2012@hotmail.com>)
 + __Priority__: 50
 + __Game__: III, Vice City, San Andreas

*************************************************************************

__Description__:
 This plugin is responsible for merging and overriding data files (.dat, .cfg, .ide, .ipl/.zon),
 it also trying to match a data line in readme files.

The following data files are currently supported:

| Data File Name | Override | Merge | Readme |
|----------------|--------- |-------|--------|
| ar_stats.dat   |    X     |   X   |        |
| animgrp.dat    |    X     |   X   |        |
| gta.dat        |    X     |   X   |   X    |
| *.ide          |    X     |   X   |   ¹    |
| *.ipl/*.zon    |    X     |       |        |
| *.ped/*.grp    |    X     |   X   |        |
| clothes.dat    |    X     |       |        |
| carcols.dat    |    X     |   X   |   X    |
| carmods.dat    |    X     |   X   |   X    |
| handling.cfg   |    X     |   X   |   X    |
| plants.dat     |    X     |   X   |        |
| melee.dat      |    X     |   X   |        |
| water.dat      |    X     |   X   |        |
| timecyc.dat    |    X     |       |        |
| popcycle.dat   |    X     |       |        |
| cargrp.dat     |    X     |   X   |        |
| pedgrp.dat     |    X     |   X   |        |
| ped.dat        |    X     |   X   |        |
| pedstats.dat   |    X     |   X   |        |
| statdisp.dat   |    X     |   X   |        |
| shopping.dat   |    X     |   X   |        |
| object.dat     |    X     |   X   |        |
| surface.dat    |    X     |   X   |        |
| surfinfo.dat   |    X     |   X   |        |
| surfaud.dat    |    X     |   X   |        |
| furnitur.dat   |    X     |       |        |
| fonts.dat      |    X     |       |        |
| roadblox.dat   |    X     |       |        |
| tracks.dat     |    X     |       |        |
| tracks2.dat    |    X     |       |        |
| tracks3.dat    |    X     |       |        |
| tracks4.dat    |    X     |       |        |
| weapon.dat     |    X     |   X   |   X    |
| procobj.dat    |    X     |   X   |        |
| stream.ini     |    X     |   X   |        |
| cullzone.dat   |    X     |       |        |
| fistfite.dat   |    X     |   X   |        |
| particle.cfg   |    X     |   X   |        |
| waterpro.dat   |    X     |       |        |
| flight.dat     |    X     |       |        |
| flight2.dat    |    X     |       |        |
| flight3.dat    |    X     |       |        |
| flight4.dat    |    X     |       |        |


 + ¹ reading ide lines from readmes is supported for *cars*, *peds* and tunning parts in *objs* section _(i.e. vehicles.ide, peds.ide, veh_mods.ide)_
    

__Additional Notes__:
   * Decision files (.ped/.grp) mustbe in a *decision/allowed* directory to work
   * IDE and IPL that doesn't have a matching gta.dat are **NOT** loaded.
   * IDE and IPL files usually needs to be structured the same way as it's gta.dat demands.
      (i.e. IPL DATA/MAPS/TEST.IPL should be structured like modloader/My Map/data/maps/test.ipl)
      although this is **NOT** necessary anymore since Mod Loader 0.2.x
   * This makes the loading screen dynamic, removing the flickering when there are too few gta.dat entries (usually on TCs)
   * Matching for data lines in readme files are supported for some data files
   * This performs caching of the data files and readmes in the modloader directory, giving a better performance in
     not needing to merge data every time you load the game.
