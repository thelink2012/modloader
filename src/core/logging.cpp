/*
 * Copyright (C) 2013-2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#include "loader.hpp"

static FILE* logfile = 0;
extern int LogException(char* buffer, LPEXCEPTION_POINTERS pException);


// Not thread-safe


/*
 *  Loader::OpenLog
 *      Opens the logging stream
 */
void Loader::OpenLog()
{
    // If the stream isn't open yet, open it
    if(!logfile)
    {
        // Opens the logging stream
        logfile = fopen("modloader/modloader.log", "w");

        // Write the log header, with version number and IsDev information
        Log("========================== Mod Loader %d.%d.%d %s==========================\n",
                MODLOADER_VERSION_MAJOR, MODLOADER_VERSION_MINOR, MODLOADER_VERSION_REVISION,
                MODLOADER_VERSION_ISDEV? "Development Build " : "");
    }
}

/*
 *  Loader::CloseLog
 *      Closes the logging stream
 */
void Loader::CloseLog()
{
    // If the logging stream is open...
    if(logfile)
    {
        // ...close it
        fclose(logfile);
        logfile = 0;
    }
}


/*
 *  Loader::Log
 *      Logs the message (msg, ..., '\n') into the logging stream
 */
void Loader::Log(const char* msg, ...)
{
    va_list va; va_start(va, msg);
    vLog(msg, va);
    va_end(va);
}

/*
 *  Loader::Log
 *      Logs the message (msg, va, '\n') into the logging stream
 */
void Loader::vLog(const char* msg, va_list va)
{
    if(logfile)
    {
        vfprintf(logfile, msg, va);
        fputc('\n', logfile);
        if(loader.bImmediateFlush) fflush(logfile);
    }
}


/*
 *  Loader::Error
 *      Shows a message box with the content of (msg, ...)
 */
void Loader::Error(const char* msg, ...)
{
    char buffer[1024];
    
    // Print message into buffer
    va_list va; va_start(va, msg);
    vsprintf(buffer, msg, va);
    va_end(va);
    
    // Show message box with the message
    MessageBoxA(NULL, buffer, "Mod Loader", MB_ICONERROR); 
}

/*
 *  Loader::LogException
 *      Logs a exception into the logging stream
 */
void Loader::LogException(void* pExceptionPointers)
{
    char buffer[1024];
    if(::LogException(buffer, (LPEXCEPTION_POINTERS)pExceptionPointers))
        Log(buffer);
}

