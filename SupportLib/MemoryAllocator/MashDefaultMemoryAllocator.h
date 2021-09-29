//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_DEFAULT_MEMORY_ALLOCATOR_H_
#define _MASH_DEFAULT_MEMORY_ALLOCATOR_H_

#include "MashMemoryAllocator.h"
#include "MashMemoryManager.h"
#include <assert.h>
#include <cstdlib>

namespace mash
{
	/*
		The default allocator doesn't do anything to special.
		- Uses malloc and free
		- No memory pools, just straight (de)allocation
		- It does align memory
		- Logs (de)allocations. This can be deactivated with a #define

		This allocator is created when needed by an internal function by calling
		"_MASH_EXPORT AeroMemoryAllocator* CreateMemoryAllocator()" which is implimented
		in the .cpp file for this object.

		You can impliment your own fancy allocator if needed. Just remove this include
		from your application and impliment the "_MASH_EXPORT MashMemoryAllocator* CreateMemoryAllocator()"
		function in your code.

		MashDefaultMemoryAllocator::Destroy will be called by the engine when it's time to destroy
		the allocator.
	*/
	class MashDefaultMemoryAllocator : public MashMemoryAllocator
	{
	public:
		MashDefaultMemoryAllocator(){}
		~MashDefaultMemoryAllocator(){}

		void* Allocate(size_t iSize, size_t alignment, eMEMORY_CATEGORY memCategory);
		void Deallocate(void *ptr);

		void Destroy();
	};
}

#endif