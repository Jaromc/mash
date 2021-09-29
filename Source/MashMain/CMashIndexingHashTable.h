//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_INDEXING_HASH_TABLE_H_
#define _C_MASH_INDEXING_HASH_TABLE_H_

#include "MashArray.h"

namespace mash
{
	template<class T>
	struct sDefaultMashIndexingHashTableCmp
	{
		bool operator()(const T &a, const T &b)const
		{
			return (a == b);
		}

		sDefaultMashIndexingHashTableCmp(){}
	};

	/*
		This is a hash table that does not store the keys internally. Instead
		a list of indices that are used to access elements from an array passed in.

		Example useage for MashVector3s. This could create a list of unique positions
		and a list of indices that access that list.

		CMashIndexingHashTable<MashVector3> indexingHashTable(vertexCount, PositionHashingFunction);

		for(uint32 i = 0; i < indexCount; ++i)
		{
			MashVector3 position = points[indices[i]];

			uint32 createdIndexValue = 0;
			uint32 foundIndexValue = indexingHashTable.Add(position, uniquePoints.Pointer(), createdIndexValue);
			if (foundIndexValue == mash::math::MaxUInt32())
				uniquePoints.PushBack(position);

			newIndexList.PushBack(createdIndexValue);
		}
	*/
	template<class T, class TCmp = sDefaultMashIndexingHashTableCmp<T> >
	class CMashIndexingHashTable
	{
	private:
		struct sEntry
		{
			sEntry *next;
			uint32 index;

			sEntry():index(mash::math::MaxUInt32()), next(0){}
			~sEntry()
			{
			}
		};

		struct sPool
		{
			sEntry *mem;
			uint32 used;
			uint32 size;

			bool operator<(const sPool &other)const
			{
				return (size - used) < (other.size - other.used);
			}
		};

		sEntry *m_hashTable;
		uint32 m_hashTableSize;
		uint32 m_uniqueItemCount;
		uint32 (*m_hashingFunction)(const T &item);
		MashArray<sPool> m_pools;
		TCmp m_cmp;
#ifdef MASH_DEBUG
		uint32 m_clashCount;
#endif
	public:
		CMashIndexingHashTable(uint32 hashTableSize, uint32 (*hashingFunction)(const T &item), TCmp cmp = TCmp()):m_hashTableSize(hashTableSize), m_uniqueItemCount(0),
			m_hashingFunction(hashingFunction), m_cmp(cmp)
		{
#ifdef MASH_DEBUG
		m_clashCount = 0;
#endif
			if (m_hashTableSize > 0)
				m_hashTable = MASH_NEW_ARRAY_T_COMMON(sEntry, m_hashTableSize);
		}

		~CMashIndexingHashTable()
		{
			if (m_hashTable)
				MASH_DELETE_ARRAY_T(sEntry, m_hashTable, m_hashTableSize);

			const uint32 poolSize = m_pools.Size();
			for(uint32 i = 0; i < poolSize; ++i)
				MASH_FREE(m_pools[i].mem);
		}

		void Clear()
		{
			const uint32 poolSize = m_pools.Size();
			for(uint32 i = 0; i < poolSize; ++i)
				m_pools[i].used = 0;

			for(uint32 i = 0; i < m_hashTableSize; ++i)
			{
				m_hashTable[i].index = mash::math::MaxUInt32();
				m_hashTable[i].next = 0;
			}

			m_uniqueItemCount = 0;

#ifdef MASH_DEBUG
		m_clashCount = 0;
#endif
		}

		/*
			Return value is the index of a previous identical entry. This will be 0xFFFFFFFF if a new index had to be created.
			indexValueOut stores the created index value. 
			The first time Add is called, itemArray maybe NULL. After that it must be a valid array.

			\param item key.
			\param itemArray This array will be searched for the key.
			\param indexValueOut Return value. This will be the keys unique value.
			\return 0xFFFFFFFF if the key was not found. Else it's equal to indexValueOut.
		*/
		uint32 Add(const T &item, const T *itemArray, uint32 &indexValueOut)
		{
			uint32 hashValue = m_hashingFunction(item) % m_hashTableSize;
			if (m_hashTable[hashValue].index == mash::math::MaxUInt32())
			{
				//new entry
				m_hashTable[hashValue].index = m_uniqueItemCount++;
				indexValueOut = m_hashTable[hashValue].index;
				return mash::math::MaxUInt32();
			}
			else
			{
				//confirm we have found a duplicate
				if (m_cmp(itemArray[m_hashTable[hashValue].index], item))
				{
					indexValueOut = m_hashTable[hashValue].index;
					return m_hashTable[hashValue].index;
				}
				else
				{
					//we have a clash, search the rest of the clash list
					sEntry *next = m_hashTable[hashValue].next;
					while(next)
					{
						if (itemArray[next->index] == item)
						{
							//value found
							indexValueOut = next->index;
							return next->index;
						}
						else
						{
							next = next->next;
						}
					}

					if (!next)
					{
#ifdef MASH_DEBUG
		++m_clashCount;
#endif
						bool createNewPool = m_pools.Empty();
						if (!m_pools.Empty())
						{
							//check if the pool on the end has enough space
							sPool *p = &m_pools.Back();
							if ((p->size - p->used) < 1)
							{
								//if it doesnt then sort the list and check again (this is done for if Clear() is used)
								m_pools.Sort();
								p = &m_pools.Back();
								if ((p->size - p->used) < 1)
									createNewPool = true;
							}
						}

						if (createNewPool)
						{
							sPool newPool;
							newPool.size = math::Max<uint32>(10, m_hashTableSize / 10);//magic size
							newPool.used = 0;
							newPool.mem = MASH_ALLOC_T_COMMON(sEntry, newPool.size);
							m_pools.PushBack(newPool);
						}

						sPool *p = &m_pools.Back();

						//no match found so append to the list
						next = &p->mem[p->used++];

						next->index = m_uniqueItemCount++;

						next->next = m_hashTable[hashValue].next;
						m_hashTable[hashValue].next = next;
						indexValueOut = next->index;
						return mash::math::MaxUInt32();
					}
				}
			}
		}
	};
}

#endif