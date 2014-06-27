/*
 * Copyright (C) 2013-2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 *  std-asi -- Standard ASI Loader Plugin for San Andreas Mod Loader
 *      Loads ASI files as libraries
 * 
 */
#include "asi.h"
#include "args_translator/translator.hpp"
#include <map>
#include <tlhelp32.h>

using namespace modloader;


template<class F>
static bool ModulesWalk(uint32_t pid, F functor);



/*
 *  Constructs a ModuleInfo object
 *  It takes a rvalue for a relative path string
 */
ThePlugin::ModuleInfo::ModuleInfo(std::string path, const modloader::file* file, HMODULE mod) : file(file)
{
    size_t last = GetLastPathComponent(path);
    bIsMainExecutable = bIsASI = bIsD3D9 = bIsMainCleo = bIsCleo = false;
    memset(&this->hacks, 0, sizeof(hacks));
    
    const char* filename = &path[last];
    
    // Setup flags
    if(!strcmp(filename, "d3d9.dll", false))
    {
        bIsD3D9 = true;
        bIsASI = true;
    }
    else if(!strcmp(filename, "gta", false))
        bIsMainExecutable = true;
    else if(!strcmp("CLEO.asi", filename, false))
        bIsMainCleo = bIsCleo = true;
    else    // .cleo or .asi
    {
        // Check if it is a .cleo plugin
        auto pos = path.find_last_of('.');
        if(pos != path.npos)
        {
            if(IsFileExtension(path.c_str() + pos + 1, "cleo"))
                bIsCleo = true;
        }
        
        // If not a .cleo, it's an .asi plugin
        if(!bIsCleo) bIsASI = true;
    }
    
    // Setup hacks flags
    if(!strcmp("shell.asi", filename, false)    // Ryosuke's Shell Mod
    || !strcmp("Render.asi", filename, false))  // SA::Render
    {
        hacks.bRyosukeModuleName = true;
    }
    
    // Setup fields
    this->module = mod;
    this->folder = path.substr(0, last);
}


/*
 *  Find all cleo plugins already loaded and push them into asi list 
 */
void ThePlugin::LocateCleo()
{
    static bool bDidIt = false;
    if(!bDidIt) { bDidIt = true; }
    else return;


    HMODULE hCleo = GetModuleHandleA("CLEO.asi");

    // We need CLEO.asi module for cleo script injection
    if(hCleo)
    {
        char buffer[MAX_PATH];
        if(GetModuleFileNameA(hCleo, buffer, sizeof(buffer)))
        {
            if(const char* p = path_translator_base::CallInfo::GetCurrentDir(buffer, this->loader->gamepath, -1))
            {
                // Find the library version
                auto CLEO_GetVersion = (int (__stdcall*)()) GetProcAddress(hCleo, "_CLEO_GetVersion@0");
                this->iCleoVersion = CLEO_GetVersion? CLEO_GetVersion() : 0;
                
                Log("CLEO library version %X found at \"%s\"", iCleoVersion, p);
                if(this->bHasNoCleoFolder = !IsPath((std::string(loader->gamepath) + "./CLEO/").c_str()))
                    Log("Warning: No CLEO folder found, may cause problems");
                
                this->asiList.emplace_back(p, nullptr, hCleo);
                this->asiList.back().PatchImports();
            }
        }
        else
            hCleo = NULL;
    }

    // Find all the already loaded cleo plugins
    if(hCleo)
    {
        ModulesWalk(GetCurrentProcessId(), [this](const MODULEENTRY32& entry)
        {
            // Find the extension for this module
            if(const char* p = strrchr(entry.szModule, '.'))
            {
                // Is it a .cleo plugin?
                if(IsFileExtension(p+1, "cleo"))
                {
                    // Yep, take the relative path and push it to the asi list
                    if(p = path_translator_base::CallInfo::GetCurrentDir(entry.szExePath, this->loader->gamepath, -1))
                    {
                        this->asiList.emplace_back(p, nullptr, entry.hModule);
                        this->asiList.back().PatchImports();
                    }
                }
            }

            return true;
        });
    }
}


/*
 *  Loads the module assigned to our field path 
 */
bool ThePlugin::ModuleInfo::Load()
{
    if(!this->module)
    {
        // Chdir to the asi path, so it can do stuff on DllMain properly
        scoped_chdir xdir((std::string(plugin_ptr->loader->gamepath) + this->folder).c_str());

        // We need the fullpath into the module because of the way Windows load dlls
        // More info at: http://msdn.microsoft.com/en-us/library/windows/desktop/ms682586(v=vs.85).aspx

        // Load the library module into our module field
        SetLastError(0);
        this->module = bIsMainExecutable? GetModuleHandleA(0) : LoadLibraryA(file->FullPath().c_str());

        // Patch the module imports to pass throught args translation.
        if(this->module) this->PatchImports();
    }
    return this->module != 0;
}
            
/*
 *  Unloads the module assigned to this object 
 */
void ThePlugin::ModuleInfo::Free()
{
    if(this->module)
    {
        HMODULE hMod;
        
        // Free module (if not main executable)
        if(!bIsMainExecutable)
            FreeLibrary(module);
        
        // Test if module has been freed or is still referenced by someone else...
        if(GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                            (char*)this->module,
                            &hMod))
        {
            // ...
            if(hMod == this->module)    // If still referenced by someone else, remove our IAT hooks
                this->RestoreImports();
        }
        
        //
        this->module = 0;
        
        // Go deleting the translators for this
        for(auto& t : this->translators)
            delete t;
            
        // Clear the translators array
        this->translators.clear();
    }
 }

/*
 *  Translator finder for a specific ASI object 
 */
path_translator_base* ThePlugin::ModuleInfo::FindTranslatorFrom(const char* symbol, const char* libname)
{
    // Find translator for symbol and libname
    for(auto& t : this->translators)
    {
        // Yes, we can compare the c-string pointers in this context ;)
        if(t->GetSymbol() == symbol && t->GetLibName() == libname)
            return t;
    }
    return nullptr;
}


/*
 *  Get the singletoned arg translators list
 */
static path_translator_base::list_type& GetTranslators()
{
    // Initialization time is done...
    if(path_translator_base::InitializationDone() == false)
    {
        // Mark as done
        path_translator_base::InitializationDone() = true;

        // Sort the list by library/symbol
        auto& list = path_translator_base::List();
        std::sort(list.begin(), list.end(), [](path_translator_base* a, path_translator_base* b)
        {
            int x = strcmp(a->GetLibName(), b->GetLibName(), false);
            if(x == 0) x = strcmp(a->GetSymbol(), b->GetSymbol());
            return x < 0;
        });
    }
    
    //
    return path_translator_base::List();
}



/*
 *  Patches an ASI Import Table for proper path translation
 */
void ThePlugin::ModuleInfo::PatchImports()
{
    // Converts a rva pointer to a actual pointer in the process space from this ASI
    auto rva_to_ptr = [this](long rva)
    { return auto_ptr_cast((void*)((char*)(this->module) + rva)); };
    
    // Used to find translators lowerbound at a sorted list of translators by library name
    auto fn_compare_translator_with_lib_lb = [](path_translator_base* a, const char* b)
    {
        return strcmp(a->GetLibName(), b, false) < 0;
    };

    // Used to find translators upperbound at a sorted list of translators by library name
    auto fn_compare_translator_with_lib_ub = [](const char* a, path_translator_base* b)
    {
        return strcmp(a, b->GetLibName(), false) < 0;
    };

    // Used to find translators lowerbound by symbol name
    auto fn_compare_translator_with_symbol = [](path_translator_base* a, const char* b)
    {
        return strcmp(a->GetSymbol(), b) < 0;
    };
    
    static std::map<uintptr_t, std::string> iat_cache;
    
    // We need a list of pointers to some functions since some linkers (the Borland Linker)
    // doesn't produce a ILT
    if(iat_cache.empty())
    {
        for(auto& x : GetTranslators())
        {
            if(auto module = GetModuleHandleA(x->GetLibName()))
            {
                if(auto sym = GetProcAddress(module, x->GetSymbol()))
                    iat_cache[(uintptr_t)sym] = x->GetSymbol();
            }
        }
    }
    
    // Get list of singletoned translators
    auto& list = GetTranslators();
    
    // Setup pointers to headers in PE module
    IMAGE_THUNK_DATA32 *fname, *faddr;
    IMAGE_DOS_HEADER*  dos      = rva_to_ptr(0);
    IMAGE_NT_HEADERS*  nt       = rva_to_ptr(dos->e_lfanew);
    IMAGE_FILE_HEADER* pe       = &nt->FileHeader;
    IMAGE_OPTIONAL_HEADER* opt  = &nt->OptionalHeader;
    IMAGE_DATA_DIRECTORY* data  = &opt->DataDirectory[0];
    
    // Get address to import table
    if(data[IMAGE_DIRECTORY_ENTRY_IMPORT].Size == 0) return;
    IMAGE_IMPORT_DESCRIPTOR* imp = rva_to_ptr(data[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

    // Iterate on each imported library...
    for(auto* lib = imp; lib->Name != 0; ++lib)
    {
        // Get library name...
        const char* libname = rva_to_ptr(lib->Name);
        
        // Check out if we have any translator for this library...
        auto it_lib = std::lower_bound(list.begin(), list.end(), libname, fn_compare_translator_with_lib_lb);
        if((it_lib != list.end() && !strcmp((*it_lib)->GetLibName(), libname, false)) == false)
        {
            // ...we don't, get 'almost any library' lower bound
            it_lib = std::lower_bound(list.begin(), list.end(), "", fn_compare_translator_with_lib_lb);
        }
        
        // If we have a lower bound to start searching symbols from, get into symbols searching!
        if(it_lib != list.end())
        {
            // Find upper bound for this library
            auto it_lib_end = std::upper_bound(it_lib, list.end(),  (*it_lib)->GetLibName(), fn_compare_translator_with_lib_ub);

            // Get pointer to thunks aka function names and function address tables
            bool bHasILT = lib->OriginalFirstThunk != 0;
            fname = rva_to_ptr(lib->OriginalFirstThunk);
            faddr = rva_to_ptr(lib->FirstThunk);

            // Iterate on each name to see if we should patch it
            for(; faddr->u1.Function; ++fname, ++faddr)
            {
                const char* symbolName;
                
                if(bHasILT)
                {
                    // Is this just a ordinal import? Skip it, we don't have a symbol name!
                    if(fname->u1.Ordinal & IMAGE_ORDINAL_FLAG) continue;

                    // Get the symbol name
                    symbolName = (char*)(((IMAGE_IMPORT_BY_NAME*)(rva_to_ptr(fname->u1.AddressOfData)))->Name);
                }
                else
                {
                    auto sym = iat_cache.find(faddr->u1.Function);
                    if(sym == iat_cache.end()) continue;
                    symbolName = sym->second.c_str();
                }
                
                // Find arg translator from symbol...
                auto it_sym = std::lower_bound(it_lib, it_lib_end, symbolName, fn_compare_translator_with_symbol);
                if(it_sym != list.end() && !strcmp((*it_sym)->GetSymbol(), symbolName))
                {
                    // Add this translator and patch this import pointer into our translator...
                    (*this->translators.emplace(translators.end(), (*it_sym)->clone()))->Patch(&faddr->u1.Function);
                }
            }
        }
    }
}

/*
 *  Unpatches all patches made in this module IAT 
 */
void ThePlugin::ModuleInfo::RestoreImports()
{
    for(auto& t : this->translators)
    {
        if(t) t->Restore();
    }
}







/*
 *  ModulesWalk
 *      Iterates on the list of modules loaded in process id @pid and calls @functor with the module information
 *      Functor prototype should be like [bool functor(const MODULEENTRY32& entry)]
 */
template<class F>
static bool ModulesWalk(uint32_t pid, F functor)
{
    HANDLE hSnapshot;
    MODULEENTRY32 entry;
    entry.dwSize = sizeof(entry);
    bool bSkip = false;
    bool bDoIt = true;
    
    // Open the modules snapshot for process id @pid
    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
    if(hSnapshot != INVALID_HANDLE_VALUE)
    {
        // Get the first module on the snapshot
        if(!Module32First(hSnapshot, &entry))
        {
            // Failed to get the module, what's it?
            if(GetLastError() != ERROR_NO_MORE_FILES)
                bSkip = true;   // Some internal failure, just skip this buffer
            else
                bDoIt = false;  // Don't continue the search
        }
        
        // Continue the search?
        if(bDoIt)
        {
            do
            {
                // Should skip this module data (because it is an invalid data)?
                if(!bSkip)
                {
                    // Call the functor with the data and check out if we should continue the search
                    if(!functor(entry)) break;
                }
                
                // Get the next module on the snapshot
                if(!Module32Next(hSnapshot, &entry))
                {
                    // Failed, why?
                    if(GetLastError() != ERROR_NO_MORE_FILES)
                        bSkip = true;   // Just skip this data
                    else
                        break;          // End of the search
                }
                
            } while(true);
        }
        
        // Search is done
        FindClose(hSnapshot);
        return true;
    }
    
    // Something gone wrong
    return false;
}



/* ----------------------------------------------------------------------------------------------------------------------- */

/*
 *  TODO more symbols & wchar version of everything below 
 */

/*
 *  Kernel32 
 *      http://msdn.microsoft.com/en-us/library/windows/desktop/ms724875(v=vs.85).aspx 
 *      http://msdn.microsoft.com/en-us/library/windows/desktop/aa364232(v=vs.85).aspx
 *      http://msdn.microsoft.com/en-us/library/windows/desktop/aa363950(v=vs.85).aspx
 */
extern const char aKernel32[] = "kernel32.dll";
extern const char aCreateFileA[] = "CreateFileA";
extern const char aLoadLibraryA[] = "LoadLibraryA";
extern const char aLoadLibraryExA[] = "LoadLibraryExA";
extern const char aGetModuleFileNameA[] = "GetModuleFileNameA";
extern const char aFindFirstFileA[] = "FindFirstFileA";
extern const char aFindNextFileA[] = "FindNextFileA";
extern const char aFindClose[] = "FindClose";
extern const char aSetCurrentDirectoryA[] = "SetCurrentDirectoryA";
extern const char aGetPrivateProfileIntA[] = "GetPrivateProfileIntA";
extern const char aGetPrivateProfileSectionA[] = "GetPrivateProfileSectionA";
extern const char aGetPrivateProfileSectionNamesA[] = "GetPrivateProfileSectionNamesA";
extern const char aGetPrivateProfileStringA[] = "GetPrivateProfileStringA";
extern const char aGetPrivateProfileStructA[] = "GetPrivateProfileStructA";
extern const char aWritePrivateProfileSectionA[] = "WritePrivateProfileSectionA";
extern const char aWritePrivateProfileStringA[] = "WritePrivateProfileStringA";
extern const char aWritePrivateProfileStructA[] = "WritePrivateProfileStructA";
extern const char aGetFileAttributesA[] = "GetFileAttributesA";
extern const char aGetFileAttributesExA[] = "GetFileAttributesExA";


// Operations
static path_translator_stdcall<aCreateFileA, aKernel32, HANDLE(LPCTSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE)>
        psCreateFileA(0, AR_PATH_INE, 0, 0, 0, 0, 0, 0);
static path_translator_stdcall<aSetCurrentDirectoryA, aKernel32, BOOL(LPCTSTR)>
        psSetCurrentDirectoryA(0, AR_PATH_INE);           // Do not work properly!! Don't use!!!


// File finding (used mostly to hack CLEO.asi)
static path_translator_stdcall<aFindFirstFileA, aKernel32, HANDLE(LPCTSTR, LPWIN32_FIND_DATAA)>
        psFindFirstFileA(0, AR_PATH_INEB, 0);
static path_translator_stdcall<aFindNextFileA, aKernel32, BOOL(HANDLE, LPWIN32_FIND_DATAA)>  // Hack CLEO.asi
        psFindNextFileA(0, 0, 0);
static path_translator_stdcall<aFindClose, aKernel32, BOOL(HANDLE)>                          // Hack CLEO.asi
        psFindClose(0, 0);


// Library routines
static path_translator_stdcall<aGetModuleFileNameA, aKernel32, DWORD(HMODULE, LPTSTR, DWORD)>
        psGetModuleFileNameA(0, 0, 0, 0); // I'll need to intercept this routine for some Ryosuke's plugins compatibility
static path_translator_stdcall<aLoadLibraryA, aKernel32, HMODULE(LPCTSTR)>
        psLoadLibraryA(0, AR_PATH_INE);
static path_translator_stdcall<aLoadLibraryExA, aKernel32, HMODULE(LPCTSTR, HANDLE, DWORD)>
        psLoadLibraryExA(0, AR_PATH_INE, 0, 0);

// Get from INI
static path_translator_stdcall<aGetPrivateProfileIntA, aKernel32, UINT(LPCTSTR, LPCTSTR, INT, LPCTSTR)>
        psGetPrivateProfileIntA(0, 0, 0, 0, AR_PATH_INE);
static path_translator_stdcall<aGetPrivateProfileSectionA, aKernel32, DWORD(LPCTSTR, LPCTSTR, DWORD, LPCTSTR)>
        psGetPrivateProfileSectionA(0, 0, 0, 0, AR_PATH_INE);
static path_translator_stdcall<aGetPrivateProfileSectionNamesA, aKernel32, DWORD(LPCTSTR, DWORD, LPCTSTR)>
        psGetPrivateProfileSectionNamesA(0, 0, 0, AR_PATH_INE);
static path_translator_stdcall<aGetPrivateProfileStringA, aKernel32, DWORD(LPCTSTR, LPCTSTR, LPCTSTR, LPTSTR, DWORD, LPCTSTR)>
        psGetPrivateProfileStringA(0, 0, 0, 0, 0, 0, AR_PATH_INE);
static path_translator_stdcall<aGetPrivateProfileStructA, aKernel32, DWORD(LPCTSTR, LPCTSTR, LPVOID, UINT, LPTSTR)>
        psGetPrivateProfileStructA(0, 0, 0, 0, 0, AR_PATH_INE);

// Write to INI
static path_translator_stdcall<aWritePrivateProfileSectionA, aKernel32, BOOL(LPCTSTR, LPCTSTR, LPCTSTR)>
        psWritePrivateProfileSectionA(0, 0, 0, AR_PATH_INE);
static path_translator_stdcall<aWritePrivateProfileStringA, aKernel32, BOOL(LPCTSTR, LPCTSTR, LPCTSTR, LPCTSTR)>
        psWritePrivateProfileStringA(0, 0, 0, 0, AR_PATH_INE);
static path_translator_stdcall<aWritePrivateProfileStructA, aKernel32, BOOL(LPCTSTR, LPCTSTR, LPVOID, UINT, LPCTSTR)>
        psWritePrivateProfileStructA(0, 0, 0, 0, 0, AR_PATH_INE);

// Something
static path_translator_stdcall<aGetFileAttributesA, aKernel32, DWORD(LPCTSTR)>
        psGetFileAttributesA(0, AR_PATH_INE);
static path_translator_stdcall<aGetFileAttributesExA, aKernel32, DWORD(LPCTSTR, GET_FILEEX_INFO_LEVELS, LPVOID)>
        psGetFileAttributesExA(0, AR_PATH_INE, 0, 0);





/*
 *  C Standard Library
 *      http://en.cppreference.com/w/cpp/io/c/fopen
 *      http://en.cppreference.com/w/cpp/io/c/freopen
 *      http://en.cppreference.com/w/cpp/io/c/rename
 *      http://en.cppreference.com/w/cpp/io/c/remove
 */
extern const char aSTDC[]       = "";           // Well, the standard library could be any dll...
extern const char afopen[]      = "fopen";
extern const char afreopen[]    = "freopen";
extern const char arename[]     = "rename";
extern const char aremove[]     = "remove";

// Translators for STDC
static path_translator_cdecl<afopen, aSTDC, void*(const char*, const char*)>
        psfopen(0, AR_PATH_INE, 0);
static path_translator_cdecl<afreopen, aSTDC, void*(const char*, const char*, void*)>
        psfreopen(0, AR_PATH_INE, 0, 0);
static path_translator_cdecl<arename, aSTDC, int(const char*, const char*)>
        psrename(0, AR_PATH_INE, AR_PATH_IN);
static path_translator_cdecl<aremove, aSTDC, void*(const char*)>
        psremove(0, AR_PATH_INE);



/*
 *  DirectX Extensions
 *      http://msdn.microsoft.com/en-us/library/windows/desktop/bb172969(v=vs.85).aspx
 */
extern const char aD3DX[] = "";     // Could be any D3DX dll
extern const char aD3DXCreateTextureFromFileA[] = "D3DXCreateTextureFromFileA";
extern const char aD3DXCompileShaderFromFileA[] = "D3DXCompileShaderFromFileA";
extern const char aD3DXAssembleShaderFromFileA[] = "D3DXAssembleShaderFromFileA";
extern const char aD3DXCreateVolumeTextureFromFileA[] = "D3DXCreateVolumeTextureFromFileA";
extern const char aD3DXCreateCubeTextureFromFileA[] = "D3DXCreateCubeTextureFromFileA";
extern const char aD3DXLoadMeshFromXA[] = "D3DXLoadMeshFromXA";
extern const char aD3DXCreateEffectFromFileA[] = "D3DXCreateEffectFromFileA";
extern const char aD3DXSaveSurfaceToFileA[] = "D3DXSaveSurfaceToFileA";

extern const char aD3DXCreateEffectFromFileW[] = "D3DXCreateEffectFromFileW";



// Translators for DirectX
static path_translator_stdcall<aD3DXCreateTextureFromFileA, aD3DX, HRESULT(void*, const char*, void*)>
        psD3DXCreateTextureFromFileA(0, 0, AR_PATH_INE, 0);
static path_translator_stdcall<aD3DXCompileShaderFromFileA, aD3DX, HRESULT(const char*, const void*, void*, void*, void*, DWORD, void*, void*, void*)>
        psD3DXCompileShaderFromFileA(0, AR_PATH_INE, 0, 0, 0, 0, 0, 0, 0, 0);
static path_translator_stdcall<aD3DXAssembleShaderFromFileA, aD3DX, HRESULT(const char*, void*, void*, DWORD, void*, void*)>
    psD3DXAssembleShaderFromFileA(0, AR_PATH_INE, 0, 0, 0, 0, 0);

static path_translator_stdcall<aD3DXCreateVolumeTextureFromFileA, aD3DX, HRESULT(void*, const char*, void*)>
        psD3DXCreateVolumeTextureFromFileA(0, 0, AR_PATH_INE, 0);
static path_translator_stdcall<aD3DXCreateCubeTextureFromFileA, aD3DX, HRESULT(void*, const char*, void*)>
        psD3DXCreateCubeTextureFromFileA(0, 0, AR_PATH_INE, 0);
static path_translator_stdcall<aD3DXLoadMeshFromXA, aD3DX, HRESULT(const char*, DWORD, void*, void*, void*, void*, void*, void*)>
        psD3DXLoadMeshFromXA(0, AR_PATH_INE, 0, 0, 0, 0, 0, 0, 0);
static path_translator_stdcall<aD3DXCreateEffectFromFileA, aD3DX, HRESULT(void*, const char*, void*, void*, DWORD, void*, void*, void*)>
        psD3DXCreateEffectFromFileA(0, 0, AR_PATH_INE, 0, 0, 0, 0, 0, 0);
static path_translator_stdcall<aD3DXSaveSurfaceToFileA, aD3DX, HRESULT(const char*, DWORD, void*, void*, void*)>
        psD3DXSaveSurfaceToFileA(0, AR_PATH_IN, 0, 0, 0, 0);

static path_translator_stdcall<aD3DXCreateEffectFromFileW, aD3DX, HRESULT(void*, const wchar_t*, void*, void*, DWORD, void*, void*, void*)>
        psD3DXCreateEffectFromFileW(0, 0, AR_PATH_INE, 0, 0, 0, 0, 0, 0);



/*
 *  Bass Library
 * 
 */
extern const char aBass[] = "bass.dll";
extern const char aBASS_MusicLoad[]  = "BASS_MusicLoad";
extern const char aBASS_SampleLoad[] = "BASS_SampleLoad";
extern const char aBASS_StreamCreateFile[] = "BASS_StreamCreateFile";

// Translators for bass.dll
static path_translator_stdcall<aBASS_MusicLoad, aBass, DWORD(BOOL, const char*, uint64_t, DWORD, DWORD, DWORD)>
        psBASS_MusicLoad(0, 0, AR_PATH_INE, 0, 0, 0, 0);
static path_translator_stdcall<aBASS_SampleLoad, aBass, DWORD(BOOL, const char*, uint64_t, DWORD, DWORD, DWORD)>
        psBASS_SampleLoad(0, 0, AR_PATH_INE, 0, 0, 0, 0);
static path_translator_stdcall<aBASS_StreamCreateFile, aBass, DWORD(BOOL, const char*, uint64_t, uint64_t, DWORD)>
        psBASS_StreamCreateFile(0, 0, AR_PATH_INE, 0, 0, 0);