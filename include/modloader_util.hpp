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


namespace modloader
{
    
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
                ||(mf.recursion && !ForeachFile(dir + mf.filepath, mask, bRecursive, cb)))
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
        // Replace all '\\' with '/', and tolower the strings (because Windows paths are case insensitive)
        tolower(ReplaceChar(file, '\\', '/'));
        tolower(ReplaceChar(folder, '\\', '/'));
        
        // We don't want a slash at the end of the folder path
        PopLastCharIf(file, '/');
        PopLastCharIf(folder, '/');
        
        if(bJust)
        {
            //
            size_t last = file.find_last_of('/');
            if(last >= folder.length())
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
        // For windows case insensitive paths
        
        /*
        // Get length to compare, normally it would be the same as the length of str_ext,
        // but if it is a directory, we must not compare the last '\\'
        size_t l = strlen(str_ext);
        const char* p = &str_ext[l - 1];
        while(l != 0 && *p == '\\' || *p == '/') --l, --p;
         * 
         */ 
        // Compare
        return (!strcmp(str_ext, ext, false));
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
     * MakeSureDirectoryExistA
     *      WinAPI-like function to make sure a directory exists, if not, create it
     *      @szPath: Directory to check
     */
    inline BOOL MakeSureDirectoryExistA(LPCTSTR szPath)
    {
        if(!IsDirectoryA(szPath))
        {
            CreateDirectory(szPath, NULL);
            return FALSE;
        }
        return TRUE;
    }


}


#endif	/* MODLOADER_UTIL_HPP */

