//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#include "MashDefaultMemoryAllocator.h"

namespace mash
{
    
/*
    It's usually best to leave memory alignment up to the compiler.
    But you could customize it if you wanted using something similar
    to the below #defined code.
 */
    
//#define _USE_CUSTOM_MEMORY_ALIGNMENT_
    
	_MASH_EXPORT MashMemoryAllocator* CreateMemoryAllocator()
	{
		return new MashDefaultMemoryAllocator();
	}

	void* MashDefaultMemoryAllocator::Allocate(size_t iSize, size_t alignment, eMEMORY_CATEGORY memCategory)
	{
		void *tempMem = malloc(iSize);
		if (!tempMem)
		{
			assert(0);
		}

#ifdef _USE_CUSTOM_MEMORY_ALIGNMENT_
		void *ptr = (void*)(((unsigned long)tempMem + sizeof(void*)+(alignment-1))&~(alignment-1));
		*((void**)ptr-1) = tempMem;

		return ptr;
#else
        return tempMem;
#endif
	}

	void MashDefaultMemoryAllocator::Deallocate(void *ptr)
	{
		if (ptr)
		{
#ifdef _USE_CUSTOM_MEMORY_ALIGNMENT_
			free(*((void**)ptr-1));
#else
            free(ptr);
#endif
		}
	}

	void MashDefaultMemoryAllocator::Destroy()
	{
		delete this;
	}
}
