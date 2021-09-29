//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_MEMORY_POOL_SIMPLE_H_
#define _MASH_MEMORY_POOL_SIMPLE_H_

#include "MashMemory.h"

namespace mash
{
    /*!
        A simple memory allocator used in containers.
    */
	class MashMemoryPoolSimple
	{
	public:
		MashMemoryPoolSimple(){}
		MashMemoryPoolSimple(const MashMemoryPoolSimple &c){}
		~MashMemoryPoolSimple(){}

        //! Gets memory from this pool.
		void* GetMemory(uint32 sizeInBytes, size_t alignment = mash::_g_globalMemoryAlignment)
		{
			return MASH_ALLOC_ALIGN(sizeInBytes, aMEMORY_CATEGORY_COMMON, alignment);
		}

        //! Returns memory back to the pool.
		void FreeMemory(void *ptr)
		{
			MASH_FREE(ptr);
		}

        //! Does nothing.
		void Clear(){}
		void Destroy(){}
	};
}

#endif