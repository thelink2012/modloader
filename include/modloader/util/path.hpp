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

#include <functional>
#include <string>
#include <cstring>
#include <windows.h>
#include <modloader/modloader.hpp>
#include <modloader/util/container.hpp>

namespace modloader
{
    static const char* szNullFile = "NUL";          // "/dev/null" on POSIX systems
    static const char cNormalizedSlash = '\\';      // The slash used in the normalized path
    
    // Information output by FilesWalk function
    struct FileWalkInfo
    {
        // File string buffer
        const char*     filebuf;
        
        // The following pointers are inside filebuf
        const char*     filepath;
        const char*     filename;
        const char*     filext;
        size_t          length;
        
        // File properties
        bool            is_dir;
        uint64_t        size;
        uint64_t        time;
        
        // If is_dir=true and you don't want recursion to take place in this folder,
        // the following property should be set to false by the callback
        bool            recursive;
    };
    
    // Gets all possible path separators in a null terminated string
    template<class T> inline const T* GetPathSeparators()
    {
        static const T slash[] = { '/', '\\', 0 };
        return slash;
    }
    
    // Gets a LONGLONG from a LARGEINTEGER
    inline LONGLONG GetLongFromLargeInteger(DWORD LowPart, DWORD HighPart)
    {
        LARGE_INTEGER l;
        l.LowPart = LowPart;
        l.HighPart = HighPart;
        return l.QuadPart;
    }
    
    
    /*
     *  MakeSureStringIsDirectory
     *      Makes sure the string @dir is a directory path. If @touchEmpty is true,
     *      dir will be made a directory even when the string is empty.
     * 
     *      A string is considered as a path when it ends with a slash
     */
    inline std::string& MakeSureStringIsDirectory(std::string& dir, bool touchEmpty = true)
    {
        if(dir.empty())
        {
            if(touchEmpty) dir = ".\\";
        }
        else if(dir.back() != cNormalizedSlash)
        {
            dir.push_back(cNormalizedSlash);
        }
        return dir;
    }
    
    
    /*
     *  NormalizePath
     *      Normalizates a path string, so for example:
     *          "somefolder/something" will output "somefolder\\something"
     *          "SOMEfoldER/something/" will output "somefolder\\something"
     *          "somefolder\\something" will output "somefolder\\something"
     *          etc
     */
    inline std::string NormalizePath(std::string path)
    {
        if(path.size())
        {
            std::replace(path.begin(), path.end(), '/', '\\');  // Replace all '/' with '\\'
            tolower(path);                                      // tolower the strings (Windows paths are case insensitive)
            while(path.back() == '/' || path.back() == '\\')    // We don't want a slash at the end of the folder path
                path.pop_back();                                // ..
            trim(path);                                         // Trim the string...
        }
        return path;
    }
    
    /*
     *  GetProperlyPath
     *      This works exactly the same as NormalizePath (see above), except if @transform is not a null pointer
     *      It will return the first time that @transform (probably a folder)  appears in the normalized path
     * 
     *      PS: @transform MUST be normalized!
     */
    inline std::string GetProperlyPath(std::string path, const char* transform)
    {
        path = NormalizePath(std::move(path));
        if(transform)
        {
            size_t pos = path.find(transform);
            if(pos != 0 && pos != path.npos) path.erase(0, pos);
        }
        return path;
    }
    

    
    /*
     *  GetLastPathComponent
     *      @path: Path to get the last component from
     *      @return: Returns the last path component position in the string
     */
    template<class T>
    inline size_t GetLastPathComponent(std::basic_string<T> path, size_t count = 1)
    {
        size_t pos = path.npos;
        size_t x = path.npos;
        
        if(path.size())
        {
            // Remove any slash at the end of the string, so our finding can be safe
            while(path.back() == '/' || path.back() == '\\') path.pop_back();
        
            // Do the search
            for(size_t i = 0; i < count; ++i)
            {
                pos = path.find_last_of(GetPathSeparators<T>(), x);
                x = pos - 1;
                if(pos == 0 || pos == path.npos) break;
            }
        }

        return (pos == path.npos? 0 : pos + 1);
    }

    template<class T>
    inline std::basic_string<T> GetFileExtension(std::basic_string<T> filename)
    {
        auto extn = filename.rfind('.');
        if(extn != filename.npos) 
        {
            ++extn;
            return filename.substr(extn, filename.size() - extn);
        }
        return "";
    }

    template<class T>
    inline std::basic_string<T> GetPathComponentBack(std::basic_string<T> path, size_t count = 1)
    {
        auto pos = GetLastPathComponent(path, count);
        auto end = path.find_first_of(GetPathSeparators<T>(), pos);
        auto len = end == path.npos? path.npos : end - pos;
        return path.substr(pos, len);
    }

    template<class T>
    inline std::basic_string<T> GetPathExtensionBack(std::basic_string<T> path, size_t count = 1)
    {
        auto comp = GetPathComponentBack(path, count);
        return GetFileExtension(std::move(path));
    }
    
    
    /*
     *      Checks if a file is inside an named folder, or just inside it
     *      @file File path to check
     *      @bJust  Returns true only if just right inside the folder
     *      @folder Folder path
     *
     */
    inline bool IsFileInsideFolder(std::string file, bool bJust, std::string folder)
    {
        file = NormalizePath(file);
        folder= NormalizePath(folder);
        
        int numFolds = std::count(file.begin(), file.end(), cNormalizedSlash);

        for(int i = 2; i <= numFolds; ++i)
        {
            if(GetPathComponentBack(file, i) == folder)
                return true;
            else if(bJust)
                break;
        }
        
        return false;
    }

    
    /*
     * FilesWalk
     *      Iterates on all files in a directory, files beggining with '.' will be ignored.
     *      @dir: Directory to search at
     *      @mask: Search mask
     *      @recursive: Recursive search?
     *      @cb: Callback to call on each file
     */
    inline bool FilesWalk(std::string dir, const std::string& glob, bool recursive, std::function<bool(FileWalkInfo&)> cb)
    {
        std::string filebuf;
        WIN32_FIND_DATAA fd;
        FileWalkInfo wf;
        HANDLE hSearch;

        // Clear structures
        memset(&fd, 0, sizeof(fd));
        memset(&wf, 0, sizeof(wf));
        filebuf.reserve(100);
        
        MakeSureStringIsDirectory(dir, false);
        
        // Opens the search
        if((hSearch = FindFirstFileA((dir + glob).c_str(), &fd)) == INVALID_HANDLE_VALUE)
            return false;

        // Iterate on all files on this directory
        do
        {
            //  Ignore files beggining with '.' (including "." & "..")
            if(fd.cFileName[0] != '.' && fd.cFileName[0] != '*')
            {
                // Setup filebuf
                filebuf = dir + fd.cFileName;
                
                // Setup properties
                wf.is_dir    = (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)!=0;
                wf.size      = GetLongFromLargeInteger(fd.nFileSizeLow, fd.nFileSizeHigh);
                wf.time      = GetLongFromLargeInteger(fd.ftLastWriteTime.dwLowDateTime, fd.ftLastWriteTime.dwHighDateTime);
                wf.recursive = recursive && wf.is_dir;
                
                // Setup string pointers
                wf.filebuf  = filebuf.data();
                wf.filepath = wf.filebuf;
                wf.length   = filebuf.length();
                wf.filename = &wf.filepath[GetLastPathComponent(filebuf)];
                
                // Setup file extension pointer
                if(wf.filext = strrchr(wf.filename, '.'))
                    wf.filext = wf.filext + 1;
                else
                    wf.filext = &wf.filebuf[wf.length];
                
                // Call 'cb' and go recursive if asked to...
                if(!cb(wf)
                ||(wf.recursive && !FilesWalk(dir + wf.filename, glob, recursive, cb)))
                    break;
            }
        }
        while(FindNextFile(hSearch, &fd));   // Next...

        // Done
        FindClose(hSearch);  
        return true;
    }
    

#if 0
    /*
     *  IsFileExtension
     *      @str: File
     *      @ext: Extension
     */
    inline bool IsFileExtension(const std::string& str, const char* ext)
    {
    }
#endif

    
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
      DWORD dwAttrib = GetFileAttributesA(szPath);
      return (dwAttrib != INVALID_FILE_ATTRIBUTES);
    }

    inline BOOL IsPathW(LPCWSTR szPath)
    {
      DWORD dwAttrib = GetFileAttributesW(szPath);
      return (dwAttrib != INVALID_FILE_ATTRIBUTES);
    }
    
    template<class T> inline BOOL IsPath(const T* szPath)
    {
        return IsPathA(szPath);
    }
    
    template<> inline BOOL IsPath(const wchar_t* szPath)
    {
        return IsPathW(szPath);
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
    
#if 0
    /*
     *  CopyDirectoryA
     *      WinAPI-like function that copies the full directory @szFrom to @szTo
     *      If @szTo doesn't exist, it is created
     */
    inline BOOL CopyDirectoryA(LPCTSTR szFrom, LPCTSTR szTo)
    {
        if(CreateDirectoryA(szTo, NULL))
        {
            ForeachFile(szFrom, "*.*", false, [&szFrom, &szTo](modloader::file& file)
            {
                CHAR szToFile[MAX_PATH], szFromFile[MAX_PATH];
                const char* pPath = file.filename;
                
                sprintf(szToFile, "%s\\%s", szTo, pPath);
                sprintf(szFromFile, "%s\\%s", szFrom, pPath);

                if(file.is_dir) // Call myself again for recursion
                    CopyDirectoryA(szFromFile, szToFile);
                else
                    CopyFileA(szFromFile, szToFile, FALSE);

                return true;
            });
            
            return TRUE;
        }
        
        return FALSE;
    }
#endif
    
#if 1
    /*
     *  DestroyDirectoryA
     *      WinAPI-like function that deletes the path @szPath fully
     */
    inline BOOL DestroyDirectoryA(LPCTSTR szPath)
    {
        FilesWalk(szPath, "*.*", false, [&szPath](FileWalkInfo& file)
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
        
        return RemoveDirectoryA(szPath);
    }
#endif
    
    
    /*
     *  GetFileSize
     *      WinAPI-like function that gets the file size of @szPath
     */
    inline LONGLONG GetFileSize(LPCTSTR szPath)
    {
        WIN32_FILE_ATTRIBUTE_DATA fad;
        return GetFileAttributesExA(szPath, GetFileExInfoStandard, &fad)?
            GetLongFromLargeInteger(fad.nFileSizeLow, fad.nFileSizeHigh) : 0;
    }
    
    
    template<class T>
    inline bool IsAbsolutePath(const T* str)
    {
        return (
                (str[0] == '\\' || str[0] == '/')
            ||  (isalpha(str[0]) && str[1] == ':' && (str[2] == '\\' || str[2] == '/'))
          );
    }
    
    
    /* RAII for SetCurrentDirectory */
    struct scoped_chdir
    {
        char buffer[MAX_PATH];
        
        scoped_chdir(const char* newDir)
        {
            GetCurrentDirectoryA(sizeof(buffer), buffer);
            SetCurrentDirectoryA(newDir);
        }
        
        ~scoped_chdir()
        {
            SetCurrentDirectoryA(buffer);
        }
    };
 
    
    struct scoped_lock
    {
        CRITICAL_SECTION* c;

        /* Enter on ctor, Leave on dtor */
        scoped_lock(CRITICAL_SECTION& cs)
        { c = &cs; EnterCriticalSection(&cs); }
        ~scoped_lock()
        { LeaveCriticalSection(c); }
    };
    
}
    
#endif	/* MODLOADER_UTIL_PATH_HPP */

