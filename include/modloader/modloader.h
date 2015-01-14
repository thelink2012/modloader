/* 
 * Mod Loader
 * Created by LINK/2012 <dma_2012@hotmail.com>
 * 
 *  Mod Loader plugin interface
 *      The interface is extremly simple, you don't even have to link with modloader.
 *      The only thing you are requiered to do is export a 'GetPluginData' function (see below for the prototype).
 *      Put your plugin at '/modloader/.data/plugins' folder
 * 
 * 
 *  This source code is offered for use in the public domain. You may
 *  use, modify or distribute it freely.
 *
 *  This code is distributed in the hope that it will be useful but
 *  WITHOUT ANY WARRANTY. ALL WARRANTIES, EXPRESS OR IMPLIED ARE HEREBY
 *  DISCLAIMED. This includes but is not limited to warranties of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE
 * 
 */
#ifndef MODLOADER_H
#define	MODLOADER_H
#pragma once

#include <stddef.h>
#include <stdarg.h>
#include <stdint.h>

#ifdef	__cplusplus
extern "C" {
#endif

/* Version */
#define MODLOADER_VERSION_MAJOR         0
#define MODLOADER_VERSION_MINOR         2
#define MODLOADER_VERSION_REVISION      2
#ifdef NDEBUG
#define MODLOADER_VERSION_ISDEV         0
#else
#define MODLOADER_VERSION_ISDEV         1
#endif


/**************************************
 *    CONSTANTS
 **************************************/

/* Check file result */
#define MODLOADER_BEHAVIOUR_NO          0
#define MODLOADER_BEHAVIOUR_YES         1
#define MODLOADER_BEHAVIOUR_CALLME      2

/* modloader_file_t flags */
#define MODLOADER_FF_IS_DIRECTORY   1
    

/**************************************
 *    COMMON DATA TYPES
 **************************************/
    
/* Forwarding */
struct modloader_t;
struct modloader_plugin_t;


/*
 * modloader_mod_t
 *      This structure represents a mod
 */
typedef struct
{
    uint64_t    id;         // Unique mod id
    uint32_t    priority;   // Mod priority
    uint32_t    _pad1;
    
} modloader_mod_t;


/*
 * modloader_file_t
 *      This structure represents a file
 */
typedef struct
{
    uint32_t            flags;          /* File flags */
    const char*         buffer;         /* Pointer to the file buffer... that's the file path relative to game dir  */
    uint8_t             pos_eos;        /* The null terminator position (length of the string)  */
    uint8_t             pos_filedir;    /* The position of the filepath relative to the mod folder (e.g. "modloader/my mod/stuff/a.dat" -> "stuff/a.dat") */
    uint8_t             pos_filename;   /* The position of the file name  */
    uint8_t             pos_filext;     /* The position of the file extension  */
    uint32_t            hash;           /* The filename hash (as in "modloader/util/hash.hpp" */
    uint32_t            _rsv1;          /* Reserved */
    modloader_mod_t*    parent;         /* The mod owner of this file */
    uint64_t            size;           /* Size of the file in bytes  */
    uint64_t            time;           /* File modification time  */
                                        /*  (as FILETIME, 100-nanosecond intervals since January 1, 1601 UTC) */
    uint64_t            behaviour;      /* The file behaviour */

} modloader_file_t;






/**************************************
 *    THE LOADER EXPORTS
 **************************************/

/*
 * Log
 *      Logs something into the modloader log file
 */
typedef void (*modloader_fLog)(const char* msg, ...);
typedef void (*modloader_fvLog)(const char* msg, va_list va);

/*
 * Error
 *      Displays a message box with a error message
 *      Log may suit your needs.
 */
typedef void (*modloader_fError)(const char* errmsg, ...);


/* ---- Interface ---- */
typedef struct modloader_t
{
    const char* gamepath;       /* game path */
    const char* _rsvc;          /* (deprecated - reserved) */
    const char* commonappdata;  /* fullpath to a "modloader/" directory in the %ProgramData% directory */
    const char* localappdata;   /* fullpath to a "modloader/" directory in the "%LocalAppData% directory */
    const char* _rsv0[2];       /* Reserved */

    uint32_t   _rsv1[4];        /* Reserved */
    uint8_t    has_game_started;
    uint8_t    has_game_loaded;
    uint8_t    _rsv3;           /* Reserved */
    uint8_t    _rsv4;           /* Reserved */

    modloader_fLog          Log;
    modloader_fvLog         vLog;
    modloader_fError        Error;
    
} modloader_t;






/**************************************
 *    THE PLUGIN EXPORTS
 **************************************/

/*
        You have to export this function!
        Then you shall JUST (and JUST) fill 'major', 'minor' and 'revision' with  the values of 
        MODLOADER_VERSION_MAJOR, MODLOADER_VERSION_MINOR,  MODLOADER_VERSION_REVISION macros.
        If the version checking goes okay, GetPluginData will get called.
*/
typedef void (*modloader_fGetLoaderVersion)(uint8_t* major, uint8_t* minor, uint8_t* revision);

/*
        You have to export this function!
        Then you shall JUST (and JUST) fill 'data' with the plugin information.
        If everything goes okay, data->OnStartup will get called.
        Note that data->modloader is already set here :)
*/
typedef void (*modloader_fGetPluginData)(modloader_plugin_t* data);

/*
 *  GetVersion
 *      Get plugin version string (e.g. "1.4")
 *      This will show up once, when the plugin get loaded
 *      @data: The plugin data   
 */
typedef const char* (*modloader_fGetVersion)(modloader_plugin_t* data);

/*
 *  GetAuthor
 *      Get plugin author (e.g. "My Name")
 *      This will show up once, when the plugin get loaded
 *      @data: The plugin data 
 */
typedef const char* (*modloader_fGetAuthor)(modloader_plugin_t* data);


/*
 *  OnStartup
 *      Plugin started up, this happens before the the game engine starts.
 *      @data: The plugin shall fill 'data' parameter with it's information.
 *      @return 0 on success and 1 on failure;
 */
typedef int (*modloader_fOnStartup)(modloader_plugin_t* data);

/*
 *  OnShutdown
 *      Plugin shut dowing
 *      @data: The plugin data
 *      @return 0 on success and 1 on failure;
 */
typedef int (*modloader_fOnShutdown)(modloader_plugin_t* data);

/*
 * GetBehaviour
 *      Gets the behaviour of this plugin in relation to the specified file
 *      @data: The plugin data   
 *      @return MODLOADER_RELATIONSHIP_NONE    for no relationship
 *              MODLOADER_RELATIONSHIP_STRONG  for strong relationship, this plugin will handle the file installation
 *              MODLOADER_RELATIONSHIP_WEAK    for weak relationship, this plugin won't handle the file installation but will receive it at Install, Uninstall and so on.
 *              
 * 
 */
typedef int (*modloader_fGetBehaviour)(modloader_plugin_t* data, modloader_file_t* file);

/*
 * InstallFile
 *      Called to install a file previosly checked as 'yes'
 *      @data: The plugin data
 *      @return 0 on success and 1 on failure;
 */
typedef int (*modloader_fInstallFile)(modloader_plugin_t* data, const modloader_file_t* file);


/*
 * ReinstallFile
 *      Called to reinstall a file  previosly installed, the file was updated
 *      @data: The plugin data
 *      @return 0 on success and 1 on failure;
 */
typedef int (*modloader_fReinstallFile)(modloader_plugin_t* data, const modloader_file_t* file);


/*
 * UninstallFile
 *      Called to uninstall a file previosly installed with 'InstallFile'
 *      @data: The plugin data
 *      @return 0 on success and 1 on failure;
 */
typedef int (*modloader_fUninstallFile)(modloader_plugin_t* data, const modloader_file_t* file);


/*
 * Update
 *      Update is called after a serie of install/uninstalls, maybe you need a delayed refresh
 *      @data: The plugin data
 */
typedef void (*modloader_fUpdate)(modloader_plugin_t* data);




/* ---- Interface ---- Should be compatible with all versions of modloader.asi */
typedef struct modloader_plugin_t
{
    // Data to be set by Mod Loader itself, read-only data for plugins!
    struct  
    {
        uint8_t major, minor, revision, _pad0;
        void *pThis, *pModule;                      /* this pointer and HMODULE */
        const char *name, *author, *version;        /* Plugin info */
        modloader_t* loader;                        /* Modloader pointer  */
        uint8_t has_started;                        /* Determines whether the plugin has started up successfully */
        uint8_t _pad1[3];                           /* Reserved */
    };
    
    /* Userdata, set it to whatever you want */
    void* userdata;
    
    /* Extension table, set it to a pointer of extensions that this plugin handles */
    const char** extable;   /* Can be null if extable_len is equal to zero */
    size_t extable_len;     /* The length of the extension table */
    
    /*
     * The plugin priority, normally this is set outside the plugin in a config file, not recommend to touch this value.
     * Mod Loader sets this to the default priority "50"; "0" means ignore this plugin.
     */
    int priority;

    /* Callbacks */
    modloader_fGetAuthor        GetAuthor;
    modloader_fGetVersion       GetVersion;
    modloader_fOnStartup        OnStartup;
    modloader_fOnShutdown       OnShutdown;
    modloader_fGetBehaviour     GetBehaviour;
    modloader_fInstallFile      InstallFile;
    modloader_fReinstallFile    ReinstallFile;
    modloader_fUninstallFile    UninstallFile;
    modloader_fUpdate           Update;

} modloader_plugin_t;







#ifdef	__cplusplus
}
#endif

#endif
