gta3.std.stream
=========================================================================
 + __Author__:   LINK/2012 (<dma_2012@hotmail.com>)
 + __Priority__: 52
 + __Game__: III, Vice City, San Andreas

*************************************************************************

__Description__:
 This plugin is responsible for loading streamed resources, that would normally go into *.img* files.
 That is, it handles:
 
  * Model Files (dff)
  * Texture Dictionaries (txd)
  * Collision Archieves (col)
  * Binary IPLs (ipl)
  * Nodes Files (nodes%d.dat)
  * Animation Files (ifp)
  * Vehicle Recordings (rrr)
  * Streamed Scripts (scm)

It can also handle *img* files itself and override it's loading from the one in the default location.

It also takes care or does the following:
  * Takes care of ped.ifp overriding.
  * Detouring of IMG/CDIMAGE, COLFILE, TEXDICTION, MODELFILE and HIERFILE from gta.dat.
  * Fixes the COLFILE command in GTA San Andreas not working properly.

__Usage__:

 You can either use it in the usual way, or create a directory with a img extension and drop the files inside it.
 
 _Special Notes_:
 
  * Newly added clothes (or coach cloth) must go inside a folder named player.img (e.g. *modloader/my new clothes/player.img/coach.dff*)
 * nodes%d.dat files will get handled only if they are inside a folder with an *img* extension (e.g. *modloader/my nodes/gta3.img/nodes1.dat*)

