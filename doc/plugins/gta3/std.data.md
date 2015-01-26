gta3.std.data
=========================================================================
 + __Author__:   LINK/2012 (<dma_2012@hotmail.com>)
 + __Priority__: 54

*************************************************************************

__Description__:
 This plugin is responsible for merging and overriding data files (.dat, .cfg, .ide, .ipl/.zon),
 it also trying to match a data line in readme files.

The following data files are currently supported:

| Data File Name | Override | Merge | Readme |
|----------------|--------- |-------|--------|
| gta.dat        |    X     |   X   |   X    |
| .ide files     |    X     |   X   |   ¹    |
| .ipl files     |    X     |       |        |
| carmods.dat    |    X     |   X   |   X    |
| carcols.dat    |    X     |   X   |   X    |
| handling.cfg   |    X     |   X   |   X    |
| plants.dat     |    X     |   X   |        |
| water.dat      |    X     |   X   |        |
| timecyc.dat    |    X     |       |        |
| popcycle.dat   |    X     |       |        |
| cargrp.dat     |    X     |   X   |        |
| pedgrp.dat     |    X     |   X   |        |
| object.dat     |    X     |   X   |        |

 + ¹ reading ide lines from readmes is supported for *cars*, *peds* and tunning parts in *objs* section _(i.e. vehicles.ide, peds.ide, veh_mods.ide)_
    

__Additional Notes__:
   * IDE and IPL that doesn't have a matching gta.dat entry are loaded anyway
   * IDE and IPL files usually needs to be structured the same way as it's gta.dat demands
      (i.e. IPL DATA/MAPS/TEST.IPL should be structured like modloader/My Map/data/maps/test.ipl)
      although this is **NOT** necessary anymore since Mod Loader 0.2.x
   * Matching for data lines in readme files are supported for some data files
   * This makes the loading screen dynamic, removing the flickering when there are too few gta.dat entries (usually on TCs)
   * This performs caching of the data files and readmes in the modloader directory, giving a better performance in
     not needing to merge data every time you load the game.



