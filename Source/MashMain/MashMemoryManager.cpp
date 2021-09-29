//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "MashMemoryManager.h"
#include "CMashMemoryTracker.h"
#include "MashMemory.h"
#include "MashMemoryAllocator.h"
#include "MashLog.h"
#include <new>

namespace mash
{
	MashMemoryManager *MashMemoryManager::m_memoryManager = 0;
	MashMemoryAllocator *MashMemoryManager::m_allocator = 0;

	MashMemoryManager::MashMemoryManager()
	{

	}

	MashMemoryManager::~MashMemoryManager()
	{
#ifdef MASH_MEMORY_TRACKING_ENABLED
		CMashMemoryTracker::DestroyInstance();
#endif
		if (m_allocator)
		{
			m_allocator->Destroy();
			m_allocator = 0;
		}
	}

	void MashMemoryManager::DestroyInstance()
	{
		if (m_memoryManager)
		{
			MASH_DELETE_T(MashMemoryManager, m_memoryManager);
			m_memoryManager = 0;
		}
	}

	MashMemoryManager* MashMemoryManager::Instance()
	{
		if (!m_allocator)
		{
			m_allocator = CreateMemoryAllocator();
			if (!m_allocator)
            {
                MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
                                 "Failed to create memory allocator from user defined memory alocator function CreateMemoryAllocator().", 
                                 "MashMemoryManager::Instance");
				return 0;
            }
		}

		if (!m_memoryManager)
		{
			void *allocMem = m_allocator->Allocate(sizeof(MashMemoryManager), _g_globalMemoryAlignment, aMEMORY_CATEGORY_SYSTEM);
			m_memoryManager = new(allocMem) MashMemoryManager();//placement new
		}

		return m_memoryManager;
	}

	void* MashMemoryManager::Allocate(size_t size, const int8 *sFile, int32 iLine, const int8 *sFunc, size_t alignment, eMEMORY_CATEGORY memCategory)
	{
		void *ptr = m_allocator->Allocate(size, alignment, memCategory);

#ifdef MASH_MEMORY_TRACKING_ENABLED
		if (ptr)
			LogAllocation(ptr, size, sFile, iLine, sFunc);
#endif

		return ptr;
	}

	void* MashMemoryManager::Allocate(size_t size, size_t alignment, eMEMORY_CATEGORY memCategory)
	{
		void *ptr = m_allocator->Allocate(size, alignment, memCategory);

#ifdef MASH_MEMORY_TRACKING_ENABLED
		if (ptr)
			LogAllocation(ptr, size, "unknown", 0, "unknown");
#endif

		return ptr;
	}

	void MashMemoryManager::Deallocate(void *ptr)
	{
#ifdef MASH_MEMORY_TRACKING_ENABLED
		LogDeallocation(ptr);
#endif
		m_allocator->Deallocate(ptr);
	}

	void MashMemoryManager::LogAllocation(void *p, int32 iSize, const int8 *sFile, int32 iLine, const int8 *sFunc)
	{
#ifdef MASH_MEMORY_TRACKING_ENABLED
		CMashMemoryTracker::Instance()->LogAllocation(p, iSize, sFile, iLine, sFunc);
#endif
	}

	void MashMemoryManager::LogDeallocation(void *p)
	{
#ifdef MASH_MEMORY_TRACKING_ENABLED
		CMashMemoryTracker::Instance()->LogDeallocation(p);
#endif
	}
}