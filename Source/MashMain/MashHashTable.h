//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _MASH_HASH_TABLE_H_
#define _MASH_HASH_TABLE_H_

#include "MashArray.h"
#include "MashList.h"

namespace mash
{
	template<class keyType, class dataType>
	class MashHashEntry
	{
	public:
		MashHashEntry(){}
		~MashHashEntry(){}
		keyType key;
		dataType data;
	};

	template<class KeyType, class DataType>
	class MashHashTable
	{
	private:
		int32 m_iNumElements;
		MashArray<MashList<MashHashEntry<KeyType, DataType>>> m_hashTable;
		unsigned long int32 (*m_hash)(KeyType);
	public:
		MashHashTable(unsigned long int32 (*pHash)(KeyType)):m_iNumElements(0),
			m_hash(pHash),
			m_hashTable()
		{
			m_hashTable.Resize(1000);//TODO : Bad hack. Fix it!!!!
		}

		MashHashTable():m_iNumElements(0),
			m_hash(0),
			m_hashTable()
		{
			m_hashTable.Resize(1000);//TODO : Bad hack. Fix it!!!!
		}

		void SetHashFunction(unsigned long int32 (*pHash)(KeyType));
		void Insert(const KeyType &key, const DataType &data);
		DataType* Find(const KeyType &key);
		bool Remove(const KeyType &key);
		void Clear();
		void Destroy();

	};

	template<class KeyType, class DataType>
	inline void MashHashTable<KeyType, DataType>::Clear()
	{
		m_iNumElements = 0;
		m_hashTable.ClearEx();
	}

	template<class KeyType, class DataType>
	inline void MashHashTable<KeyType, DataType>::Destroy()
	{
		m_iNumElements = 0;
		m_hashTable.Destroy();
	}

	template<class KeyType, class DataType>
	inline void MashHashTable<KeyType, DataType>::SetHashFunction(unsigned long int32 (*pHash)(KeyType))
	{
		m_hash = pHash;
	}

	template<class KeyType, class DataType>
	inline bool MashHashTable<KeyType, DataType>::Remove(const KeyType &key)
	{
		if (m_iNumElements == 0)
			return false;

		int32 iIndex = m_hash(key) % m_iNumElements;
		MashList<MashHashEntry<KeyType, DataType> >::iterator iter = m_hashTable[iIndex].begin();
		MashList<MashHashEntry<KeyType, DataType> >::iterator end = m_hashTable[iIndex].end();
		for(; iter != end; ++iter)
		{
			if (iter->key == key)
			{
				m_hashTable[iIndex].erase(iter);
				--m_iNumElements;
				return true;
			}
		}

		return false;
	}

	template<class KeyType, class DataType>
	inline DataType* MashHashTable<KeyType, DataType>::Find(const KeyType &key)
	{
		if (m_iNumElements == 0)
			return 0;

		int32 iIndex = m_hash(key) % m_iNumElements;
		MashList<MashHashEntry<KeyType, DataType> >::iterator iter = m_hashTable[iIndex].begin();
		MashList<MashHashEntry<KeyType, DataType> >::iterator end = m_hashTable[iIndex].end();
		for(; iter != end; ++iter)
		{
			if (iter->key == key)
				return &iter->data;
		}

		return 0;
	}

	template<class KeyType, class DataType>
	inline void MashHashTable<KeyType, DataType>::Insert(const KeyType &key, const DataType &data)
	{
		MashHashEntry<KeyType, DataType> entry;
		entry.data = data;
		entry.key = key;

		++m_iNumElements;

		int32 iIndex = m_hash(key) % m_iNumElements;
		m_hashTable[iIndex].PushBack(entry);			
	}
}

#endif