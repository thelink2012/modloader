/*
 * Copyright (C) 2013-2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#include <stdinc.hpp>
#include "loader.hpp"

// Not thread-safe
static FILE* logfile = 0;

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
        this->TruncateLog();
        
        // Write the log header, with version number and IsDev information
        Log("========================== Mod Loader %d.%d.%d %s==========================\n",
                MODLOADER_VERSION_MAJOR, MODLOADER_VERSION_MINOR, MODLOADER_VERSION_REVISION,
                MODLOADER_VERSION_ISDEV? "Development Build " : "");
    }
}

/*
 *  Loader::TruncateLog
 *      Clears the log file
 */
void Loader::TruncateLog()
{
    // Clears the ammount of bytes written...
    auto path = this->gamePath + "modloader/modloader.log";
    numBytesInLog = 0;
    
    if(logfile == nullptr)
    {
        // Log file hadn't been open before, open it now.
        logfile = fopen(path.c_str(), "w");
        if(!logfile) Error("Failed to open the log file");
    }
    else
    {
        // Reopen the file for truncation
        if((logfile = freopen(path.c_str(), "w", logfile)) == 0)
        {
            // Wuut, we couldn't do it?
            Error("Failed to truncate log! Closing it for safeness %s.", strerror(errno) );
            CloseLog();
        }
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
        loader.numBytesInLog += vfprintf(logfile, msg, va) + 2;
        fputc('\n', logfile);
        if(loader.bImmediateFlush) fflush(logfile);

        // Truncate log if it's too big
        if(loader.numBytesInLog >= loader.maxBytesInLog)
            loader.TruncateLog();
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
 *  Loader::FatalError
 *      Shows a message box with the content of ("Fatal Error: ", msg, ...) then terminates application
 *      This should be called only on very unhandleable cases.
 */
void Loader::FatalError(const char* msg, ...)
{
    char buffer[1024];
    
    // Print message into buffer
    va_list va; va_start(va, msg);
    vsprintf(buffer, msg, va);
    va_end(va);
    strcat(buffer, "\n\n"
                    "You can either continue the program execution or terminate it. Continuation of the execution WILL cause problems in-game or with Mod Loader itself!!!\n"
                    "Do you want to continue program execution? It is NOT recommended to do so.");
    
    // Fatal error, continuing the execution may be a problem
    if(MessageBoxA(NULL, buffer, "Mod Loader", MB_ICONERROR | MB_YESNO) == IDNO)
        std::terminate();
}
