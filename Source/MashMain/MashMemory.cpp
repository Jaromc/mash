//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "MashMemory.h"
#include "MashMemoryManager.h"

namespace mash
{
	void* _MashAllocateMemory(size_t iSize, const int8 *sFile, int32 iLine, const int8 *sFunc, size_t alignment, eMEMORY_CATEGORY memCategory)
	{
		return MashMemoryManager::Instance()->Allocate(iSize, sFile, iLine, sFunc, alignment, memCategory);
	}

	void _MashFreeMemory(void *ptr)
	{
		return MashMemoryManager::Instance()->Deallocate(ptr);
	}

	void* _MashAllocateMemory(size_t iSize, size_t alignment, eMEMORY_CATEGORY memCategory)
	{
		return MashMemoryManager::Instance()->Allocate(iSize, alignment, memCategory);
	}
}