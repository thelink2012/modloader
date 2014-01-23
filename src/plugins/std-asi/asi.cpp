/*
 * Copyright (C) 2013  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 *  std-asi -- Standard ASI Loader Plugin for San Andreas Mod Loader
 *      Loads ASI files as libraries
 * 
 */
#include "asi.h"
#include "args_translator.hpp"
#include <modloader_util.hpp>
#include <map>


//
CThePlugin* asiPlugin;
static CThePlugin plugin;

// <lower_case_asi_name, file_size>
static std::map<std::string, size_t> incompatible;


/*
 *  Export plugin object data
 */
extern "C" __declspec(dllexport)
void GetPluginData(modloader_plugin_t* data)
{
    asiPlugin = &plugin;
    modloader::RegisterPluginData(plugin, data, plugin.default_priority);
}

/*
 *  Basic plugin informations
 */
const char* CThePlugin::GetName()
{
    return "std-asi";
}

const char* CThePlugin::GetAuthor()
{
    return "LINK/2012";
}

const char* CThePlugin::GetVersion()
{
    return "0.8";
}

const char** CThePlugin::GetExtensionTable()
{
    static const char* table[] = { "asi", "dll", 0 };
    return table;
}

/*
 *  Startup / Shutdown
 */
bool CThePlugin::OnStartup()
{
    // Register GTA module for some arg translation
    this->asiList.emplace_front("gta");
    
    // Register incompatibilities
    incompatible.emplace("thebirdsupdate.asi", 23552);
    incompatible.emplace("ragdoll.asi", 198656);
    incompatible.emplace("normalmap.asi", 83456);
    incompatible.emplace("outfit.asi", 88064);
    incompatible.emplace("colormod.asi", 111104);
    incompatible.emplace("dof.asi", 110592);
    incompatible.emplace("bullet.asi", 97280);
    incompatible.emplace("google.asi", 112128);
    incompatible.emplace("airlimit.asi", 79872);
    incompatible.emplace("killlog.asi", 98816);
    incompatible.emplace("weaponlimit.asi", 99840);
    
    return true;
}

bool CThePlugin::OnShutdown()
{
    // Free the asi files we loaded
    for(auto& asi : this->asiList) asi.Free();
    return true;
}

/*
 *  Check if the file is the one we're looking for
 */
bool CThePlugin::CheckFile(modloader::ModLoaderFile& file)
{
    if(!file.is_dir)
    {
        if(IsFileExtension(file.filext, "asi"))
        {
            // Don't load CLEO neither modloader!
            if(!strcmp(file.filename, "cleo.asi", false) || !strcmp(file.filename, "modloader.asi", false))
            {
                Log("Warning: Forbidden ASI file found \"%s\"", GetFilePath(file).c_str());
                return false;
            }
        }
        else if(strcmp(file.filename, "d3d9.dll", false) != 0)
            return false;
        
        // Check out if the file is incompatible
        std::string filename = file.filename;
        auto it = incompatible.find(modloader::tolower(filename));
        if(it != incompatible.end())
        {
            auto size = modloader::GetFileSize(file.filepath);
            if(size == it->second)
            {
                Error("Incompatible ASI file found: %s\nPlease install it at the game root directory.\nSkipping it!",
                      file.filename);
                return false;
            }
        }
        return true;
    }
    return false;
}

/*
 * Process the replacement
 */
bool CThePlugin::ProcessFile(const modloader::ModLoaderFile& file)
{
    this->asiList.emplace_back(GetFilePath(file));
    return true;
}

/*
 * Called after all files have been processed
 */
bool CThePlugin::PosProcess()
{
    // Iterate on the asi list loading each asi file
    for(auto& asi : this->asiList)
    {
        scoped_chdir xdir(asi.folder.c_str());
        SetLastError(0);
        
        // ....
        bool bLoaded = asi.Load();
        if(asi.bIsMainExecutable == false)
        {
            Log("[%s] %s \"%s\"; errcode: 0x%X",
                (asi.bIsASI? "ASI" : asi.bIsD3D9? "D3D9" : "???"),
                (bLoaded? "Module has been loaded:" : "Failed to load module"),
                asi.path.c_str(),
                GetLastError());
        }
    }
    return true;
}




/*
 *  Constructs a ModuleInfo object
 *  It takes a rvalue for a relative path string
 */
CThePlugin::ModuleInfo::ModuleInfo(std::string&& path)
{
    size_t last = GetLastPathComponent(path);
    bIsASI = bIsMainExecutable = bIsD3D9 = false;
    memset(&this->hacks, 0, sizeof(hacks));
    
    const char* filename = &path[last];
    
    // Setup flags
    if(!strcmp(filename, "d3d9.dll", false))
        bIsD3D9 = true;
    else if(!strcmp(filename, "gta", false))
        bIsMainExecutable = true;
    else
        bIsASI = true;
    
    // Setup hacks flags
    if(!strcmp("shell.asi", filename, false)    // Ryosuke's Shell Mod
    || !strcmp("Render.asi", filename, false))  // SA::Render
    {
        hacks.bRyosukeModuleName = true;
    }
    
    
    // Setup fields
    this->module = 0;
    this->folder = path.substr(0, last);
    this->name   = filename;
    this->path = std::move(path);
}
            
/*
 *  Loads the module assigned to our field path 
 */
bool CThePlugin::ModuleInfo::Load()
{
    if(!this->module)
    {
        // We need the fullpath into the module because of the way Windows load dlls
        // More info at: http://msdn.microsoft.com/en-us/library/windows/desktop/ms682586(v=vs.85).aspx
        char fullpath[MAX_PATH];
        if(GetFullPathNameA(name.c_str(), sizeof(fullpath), fullpath, 0))
        {
            // Let Windows search for dependencies in this folder too
            //SetDllDirectoryA(test.c_str());       -- Doesn't work on some older Wine version... grh...
            {
                // Load the library module into our module field
                this->module = bIsMainExecutable? GetModuleHandleA(0) : LoadLibraryA(fullpath);
                
                // Patch the module imports to pass throught args translation.
                if(this->module) this->PatchImports();
            }
            //SetDllDirectoryA(NULL);
        }
    }
    return this->module != 0;
}
            
/*
 *  Unloads the module assigned to this object 
 */
void CThePlugin::ModuleInfo::Free()
{
    if(this->module)
    {
        this->RestoreImports();
        
        if(!bIsMainExecutable)
        {
            FreeLibrary(module);
            this->module = 0;
        }
    }
 }

/*
 *  Translator finder for a specific ASI object 
 */
path_translator_base* CThePlugin::ModuleInfo::FindTranslatorFrom(const char* symbol, const char* libname)
{
    // Find translator for symbol and libname
    for(auto& t : this->translators)
    {
        // Yes, we can compare the c-string pointers in this context ;)
        if(t->GetSymbol() == symbol && t->GetLibName() == libname)
            return t.get();
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
void CThePlugin::ModuleInfo::PatchImports()
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
            fname = rva_to_ptr(lib->OriginalFirstThunk);
            faddr = rva_to_ptr(lib->FirstThunk);
            
            // Iterate on each name to see if we should patch it
            for(; fname->u1.Function; ++fname, ++faddr)
            {
                // Is this just a ordinal import? Skip it, we don't have a symbol name!
                if(fname->u1.Ordinal & IMAGE_ORDINAL_FLAG) continue;
                
                // Get the symbol name
                const char* symbolName = (char*)(((IMAGE_IMPORT_BY_NAME*)(rva_to_ptr(fname->u1.AddressOfData)))->Name);
                
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
void CThePlugin::ModuleInfo::RestoreImports()
{
    this->translators.clear();
}


/* ----------------------------------------------------------------------------------------------------------------------- */

// TODO wchar version of everything below


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
extern const char aSetCurrentDirectoryA[] = "SetCurrentDirectoryA";
extern const char aGetPrivateProfileIntA[] = "GetPrivateProfileIntA";
extern const char aGetPrivateProfileSectionA[] = "GetPrivateProfileSectionA";
extern const char aGetPrivateProfileSectionNamesA[] = "GetPrivateProfileSectionNamesA";
extern const char aGetPrivateProfileStringA[] = "GetPrivateProfileStringA";
extern const char aGetPrivateProfileStructA[] = "GetPrivateProfileStructA";
extern const char aWritePrivateProfileSectionA[] = "WritePrivateProfileSectionA";
extern const char aWritePrivateProfileStringA[] = "WritePrivateProfileStringA";
extern const char aWritePrivateProfileStructA[] = "WritePrivateProfileStructA";

// TODO MORE SYMBOLS

// Operations
static path_translator_stdcall<aCreateFileA, aKernel32, HANDLE(LPCTSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE)>
        psCreateFileA(0, AR_PATH_INE, 0, 0, 0, 0, 0, 0);
static path_translator_stdcall<aFindFirstFileA, aKernel32, HMODULE(LPCTSTR, LPWIN32_FIND_DATA)>
        psFindFirstFileA(0, AR_PATH_IN, 0);
//static path_translator_stdcall<aSetCurrentDirectoryA, aKernel32, BOOL(LPCTSTR)>
//        psSetCurrentDirectoryA(0, AR_PATH_INE);


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
        psfopen(0, AR_PATH_IN, 0);
static path_translator_cdecl<afreopen, aSTDC, void*(const char*, const char*, void*)>
        psfreopen(0, AR_PATH_IN, 0, 0);
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



// TODO MORE ^ 

