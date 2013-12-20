/*
 *  LINK/2012's Injectors - Light Version
 *	Header with helpful stuff for ASI memory hacking
 *
 *	by LINK/2012 <dma_2012@hotmail.com>
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
#include <windows.h>
#include <type_traits>
#include <cstdint>
#include <cassert>

#ifdef _DEBUG
#include <cstdio>
#include <cstdarg>
#endif


//
//	ASM EPILOG and ASM PROLOG macros for MSVC/MASM
//
#define ASM_PROLOG() _asm					\
	{										\
		_asm push	ebp						\
		_asm mov	ebp, esp				\
		_asm sub	esp, __LOCAL_SIZE		\
	}

#define ASM_EPILOG() _asm					\
	{										\
		_asm mov	esp, ebp				\
		_asm pop	ebp						\
	}


//
//
//

union memory_pointer_a	// used to hack the compiler, don't use for general purposes
{
	void*	 p;
	uintptr_t a;

	memory_pointer_a()				{ p = nullptr; }
	memory_pointer_a(void* x) : p(x)		{}
	memory_pointer_a(uint32_t x) : a(x)		{}

	template<class T>
	operator T*() { return reinterpret_cast<T*>(p); }
};

union memory_pointer    // use for general purposes
{
	void*	 p;
	uintptr_t a;

	memory_pointer()						{ p = nullptr; }
	memory_pointer(void* x) : p(x)			{}
	memory_pointer(uint32_t x) : a(x)		{}

    /* Use to get pointer and automatically cast to another type */
    memory_pointer_a get()  { return memory_pointer_a(p); }
    
	operator void*()		{ return p; }
	operator uintptr_t()	{ return a; }

	memory_pointer& operator=(void* x)		{ return p = x, *this; }
	memory_pointer& operator=(uintptr_t x)	{ return a = x, *this; }
};


//
//
//
inline bool ProtectMemory(memory_pointer Address, size_t size, DWORD Protect)
{
	return VirtualProtect(Address, size, Protect, &Protect) != 0;
}

inline bool UnprotectMemory(memory_pointer Address, size_t size, DWORD& Out_OldProtect)
{
	return VirtualProtect(Address, size, PAGE_EXECUTE_READWRITE, &Out_OldProtect) != 0;
}

inline void WriteMemoryRaw(memory_pointer Address, void* value, size_t size, bool vp)
{
	DWORD oldProtect;
	if(vp) UnprotectMemory(Address, size, oldProtect);
 
	memcpy(Address, value, size);
 
	if(vp) ProtectMemory(Address, size, oldProtect);
}

inline void ReadMemoryRaw(memory_pointer Address, void* ret, size_t size, bool vp)
{
	DWORD oldProtect;
	if(vp) UnprotectMemory(Address, size, oldProtect);
 
	if(size == 1) *(uint8_t*)ret = *(uint8_t*)Address.p;
	else if(size == 2) *(uint16_t*)ret = *(uint16_t*)Address.p;
	else if(size == 4) *(uint32_t*)ret = *(uint32_t*)Address.p;
	else memcpy(ret, Address, size);
	
	if(vp) ProtectMemory(Address, size, oldProtect);
}



template<class T> inline
	void WriteMemoryTo(memory_pointer Address, const T& value, bool vp = false)
	{
		DWORD oldProtect;
		if(vp) UnprotectMemory(Address, sizeof(T), oldProtect);
		*(T*)Address.p = value;
		if(vp) VirtualProtect(Address, sizeof(T), oldProtect, &oldProtect);
	}

template<class T> inline
	T& ReadMemoryTo(memory_pointer Address, T& value, bool vp = false)
	{
		DWORD oldProtect;
		if(vp) UnprotectMemory(Address, sizeof(T), oldProtect);
		value = *(T*)Address.p;
		if(vp) VirtualProtect(Address, sizeof(T), oldProtect, &oldProtect);
		return value;
	}

template<class T> inline
	void WriteMemory(memory_pointer Address, T value, bool vp = false)
	{
		WriteMemoryTo(Address, value, vp);
	}

template<class T> inline
	T ReadMemory(memory_pointer Address, bool vp = false)
	{
		T value;
		return ReadMemoryTo(Address, value, vp), value;
	}






//
//
//
inline memory_pointer GetAbsoluteOffset(int rel_value, memory_pointer end_of_instruction)
{
	return end_of_instruction + rel_value;
}

inline long GetRelativeOffset(int abs_value, memory_pointer end_of_instruction)
{
	return abs_value - end_of_instruction;
}

inline memory_pointer ReadRelativeOffset(memory_pointer at, size_t sizeof_addr = 4)
{
	switch(sizeof_addr)
	{
		case 1: return (GetAbsoluteOffset(ReadMemory<int8_t>(at, true), at+sizeof_addr));
		case 2: return (GetAbsoluteOffset(ReadMemory<int16_t>(at, true), at+sizeof_addr));
		case 4: return (GetAbsoluteOffset(ReadMemory<int32_t>(at, true), at+sizeof_addr));
	}
	return nullptr;
}

inline void MakeRelativeOffset(memory_pointer at, memory_pointer dest, size_t sizeof_addr = 4)
{
	switch(sizeof_addr)
	{
		case 1: WriteMemory<int8_t>(at, static_cast<int8_t>(GetRelativeOffset(dest, at+sizeof_addr)), true);
		case 2: WriteMemory<int16_t>(at, static_cast<int16_t>(GetRelativeOffset(dest, at+sizeof_addr)), true);
		case 4: WriteMemory<int32_t>(at, static_cast<int32_t>(GetRelativeOffset(dest, at+sizeof_addr)), true);
	}
}

inline memory_pointer GetAbsoluteOffsetInOpcode(memory_pointer at)
{
	switch(ReadMemory<uint8_t>(at, true))
	{
        // We need to handle other instructions (and prefixes)...
		case 0xE8:	// call rel
		case 0xE9:	// jmp rel
			return ReadRelativeOffset(at+1, 4);
	}
	return nullptr;
}



inline memory_pointer MakeJMP(memory_pointer at, memory_pointer dest)
{
	memory_pointer p = GetAbsoluteOffsetInOpcode(at);
	WriteMemory<uint8_t>(at, 0xE9, true);
	MakeRelativeOffset(at+1, dest);
	return p;
}

inline memory_pointer MakeCALL(memory_pointer at, memory_pointer dest)
{
	memory_pointer p = GetAbsoluteOffsetInOpcode(at);
	WriteMemory<uint8_t>(at, 0xE8, true);
	MakeRelativeOffset(at+1, dest);
	return p;
}

inline void MakeJA(memory_pointer at, memory_pointer dest)
{
	WriteMemory<uint16_t>(at, 0x87F0, true);
	MakeRelativeOffset(at+2, dest);
}

inline void MakeNOP(memory_pointer at, size_t size)
{
	DWORD oldProtect;
	UnprotectMemory(at, size, oldProtect);
	memset(at.p, 0x90, size);
	VirtualProtect(at, size, oldProtect, &oldProtect);
}

