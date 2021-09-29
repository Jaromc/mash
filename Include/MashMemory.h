//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_MEMORY_H_
#define _MASH_MEMORY_H_

#include "MashMemoryTypes.h"
#include "MashDataTypes.h"
#include <new>
#include <memory.h>

/*!
    Holds all the dynamic memory functions.
    Debug mode uses logging methods, Release build removes logging.
 
    Memory categories can be used to allocate memory from particular
    memory pools. See eMEMORY_CATEGORY For more details.
*/

namespace mash
{
	template<class T>
	class MashMemoryAllocatorHelper
	{
	public:
		static void Construct(T *element)
		{
			new ((void*)element) T();
		}

		static void Construct(T *element, const T &c)
		{
			new ((void*)element) T(c);
		}

		static T* ConstructArray(T *ptr, size_t count)
		{
			for(size_t i = 0; i < count; ++i)
			{
				new ((void*)(ptr+i)) T();
			}
        
			return ptr;
		}

		static void Destruct(T *ptr)
		{
			ptr->~T();
		}

		static void DestructArray(T *ptr, size_t count)
		{
			for(size_t i = 0; i < count; ++i)
			{
				(ptr)[i].~T();
			}
		}
	};
    
#ifdef MASH_DEBUG

	_MASH_EXPORT void* _MashAllocateMemory(size_t size, const int8 *file, int32 line, const int8 *func, size_t alignment = mash::_g_globalMemoryAlignment, mash::eMEMORY_CATEGORY memCategory = mash::aMEMORY_CATEGORY_COMMON);
    _MASH_EXPORT void* _MashAllocateMemory(size_t size, size_t alignment = mash::_g_globalMemoryAlignment, mash::eMEMORY_CATEGORY memCategory = mash::aMEMORY_CATEGORY_COMMON);
	_MASH_EXPORT void _MashFreeMemory(void *ptr);

	/*
		C dynamic allocation calls. Constructors and Destructors for objects are not called automatically.
		MashMemoryAllocatorHelper will need to be used to do this manually.
	*/
#define MASH_ALLOC_ALIGN(bytes, memCategory, alignment) mash::_MashAllocateMemory(bytes, __FILE__ , __LINE__ , __FUNCTION__, alignment, memCategory)
#define MASH_ALLOC(bytes, memCategory) mash::_MashAllocateMemory(bytes, __FILE__ , __LINE__ , __FUNCTION__, mash::_g_globalMemoryAlignment, memCategory)
#define MASH_ALLOC_COMMON(bytes) MASH_ALLOC(bytes, mash::aMEMORY_CATEGORY_COMMON)

#define MASH_ALLOC_T(T, count, memCategory) (T*)mash::_MashAllocateMemory(sizeof(T) * (count), __FILE__ , __LINE__ , __FUNCTION__, mash::_g_globalMemoryAlignment, memCategory)
#define MASH_ALLOC_T_COMMON(T, count) MASH_ALLOC_T(T, count, mash::aMEMORY_CATEGORY_COMMON)

#define MASH_FREE(ptr) mash::_MashFreeMemory(ptr)

	/*
		C++ dynamic allocation calls. Constructors and Destructors are called automatically.
	*/
#define MASH_NEW_T(T, memCategory) new (mash::_MashAllocateMemory(sizeof(T), __FILE__, __LINE__ , __FUNCTION__, mash::_g_globalMemoryAlignment, memCategory)) T
#define MASH_NEW_T_COMMON(T) MASH_NEW_T(T, mash::aMEMORY_CATEGORY_COMMON)

#define MASH_NEW_ARRAY_T(T, count, memCategory) (MashMemoryAllocatorHelper<T>::ConstructArray(static_cast<T*>(mash::_MashAllocateMemory(sizeof(T) * (count), __FILE__ , __LINE__ , __FUNCTION__, mash::_g_globalMemoryAlignment, memCategory)), (count)))
#define MASH_NEW_ARRAY_T_COMMON(T, count) MASH_NEW_ARRAY_T(T, count, mash::aMEMORY_CATEGORY_COMMON)

#define MASH_DELETE_T(T, ptr) \
	if(ptr){\
	MashMemoryAllocatorHelper<T>::Destruct(ptr);\
	mash::_MashFreeMemory(ptr);\
	}

#define MASH_DELETE_ARRAY_T(T, ptr, count) \
	if(ptr)\
	{\
		MashMemoryAllocatorHelper<T>::DestructArray(ptr, count);\
		mash::_MashFreeMemory(ptr);\
	}

	#define MASH_NEW(memCategory) new(__FILE__, __LINE__, __FUNCTION__, mash::_g_globalMemoryAlignment, memCategory)
	#define MASH_NEW_COMMON MASH_NEW(mash::aMEMORY_CATEGORY_COMMON)

	#define MASH_DELETE delete 

#else
    
    _MASH_EXPORT void* _MashAllocateMemory(size_t size, const int8 *file, int32 line, const int8 *func, size_t alignment = mash::_g_globalMemoryAlignment, mash::eMEMORY_CATEGORY memCategory = mash::aMEMORY_CATEGORY_COMMON);
	_MASH_EXPORT void* _MashAllocateMemory(size_t size, size_t alignment = mash::_g_globalMemoryAlignment, mash::eMEMORY_CATEGORY memCategory = mash::aMEMORY_CATEGORY_COMMON);
	_MASH_EXPORT void _MashFreeMemory(void *ptr);

	/*
		C dynamic allocation calls. Constructors and Destructors for objects are not called automatically.
		MashMemoryAllocatorHelper will need to be used to do this manually.
	*/
#define MASH_ALLOC_ALIGN(bytes, memCategory, alignment) mash::_MashAllocateMemory(bytes, alignment, memCategory)
#define MASH_ALLOC(bytes, memCategory) mash::_MashAllocateMemory(bytes, mash::_g_globalMemoryAlignment, memCategory)
#define MASH_ALLOC_COMMON(bytes) MASH_ALLOC(bytes, mash::aMEMORY_CATEGORY_COMMON)

#define MASH_ALLOC_T(T, count, memCategory) (T*)mash::_MashAllocateMemory(sizeof(T) * (count), mash::_g_globalMemoryAlignment, memCategory)
#define MASH_ALLOC_T_COMMON(T, count) MASH_ALLOC_T(T, count, mash::aMEMORY_CATEGORY_COMMON)

#define MASH_FREE(ptr) mash::_MashFreeMemory(ptr)

	/*
		C++ dynamic allocation calls. Constructors and Destructors are called automatically.
	*/
#define MASH_NEW_T(T, memCategory) new (mash::_MashAllocateMemory(sizeof(T), mash::_g_globalMemoryAlignment, memCategory)) T
#define MASH_NEW_T_COMMON(T) MASH_NEW_T(T, mash::aMEMORY_CATEGORY_COMMON)

#define MASH_NEW_ARRAY_T(T, count, memCategory) (MashMemoryAllocatorHelper<T>::ConstructArray(static_cast<T*>(mash::_MashAllocateMemory(sizeof(T) * (count), mash::_g_globalMemoryAlignment, memCategory)), (count)))
#define MASH_NEW_ARRAY_T_COMMON(T, count) MASH_NEW_ARRAY_T(T, count, mash::aMEMORY_CATEGORY_COMMON)

#define MASH_DELETE_T(T, ptr) \
	if(ptr){\
	MashMemoryAllocatorHelper<T>::Destruct(ptr);\
	mash::_MashFreeMemory(ptr);\
	}

#define MASH_DELETE_ARRAY_T(T, ptr, count) \
	if(ptr)\
	{\
		MashMemoryAllocatorHelper<T>::DestructArray(ptr, count);\
		mash::_MashFreeMemory(ptr);\
	}

	#define MASH_NEW(memCategory) new(mash::_g_globalMemoryAlignment, memCategory)
	#define MASH_NEW_COMMON MASH_NEW(mash::aMEMORY_CATEGORY_COMMON)

	#define MASH_DELETE delete 
#endif
}

#endif
