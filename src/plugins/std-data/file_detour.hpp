/*
 * Copyright (C) 2013  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 * Filepath detouring
 * 
 */
#ifndef FILE_DETOUR_HPP
#define	FILE_DETOUR_HPP

extern void Log(const char* msg, ...);

#include <modloader_util_injector.hpp>


/*
 *  File detour
 *      Hooks a fopen call to open another file instead of the sent to the function
 *      @addr is the address to hook 
 *      Note the object is dummy (has no content), all stored information is static
 */
template<uintptr_t addr>
class CFileDetour : function_hooker<addr, void*(const char*, const char*)>
{
    protected:
        typedef function_hooker<addr, void*(const char*, const char*)> super;
        typedef typename super::func_type func_type;
        
        // The filepath to open instead
        static const char*& filepath()
        { static const char* p = ""; return p; }
   
        // What is the file type?
        static const char*& what()
        { static const char* p = ""; return p; }
        
        // The fopen hook
        static void* OpenFile(func_type fopen, const char*& filename, const char*& mode)
        {
            const char* w = what();
            const char* f = filepath();
            
            // If the file type is known, log what we're loading
            if(w && w[0]) Log("Loading %s \"%s\"", w, f);
            
            // Call fopen with the new filepath
            return fopen(f, mode);
        }
        
    public:
        /*
         *  Constructs a file detour. This should be called only once.
         */
        CFileDetour(const char* newfilepath, const char* what_is_it)
            : super(OpenFile)
        {
            filepath() = newfilepath;
            what()     = what_is_it;
        }
};


/*
 *  Helper function to create a file detour
 */
template<uintptr_t at_address>
CFileDetour<at_address> make_file_detour(const char* filepath, const char* what)
{
    return CFileDetour<at_address>(filepath, what);
}



#endif	/* FILE_DETOUR_HPP */

