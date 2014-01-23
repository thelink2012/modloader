/*
 * Copyright (C) 2013  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 *  Path Translation System 
 *      Redirects ASI system calls that uses a path as input and translates the path to it's actual folder
 * 
 *  This file should be inlined in asi.cpp
 */

#ifndef ARGSTRANSLATOR_HPP
#define	ARGSTRANSLATOR_HPP

#include "asi.h"
#include "Injector.h"
#include <memory>
#include <modloader_util_path.hpp>
using namespace injector;

//
extern const char aKernel32[];
extern const char aGetModuleFileNameA[];

// Argument type
enum eArgsType
{
    AR_DUMMY        = 0,           // Don't touch argument
    AR_PATH_INE,                   // Input path for existing file
    AR_PATH_IN,                    // Input path
};

namespace hacks
{
    // Some interception in GetModuleFileName to be compatible with some ryosuke's plugins
    
    template<class X, class Y>
    inline void RyosukeSetModule(X*,Y)
    {
        asiPlugin->Error("std-asi: RyosukeSetModule called with unknown types!\n");
    }
    
    template<>
    inline void RyosukeSetModule(HMODULE* x, HMODULE asi)
    {
        if(*x == 0) *x = asi;
    }
};


/*
 *  path_translator_base
 *      Path translator for modules, the base.
 *      Virtual methods to communicate with the translator for each symbol
 */
struct path_translator_base
{
    typedef CThePlugin::ModuleInfo ModuleInfo;
    typedef std::vector<path_translator_base*> list_type;
    typedef std::unique_ptr<path_translator_base> smart_ptr;
    
    // List of singletons
    static list_type& List()
    { static list_type a; return a; }
    
    // Tells whether singleton list initialization is done
    static bool& InitializationDone()
    { static bool bDone = false; return bDone; }
    
    // Helper function to clone a translator object
    template<class T> static smart_ptr DoClone(const T* self)
    {
        return smart_ptr(new T(*self));
    }
    
    
    // Pure abstract
    virtual smart_ptr clone()=0;                        // Clone this object.
    virtual const char* GetSymbol()=0;                  // Gets symbol the translator translates for
    virtual const char* GetLibName()=0;                 // Gets library name the translator translates for
    virtual const void* GetWrapper()=0;                 // Gets pointer to the replacement (translator) function
    
    virtual path_translator_base*& GetSingleton()=0;    // Each translator <symbol,lib> must have a singleton
                                                        // for safeness, so we have a safe spot to hide ourselfs when
                                                        // we fail to capture the stack frame to find the caller.

    // Type
    bool bIsSingleton;
    bool bGetModuleFileName;        // Must be set by child
    
    // Almost static vars (he), the value is always the same in all objects
    int npath;              // Num args that uses a path
    char argtype[32];       // eArgsType for [Ret + Args]
    
    // Object-dependent vars
    void* iat;              // Pointer to this func on IAT
    void* fun;              // Original function pointer
    
    
    
    //
    path_translator_base() : npath(0), fun(0), iat(0), bIsSingleton(false)
    { }

    //
    ~path_translator_base()
    {
        Restore();
    }
    
    // Patch address @addr at IAT to point into our wrapper
    void Patch(void* addr)
    {
        this->iat = addr;
        
        // Replace IAT pointers and get original function pointer into @fun
        this->fun = ReadMemory<void*>(raw_ptr(iat), true);
        WriteMemory<const void*>(raw_ptr(iat), this->GetWrapper(), true);
        
        // If the singleton has no function assigned to it yet, assign one
        // This will be our safe exit when we can't capture stack frames ;)
        auto& s = GetSingleton();
        if(s->fun == nullptr) s->fun = this->fun;
    }

    // Unpatches the IAT
    void Restore()
    {
        if(this->iat)
        {
            WriteMemory<void*>(raw_ptr(iat), this->fun, true);
            this->iat = 0;
        }
    }
    
    
    
    // Tries to make this object part of the singleton list
    // Must be called by the child's constructor!!!!
    void SetupSingleton()
    {
        if(InitializationDone() == false)
        {
            this->bIsSingleton = true;
            List().push_back(GetSingleton() = this);
        }
    }
    
    
    // Finishes the RegisterPathType function when there's no more arguments to register
    template<size_t N = 0>
    void RegisterPathType() {}
    
    // Goes registering and identifying the argument types (eArgsType) sent in the constructor
    template<size_t N = 0, class... ArgsType>
    void RegisterPathType(char t, ArgsType... ts)
    {
        if(InitializationDone() == false)   // Still initializing singletons?
        {
            if(t == AR_PATH_INE || t == AR_PATH_IN)
                ++npath;                            // Increase num paths
            this->argtype[N] = t;                   // Register
            return RegisterPathType<N+1>(ts...);    // Next
        }
        else // Just copy stuff from the singleton
        {
            auto& s = GetSingleton();
            this->npath = s->npath;
            memcpy(this->argtype, s->argtype, sizeof(this->argtype));
        }
    }
    
    
    
    // Function that helps in the calling/translation process
    struct CallInfo
    {
        // TODO optimize the pool. Make a actual pool that never needs allocation.
        
        // Size of each element in the character pool
        static const size_t pool_elem_size = MAX_PATH * sizeof(wchar_t);

        std::unique_ptr<char[]> path_pool;  // Character pool
        size_t path_pool_used;              // Size (not number of elements) used in the pool
        
        path_translator_base* base;         // Translator
        ModuleInfo* asi;                    // Pointer to Calling ASI module
        
        // Construct the information
        CallInfo() : asi(0), base(0)
        {}
        
        // Allocates a path in the pool. @size is the num characters the path will need.
        auto_ptr_cast AllocPath(size_t size)
        {
            if(!path_pool)  // Lazy pool allocation ;)
            {
                // Allocate it
                path_pool.reset(new char[base->npath * pool_elem_size]);
                path_pool_used = 0;
            }
            
            // Get space on it
            void* p = &path_pool[path_pool_used];
            path_pool_used += size;
            return auto_ptr_cast(p);
        }
        
        
        // Gets asi and translator information from symbol and library name
        bool SetupASI(ModuleInfo* asi, const char* symbol, const char* libname)
        {
            if(asi)
            {
                // Find translator in asi for symbol and library
                if(this->base = asi->FindTranslatorFrom(symbol, libname))
                {
                    // Yay, we did it!
                    this->asi = asi;
                    return true;
                }
            }
            return false;
        }
        
        
        // Translate path for a unknown type
        template<class T> void TranslatePath(T&& arg, char type)
        {
            // ...He, no, what is this type?
            asiPlugin->Error("std-asi: TranslatePath called with unknown type\n"
                         "Symbol: %s\nLibrary: %s",
                         base->GetSymbol(), base->GetLibName());
        }

        // Gets the current working directory relative to the the game path
        // If working directory is not anywhere ahead the game path, return null
        const char* GetCurrentDir(char* buffer, size_t max)
        {
            char *fullpath = buffer, *currdir = 0;
            
            // Get working directory...
            if(GetCurrentDirectoryA(max, fullpath))
            {
                // Iterate on the game path comparing it with the current working dir
                // Note gamePath ends with a '\\'
                const char* gamePath = asiPlugin->modloader->gamepath;
                for(size_t i = 0; i < max; ++i)
                {
                    if(gamePath[i] == 0)    // End of game path?
                    {
                        // Then here starts the relative part
                        currdir = &fullpath[i];
                        break;
                    }
                    else if(gamePath[i] != fullpath[i]) // Piece of gamepath not equal to the working dir? wow
                    {
                        // Let's calm down, if working directory ended and game path is ending, we're still 'equal'
                        if(fullpath[i] == 0 && gamePath[i] == '\\' && gamePath[i+1] == 0)
                        {
                            // Point current directory to "\0" part of fullpath
                            currdir = &fullpath[i];
                        }
                        break;
                    }
                }
            }
            
            return currdir;
        }
        
        /*
         *  Translates path @arg for module @asi in a hacky way if necessary 
         */
        template<class T, class F>
        bool TranslatePathHacky(const T*& arg, char type, F build_path)
        {
            return false;
        }
        
        /*
         *  Translates path @arg for module @asi
         */
        template<class T, class F>
        void TranslatePath(const T*& arg, char type, F build_path)
        {
            const int path_size = MAX_PATH * sizeof(T);
            char buffer[MAX_PATH];
            
            // If it's not a absolute path, go ahead for the translation
            if(!IsAbsolutePath(arg))
            {
                // If this is the module for the main game executable, calm down! It's special of course...
                if(asi->bIsMainExecutable)
                {
                    // If the argument type is a INPUT THAT EXISTS, check if it exists on base path...
                    // if not, try on the ASI paths.
                    if(type == AR_PATH_INE && !IsPath(arg))
                    {
                        if(auto* currdir = GetCurrentDir(buffer, sizeof(buffer)))
                        {
                            T* p = AllocPath(path_size);
                            
                            // Iterate on each asi module trying to find a file in it's path
                            for(auto& module : asiPlugin->asiList)
                            {
                                if(!module.bIsMainExecutable)
                                {
                                    if(IsPath(build_path(p, module.folder.c_str(), currdir, arg)))
                                    {
                                        arg = p;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
                else
                {
                    // Translate path for ASI files in the following manner "$ASI_PATH/$CWD/$ARGUMENT"
                    if(auto* currdir = GetCurrentDir(buffer, sizeof(buffer)))
                    {
                        if(TranslatePathHacky(arg, type, build_path) == false)
                            arg = build_path(AllocPath(path_size), asi->folder.c_str(), currdir, arg);
                    }
                }
            }
        }
        
        // Translate path for const char* type
        void TranslatePath(const char*& arg, char type)
        {
            TranslatePath(arg, type, [](char* out, const char* prefix, const char* currdir, const char* suffix)
            {
                if(currdir)
                {
                    if(suffix)
                        sprintf(out, "%s\\%s\\%s", prefix, currdir, suffix);
                    else
                        sprintf(out, "%s\\%s", prefix, currdir);
                }
                else
                    sprintf(out, "%s", prefix);
                return out;
            });
            //printf(">>%s<< %s:%s at %s\n", arg, base->GetLibName(), base->GetSymbol(), asi->path.data());
        }
        
        // Translate path for const wchar_t* type
        void TranslatePath(const wchar_t*& arg, char type)
        {
            TranslatePath(arg, type, [](wchar_t* out, const char* prefix, const char* currdir, const wchar_t* suffix)
            {
                const int size = pool_elem_size / sizeof(wchar_t);
                if(currdir)
                {
                    if(suffix)
                        swprintf(out, size, L"%s\\%s\\%ls", prefix, currdir, suffix);
                    else
                        swprintf(out, size, L"%s\\%s", prefix, currdir);
                }
                else
                    swprintf(out, size, L"%s", prefix);
                return out;
            });
        }
        
        
        // Finishes the TranslateForCall function when there's no more arguments to translate
        template<size_t N = 1>
        void TranslateForCall()
        { }

        // Translates path argument @x at position @N
        template<size_t N = 1, class T, class... A>
        void TranslateForCall(T& x, A&&... a)
        {
            // Is it a input path?
            if(base->argtype[N] == AR_PATH_IN || base->argtype[N] == AR_PATH_INE)
            {
                TranslatePath(x, base->argtype[N]);
            }
            else
            {
                // Hack for ryosuke's plugins
                if(base->bGetModuleFileName && asi->hacks.bRyosukeModuleName)
                {
                    // If first argument for GetModuleFileName in a Ryosuke plugin
                    // override this argument with the plugin module instead of a null module.
                    if(N == 1) hacks::RyosukeSetModule(&x, asi->module);
                }
            }
            
            // Translate the next parameter
            return TranslateForCall<N+1>(a...);
        }
    };

};


/*
 *  path_translator_xbase
 *      Now, this is the one that instantiates each Symbol X Library
 *      Must have child objects for each calling convenition! 
 */
template<const char* Symbol, const char* LibName> 
struct path_translator_xbase : public path_translator_base
{
    static path_translator_base*& StaticGetSingleton()
    { static path_translator_base* a; return a; }
    
    //
    virtual path_translator_base*& GetSingleton() { return StaticGetSingleton(); }
    virtual const char* GetSymbol()  { return Symbol; }
    virtual const char* GetLibName() { return LibName; }
    
    path_translator_xbase()
    {
        SetupSingleton();
        bGetModuleFileName = (Symbol == aGetModuleFileNameA && LibName == aKernel32);
    }
    
    // Let's extend CallInfo to work with out singleton, so we'll operate safe!
    struct CallInfo : public path_translator_base::CallInfo
    {
        // Gets information from pointer @pReturn inside a module
        bool FindInfo(void* pCaller)
        {
            static bool bAntiFlood = false;
            
            if(pCaller) // Succesfully got caller pointer?
            {
                if(SetupASI(asiPlugin->FindModuleFromAddress(pCaller), Symbol, LibName))
                    return true;
                else
                    asiPlugin->Error("std-asi: translator.SetupASI failed to identify caller ASI!");
            }
            else
            {
                // We don't have the caller pointer!!! WHAT TO DO?!?!?!?!
                // Let's calm down, and use our singleton instead ;)
                this->base = StaticGetSingleton();
                
                if(!bAntiFlood)
                {
                    // Let's log about this situation we have here...
                    asiPlugin->Log("Warning: std-asi failed to CaptureStackBackTrace for %s:%s! Trying singleton!",
                                   LibName, Symbol);
                    bAntiFlood = true;
                }
            }
            return false;
        }
    };
};



/*
 *  Path translator for stdcall functions (WINAPI functions)
 */

template<const char* Symbol, const char* LibName, class Prototype>
struct path_translator_stdcall;

template<const char* Symbol, const char* LibName, class Ret, class... Args>
struct path_translator_stdcall<Symbol, LibName, Ret(Args...)> : public path_translator_xbase<Symbol, LibName>
{
    typedef path_translator_xbase<Symbol, LibName> super;
    typedef CThePlugin::ModuleInfo ModuleInfo;
    typedef Ret(__stdcall *func_type)(Args...);
    static const int num_args = sizeof...(Args);        // number of Args

    /*
     *  Virtual methods 
     */
    path_translator_base::smart_ptr clone()
    { return path_translator_base::DoClone<path_translator_stdcall>(this); }
    const void* GetWrapper() { return (void*)(&call); }

    // Constructs the object, pass the argument types (arg 0 is return type)
    template<class... ArgsType>  // All args should be 'char'
    path_translator_stdcall(ArgsType... t)
    {
        static_assert(sizeof...(t) == 1 + num_args, "Invalid num arguments on constructor");
        path_translator_base::RegisterPathType(t...);     // Register types
    }
    
    
    // The wrapper function that translates the path
    static Ret __stdcall call(Args... a)
    {
        typename super::CallInfo info;
        void* pReturn;

        // Get pointer to a address in the caller module (the return pointer actually)...
        if(!CaptureStackBackTrace(1, 1, &pReturn, 0)) pReturn = nullptr;
        
        // Find the ASI information from the caller return pointer...
        if(info.FindInfo(pReturn))
        {
            // Translate the paths
            info.TranslateForCall(a...);    
        }
            
        // Call the original function
        auto f = (func_type) info.base->fun;
        Ret result = f(a...);
        
        return result;
    }

};



/*
 *  Path translator for cdecl functions (CSTD functions)
 */

template<const char* Symbol, const char* LibName, class Prototype>
struct path_translator_cdecl;

template<const char* Symbol, const char* LibName, class Ret, class... Args>
struct path_translator_cdecl<Symbol, LibName, Ret(Args...)> : public path_translator_xbase<Symbol, LibName>
{
    typedef path_translator_xbase<Symbol, LibName> super;
    typedef CThePlugin::ModuleInfo ModuleInfo;
    typedef Ret(__cdecl *func_type)(Args...);
    static const int num_args = sizeof...(Args);        // number of Args

    /*
     *  Virtual methods 
     */
    path_translator_base::smart_ptr clone()
    { return path_translator_base::DoClone<path_translator_cdecl>(this); }
    const void* GetWrapper() { return (void*)(&call); }

    // Constructs the object, pass the argument types (arg 0 is return type)
    template<class... ArgsType>  // All args should be 'char'
    path_translator_cdecl(ArgsType... t)
    {
        static_assert(sizeof...(t) == 1 + num_args, "Invalid num arguments on constructor");
        path_translator_base::RegisterPathType(t...);     // Register types
    }
    
    
    // The wrapper function that translates the path
    static Ret __cdecl call(Args... a)
    {
        typename super::CallInfo info;
        void* pReturn;

        // Get pointer to a address in the caller module (the return pointer actually)...
        if(!CaptureStackBackTrace(1, 1, &pReturn, 0)) pReturn = nullptr;
        
        // Find the ASI information from the caller return pointer...
        if(info.FindInfo(pReturn)) info.TranslateForCall(a...);    // Translate the paths
        
        // Call the original function
        auto f = (func_type) info.base->fun;
        Ret result = f(a...);
        
        return result;
    }

};


#endif	/* ARGSTRANSLATOR_HPP */

