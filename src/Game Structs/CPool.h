#pragma once
#include <cstdint>
#include "Injector.h"

#pragma pack(push, 1)
template<class BaseT, class DerivedT = BaseT>
struct CPool
{
	static_assert(sizeof(BaseT) <= sizeof(DerivedT), "CPool: Incorrect BaseT!");

	DerivedT* objects;
	uint8_t* flags;
	size_t size;
	int top;
	bool initialized;
	uint8_t field_11;
	uint16_t objectSize;

	BaseT* GetAt(size_t handle)
	{
		size_t i = handle >> 8;
		if(i >= size || this->flags[i] != (uint8_t)(handle))
			return nullptr;
		return reinterpret_cast<BaseT*>(&objects[i]);
	}

	size_t GetIndex(BaseT* from)
	{
		size_t index = (reinterpret_cast<DerivedT*>(from) - objects);
		return index < size? (index << 8) | flags[index] : -1;
	}

	size_t GetHandle(BaseT* from) { return GetIndex(from); }

	BaseT* operator[](size_t i)				{ return reinterpret_cast<BaseT*>(&this->objects[i]); }

	bool Exists(const BaseT* obj)
	{
		if(obj == nullptr) return false;
		size_t index = ((size_t)obj - (size_t)objects) / sizeof(DerivedT);
		return (this->flags[index] & 0x80)==0;
	}

	template<class FuncT>
		void for_each(FuncT func)
		{
			for(int i = this->size - 1; i >= 0; --i)
			{
				if( !(this->flags[i] & 0x80) )
					func(this->operator[](i));
			}
		}
};


static_assert(sizeof(CPool<void*, void*>) == 0x14, "Incorrect struct size: CPool");

template<class BaseT, class ParentT, class FuncT>
inline void for_each(CPool<BaseT, ParentT>& pool, FuncT func)
{
	pool.for_each(func);
}


template<size_t size>
struct CFake	// Used to create fake structs to put in ParentT
{
	uint8_t a[size];
};
static_assert(sizeof(CFake<0x7>) == 0x7, "Incorrect struct size: CFake");

typedef CPool< struct CVehicle,	CFake<0xA18> >		CVehiclePool;	// CFake is CHeli
typedef CPool< struct CPed,		CFake<0x7C4> >		CPedPool;		// CFake is CPlayerPed
typedef CPool< struct CObject,	CFake<0x19C> >		CObjectPool;	// CFake is ?

#pragma pack(pop)
