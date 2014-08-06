/*
 *  Injectors - Classes for making your hooking life easy
 *
 *  Copyright (C) 2012-2014 LINK/2012 <dma_2012@hotmail.com>
 *
 *  This software is provided 'as-is', without any express or implied
 *  warranty. In no event will the authors be held liable for any damages
 *  arising from the use of this software.
 * 
 *  Permission is granted to anyone to use this software for any purpose,
 *  including commercial applications, and to alter it and redistribute it
 *  freely, subject to the following restrictions:
 * 
 *     1. The origin of this software must not be misrepresented; you must not
 *     claim that you wrote the original software. If you use this software
 *     in a product, an acknowledgment in the product documentation would be
 *     appreciated but is not required.
 * 
 *     2. Altered source versions must be plainly marked as such, and must not be
 *     misrepresented as being the original software.
 * 
 *     3. This notice may not be removed or altered from any source
 *     distribution.
 *
 */
#pragma once
#include "injector.hpp"
#include <cassert>
#include <functional>

namespace injector
{

/*
    The content between the #pragma pack(1) is very...... hacky
    ............... don't get angry ....................
*/

#pragma pack(push, 1)

    /*
     *  Base interface for the RAII wrappers present on this header
     *  This object shouldn't be instantiated, instantiate scoped_basic or any of it's childs
     */
    struct scoped_base
    {
        public:
            memory_pointer_raw addr;        // Data saved from this address
            size_t  size;                   // Size saved
            const size_t  bufsize;          // IMUTABLE - We need to store this because of our casting rule, the ctor of scoped_basic should set this
            bool    saved;                  // Something saved?
            bool    vp;                     // Virtual protect?
            bool    _rsv1, _rsv2;           // Padding
            uint8_t buf[1];                 // Saved buffer start

        public:
            // Restore the previosly saved data
            // Problems may happen if someone else hooked the same place using the same method
            void restore()
            {
                #ifndef INJECTOR_SCOPED_NOSAVE_NORESTORE
                    if(this->saved)
                    {
                        WriteMemoryRaw(this->addr, this->buf, this->size, this->vp);
                        this->saved = false;
                    }
                #endif
            }

            // Save buffer at @addr with @size and virtual protect @vp
            void save(memory_pointer_tr addr, size_t size, bool vp)
            {
                #ifndef INJECTOR_SCOPED_NOSAVE_NORESTORE
                    assert(size <= bufsize);            // Debug Safeness
                    this->restore();                    // Restore anything we have saved
                    this->saved = true;                 // Mark that we have data save
                    this->addr = addr.get<void>();      // Save address
                    this->size = size;                  // Save size
                    this->vp = vp;                      // Save virtual protect
                    ReadMemoryRaw(addr, buf, size, vp); // Save buffer
                #endif
            }

            template<class To, class From>
            static To& cast(From& s) { return *static_cast<To*>(&s); }

            template<class To>
            To& cast() { return cast<To>(*this); }

        protected:
            // Constructor, initialises
            scoped_base(size_t bufsize) : saved(false), bufsize(bufsize)
            {}
            
            // Destructor, restore previous overwritten buffer
            ~scoped_base()
            {
                this->restore();
            }

            // No copy construction, we can't do this! Sure we can move construct :)
            scoped_base(const scoped_base&) = delete;
            scoped_base(size_t bufsize, scoped_base&& rhs) : bufsize(bufsize)
            {
                *this = std::move(rhs);
            }

        public:
            scoped_base& operator=(const scoped_base& rhs) = delete;
            scoped_base& operator=(scoped_base&& rhs)
            {
                #ifndef INJECTOR_SCOPED_NOSAVE_NORESTORE
                    if(this->saved = rhs.saved)
                    {
                        // Assert if we can hold the contents of the other buffer
                        assert(this->bufsize >= rhs.size);

                        // Copy basic fields
                        this->addr = rhs.addr;
                        this->size = rhs.size;
                        this->vp = rhs.vp;

                        // Copy the buffer
                        memcpy(buf, rhs.buf, rhs.size);

                        // Make rhs have non-saved state
                        rhs.saved = false;  
                    }
                #endif
                return *this;
            }
    };

    /*
     *  Basic interface for the RAII wrappers present on this header
     *  Notice anything that inherits from this should have exactly the same size as it.
     *  In other words, the child shouldn't have member variables
     */
    template<size_t bufsize_>
    struct scoped_basic : scoped_base
    {
        public:
            uint8_t buf_continue[bufsize_ - 1];  // Saved buffer continuation

        public:
            // Constructors, move constructors, assigment operators........ (MAKE SURE TO INIT bufsize)
            scoped_basic() : scoped_base(bufsize_)
            { assert(&this->buf_continue[0] == &this->buf[1]); }
            scoped_basic(const scoped_basic&) = delete;
            scoped_basic(scoped_basic&& rhs) : scoped_base(bufsize_, std::move(rhs)) {}
            scoped_basic& operator=(const scoped_basic& rhs) = delete;
            scoped_basic& operator=(scoped_basic&& rhs)
            { scoped_base::operator=(std::move(rhs)); return *this; }
    };
#pragma pack(pop)

    






    /*
     *  RAII wrapper for memory writes
     *  Can save only basic and POD types
     */
    template<size_t bufsize_ = 10>
    struct scoped_write : public scoped_basic<bufsize_>
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

            // Constructors, move constructors, assigment operators........
            scoped_write() = default;
            scoped_write(const scoped_write&) = delete;
            scoped_write(scoped_write&& rhs) : scoped_basic<bufsize_>(std::move(rhs)) {}
            scoped_write& operator=(const scoped_write& rhs) = delete;
            scoped_write& operator=(scoped_write&& rhs)
            { scoped_basic<bufsize_>::operator=(std::move(rhs)); return *this; }

    };

    /*
     *  RAII wrapper for filling
     */
    template<size_t bufsize_ = 5>
    struct scoped_fill : public scoped_basic<bufsize_>
    {
        public:
            // Fills memory at @addr with value @value and size @size and virtual protect @vp
            void fill(memory_pointer_tr addr, uint8_t value, size_t size, bool vp)
            {
                this->save(addr, size, vp);
                return MemoryFill(addr, value, size, vp);
            }

            // Constructors, move constructors, assigment operators........
            scoped_fill() = default;
            scoped_fill(const scoped_fill&) = delete;
            scoped_fill(scoped_fill&& rhs) : scoped_basic<bufsize_>(std::move(rhs)) {}
            scoped_fill& operator=(const scoped_fill& rhs) = delete;
            scoped_fill& operator=(scoped_fill&& rhs)
            { scoped_basic<bufsize_>::operator=(std::move(rhs)); return *this; }
    };
    
    /*
     *  RAII wrapper for nopping
     */
    template<size_t bufsize_>
    struct scoped_nop : public scoped_basic<bufsize_>
    {
        public:
            // Makes NOP at @addr with value @value and size @size and virtual protect @vp
            void make_nop(memory_pointer_tr addr, size_t size = 1, bool vp = true)
            {
                this->save(addr, size, vp);
                return MakeNOP(addr, size, vp);
            }

            // Constructors, move constructors, assigment operators........
            scoped_nop() = default;
            scoped_nop(const scoped_nop&) = delete;
            scoped_nop(scoped_nop&& rhs) : scoped_basic<bufsize_>(std::move(rhs)) {}
            scoped_nop& operator=(const scoped_nop& rhs) = delete;
            scoped_nop& operator=(scoped_nop&& rhs)
            { scoped_basic<bufsize_>::operator=(std::move(rhs)); return *this; }
    };
    
    /*
     *  RAII wrapper for MakeJMP 
     */
    struct scoped_jmp : public scoped_basic<5>
    {
        public:
            // Makes NOP at @addr with value @value and size @size and virtual protect @vp
            memory_pointer_raw make_jmp(memory_pointer_tr at, memory_pointer_raw dest, bool vp = true)
            {
                this->save(at, 5, vp);
                return MakeJMP(at, dest, vp);
            }

            // Constructors, move constructors, assigment operators........
            scoped_jmp() = default;
            scoped_jmp(const scoped_jmp&) = delete;
            scoped_jmp(scoped_jmp&& rhs) : scoped_basic<5>(std::move(rhs)) {}
            scoped_jmp& operator=(const scoped_jmp& rhs) = delete;
            scoped_jmp& operator=(scoped_jmp&& rhs)
            { scoped_basic<5>::operator=(std::move(rhs)); return *this; }
    };
    
    /*
     *  RAII wrapper for MakeCALL 
     */
    struct scoped_call : public scoped_basic<5>
    {
        public:
            // Makes NOP at @addr with value @value and size @size and virtual protect @vp
            memory_pointer_raw make_call(memory_pointer_tr at, memory_pointer_raw dest, bool vp = true)
            {
                this->save(at, 5, vp);
                return MakeCALL(at, dest, vp);
            }

            // Constructors, move constructors, assigment operators........
            scoped_call() = default;
            scoped_call(const scoped_call&) = delete;
            scoped_call(scoped_call&& rhs) : scoped_basic<5>(std::move(rhs)) {}
            scoped_call& operator=(const scoped_call& rhs) = delete;
            scoped_call& operator=(scoped_call&& rhs)
            { scoped_basic<5>::operator=(std::move(rhs)); return *this; }
    };
    


#if __cplusplus >= 201103L || _MSC_VER >= 1800  // MSVC 2013


    template<uintptr_t addr1, class FuncType, class Ret, class ...Args>
    struct function_hooker_base : scoped_call
    {
        public:
            static const uintptr_t addr = addr1;
            typedef FuncType       func_type;
            typedef std::function<Ret(func_type, Args&...)> functor_type;

        public:
            // Constructors, move constructors, assigment operators........
            function_hooker_base() = default;
            function_hooker_base(const function_hooker_base&) = delete;
            function_hooker_base(function_hooker_base&& rhs) : scoped_call(std::move(rhs)) {}
            function_hooker_base& operator=(const function_hooker_base& rhs) = delete;
            function_hooker_base& operator=(function_hooker_base&& rhs)
            { scoped_call::operator=(std::move(rhs)); return *this; }

        protected:
            struct hook_store
            {
                func_type       original;
                functor_type    functor;
            };

            // Stores the previous function pointer
            static hook_store& store()
            { static hook_store s; return s; }

    };

    template<uintptr_t addr1, class Prototype>
    struct function_hooker;

    template<uintptr_t addr1, class Ret, class ...Args>
    struct function_hooker<addr1, Ret(Args...)> : function_hooker_base<addr1, Ret(*)(Args...), Ret, Args...>
    {
        public:
            using base = function_hooker_base<addr1, Ret(*)(Args...), Ret, Args...>;

            // Makes the hook
            void make_call(typename base::functor_type functor)
            {
                base::store().functor = std::move(functor);
                base::store().original = scoped_call::make_call(addr1, raw_ptr(call)).get();
            }


            // The hook caller
            static Ret call(Args... a)
            {
                return base::store().functor(base::store().original, a...);
            }

            // Constructors, move constructors, assigment operators........
            function_hooker() = default;
            function_hooker(const function_hooker&) = delete;
            function_hooker(function_hooker&& rhs) : base(std::move(rhs)) {}
            function_hooker& operator=(const function_hooker& rhs) = delete;
            function_hooker& operator=(function_hooker&& rhs)
            { base::operator=(std::move(rhs)); return *this; }
    };



    template<uintptr_t addr1, class Prototype>
    struct function_hooker_stdcall;

    template<uintptr_t addr1, class Ret, class ...Args>
    struct function_hooker_stdcall<addr1, Ret(Args...)> : function_hooker_base<addr1, Ret(__stdcall*)(Args...), Ret, Args...>
    {
        public:
            using base = function_hooker_base<addr1, Ret(__stdcall*)(Args...), Ret, Args...>;

            // Makes the hook
            void make_call(typename base::functor_type functor)
            {
                base::store().functor = std::move(functor);
                base::store().original = scoped_call::make_call(addr1, raw_ptr(call)).get();
            }


            // The hook caller
            static Ret __stdcall call(Args... a)
            {
                return base::store().functor(base::store().original, a...);
            }

            // Constructors, move constructors, assigment operators........
            function_hooker_stdcall() = default;
            function_hooker_stdcall(const function_hooker_stdcall&) = delete;
            function_hooker_stdcall(function_hooker_stdcall&& rhs) : base(std::move(rhs)) {}
            function_hooker_stdcall& operator=(const function_hooker_stdcall& rhs) = delete;
            function_hooker_stdcall& operator=(function_hooker_stdcall&& rhs)
            { base::operator=(std::move(rhs)); return *this; }
    };


    template<uintptr_t addr1, class Prototype>
    struct function_hooker_fastcall;

    template<uintptr_t addr1, class Ret, class ...Args>
    struct function_hooker_fastcall<addr1, Ret(Args...)> : function_hooker_base<addr1, Ret(__fastcall*)(Args...), Ret, Args...>
    {
        public:
            using base = function_hooker_base<addr1, Ret(__fastcall*)(Args...), Ret, Args...>;

            // Makes the hook
            void make_call(typename base::functor_type functor)
            {
                base::store().functor = std::move(functor);
                base::store().original = scoped_call::make_call(addr1, raw_ptr(call)).get();
            }


            // The hook caller
            static Ret __fastcall call(Args... a)
            {
                return base::store().functor(base::store().original, a...);
            }

            // Constructors, move constructors, assigment operators........
            function_hooker_fastcall() = default;
            function_hooker_fastcall(const function_hooker_fastcall&) = delete;
            function_hooker_fastcall(function_hooker_fastcall&& rhs) : base(std::move(rhs)) {}
            function_hooker_fastcall& operator=(const function_hooker_fastcall& rhs) = delete;
            function_hooker_fastcall& operator=(function_hooker_fastcall&& rhs)
            { base::operator=(std::move(rhs)); return *this; }
    };



    template<uintptr_t addr1, class Prototype>
    struct function_hooker_thiscall;

    template<uintptr_t addr1, class Ret, class ...Args>
    struct function_hooker_thiscall<addr1, Ret(Args...)> : function_hooker_base<addr1, Ret(__thiscall*)(Args...), Ret, Args...>
    {
        public:
            using base = function_hooker_base<addr1, Ret(__thiscall*)(Args...), Ret, Args...>;

            // Makes the hook
            void make_call(typename base::functor_type functor)
            {
                base::store().functor = std::move(functor);
                base::store().original = (Ret(__thiscall*)(Args...)) (void*) scoped_call::make_call(addr1, raw_ptr(call)).get();
            }


            // The hook caller
            static Ret __thiscall call(Args... a)
            {
                return base::store().functor(base::store().original, a...);
            }

            // Constructors, move constructors, assigment operators........
            function_hooker_thiscall() = default;
            function_hooker_thiscall(const function_hooker_thiscall&) = delete;
            function_hooker_thiscall(function_hooker_thiscall&& rhs) : base(std::move(rhs)) {}
            function_hooker_thiscall& operator=(const function_hooker_thiscall& rhs) = delete;
            function_hooker_thiscall& operator=(function_hooker_thiscall&& rhs)
            { base::operator=(std::move(rhs)); return *this; }
    };



    /* Helpers */
    template<class T, class F> inline
    T& make_static_hook(F functor)
    {
        static T a;
        a.make_call(std::move(functor));
        return a;
    }

    template<class T, class F> inline
    T make_function_hook(F functor)
    {
        T a;
        a.make_call(std::move(functor));
        return a;
    }

#endif   

} 
