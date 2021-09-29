//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "MashGenericArray.h"
#include "MashMathHelper.h"
#include "MashMemory.h"
#include <cstring>
namespace mash
{
	MashGenericArray::MashGenericArray():m_data(0), m_reservedSize(0), m_currentSize(0)
	{
		
	}

	MashGenericArray::~MashGenericArray()
	{
		DeleteData();
	}

	void MashGenericArray::DeleteData()
	{
		if (m_data)
		{
			MASH_FREE(m_data);
			m_data = 0;
		}

		m_reservedSize = 0;
		m_currentSize = 0;
	}

	void MashGenericArray::Reserve(uint32 size)
	{
		void *pNewVertices = MASH_ALLOC_COMMON(size);

		//copy over old data
		if (m_data)
		{
			/*
				Only copy over old data up to the size of the new buffer
			*/
			uint32 dataSizeToCopy = math::Min<uint32>(size, m_reservedSize);
			memcpy(pNewVertices, m_data, dataSizeToCopy);

			MASH_FREE(m_data);
			m_data = 0;
		}

		m_data = pNewVertices;
		m_reservedSize = size;
	}

	void MashGenericArray::Assign(int32 val, uint32 size)
	{
		Reserve(size);
		memset(m_data, val, size);
		m_currentSize = size;
	}

	void MashGenericArray::Append(const void *data, uint32 size)
	{
		//resize the buffer if necessary
		const uint32 newSizeInBytes = size + m_currentSize;
		if (newSizeInBytes > m_reservedSize)
			Reserve(newSizeInBytes * 2);

		memcpy(&((int8*)m_data)[m_currentSize], data, size);
		m_currentSize += size;
	}

	void MashGenericArray::AppendFloat(f32 v)
	{
		Append(&v, sizeof(f32));
	}

	void MashGenericArray::AppendInt(int32 v)
	{
		Append(&v, sizeof(int32));
	}

	void MashGenericArray::AppendUnsignedInt(uint32 v)
	{
		Append(&v, sizeof(uint32));
	}

	void MashGenericArray::AppendChar(int8 v)
	{
		Append(&v, sizeof(int8));
	}
}