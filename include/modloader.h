/* 
 * San Andreas Mod Loader
 * Created by LINK/2012 <dma_2012@hotmail.com>
 * 
 *  Mod Loader plugin interface
 *      The interface is extremly simple, you don't even have to link with modloader.
 *      The only thing you are requiered to do is export a 'GetPluginData' function (see below for the prototype).
 *      Put your plugin at '/modloader/.data/plugins' folder
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

// TODO UPDATE DOCUMENTATION (here and at doc/)

#ifdef	__cplusplus
extern "C" {
#endif

/* Version */
#define MODLOADER_VERSION_MAJOR         0
#define MODLOADER_VERSION_MINOR         2
#define MODLOADER_VERSION_REVISION      0
#ifdef NDEBUG
#define MODLOADER_VERSION_ISDEV         0
#else
#define MODLOADER_VERSION_ISDEV         1
#endif


/**************************************
 *    CONSTANTS
 **************************************/

/* Check file result */
#define MODLOADER_CHECK_NO          0
#define MODLOADER_CHECK_YES         1
#define MODLOADER_CHECK_CALL_ME     2

/* modloader_file_t flags */
#define MODLOADER_FF_IS_DIRECTORY   1
    

/**************************************
 *    COMMON DATA TYPES
 **************************************/
    
/* Forwarding */
struct modloader_t;
struct modloader_plugin_t;




/*
 * modloader_file_serial_t
 *      This structure represents a file
 */
typedef struct
{
    uint32_t        flags;                  /* File flags */
    const char*     buffer;                 /* Pointer to the file buffer... that's the file path relative to game dir  */
    uint8_t         pos_eos;                /* The null terminator position (length of the string)  */
    uint8_t         pos_filepath;           /* The position of the filepath relative to the mod folder (e.g. "modloader/my mod/stuff/a.dat" -> "stuff/a.dat") */
    uint8_t         pos_filename;           /* The position of the file name  */
    uint8_t         pos_filext;             /* The position of the file extension  */
    uint32_t        _rsv2;                  /* Reserved (Padding)  */
    uint64_t        size;                   /* Size of the file in bytes  */
    uint64_t        time;                   /* File modification time  */
                                            /*  (as FILETIME, 100-nanosecond intervals since January 1, 1601 UTC) */
    
    uint32_t        file_id;
    uint32_t        mod_id;
    
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

/*
 * NewChunkLoaded
 *      Tells modloader to increase the loading bar a bit
 *      PS: You must setup how much you will call it at modloader_plugin_data.loadbar_chunks
 */
typedef void (*modloader_fNewChunkLoaded)( );


/* ---- Interface ---- */
typedef struct modloader_t
{
    const char* gamepath;   /* game path */
    const char* cachepath;  /* cache path, normally "modloader/.data/cache" */
    
    modloader_fLog              Log;
    modloader_fvLog             vLog;
    modloader_fError            Error;
    
} modloader_t;




/**************************************
 *    THE PLUGIN EXPORTS
 **************************************/

/*
        You have to export the function (with C naming, extern "C"):
                void GetPluginData(modloader_plugin_t* data);
        Then you shall JUST (and JUST) fill 'data' with the plugin information.
        If everything goes okay (version checking, etc), data->OnStartUp will be called...
        Note data->modloader is already set here.
*/
typedef void (*modloader_fGetPluginData)(modloader_plugin_t* data);

/*
 *  GetName
 *      Get plugin name (e.g. "X Nope Loader")
 *      @data: The plugin data   
 */
typedef const char* (*modloader_fGetName)(modloader_plugin_t* data);

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
 *  OnStartUp
 *      Plugin started up, this happens before the the game engine starts.
 *      @data: The plugin shall fill 'data' parameter with it's information.
 *      @return 0 on success and 1 on failure;
 */
typedef int (*modloader_fOnStartup)(modloader_plugin_t* data);

/*
 *  OnShutdown
 *      Called when the plugin gets unloaded, this probably happens when the game closes
 *      @data: The plugin data
 *      @return 0 on success and 1 on failure;
 */
typedef int (*modloader_fOnShutdown)(modloader_plugin_t* data);

/*
 * CheckFile
 *      Called to check if a specific file is to be processed by this plugin.
 *      @data: The plugin data   
 *      @return 0 for 'yes' and 1 for 'no';
 */
typedef int (*modloader_fCheckFile)(modloader_plugin_t* data, const modloader_file_t* file);

/*
 * ProcessFile
 *      Called to process a file previosly checked as 'yes'
 *      @data: The plugin data
 *      @return 0 on success and 1 on failure;
 */
typedef int (*modloader_fProcessFile)(modloader_plugin_t* data, const modloader_file_t* file);

/*
 * InstallFile
 *      Called to install a file previosly checked as 'yes'
 *      @data: The plugin data
 *      @return 0 on success and 1 on failure;
 */
typedef int (*modloader_fInstallFile)(modloader_plugin_t* data, const modloader_file_t* file);

/*
 * UninstallFile
 *      Called to uninstall a file previosly installed with 'InstallFile'
 *      @data: The plugin data
 *      @return 0 on success and 1 on failure;
 */
typedef int (*modloader_fUninstallFile)(modloader_plugin_t* data, const modloader_file_t* file);

/*
 *  PosProcess
 *      Called after all files have been processed (with ProcessFile)
 *      @data: The plugin data
 *      @return: 0 on success and 1 on failure
 */
typedef int (*modloader_fPosProcess)(modloader_plugin_t* data);

/*
 *  OnSplash
 *      Called when the splash screen written "GTA San Andreas" and copyright notices shows up.
 *      This will be the first call after the splash screen.
 *      @data: The plugin data
 *      @return: 0 on success and 1 on failure
 * 
 */
typedef int (*modloader_fOnSplash)(modloader_plugin_t* data);

/*
 *  OnLoad
 *      Called on the loading screen (with a loading bar, etc).
 *      This will be the first call after the loading screen starts.
 *      You must make your time-consuming tasks and calls to NewChunkLoaded here on this callback
 *      @data: The plugin data
 *      @return: 0 on success and 1 on failure
 * 
 */
typedef int (*modloader_fOnLoad)(modloader_plugin_t* data);


/*
 *  OnReload
 *      Called when the user enters the game (on newgame/loadgame)
 *      @data: The plugin data
 *      @return: 0 on success and 1 on failure
 * 
 */
typedef int (*modloader_fOnReload)(modloader_plugin_t* data);



/* ---- Interface ---- */
typedef struct modloader_plugin_t
{
    /*
     * Set those values to MODLOADER_VERSION_*, e.g. MODLOADER_VERSION_MAJOR. Ignore pad0!
     * Those values will be helpful to know if your plugin is compatible with the current version of modloader
     */
    uint8_t major, minor, revision, _pad0;
    
    // Data to be set by modloader itself, read-only data for plugins!
    struct  
    {
        void *pThis, *pModule;                      /* this pointer and HMODULE */
        const char *name, *author, *version;        /* Plugin info */
        modloader_t* modloader;                     /* Modloader pointer  */
        uint8_t checked, _pad1[3];
    };
    
    /* Userdata, set it to whatever you want */
    void* userdata;
    
    /* Extension table, set it to a pointer of extensions that this plugin handles */
    const char** extable;   /* Can be null if extable_len is equal to zero */
    size_t extable_len;     /* The length of the extension table */
    
    /*
     * The plugin priority, normally this is set outside the plugin in a config file, not recommend to touch this value.
     * Modloader sets this to the default priority "50"; "0" means ignore this plugin.
     */
    int priority;
    
    /* Callbacks */ // TODO DOCUMENT
    modloader_fGetName          GetName;
    modloader_fGetAuthor        GetAuthor;
    modloader_fGetVersion       GetVersion;
    modloader_fOnStartup        OnStartup;
    modloader_fOnShutdown       OnShutdown;
    modloader_fCheckFile        CheckFile;
    modloader_fProcessFile      ProcessFile;    // TODO REMOVE
    modloader_fPosProcess       PosProcess;     // TODO REMOVE
    modloader_fInstallFile      InstallFile;
    modloader_fUninstallFile    UninstallFile;
    modloader_fOnSplash         OnSplash;       // TODO REMOVE
    modloader_fOnLoad           OnLoad;         // TODO REMOVE
    modloader_fOnReload         OnReload;
    
} modloader_plugin_t;



#ifdef	__cplusplus
}
#endif

#endif
