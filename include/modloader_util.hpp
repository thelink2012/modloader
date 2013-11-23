/* 
 * San Andreas modloader
 * Copyright (C) 2013  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 *  Modloader util functionalities, helpful for other plugins I'd say
 *  modloader.hpp must be included before this
 * 
 */
#ifndef MODLOADER_UTIL_HPP
#define	MODLOADER_UTIL_HPP
#pragma once


#include <windows.h>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <string>
#include <algorithm>
#include <cctype>

#include "modloader.hpp"

namespace modloader
{
    static const char cNormalizedSlash = '\\';
    std::string DoPathNormalization(std::string path);
    
    /* Get length of null terminated array
     */
    template<class T>
    inline size_t GetArrayLength(const T* xarray)
    {
        size_t len = 0;
        for(; *xarray; ++xarray) ++len;
        return len;
    }
    
    inline std::string& MakeSureStringIsDirectory(std::string& dir, bool touchEmpty = true)
    {
        if(dir.empty()) { if(touchEmpty) dir = ".\\"; }
        else if(dir.back() != '\\')
        { dir.push_back('\\'); }
        
        return dir;
    }
    
    inline std::string& TrimString(std::string& s, bool trimLeft = true, bool trimRight = true)
    {
        if(trimLeft)
            s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
        if(trimRight)
            s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
        return s;
    }
    
    template<class C>
    inline typename C::reference& AddNewItemToContainer(C& container)
    {
        container.resize(container.size() + 1);
        return container.back();
    }
    
    inline std::string GetFilePath(const ModLoaderFile& file)
    {
        return (std::string(file.modpath) + file.filepath);
    }

    inline uint32_t fvn1a_32_init()
    { return (2166136261U); }
    inline uint32_t fnv1a_32_transform(uint32_t fvn, uint8_t c)
    { return ((((fvn) ^ (uint32_t)(c)) * 16777619)); }
    inline uint32_t fnv1a_32_final(uint32_t fnv)
    { return (fnv); }

    inline uint32_t fnv1a_32_binary(const char* bytes, size_t size)
    {
        uint32_t fnv = fvn1a_32_init();
        for( ; size--; ++bytes)
            fnv = fnv1a_32_transform(fnv, *bytes);
        return fnv1a_32_final(fnv);
    }
    
    inline uint32_t fnv1a_32(const char* string)
    {
        uint32_t fnv = fvn1a_32_init();
        for(const char* str = string; *str; ++str)
            fnv = fnv1a_32_transform(fnv, *str);
        return fnv1a_32_final(fnv);
    }
    
    inline uint32_t fnv1a_upper_32(const char* string)
    {
        uint32_t fnv = fvn1a_32_init();
        for(const char* str = string; *str; ++str)
            fnv = fnv1a_32_transform(fnv, toupper(*str));
        return fnv1a_32_final(fnv);
    }
    
    inline uint32_t fnv1a(const char* string)
    { return fnv1a_32(string); }
    
    inline uint32_t fnv1a_upper(const char* string)
    { return fnv1a_upper_32(string); }
    
    inline bool RegisterReplacementFile(const CPlugin& plugin, const char* name,  std::string& buf, const char* path)
    {
        if(!buf.empty())
        {
            plugin.Log
               ("warning: Failed to replace a file %s with \"%s\" because the file already has a replacement!\n"
                "\tReplacement: %s", name, path, buf.c_str());
            return false;
        }
        else
        {
            plugin.Log("Found replacement file for %s\n\tReplacement: %s", name, path);
            buf = path;
            return true;
        }
    }
    
    
    /*
     *  ReadEntireFile
     *      Reads a file into memory
     *      @filepath: File to be read
     *      @out: Vector that will receive the file data
     *      @sizeLimit: If file size is greater than this value, failure happens.
     */
    inline bool ReadEntireFile(const char* filepath, std::vector<char>& out, uint64_t sizeLimit = -1)
    {
        /* 64 bits file not supported */
        
        bool bResult = false;
        uint64_t fsize;
        
        if(FILE* f = fopen(filepath, "rb"))
        { 
            /* Get file size */
            if(!fseek(f, 0, SEEK_END) && ((fsize = ftell(f)) != 1L) && !fseek(f, 0, SEEK_SET))
            {
                /* Read the file data into the vector */
                out.resize(fsize);
                if(fread(out.data(), 1, fsize, f) == fsize)
                    bResult = true; /* Success */
                else
                    out.clear();    /* Failure, clear output */
                
                bResult = true;
            }

            fclose(f);
        }
        
        /* Bye function */
        return bResult;
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

    inline std::string& ReplaceChar(std::string& str, char find, char replace)
    {
        for(auto& c : str)
            if(c == find) c = replace;
        return str;
    }
    
    inline std::string& PopLastCharIf(std::string& str, char ifc)
    {
        if(str.back() == ifc) str.resize(str.size() - 1);
        return str;
    }
    
    /*
     *  String transformations 
     */
    
    inline std::string& tolower(std::string& str)
    {
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
        return str;
    }
    
    inline std::string& toupper(std::string& str)
    {
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
        return str;
    }
    
    /*
     *  String comparision funcs
     */
    using std::strcmp;
    
    inline int strcmp(const char* str1, const char* str2, bool csensitive)
    {
        return (csensitive? ::strcmp(str1, str2) : ::_stricmp(str1, str2));
    }
    
    inline int strcmp(const char* str1, const char* str2, size_t num, bool csensitive)
    {
        return (csensitive? ::strncmp(str1, str2, num) : ::_strnicmp(str1, str2, num));
    }
    
    inline int compare(const std::string& str1, const std::string& str2, bool case_sensitive)
    {
        return strcmp(str1.c_str(), str2.c_str(), case_sensitive);
    }
    
    inline int compare(const std::string& str1, const std::string& str2, size_t num, bool case_sensitive)
    {
        return strcmp(str1.c_str(), str2.c_str(), num, case_sensitive);
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
        file = DoPathNormalization(file);
        folder= DoPathNormalization(folder);
        
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
     *      @return: Returns the last path component
     */
    inline std::string GetLastPathComponent(std::string path)
    {
        PopLastCharIf(path, '/');
        PopLastCharIf(path, '\\');
        size_t pos = path.find_last_of("/\\");
        
        return (pos == path.npos? path : path.substr(path.find_last_of("/\\")));
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
     *  DoPathNormalization
     *      Normalizates a path string, so for example:
     *          "somefolder/something" will output "SOMEFOLDER\\SOMETHING"
     *          "SOMEfoldER/something/" will output "SOMEFOLDER\\SOMETHING"
     *          "somefolder\\something" will output "SOMEFOLDER\\SOMETHING"
     *          etc
     */
    inline std::string DoPathNormalization(std::string path)
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


#endif	/* MODLOADER_UTIL_HPP */

