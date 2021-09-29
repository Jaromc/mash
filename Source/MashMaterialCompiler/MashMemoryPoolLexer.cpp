//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "MashMemoryPoolLexer.h"
#include "MashMemory.h"

namespace mash
{
	MashMemoryPoolLexer::~MashMemoryPoolLexer()
	{
		uint32 poolSize = m_memoryPools.Size();
		for(uint32 i = 0; i < poolSize; ++i)
		{
			if (m_memoryPools[i].memory)
			{
				MASH_FREE(m_memoryPools[i].memory);
			}
		}

		m_memoryPools.Clear();
	}

	void* MashMemoryPoolLexer::GetMemory(uint32 sizeInBytes)
	{
		sPool *poolToAllocateMemFrom = 0;
		bool createNewPool = false;

		/*
			We could search through all created pools here and find one that has enough
			space on the end. However, strings are almost always alive till the end, and fairly small so
			a search really shouldn't be needed. We instead just create a new pool when needed.
		*/

		if (m_memoryPools.Empty())
		{
			createNewPool = true;
		}
		else
		{
			poolToAllocateMemFrom = &m_memoryPools.Back();
			int32 sizeAvalaible = (poolToAllocateMemFrom->poolSize - poolToAllocateMemFrom->nextAvaliableByte);
			if (sizeAvalaible < sizeInBytes)
				createNewPool = true;
		}

		/*
			Memory pools are never reallocated! Otherwise anything that was referencing that pool would
			be invalidated. Instead we create a new pool when needed.
		*/
		if (createNewPool)
		{
			/*
				300 bytes per pool should do
			*/
			const uint32 poolSize = 300;
			sPool newPool;
			newPool.poolSize = poolSize;
			//make sure enough memory is allocated
			if (newPool.poolSize < sizeInBytes)
				newPool.poolSize = sizeInBytes * 2;

			newPool.memory = (uint8*)MASH_ALLOC_COMMON(newPool.poolSize);
			
			newPool.nextAvaliableByte = 0;

			m_memoryPools.PushBack(newPool);
		}

		poolToAllocateMemFrom = &m_memoryPools.Back();

		void *ptrToReturn = &poolToAllocateMemFrom->memory[poolToAllocateMemFrom->nextAvaliableByte];
		poolToAllocateMemFrom->nextAvaliableByte += sizeInBytes;

		//sanity check
		MASH_ASSERT(poolToAllocateMemFrom->nextAvaliableByte <= poolToAllocateMemFrom->poolSize);

		return ptrToReturn;
	}
}