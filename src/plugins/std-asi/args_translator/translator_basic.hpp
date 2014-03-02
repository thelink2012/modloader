/*
 * Copyright (C) 2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 * Arguments Translation System 
 *      Base and basic classes
 * 
 */

#ifndef ARGS_TRANSLATOR_BASIC_HPP
#define	ARGS_TRANSLATOR_BASIC_HPP

#include "../asi.h"
#include "Injector.h"
#include <memory>
#include <charov.hpp>
#include <type_traits>
#include <modloader_util_path.hpp>
#include <modloader_util_container.hpp>

#include "xtranslator.hpp"
#include "hacks/RyosukeSetModule.hpp"

using namespace injector;
using namespace cwc;


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
    bool bFindFirstFile;            // ^
    bool bFindNextFile;             // ^
    bool bFindClose;                // ^
    
    // Almost static vars (he), the value is always the same in all objects
    int npath;              // Num args that uses a path
    char argtype[32];       // eArgsType for [Ret + Args]
    
    // Object-dependent vars
    void* iat;              // Pointer to this func on IAT
    void* fun;              // Original function pointer
    
    
    
    //
    path_translator_base() : npath(0), fun(0), iat(0), bIsSingleton(false)
    { }

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
        
        
        // Attaches @asi and translator from @symbol and @libname into this object
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
            
            // Hen, symbol or library not present in this asi
            return false;
        }
        
        
        
        
        
        // Finishes the TranslateForCall function when there's no more arguments to translate
        template<size_t N = 1>
        void TranslateForCall()
        { }

        // Translates argument @x at position @N
        template<size_t N = 1, class T, class... A>
        void TranslateForCall(T& x, A&&... a)
        {
            // Is it a input path?
            if(base->argtype[N] == AR_PATH_IN || base->argtype[N] == AR_PATH_INE)
            {
                // Translate this path to be side by side with the asi path
                TranslatePath(x, base->argtype[N]);
            }
            else
            {
                // Hack for ryosuke's plugins
                if(base->bGetModuleFileName && asi->hacks.bRyosukeModuleName)
                {
                    // If translating first argument for GetModuleFileName in a Ryosuke plugin
                    // override this argument with the plugin module instead of a (NULL) module.
                    if(N == 1) hacks::RyosukeSetModule(&x, asi->module);
                }
            }
            
            // Translate the next parameter
            return TranslateForCall<N+1>(a...);
        }
        
        

        // Translate path for a unknown type, something is wrong if this thing gets called
        template<class T> void TranslatePath(T&&, char)
        {
            // ...He, no, what is this type?
            asiPlugin->Error("std-asi: TranslatePath called with unknown type\n"
                         "Symbol: %s\nLibrary: %s",
                         base->GetSymbol(), base->GetLibName());
        }
      
        // Translate path for char type
        void TranslatePath(const char*& arg, char type)
        {
            return TranslatePathChar(arg, type);
        }
        
        // Translate path for wide char type
        void TranslatePath(const wchar_t*& arg, char type)
        {
            return TranslatePathChar(arg, type);
        }
        

        // Translate path for some character type (char or wchar) type
        template<class T>
        void TranslatePathChar(const T*& arg, char type)
        {
            // Call the path translator giving a functor that builds the output
            TranslatePath(arg, type, [](T* out, const char* prefix, const char* currdir, const T* suffix)
            {
                typedef typename std::decay<T>::type decayed_type;
                static const bool bIsWideChar = std::is_same<decayed_type, wchar_t>::value;
                
                // Character independent strings
                static const T a1[] = { '%', 's', '\0' };                   // "%s"
                static const T a2[] = { '%', 's', '\\', '%', 's', '\0' };   // "%s\\%s"
                static const T a3[] = { '%', 's', '\\', '%', 's', '\\',     // "%s\\%s\\%s" or "%s\\%s\\%ls"
                                        '%', bIsWideChar? 'l' : 's', bIsWideChar? 's' : '\0',
                                        '\0' };

                // Build output string based on received prefixes / suffixes / whatever
                if(currdir)
                {
                    if(suffix)
                        sprintf(out, a3, prefix, currdir, suffix);  // "%s\\%s\\%s" or "%s\\%s\\%ls"
                    else
                        sprintf(out, a2, prefix, currdir);          // "%s\\%s"
                }
                else
                    sprintf(out, a1, prefix);   // "%s"
                
                // Done
                return out;
            });
        }

        /*
         *  Translates path @arg for module @asi
         */
        template<class T, class F>
        void TranslatePath(const T*& arg, char type, F build_path)
        {
            // If it isn't a absolute path, go ahead for the translation
            if(!IsAbsolutePath(arg))
            {
                if(asi->bIsMainExecutable)
                {
                    // If this is the module for the main game executable, calm down! It's special of course...
                    return TranslatePathForMainExecutable(arg, type, build_path);
                }
                else if(asi->bIsMainCleo)
                {
                    // If this is the CLEO.asi module, translate path for a cs script
                    return TranslatePathForCleo(arg, type, build_path);
                }
                else if(asi->bIsASI || asi->bIsD3D9)
                {
                    // If this is an ASI plugin module, translate path to be relative to it
                    return TranslatePathForASI(arg, type, build_path);
                }
            }
        }

        
        // The actual path translation goes on those functions, really specific
        template<class T, class F> void TranslatePathForMainExecutable(const T*&, char, F);
        template<class T, class F> void TranslatePathForCleo(const T*&, char, F);
        template<class T, class F> void TranslatePathForASI(const T*&, char, F);
        
        
        
        
        
        
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
        
    };

};


/*
 *  path_translator_basic
 *      Now, this is the one that instantiates each Symbol X Library
 *      Must have child objects for each calling convenition! 
 */
template<const char* Symbol, const char* LibName> 
struct path_translator_basic : public path_translator_base
{
    // Store singleton for this Symbol X Library
    static path_translator_base*& StaticGetSingleton()
    { static path_translator_base* a; return a; }
    
    //
    virtual path_translator_base*& GetSingleton() { return StaticGetSingleton(); }
    virtual const char* GetSymbol()  { return Symbol; }
    virtual const char* GetLibName() { return LibName; }
    
    // Constructor, initialization
    path_translator_basic()
    {
        this->SetupSingleton();
        bGetModuleFileName = (Symbol == aGetModuleFileNameA && LibName == aKernel32);
        bFindFirstFile     = (Symbol == aFindFirstFileA && LibName == aKernel32);
        bFindNextFile      = (Symbol == aFindNextFileA && LibName == aKernel32);
        bFindClose         = (Symbol == aFindClose && LibName == aKernel32);
    }
    
    // Let's extend CallInfo to work with out singleton, so we'll operate safe!
    struct CallInfo : public path_translator_base::CallInfo
    {
        // Gets information from pointer @pReturn inside a module
        bool FindInfo(void* pCaller)
        {
            static bool bAntiFlood = false; // Avoid flooding error message
            
            if(pCaller) // Succesfully got caller pointer?
            {
                // Identify the caller ASI from code segment pointer
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
                
                // Log about failure on this symbol/library only once
                if(!bAntiFlood)
                {
                    // Let's log about this situation we have here...
                    asiPlugin->Log("Warning: std-asi failed to CaptureStackBackTrace for %s:%s! Trying singleton!",
                                   LibName, Symbol);
                    bAntiFlood = true;
                }
            }
            
            // We failed to find the asi module from the caller pointer...
            return false;
        }
    };
    
    
    
};


#endif
