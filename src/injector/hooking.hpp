/*
 *  Injectors - Classes for making your hooking life easy
 *	Header with helpful stuff for ASI memory hacking
 *
 *  (C) 2012-2014 LINK/2012 <dma_2012@hotmail.com>
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
#pragma once
#include "injector.hpp"
#include <cassert>

namespace injector
{
    /*
     * 
     */
    template<uintptr_t addr1, class Prototype>
    struct function_hooker;

    template<uintptr_t addr1, class Ret, class ...Args>
    struct function_hooker<addr1, Ret(Args...)>
    {
        public:
            static const uintptr_t addr = addr1;
            typedef Ret(*func_type)(Args...);
            typedef Ret(*hook_type)(func_type, Args&...);

        protected:
            
            // Stores the previous function pointer
            static func_type& original()
            { static func_type f; return f; }
            
            // Stores our hook pointer
            static hook_type& hook()
            { static hook_type h; return h; }

            // The hook caller
            static Ret call(Args... a)
            {
                return hook()(original(), a...);
            }

        public:
            // Constructs passing information to the static variables
            function_hooker(hook_type hooker)
            {
                hook() = hooker;
                original() = MakeCALL(addr, raw_ptr(call)).get();
            }
            
            // Restores the previous call before the hook happened
            static void restore()
            {
                MakeCALL(addr, raw_ptr(original()));
            }
    };
    
    //
    //
    template<uintptr_t addr1, class Prototype>
    struct function_hooker_fastcall;

    template<uintptr_t addr1, class Ret, class ...Args>
    struct function_hooker_fastcall<addr1, Ret(Args...)>
    {
        public:
            static const uintptr_t addr = addr1;
            typedef Ret(__fastcall *func_type)(Args...);
            typedef Ret(*hook_type)(func_type, Args&...);

        protected:
            
            // Stores the previous function pointer
            static func_type& original()
            { static func_type f; return f; }
            
            // Stores our hook pointer
            static hook_type& hook()
            { static hook_type h; return h; }

            // The hook caller
            static Ret __fastcall call(Args... a)
            {
                return hook()(original(), a...);
            }

        public:
            // Constructs passing information to the static variables
            function_hooker_fastcall(hook_type hooker)
            {
                hook() = hooker;
                original() = MakeCALL(addr, raw_ptr(call)).get();
            }
            
            // Restores the previous call before the hook happened
            static void restore()
            {
                MakeCALL(addr, raw_ptr(original()));
            }
    };
    
    template<uintptr_t addr1, class Prototype>
    struct function_hooker_stdcall;

    template<uintptr_t addr1, class Ret, class ...Args>
    struct function_hooker_stdcall<addr1, Ret(Args...)>
    {
        public:
            static const uintptr_t addr = addr1;
            typedef Ret(__stdcall *func_type)(Args...);
            typedef Ret(*hook_type)(func_type, Args&...);

        protected:
            
            // Stores the previous function pointer
            static func_type& original()
            { static func_type f; return f; }
            
            // Stores our hook pointer
            static hook_type& hook()
            { static hook_type h; return h; }

            // The hook caller
            static Ret __stdcall call(Args... a)
            {
                return hook()(original(), a...);
            }

        public:
            // Constructs passing information to the static variables
            function_hooker_stdcall(hook_type hooker)
            {
                hook() = hooker;
                original() = MakeCALL(addr, raw_ptr(call)).get();
            }
            
            // Restores the previous call before the hook happened
            static void restore()
            {
                MakeCALL(addr, raw_ptr(original()));
            }
    };
    
    
    
    /* Helpers */
    template<class T> inline
    T make_function_hook(typename T::hook_type hooker)
    {
        return T(hooker);
    }
    
    template<uintptr_t addr, class T, class U> inline
    function_hooker<addr, T> make_function_hook(U hooker)
    {
        typedef function_hooker<addr, T> type;
        return make_function_hook<type>(hooker);
    }

    
    /*
     *  RAII wrapper for the functions hookers
     */
    template<class T>
    struct scoped_hook
    {
        scoped_hook(typename T::hook_type hooker)   { make_function_hook<T>(hooker); }
        scoped_hook()                               {}
        scoped_hook(const T&)                       {}
        ~scoped_hook()                              { T::restore(); }
    };

    
    
    
    
    
    
    
    
    
    
    
    /*
     *  Basic interface for the RAII wrappers present on this header
     */
    template<size_t bufsize>
    struct scoped_basic
    {
        protected:
            memory_pointer_raw addr;        // Data saved from this address
            size_t  size;                   // Size saved
            uint8_t buf[bufsize];           // Saved buffer
            bool    saved;                  // Something saved?
            bool    vp;                     // Virtual protect?
        
        public:
            
            // Constructor, initialises
            scoped_basic() : saved(false)
            {}
            
            // Destructor, restore previous overwritten buffer
            ~scoped_basic()
            {
                this->restore();
            }
            
            // Restore the previosly saved data
            void restore()
            {
                if(this->saved)
                {
                    WriteMemoryRaw(this->addr, this->buf, this->size, this->vp);
                    this->saved = false;
                }
            }

            // Save buffer at @addr with @size and virtual protect @vp
            void save(memory_pointer_tr addr, size_t size, bool vp)
            {
                assert(size <= bufsize);            // Debug Safeness
                this->restore();                    // Restore anything we have saved
                this->saved = true;                 // Mark that we have data save
                this->addr = addr.get<void>();      // Save address
                this->size = size;                  // Save size
                this->vp = vp;                      // Save virtual protect
                ReadMemoryRaw(addr, buf, size, vp); // Save buffer
            }
    };
    
    /*
     *  RAII wrapper for memory writes
     *  Can save only basic and POD types
     */
    template<size_t bufsize = 10>
    struct scoped_write : public scoped_basic<bufsize>
    {
        public:
            // Save buffer at @addr with @size and virtual protect @vp and then overwrite it with @value
            void write(memory_pointer_tr addr, void* value, size_t size, bool vp)
            {
                this->save(addr, size, vp);
                return WriteMemoryRaw(addr, value, size, vp);
            }

            // Save buffer at @addr with size sizeof(@value) and virtual protect @vp and then overwrite it with @value
            template<class T>
            void write(memory_pointer_tr addr, T value, bool vp = false)
            {
                this->save(addr, sizeof(T), vp);
                return WriteMemory<T>(addr, value, vp);
            }
    };

    /*
     *  RAII wrapper for filling
     */
    template<size_t bufsize = 5>
    struct scoped_fill : public scoped_basic<bufsize>
    {
        public:
            // Fills memory at @addr with value @value and size @size and virtual protect @vp
            void fill(memory_pointer_tr addr, uint8_t value, size_t size, bool vp)
            {
                this->save(addr, size, vp);
                return MemoryFill(addr, value, size, vp);
            }
    };
    
    /*
     *  RAII wrapper for nopping
     */
    template<size_t bufsize = 5>
    struct scoped_nop : public scoped_basic<bufsize>
    {
        public:
            // Makes NOP at @addr with value @value and size @size and virtual protect @vp
            void make_nop(memory_pointer_tr addr, size_t size = 1, bool vp = true)
            {
                this->save(addr, size, vp);
                return MakeNOP(addr, size, vp);
            }
    };
    
    /*
     *  RAII wrapper for MakeJMP 
     */
    struct scoped_jmp : public scoped_basic<5>
    {
        public:
            // Makes NOP at @addr with value @value and size @size and virtual protect @vp
            memory_pointer_raw make_jmp(memory_pointer_tr at, memory_pointer_tr dest, bool vp = true)
            {
                this->save(at, 5, vp);
                return MakeJMP(at, dest, vp);
            }
    };
    
    /*
     *  RAII wrapper for MakeCALL 
     */
    struct scoped_call : public scoped_basic<5>
    {
        public:
            // Makes NOP at @addr with value @value and size @size and virtual protect @vp
            memory_pointer_raw make_call(memory_pointer_tr at, memory_pointer_tr dest, bool vp = true)
            {
                this->save(at, 5, vp);
                return MakeCALL(at, dest, vp);
            }
    };
    

} 
