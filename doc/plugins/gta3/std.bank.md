gta3.std.bank
=========================================================================
 + __Author__:   LINK/2012 (<dma_2012@hotmail.com>)
 + __Priority__: 50
 + __Game__: San Andreas

*************************************************************************

__Description__:

 This plugin is responsible for loading sound effects that would normally go into sfxpak files.
 It can either load the effects as pure wave or in SFXPaks itself.
 It can also handle SFX CONFIG files, as in the main game audio/ folder.
 

__Usage__:

 For wave files, the directory must be structured as: `<SFXPak_Name>/Bank_<X>/sound_<Y>.wav `

 For example `modloader/My Sounds/GENRL/Bank_007/sound_001.wav`;
 If SFXPak_Name is not present, it'll use GENRL, but don't rely on this behaviour, it's bad! The wave file **must** be in Mono PCM 16 bits format to work!!!
 
 Other files follow the usual installation convention.
