/* 
 * San Andreas Mod Loader Utilities Headers
 * Created by LINK/2012 <dma_2012@hotmail.com>
 * 
 *  This file provides helpful functions for plugins creators.
 * 
 *  This source code is offered for use in the public domain. You may
 *  use, modify or distribute it freely.
 *
 *  This code is distributed in the hope that it will be useful but
 *  WITHOUT ANY WARRANTY. ALL WARRANTIES, EXPRESS OR IMPLIED ARE HEREBY
 *  DISCLAIMED. This includes but is not limited to warranties of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */
#ifndef MODLOADER_UTIL_PATH_HPP
#define	MODLOADER_UTIL_PATH_HPP

/*
 *  TODO this header needs revision
 */

#include <string>
#include <cstring>
#include <windows.h>
#include <modloader.hpp>
#include <modloader_util_container.hpp>

namespace modloader
{
    static const char* szNullFile = "NUL";    // "/dev/null" on POSIX systems
    static const char cNormalizedSlash = '\\';
    std::string NormalizePath(std::string path);
    
    inline std::string& MakeSureStringIsDirectory(std::string& dir, bool touchEmpty = true)
    {
        if(dir.empty()) { if(touchEmpty) dir = ".\\"; }
        else if(dir.back() != '\\')
        { dir.push_back('\\'); }
        
        return dir;
    }
    
    inline std::string GetFilePath(const ModLoaderFile& file)
    {
        return (std::string(file.modpath) + file.filepath);
    }
    

    
    /*
     * ForeachFile
     *      Iterates on all files in a directory, files beggining with '.' will be ignored.
     *      @dir: Directory to search at
     *      @mask: Search mask
     *      @bRecursive: Recursive search?
     *      @cb: Callback to call on each file
     */
    template<class T>
    inline bool ForeachFile(std::string dir, std::string mask, bool bRecursive, T cb)
    {
        HANDLE hSearch;
        WIN32_FIND_DATAA fd;
        ModLoaderFile mf;
        memset(&fd, 0, sizeof(fd));
        memset(&mf, 0, sizeof(mf));
        
        std::string filepath_buffer;
        filepath_buffer.reserve(100);
        
        MakeSureStringIsDirectory(dir, false);
        
        // Opens the search
        if((hSearch = FindFirstFileA((dir + mask).c_str(), &fd)) == INVALID_HANDLE_VALUE)
            return true;
        

        // Iterate on all files on this directory
        do
        {
            //  Ignore files beggining with '.' (including "." & "..")
            if(fd.cFileName[0] != '.' && fd.cFileName[0] != '*')
            {
                // Setup mf structure  
                mf.filename = (fd.cFileName);  // Just the filename
                mf.is_dir   = (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)!=0;  // Is it a directory?
                mf.recursion = bRecursive && mf.is_dir;                             // cb() can set this to false if it don't want to go recursive
                if(mf.filext = strrchr(mf.filename, '.'))
                    mf.filext = mf.filext + 1;
                else
                    mf.filext = "";
                
                // get dir+filename, if dir append '\\'
                filepath_buffer = dir + mf.filename;
                if(mf.is_dir) MakeSureStringIsDirectory(filepath_buffer);
                
                // Finish setupping mf structure
                mf.filepath = filepath_buffer.c_str();          // Filename and directory related to the first call to ForeachFile
                mf.filepath_len = filepath_buffer.length();     // Length, may be helpful for plugins analyzes
     
                // Call cb() and go recursive if asked to...
                if(!cb(mf)
                ||(mf.recursion && !ForeachFile(dir + mf.filename, mask, bRecursive, cb)))
                {
                    FindClose(hSearch);
                    return false;
                }
            }
        }
        while(FindNextFile(hSearch, &fd));   // Next...

        // Done
        FindClose(hSearch);  
        return true;
    }

    template<class T>
    inline bool ForeachFile(std::string mask, bool bRecursive, T cb)
    {
        return ForeachFile<T>("", mask, bRecursive, cb);
    }
    
    
    /*
     *      Checks if a file is inside an named folder, or just inside it
     *      @file File path to check
     *      @bJust  Returns true only if just right inside the folder
     *      @folder Folder path
     * 
     *      XXX this func can be better optimized
     */
    inline bool IsFileInsideFolder(std::string file, bool bJust, std::string folder)
    {
        file = NormalizePath(file);
        folder= NormalizePath(folder);
        
        if(bJust)
        {
            //
            size_t last = file.find_last_of(cNormalizedSlash);
            if(last != file.npos && last >= folder.length())
            {
                // Find pos to start the comparation and number chars to compare
                size_t pos = last - folder.length();
                size_t n  = last - pos;
                // Get result
                return (file.compare(pos, n, folder) == 0);
            }
            return false;
        }
        else
        {
            // Simple enougth, just find folder in file
            return (file.find(folder) != file.npos);
        }
    }
    
    /*
     *  GetLastPathComponent
     *      @path: Path to get the last component from
     *      @return: Returns the last path component position in the string
     */
    inline std::string::size_type GetLastPathComponent(std::string path)
    {
        PopLastCharIf(path, '/');
        PopLastCharIf(path, '\\');
        size_t pos = path.find_last_of("/\\");
        return (pos == path.npos? 0 : pos + 1);
    }
    
    
    /*
     *  IsFileExtension
     *      @str: File
     *      @ext: Extension
     */
    inline bool IsFileExtension(const char* str_ext, const char* ext)
    {
        return (!strcmp(str_ext, ext, false));
    }
    
    
    /*
     *  NormalizePath
     *      Normalizates a path string, so for example:
     *          "somefolder/something" will output "SOMEFOLDER\\SOMETHING"
     *          "SOMEfoldER/something/" will output "SOMEFOLDER\\SOMETHING"
     *          "somefolder\\something" will output "SOMEFOLDER\\SOMETHING"
     *          etc
     */
    inline std::string NormalizePath(std::string path)
    {
        // Replace all '/' with '\\', and toupper the strings (because Windows paths are case insensitive)
        toupper(ReplaceChar(path, '/', '\\'));
        // We don't want a slash at the end of the folder path
        PopLastCharIf(path, '\\');
        TrimString(path);
        return path;
    }
    
    
    /*
     * IsDirectoryA
     *      WinAPI-like function to check if a directory exists
     *      @szPath: Directory to check
     */
    inline BOOL IsDirectoryA(LPCTSTR szPath)
    {
      DWORD dwAttrib = GetFileAttributes(szPath);
      return (dwAttrib != INVALID_FILE_ATTRIBUTES && 
             (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
    }
    
    /*
     * IsFileA
     *      WinAPI-like function to check if a file or directory exists
     *      @szPath: Directory to check
     */
    inline BOOL IsPathA(LPCTSTR szPath)
    {
      DWORD dwAttrib = GetFileAttributes(szPath);
      return (dwAttrib != INVALID_FILE_ATTRIBUTES);
    }

    /*
     * MakeSureDirectoryExistA
     *      WinAPI-like function to make sure a directory exists, if not, create it
     *      @szPath: Directory to check
     */
    inline BOOL MakeSureDirectoryExistA(LPCTSTR szPath)
    {
        if(!IsDirectoryA(szPath))
        {
            CreateDirectoryA(szPath, NULL);
            return FALSE;
        }
        return TRUE;
    }
    
    inline BOOL CopyDirectoryA(LPCTSTR szFrom, LPCTSTR szTo)
    {
        // TODO error checking
        if(CreateDirectoryA(szTo, NULL))
        {
            ForeachFile(szFrom, "*.*", false, [&szFrom, &szTo](ModLoaderFile& file)
            {
                CHAR szToFile[MAX_PATH], szFromFile[MAX_PATH];
                const char* pPath = file.filename;
                
                sprintf(szToFile, "%s\\%s", szTo, pPath);
                sprintf(szFromFile, "%s\\%s", szFrom, pPath);

                if(file.is_dir)
                    CopyDirectoryA(szFromFile, szToFile);
                else
                    CopyFileA(szFromFile, szToFile, FALSE);

                return true;
            });
            
            return TRUE;
        }
        
        return FALSE;
    }
    
    inline BOOL DestroyDirectoryA(LPCTSTR szPath)
    {
        // TODO error checking
        
        ForeachFile(szPath, "*.*", false, [&szPath](ModLoaderFile& file)
        {
                CHAR szPathFile[MAX_PATH];
                const char* pPath = file.filename;
                
                sprintf(szPathFile, "%s\\%s", szPath, pPath);
                
                if(file.is_dir)
                    DestroyDirectoryA(szPathFile);
                else
                    DeleteFileA(szPathFile);

                return true;
        });
        
        RemoveDirectoryA(szPath);
        return TRUE;
    }
    
    
    
    /* RAII for SetCurrentDirectory */
    struct CSetCurrentDirectory
    {
        char buffer[MAX_PATH];
        
        CSetCurrentDirectory(const char* newDir)
        {
            GetCurrentDirectoryA(sizeof(buffer), buffer);
            SetCurrentDirectoryA(newDir);
        }
        
        ~CSetCurrentDirectory()
        {
            SetCurrentDirectoryA(buffer);
        }
    };
    
}
    
#endif	/* MODLOADER_UTIL_PATH_HPP */

