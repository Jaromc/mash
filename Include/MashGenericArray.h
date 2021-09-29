//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_GENERIC_ARRAY_H_
#define _MASH_GENERIC_ARRAY_H_

#include "MashDataTypes.h"

namespace mash
{
	/*!
		Simple non templated array.
	*/
	class _MASH_EXPORT MashGenericArray
	{
	private:
		void *m_data;
		uint32 m_reservedSize;
		uint32 m_currentSize;
	public:
		MashGenericArray();
		~MashGenericArray();

		//! Reserves a size and assigns a value to that memory.
		/*!
			\param val Value to assign.
			\param size Reserve size.
		*/
		void Assign(int32 val, uint32 size);

		//! Appends new data.
		/*!
			\param data Data to append.
			\param size Size in bytes.
		*/
		void Append(const void *data, uint32 size);

		//! Appends a f32.
		void AppendFloat(f32 v);

		//! Appends an int32.
		void AppendInt(int32 v);

		//! Appends an uint32
		void AppendUnsignedInt(uint32 v);

		//! Appends a int8.
		void AppendChar(int8 v);

		//! Reserves some memory.
		/*!
			Reserves but doesn't change the number of elements in the array.
         
			\param size Size in bytes.
		*/
		void Reserve(uint32 size);

		//! Returns the raw array.
		void* Pointer()const;

		//! Gets the number of elements in the array.
		uint32 GetCurrentSize()const;

		//! Resets the element count back to zero. Doesn't delete any memory.
		void Clear();

		//! Deletes the array, releases memory and sets all elements back to zero.
		void DeleteData();

		//! Returns true if this array has zero elements.
		bool Empty()const;
	};

	inline bool MashGenericArray::Empty()const
	{
		return (m_currentSize == 0);
	}

	inline void MashGenericArray::Clear()
	{
		m_currentSize = 0;
	}

	inline void* MashGenericArray::Pointer()const
	{
		return m_data;
	}

	inline uint32 MashGenericArray::GetCurrentSize()const
	{
		return m_currentSize;
	}
}

#endif