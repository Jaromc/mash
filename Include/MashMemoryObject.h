//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_MEMORY_OBJECT_H_
#define _MASH_MEMORY_OBJECT_H_

#include "MashCompileSettings.h"
#include "MashMemory.h"
#ifdef MASH_APPLE
#include <cstddef>
#endif

namespace mash
{
    /*!
        Implimented by the reference counter. Most objects within the engine derive from this.
    */
	class _MASH_EXPORT MashMemoryObject
	{
	public:
		MashMemoryObject();
		virtual ~MashMemoryObject();

		void* operator new (size_t size, const int8 *file, int32 line, const int8 *func, size_t alignment, eMEMORY_CATEGORY memCategory);

		void* operator new (size_t size, size_t alignment, eMEMORY_CATEGORY memCategory);

		void* operator new[] (size_t size, const int8 *file, int32 line, const int8 *func, size_t alignment, eMEMORY_CATEGORY memCategory);

		void* operator new[] (size_t size, size_t alignment, eMEMORY_CATEGORY memCategory);

		void operator delete (void *ptr);

		void operator delete[] (void *ptr);
	};
}

#endif