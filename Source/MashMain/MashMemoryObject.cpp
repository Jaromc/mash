//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "MashMemoryObject.h"

namespace mash
{
	MashMemoryObject::MashMemoryObject()
	{

	}

	MashMemoryObject::~MashMemoryObject()
	{

	}

	void* MashMemoryObject::operator new (size_t iSize, const int8 *sFile, int32 iLine, const int8 *sFunc, size_t alignment, eMEMORY_CATEGORY memCategory)
	{
		return _MashAllocateMemory(iSize, sFile, iLine, sFunc, alignment, memCategory);
	}

	void* MashMemoryObject::operator new (size_t iSize, size_t alignment, eMEMORY_CATEGORY memCategory)
	{
		return _MashAllocateMemory(iSize, alignment, memCategory);
	}

	void* MashMemoryObject::operator new[] (size_t iSize, const int8 *sFile, int32 iLine, const int8 *sFunc, size_t alignment, eMEMORY_CATEGORY memCategory)
	{
		return _MashAllocateMemory(iSize, sFile, iLine, sFunc, alignment, memCategory);
	}

	void* MashMemoryObject::operator new[] (size_t iSize, size_t alignment, eMEMORY_CATEGORY memCategory)
	{
		return _MashAllocateMemory(iSize, alignment, memCategory);
	}

	void MashMemoryObject::operator delete (void *ptr)
	{
		return _MashFreeMemory(ptr);
	}

	void MashMemoryObject::operator delete[] (void *ptr)
	{
		return _MashFreeMemory(ptr);
	}
}