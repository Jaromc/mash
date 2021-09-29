//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_MEMORY_POOL_H_
#define _MASH_MEMORY_POOL_H_

#include "MashMemory.h"

namespace mash
{
	enum eMEMORY_POOL_ERRORS
	{
		aFAILED_TO_ALLOC_NULL_PTR
	};

	struct sMashMemoryPoolError
	{
		void operator()(eMEMORY_POOL_ERRORS msg)
		{
			MASH_ASSERT(false);
		}
	};

	struct sMashAllocAllocatorFunctor
	{
		void* operator()(size_t size, size_t alignment, eMEMORY_CATEGORY memCategory)
		{
			return MASH_ALLOC_ALIGN(size, memCategory, alignment);
		}
	};

	struct sMashFreeDeallocatorFunctor
	{
		void operator()(void *ptr)
		{
			return MASH_FREE(ptr);
		}
	};

    /*!
        Allocates large chunks of memory to be used in containers or directly. This can help
        reduce memory fragmentation.
     
        When memory is returned to the pool, it is avaliable to be re-allocated if requested later.
        Re-allocations will always be used before new memory is allocated. Allocation requests always 
        take the smallest re-allocation possible.
     
        In debug mode, bounds checking is made on each allocation request and an assert will
        be triggered if a check fails.
     
        A default implimentation of this pool is :
        typedef MashMemoryPool<sMashMemoryPoolError, sMashAllocAllocatorFunctor, sMashFreeDeallocatorFunctor> MemPoolType;
    */
	template<class TErrorFunctor, class TAllocatorFunctor, class TDeallocatorFunctor, bool TIsFixedSize = false>
	class MashMemoryPool
	{
	private:
		class InternalPool
		{
		private:
			struct sPool
			{
				uint32 poolSize;
				uint32 nextAvaliableByte;

				sPool *previous;
				sPool *next;

				sPool():poolSize(0), nextAvaliableByte(0), previous(0), next(0){}
			};

			/*
				This will sort pools from largest to smallest.
			*/
			struct sPoolSort
			{
				bool operator()(const sPool *a, const sPool *b)const
				{
					return (a->poolSize - a->nextAvaliableByte) > (b->poolSize - b->nextAvaliableByte);
				}
			};

			struct sAllocation
			{
				uint32 allocationSize;
				sAllocation *previous;
				sAllocation *next;

				sAllocation():allocationSize(0), previous(0), next(0){}
			};

			/*
				This will sort deallocations from smallest to largest
			*/
			struct sDeallocationSort
			{
				bool operator()(const sAllocation *a, const sAllocation *b)const
				{
					//pointer size is stored at the start
					return a->allocationSize < b->allocationSize;
				}
			};

			struct sDebugData
			{
				uint32 totalAllocations;
				uint32 totalDeallocations;
				uint32 currentDeallocations;
				uint32 maxCurrentDeallocations;

				sDebugData():totalAllocations(0), totalDeallocations(0), maxCurrentDeallocations(0), currentDeallocations(0){}
			};

			typedef algorithms::sGetObjectData<sPool*, sPool*> GetPoolDataPred;
			typedef algorithms::sGetObjectData<sAllocation*, sAllocation*> GetAllocDataPred;

		private:
			sPool *memoryPools;
			sAllocation *deallocations;
			eMEMORY_CATEGORY category;
			uint32 initialPoolSizeInBytes;
			uint32 poolSizeMultiplyer;
			uint32 ref;

			sPool *activeMemoryPool;

#ifdef MASH_DEBUG
			sDebugData debugData;
#endif
		public:
			InternalPool(uint32 _initialPoolSizeInBytes, uint32 initialPoolCount, uint32 _poolSizeMultiplyer, eMEMORY_CATEGORY _category):activeMemoryPool(0),
				memoryPools(0), deallocations(0), ref(1), initialPoolSizeInBytes(_initialPoolSizeInBytes), poolSizeMultiplyer(_poolSizeMultiplyer),
				category(_category)
			{

				if (initialPoolSizeInBytes == 0)
					initialPoolSizeInBytes = 1;

				if (poolSizeMultiplyer == 0)
					poolSizeMultiplyer = 1;

				if (initialPoolCount > 0)
				{
					for(uint32 i = 0; i < initialPoolCount; ++i)
					{
						uint32 newPoolSize = initialPoolSizeInBytes;

						/*
							subtract initial allocation from the first pool
						*/
						if (i == 0 && (newPoolSize >= sizeof(InternalPool)))
							newPoolSize -= sizeof(InternalPool);

						//make sure it is at least the size of the pool struct
						if (newPoolSize < sizeof(sPool))
							newPoolSize = sizeof(sPool);

						activeMemoryPool = (sPool*)TAllocatorFunctor()(newPoolSize, mash::_g_globalMemoryAlignment, category);
						activeMemoryPool->poolSize = newPoolSize;
						activeMemoryPool->nextAvaliableByte = 0;

						if (!memoryPools)
							memoryPools = activeMemoryPool;
						else
						{
							activeMemoryPool->next = memoryPools;
							memoryPools->previous = activeMemoryPool;

							memoryPools = activeMemoryPool;
						}
					}

					activeMemoryPool = memoryPools;
				}
			}

			~InternalPool()
			{
				Destroy();
			}

			void Drop()
			{
				--ref;
				if (ref == 0)
				{
					MashMemoryAllocatorHelper<InternalPool>::Destruct(this);
					TDeallocatorFunctor()(this);
				}
			}

			void Grab()
			{
				++ref;
			}

			void* GetMemory(uint32 sizeInBytes, size_t alignment = mash::_g_globalMemoryAlignment)
			{
				
	#ifdef MASH_DEBUG
				//plus two for debug markers
				sizeInBytes += sizeof(sAllocation) + 2;
				++debugData.totalAllocations;
	#else
				sizeInBytes += sizeof(sAllocation);
	#endif
				/*
					Finds the smallest memory slot that will fit the requested size.
					This list has been previously sorted from smallest to largest.
				*/
				sAllocation *deallocIter = deallocations;
				while(deallocIter)
				{
					/*
						TODO : Check memory alignments match
					*/
					if (deallocIter->allocationSize >= sizeInBytes)
					{
#ifdef MASH_DEBUG
			--debugData.currentDeallocations;
#endif
						sAllocation *found = deallocIter;

						//size found so remove and relink nodes
						if (deallocIter->previous)
							deallocIter->previous->next = deallocIter->next;
						if (deallocIter->next)
							deallocIter->next->previous = deallocIter->previous;

						if (deallocIter == deallocations)
							deallocations = deallocations->next;

						int8* memPtr = (int8*)found;
#ifdef MASH_DEBUG
						return &memPtr[sizeof(sAllocation) + 1];//plus one for debug marker
#else
						return &memPtr[sizeof(sAllocation)];
#endif
					}

					deallocIter = deallocIter->next;
				}

				bool createNewPool = false;

				//check if a new pool needs to be created
				if (!activeMemoryPool)
				{
					createNewPool = true;
				}
				else
				{
					/*
						Sorting the pool list is only done when the pool at the end doesn't have enough memory to
						fulfill the new request.
					*/					
					int32 sizeAvalaible = (activeMemoryPool->poolSize - activeMemoryPool->nextAvaliableByte);
					if (sizeAvalaible < sizeInBytes)
					{
						/*
							Before we decide to create a new pool, search through previous pools to
							see if any have some space left.
						*/

						//This will sort pools from largest to smallest space available.
						sPool *endHint = 0;
						memoryPools = mash::algorithms::MergeSort<sPool, sPoolSort, GetPoolDataPred>(memoryPools, &endHint, sPoolSort(), GetPoolDataPred());
						activeMemoryPool = memoryPools;

						sizeAvalaible = (activeMemoryPool->poolSize - activeMemoryPool->nextAvaliableByte);
						if (sizeAvalaible < sizeInBytes)
							createNewPool = true;
					}
				}

				if (createNewPool)
				{
					if (TIsFixedSize)
					{
						//new pools can't be created when fixed size is enabled
						MASH_ASSERT(0);
						return 0;
					}
					else
					{
						uint32 newPoolSize = initialPoolSizeInBytes;
						//calculate pool size
						if (activeMemoryPool)
							newPoolSize = activeMemoryPool->poolSize * poolSizeMultiplyer;

						uint32 totalSizeNeeded = sizeInBytes + sizeof(sPool);
						//make sure enough memory is allocated
						if (newPoolSize < totalSizeNeeded)
							newPoolSize = totalSizeNeeded;
						
						int8 *newPoolMemory = (int8*)TAllocatorFunctor()(newPoolSize, mash::_g_globalMemoryAlignment, category);
						if (!newPoolMemory)
						{
							//memory couldn't be allocated
							TErrorFunctor exp;
							exp(aFAILED_TO_ALLOC_NULL_PTR);

							//try one more time, TErrorFunctor may have freed memory
							newPoolMemory = (int8*)TAllocatorFunctor()(newPoolSize, mash::_g_globalMemoryAlignment, category);
							if (!newPoolMemory)
								return 0;
						}

						activeMemoryPool = (sPool*)newPoolMemory;
						activeMemoryPool->poolSize = newPoolSize;
						activeMemoryPool->nextAvaliableByte = sizeof(sPool);
						activeMemoryPool->next = 0;
						activeMemoryPool->previous = 0;

						if (!memoryPools)
							memoryPools = activeMemoryPool;
						else
						{
							memoryPools->previous = activeMemoryPool;
							activeMemoryPool->next = memoryPools;
							memoryPools = activeMemoryPool;
						}
					}
				}

				const uint32 nextByte = activeMemoryPool->nextAvaliableByte;
				sAllocation *ptr = (sAllocation*)&((int8*)activeMemoryPool)[nextByte];
				ptr->allocationSize = sizeInBytes;
				activeMemoryPool->nextAvaliableByte += sizeInBytes;
				MASH_ASSERT(activeMemoryPool->nextAvaliableByte <= activeMemoryPool->poolSize);

				int8* cptr = (int8*)ptr;
#ifdef MASH_DEBUG
				cptr[sizeof(sAllocation)] = 127;
				cptr[sizeInBytes - 1] = 127;
				return &cptr[sizeof(sAllocation) + 1];
#else
				return &cptr[sizeof(sAllocation)];
#endif
			}

			void FreeMemory(void *ptr)
			{
				int8 *cptr = (int8*)ptr;
#ifdef MASH_DEBUG
				++debugData.totalDeallocations;
				++debugData.currentDeallocations;
				if (debugData.currentDeallocations > debugData.maxCurrentDeallocations)
					debugData.maxCurrentDeallocations = debugData.currentDeallocations;
	
				cptr = cptr - (sizeof(sAllocation) + 1);
				sAllocation *allocPtr = (sAllocation*)cptr;

				/*
					If these markers are gone then data has been written beyond the
					allocated bounds.
				*/
				MASH_ASSERT(cptr[sizeof(sAllocation)] == 127);
				MASH_ASSERT(cptr[allocPtr->allocationSize-1] == 127);
#else
				cptr = cptr - sizeof(sAllocation);
				sAllocation *allocPtr = (sAllocation*)cptr;
#endif
				//add to deallocation list
				allocPtr->next = 0;
				allocPtr->previous = 0;
				if (!deallocations)
					deallocations = allocPtr;
				else
				{
					allocPtr->next = deallocations;
					deallocations->previous = allocPtr;

					deallocations = allocPtr;
				}

				sAllocation *endHint = 0;
				//sorts from smallest to largest size
				deallocations = mash::algorithms::MergeSort<sAllocation, sDeallocationSort, GetAllocDataPred>(deallocations, &endHint, sDeallocationSort(), GetAllocDataPred());
			}

			void Clear()
			{
				deallocations = 0;

#ifdef MASH_DEBUG
				debugData.currentDeallocations = 0;
#endif	

				sPool *poolIter = memoryPools;
				while(poolIter)
				{
					poolIter->nextAvaliableByte = sizeof(sPool);
					poolIter = poolIter->next;
				}
			}

			void Destroy()
			{
				Clear();
		
				sPool *poolIter = memoryPools;
				while(poolIter)
				{
					sPool *nextPool = poolIter->next;
					TDeallocatorFunctor()(poolIter);
					poolIter = nextPool;
				}

				memoryPools = 0;
				activeMemoryPool = 0;
			}
		};

		InternalPool *m_classDataInstance;
	public:
        //! Constructor.
        /*!
            \param initialPoolSizeInBytes Each pool will be at least this size. A pool maybe larger if needed.
            \param initialPoolCount Pools will be created as needed.
            \param poolSizeMultiplyer New pool size will be equal to (last pool size * poolSizeMultiplyer). Best to leave this at 1 for equal pool sizes.
            \param category Memory category.
        */
		MashMemoryPool(uint32 initialPoolSizeInBytes, uint32 initialPoolCount = 0, uint32 poolSizeMultiplyer = 1, eMEMORY_CATEGORY category = aMEMORY_CATEGORY_COMMON)
		{
			m_classDataInstance = (InternalPool*)TAllocatorFunctor()(sizeof(InternalPool), mash::_g_globalMemoryAlignment, category);//MASH_NEW(category) sClassData();
			new ((void*)m_classDataInstance)  InternalPool(initialPoolSizeInBytes, initialPoolCount, poolSizeMultiplyer, category);
		}

        //! Copy constructor.
		MashMemoryPool(const MashMemoryPool &copy):m_classDataInstance(0)
		{
			*this = copy;
		}

		/*!
			This is only here so that a default constructor is defined for
			specific cases. Instead use the constructor that takes paramters because
			m_classDataInstance must be valid before use.
		*/
		MashMemoryPool():m_classDataInstance(0)
		{
		}

		~MashMemoryPool()
		{			
			Destroy();
		}

        //! Assignment operator.
		void operator=(const MashMemoryPool &copy)
		{
			if (this == &copy)
				return;

			if (copy.m_classDataInstance)
			{
				copy.m_classDataInstance->Grab();
			}

			if (m_classDataInstance)
			{
				Destroy();
			}

			if (copy.m_classDataInstance)
			{
				m_classDataInstance = copy.m_classDataInstance;
			}
		}

        //! Gets some memory from the pool.
        /*!
            \param sizeInBytes Bytes to get.
            \param alignment Memory alignment.
        */
		void* GetMemory(uint32 sizeInBytes, size_t alignment = mash::_g_globalMemoryAlignment)
		{
			if (m_classDataInstance)
				return m_classDataInstance->GetMemory(sizeInBytes, alignment);

			return 0;
		}

        //! Returns memory to the pool.
		void FreeMemory(void *ptr)
		{
			if (m_classDataInstance)
				m_classDataInstance->FreeMemory(ptr);
		}

        //! Clear allocations, but does not delete any memory.
		void Clear()
		{
			if (m_classDataInstance)
				m_classDataInstance->Clear();
		}

        //! Destroys all memory. This pool is no longer valid after this call.
        /*!
            The assignment operator maybe used after this call to re-initialise.
        */
		void Destroy()
		{
			if (m_classDataInstance)
			{
				m_classDataInstance->Drop();
				m_classDataInstance = 0;
			}
		}
	};
}

#endif