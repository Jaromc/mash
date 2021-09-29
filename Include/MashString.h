//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_STRING_H_
#define _MASH_STRING_H_

#include "MashMemoryPoolSimple.h"
#include "MashDataTypes.h"
#include "MashHelper.h"
#include <cstring>

namespace mash
{
    /*!
        Templated dynamic string.
    */
	template <class T, class TAlloc>
	class MashBaseString
	{
	public:
        //! Defines an invalid position.
		static const uint32 npos;

		class ConstIterator;

		class Iterator
		{
			friend class MashBaseString<T, TAlloc>;
		private:
			T *item;
		public:
			Iterator():item(0){}
			Iterator(T *_item):item(_item){}
			Iterator(const Iterator &other):item(other.item){}
			~Iterator(){}

			uint32 Distance(const Iterator &end)const
			{
				return end.item - item;
			}

			bool operator==(const Iterator &other)const
			{
				return (item == other.item);
			}

			bool operator!=(const Iterator &other)const
			{
				return (item != other.item);
			}

			bool operator==(const ConstIterator &other)const
			{
				return (item == other.item);
			}

			bool operator!=(const ConstIterator &other)const
			{
				return (item != other.item);
			}

			Iterator& operator=(const Iterator &other)
			{
				item = other.item;
				return *this;
			}

			Iterator& operator+=(int32 val)
			{
				item += val;

				return *this;
			}

			Iterator& operator-=(int32 val)
			{
				return ((*this) += -val);
			}

			Iterator operator+(int32 val)const
			{
				Iterator iter(*this);
				iter += val;
				return iter;
			}

			Iterator operator-(int32 val)const
			{
				Iterator iter(*this);
				iter -= val;
				return iter;
			}

			Iterator& operator++()
			{
				++item;
				return *this;
			}

			Iterator& operator--()
			{
				--item;
				return *this;
			}

			Iterator operator++(int32 val)
			{
				Iterator t = *this;
				++item;
				return t;
			}

			Iterator operator--(int32 val)
			{
				Iterator t = *this;
				--item;
				return t;
			}

			T& operator*()
			{
				return *item;
			}

			T* operator->()
			{
				return item;
			}
		};

		class ConstIterator
		{
			friend class MashBaseString<T, TAlloc>;
		private:
			T *item;
		public:
			ConstIterator():item(0){}
			ConstIterator(T *_item):item(_item){}
			ConstIterator(const ConstIterator &other):item(other.item){}
			~ConstIterator(){}

			uint32 Distance(const ConstIterator &end)const
			{
				return end.item - item;
			}

			bool operator==(const Iterator &other)const
			{
				return (item == other.item);
			}

			bool operator!=(const Iterator &other)const
			{
				return (item != other.item);
			}

			bool operator==(const ConstIterator &other)const
			{
				return (item == other.item);
			}

			bool operator!=(const ConstIterator &other)const
			{
				return (item != other.item);
			}

			ConstIterator& operator=(const ConstIterator &other)
			{
				item = other->item;
				return *this;
			}

			ConstIterator& operator+=(int32 val)
			{
				item += val;

				return *this;
			}

			ConstIterator& operator-=(int32 val)
			{
				return ((*this) += -val);
			}

			ConstIterator operator+(int32 val)const
			{
				ConstIterator iter(*this);
				iter += val;
				return iter;
			}

			ConstIterator operator-(int32 val)const
			{
				ConstIterator iter(*this);
				iter -= val;
				return iter;
			}

			ConstIterator& operator++()
			{
				++item;
				return *this;
			}

			ConstIterator& operator--()
			{
				--item;
				return *this;
			}

			ConstIterator operator++(int32 val)
			{
				ConstIterator t = *this;
				++item;
				return t;
			}

			ConstIterator operator--(int32 val)
			{
				ConstIterator t = *this;
				--item;
				return t;
			}

			T& operator*()
			{
				return *item;
			}

			T* operator->()
			{
				return item;
			}
		};
	private:
		TAlloc m_memoryPool;
		T *m_string;
		uint32 m_reservedElements;
		uint32 m_usedElements;

		enum
		{
			INTERNAL_POOL_SIZE = 16
		};

		T m_internalPool[INTERNAL_POOL_SIZE];

		void _Reserve(uint32 count)
		{
			if (count <= INTERNAL_POOL_SIZE)
			{
				MASH_ASSERT(m_reservedElements <= INTERNAL_POOL_SIZE);

				m_string = &m_internalPool[0];
				m_reservedElements = INTERNAL_POOL_SIZE;
			}
			else
			{
				//only free if the old data was dynamic
				bool freeOldData = true;
				if (m_reservedElements <= INTERNAL_POOL_SIZE)
					freeOldData = false;

				T *newMem = (T*)m_memoryPool.GetMemory(sizeof(T) * count);

                MASH_ASSERT(newMem);

				//set null terminator
				newMem[0] = 0;

				m_reservedElements = count;

				//copy over old string
				if (m_usedElements > 0)
					memcpy(newMem, m_string, sizeof(T) * m_usedElements);

				//release old memory
				if (freeOldData)
					m_memoryPool.FreeMemory(m_string);

				//store new pointer
				m_string = newMem;
			}
		}

		MashBaseString<T, TAlloc>& _Append(const T *append, uint32 appendStrLength, uint32 multiplier = 1)
		{
			if (!append)
				return *this;

			if (appendStrLength == 0)
				return *this;

			int32 spaceLeft = m_reservedElements - m_usedElements;
			int32 spaceNeeded = appendStrLength;
			//make sure there is enough memory
			if (spaceLeft <= spaceNeeded)
			{
				int32 newBlockSize = (spaceNeeded + m_reservedElements) * multiplier;
				_Reserve(newBlockSize);
			}

			//copy over string
			memcpy(&m_string[m_usedElements-1], append, sizeof(T) * appendStrLength);
			m_usedElements += appendStrLength;

			//set null terminator
			m_string[m_usedElements-1] = 0;

			return *this;
		}
	public:
		MashBaseString():m_usedElements(0), m_reservedElements(0), m_string(0)
		{
			_Reserve(1);
			m_usedElements = 1;
			m_string[0] = 0;
		}

        //! Constructor.
        /*!
            \param alloc Allocator.
        */
		MashBaseString(const TAlloc &alloc):m_memoryPool(alloc), m_usedElements(0), m_reservedElements(0), m_string(0)
		{
			_Reserve(1);
			m_usedElements = 1;
			m_string[0] = 0;
		}

        //! Copy constructor.
		MashBaseString(const MashBaseString<T, TAlloc> &other):
			m_memoryPool(other.m_memoryPool),
			m_usedElements(0),
			m_reservedElements(0), 
			m_string(0)
		{
			_Reserve(1);
			m_usedElements = 1;
			m_string[0] = 0;

			*this = other;
		}

        //! Copy constructor.
		MashBaseString(const T *other):
			m_usedElements(0),
			m_reservedElements(0), 
			m_string(0)
		{
			_Reserve(1);
			m_usedElements = 1;
			m_string[0] = 0;

			*this = other;
		}

        //! Constructor.
        /*!
            \param other String to copy over.
            \param alloc Allocator.
        */
		MashBaseString(const T *other, const TAlloc &alloc):
			m_memoryPool(alloc),
			m_usedElements(0),
			m_reservedElements(0), 
			m_string(0)
		{
			_Reserve(1);
			m_usedElements = 1;
			m_string[0] = 0;

			*this = other;
		}

        //! Constructor.
        /*!
            \param other String to copy over.
            \param count number of elements to copy over, not including the null terminator.
         */
		MashBaseString(const T *other, uint32 count):
			m_usedElements(0),
			m_reservedElements(0), 
			m_string(0)
		{
			_Reserve(1);
			m_usedElements = 1;
			m_string[0] = 0;

			Insert(0, other, count);
		}

		~MashBaseString()
		{
			if (m_reservedElements > INTERNAL_POOL_SIZE)
			{
				m_memoryPool.FreeMemory(m_string);
				m_string = 0;
				m_reservedElements = 0;
				m_usedElements = 0;
			}
		}

        //! Constructor.
        /*!
            \param num Converts an int to a string.
         */
        static MashBaseString<T, TAlloc> CreateFrom(int32 num)
        {
            MashBaseString<T, TAlloc> s;
            s.ToString(num);
            return s;
        }

        //! Constructor.
        /*!
            \param num Converts a uint to a string.
        */
        static MashBaseString<T, TAlloc> CreateFrom(uint32 num)
        {
            MashBaseString<T, TAlloc> s;
            s.ToString(num);
            return s;
        }

        //! Constructor.
        /*!
            \param num Converts a float to a string.
         */
        static MashBaseString<T, TAlloc> CreateFrom(f32 num)
        {
            MashBaseString<T, TAlloc> s;
            s.ToString(num);
            return s;
        }

        //! Returns an iterator to the first element.
		ConstIterator Begin()const
		{
			return ConstIterator(m_string);
		}

        //! Returns an iterator to one past the valid elements.
        /*!
            This should not be dereferenced.
        */
		ConstIterator End()const
		{
			return ConstIterator(m_string + (m_usedElements - 1));
		}

        //! Returns an iterator to the first element.
		Iterator Begin()
		{
			return Iterator(m_string);
		}

        //! Returns an iterator to one past the valid elements.
        /*!
            This should not be dereferenced.
         */
		Iterator End()
		{
			return Iterator(m_string + (m_usedElements - 1));
		}
        
        //! Removes an element from the back of the string.
        /*!
            It is safe to call this if empty.
        */
        void PopBack()
        {
            if (m_usedElements > 1)
            {
                Erase(End() - 1);
            }
        }
        
        //! Removes an element from the front of the string.
        /*!
            It is safe to call this if empty.
         */
        void PopFront()
        {
            if (m_usedElements > 1)
            {
                Erase(Begin());
            }
        }

        //! Removes a instances of the given char.
		void Remove(T removeChar)
		{
			//can't remove null char
			if (removeChar == 0)
				return;

			uint32 i = 0;
			while(i < m_usedElements)
			{
				if (m_string[i] == removeChar)
					Erase(i);
				else
					++i;
			}
		}

        //! Replaces all instances of the given char with another.
		void Replace(T find, T replaceWith)
		{
			for(uint32 i = 0; i < m_usedElements; ++i)
			{
				if (m_string[i] == find)
					m_string[i] = replaceWith;
			}
		}

        //! Replaces all chars within a range.
        /*!
            \param pos Position to start replacing.
            \param count Number of chars to remove from this string.
            \param str Null terminated string to insert.
        */
		void Replace(uint32 pos, uint32 count, const T *str)
		{
			Erase(pos, count);
			Insert(pos, str);
		}
        
        //! Returns true if the given char is found.
        bool Contains(T contains)
        {
            if (Find(contains) == npos)
                return false;
            
            return true;
        }

        //! Returns an index to the first instance of the given char if found.
        /*!
            returns MashString::npos if not found.
        */
		uint32 Find(T find)
		{
			for(uint32 i = 0; i < m_usedElements; ++i)
			{
				if (m_string[i] == find)
					return i;
			}

			return npos;
		}

        //! Finds a substring within this string.
        /*!
            Returns an index to the start of the first instance of the given string if found.
            Returns MashString::npos if not found.
         
            \param findStr Sub string.
            \param fromPos Position in this string to start searching.
         */
		uint32 Find(const MashBaseString<T, TAlloc> &findStr, uint32 fromPos = 0)
		{
			return Find(findStr.GetCString(), fromPos);
		}

        //! Finds a substring within this string.
        /*!
            Returns an index to the start of the first instance of the given string if found.
            Returns MashString::npos if not found.

            \param findStr Sub string.
            \param fromPos Position in this string to start searching.
         */
		uint32 Find(const T *findStr, uint32 fromPos = 0)
		{
			const uint32 findStrLen = strlen(findStr);
			uint32 matchCount;

			for(uint32 i = fromPos;; ++i)
			{
				if ((m_usedElements - i) < findStrLen)
					break;

				matchCount = 0;

				for(uint32 sub = 0; sub < findStrLen; ++sub)
				{
					if (m_string[i+sub] == findStr[sub])
						++matchCount;
					else
						break;
				}
				
				if (matchCount == findStrLen)
					return i;
			}

			return npos;
		}

        //! Inserts a char.
        /*!
            \param location Character will be inserted before this position.
            \param ch Character to insert.
            \return Iterator to newly inserted char.
        */
		Iterator Insert(const Iterator &location, T ch)
		{
			return Insert(location.item - m_string, &ch, 1);
		}

        //! Inserts a char.
        /*!
            \param location Character will be inserted before this position.
            \param ch Character to insert.
            \return Iterator to newly inserted char.
         */
		Iterator Insert(uint32 location, T ch)
		{
			return Insert(location, &ch, 1);
		}

        //! Inserts a string.
        /*!
            \param location String will be inserted before this position.
            \param str Null terminated string to insert.
            \return Iterator to newly inserted string.
         */
		Iterator Insert(uint32 location, const T *str)
		{
			if (str)
				return Insert(location, str, strlen(str));
            
            return End();
		}

        //! Inserts a string.
        /*!
            \param location String will be inserted before this position.
            \param other String to insert.
            \return Iterator to newly inserted string.
         */
		Iterator Insert(const Iterator &location, const MashBaseString<T, TAlloc> &other)
		{
			return Insert(location.item - m_string, other.GetCString(), other.Size());
		}

        //! Inserts a string.
        /*!
            \param location String will be inserted before this position.
            \param other String to insert.
            \return Iterator to newly inserted string.
         */
		Iterator Insert(uint32 location, const MashBaseString<T, TAlloc> &other)
		{
			return Insert(location, other.GetCString(), other.Size());
		}

        //! Inserts a string.
        /*!
            \param location String will be inserted before this position.
            \param other String to insert.
            \param count Number of characters to copy over.
            \return Iterator to newly inserted string.
         */
		Iterator Insert(uint32 location, const T *other, uint32 count)
		{
			if (!other)
				return End();

			if (count == 0)
				return End();

			MASH_ASSERT(location < m_usedElements);

			int32 spaceLeft = m_reservedElements - m_usedElements;
			int32 spaceNeeded = count;
			//make sure there is enough memory
			if (spaceLeft < spaceNeeded)
			{
				int32 newBlockSize = (spaceNeeded + m_reservedElements) * 2;
				_Reserve(newBlockSize);
			}

			//move old string
			memmove(&m_string[location + count], &m_string[location], sizeof(T) * (m_usedElements - location));
			
			//insert new string
			memcpy(&m_string[location], other, count);

			m_usedElements += count;
            
            return Iterator(m_string + location);
		}

        //! Appends a string to this string.
        /*!
            \param append String to append.
            \param count Number of elements to copy over, not including the null terminator.
        */
		void Append(const MashBaseString<T, TAlloc> &append, uint32 count)
		{
			_Append(append.GetCString(), count, 2);
		}

        //! Appends a string to this string.
        /*!
            \param append String to append.
         */
		void Append(const MashBaseString<T, TAlloc> &append)
		{
			_Append(append.GetCString(), append.Size(), 2);
		}

        //! Appends a string to this string.
        /*!
            \param append String to append.
            \param count Number of elements to copy over, not including the null terminator.
         */
		void Append(const T *append, uint32 count)
		{
			_Append(append, count, 2);
		}

        //! Appends a string to this string.
        /*!
            \param append Null terminated string to append.
         */
		void Append(const T *append)
		{
			_Append(append, strlen(append), 2);
		}

        //! Appends a character.
		void Append(T ch)
		{
			_Append(&ch, 1, 2);
		}

        //! Reserves memory for efficient appending of characters.
        /*!
            This will not shrink the current reserved memory. Instead
            look at using ShrinkToFit().
         
            This does not change the Size().
         
            \param count Number of elements to reserve.
        */
		void Reserve(uint32 count)
		{
			if (count < m_reservedElements)
				return;

			_Reserve(count);
		}
        
        //! Resizes the Size().
        /*!
             This will not shrink the current reserved memory. Instead
             look at using ShrinkToFit().
         
            \param elementCount New element count, not including the null terminator.
            \param val Default value for new elements.
        */
        void Resize(uint32 elementCount, const T &val = T())
		{
            elementCount += 1;//+1 for null terminator
            
			if (elementCount == m_usedElements)
				return;
            
			if (elementCount > m_usedElements)
			{
				_Reserve(elementCount);
                
                if (helpers::MashIsInt8<T>::value || helpers::MashIsUInt8<T>::value)
				{
                    memset(&m_string[m_usedElements-1], val, (elementCount - m_usedElements));
                }
                else
                {
                    for(uint32 i = m_usedElements-1; i < elementCount-1; ++i)
                        m_string[i] = val;
                }
			}
            
			m_usedElements = elementCount;
		}

        //! Removes a range of element from this string.
        /*!
            \param start First element to remove.
            \param end One past the last element to remove.
        */
		void Erase(const Iterator &start, const Iterator &end)
		{
            uint32 s = start.item - m_string;
            uint32 e = end.item - m_string;
            uint32 d = (e - s);

            MASH_ASSERT(e < m_usedElements);

            while(e < m_usedElements)
                m_string[s++] = m_string[e++];
            
            m_usedElements -= d;
		}

        //! Removes an element from this string.
        /*!
            \param index The index of the element to remove.
        */
		void Erase(uint32 index)
		{
			MASH_ASSERT(index < (m_usedElements - 1)/*-1 because null terminator can't be erased*/);

            for(uint32 i = index+1; i < m_usedElements; ++i)
                m_string[i-1] = m_string[i];
            
            --m_usedElements;
		}

        //! Removes an element from this string.
        /*!
            \param location The index of the element to remove.
         */
		void Erase(Iterator location)
		{
			Erase(location.item - m_string);
		}

        //! Removes a range of element from this string.
        /*!
            \param index First element to remove.
            \param count Number of elements to remove.
         */
		void Erase(uint32 index, uint32 count)
		{
			Erase(Begin() + index, Begin() + index + count);
		}

        //! Clears the current contents and converts an int to a string.
		void ToString(uint32 num)
		{
			Clear();

			char buffer[33] = {0};
            mash::helpers::NumberToString(buffer, sizeof(buffer), num);
			_Append(buffer, strlen(buffer), 1);
		}

        //! Clears the current contents and converts an int to a string.
		void ToString(int32 num)
		{
			Clear();

			char buffer[33] = {0};
			mash::helpers::NumberToString(buffer, sizeof(buffer), num);
			_Append(buffer, strlen(buffer), 1);
		}

        //! Clears the current contents and converts a float to a string.
		void ToString(f32 num)
		{
			Clear();

			char buffer[33] = {0};
			mash::helpers::FloatToString(buffer, sizeof(buffer), num);
			_Append(buffer, strlen(buffer), 1);
		}

        //! Less than comparison.
		bool operator<(const MashBaseString<T, TAlloc> &other)const
		{
			return strcmp(m_string, other.GetCString()) < 0;
		}

        //! Equals to.
		bool operator==(const MashBaseString<T, TAlloc> &other)const
		{
			return (*this == other.GetCString());
		}

        //! Not equals to.
		bool operator!=(const MashBaseString<T, TAlloc> &other)const
		{
			return !(*this == other.GetCString());
		}

        //! Equals to (without conversion).
		bool operator==(const T *other)const
		{
			if (!other)
				return false;

			if (!m_string)
				return false;

			uint32 i = 0;
			for(; other[i] && m_string[i]; ++i)
			{
				if (other[i] != m_string[i])
					return false;
			}

			return (!other[i] && !m_string[i]);
		}

        //! Not equals to (without conversion).
		bool operator!=(const T *other)const
		{
			return !(*this == other);
		}

        //! Assignment.
		MashBaseString<T, TAlloc>& operator=(const T *other)
		{
			m_usedElements = 1;
			m_string[0] = 0;

			if (!other)
				return *this;

			return _Append(other, strlen(other), 1);
		}

        //! Assignment.
		MashBaseString<T, TAlloc>& operator=(const MashBaseString<T, TAlloc> &other)
		{
			if (this == &other)
				return *this;

			m_usedElements = 1;
			m_string[0] = 0;
			return _Append(other.GetCString(), other.Size(), 1);
		}

        //! Appends a character.
		MashBaseString<T, TAlloc>& operator+=(const T &append)
		{
			return _Append(&append, 1, 2);
		}

        //! String concat.
		MashBaseString<T, TAlloc> operator+(const MashBaseString<T, TAlloc> &append)const
		{
			MashBaseString<T, TAlloc> newString(*this);
			newString += append;
			return newString;
		}

        //! String concat (without conversion).
		MashBaseString<T, TAlloc> operator+(const T *append)const
		{
			MashBaseString<T, TAlloc> newString(*this);
			newString += append;
			return newString;
		}

        //! String concat (without conversion).
		MashBaseString<T, TAlloc>& operator+=(const T *append)
		{
			return _Append(append, strlen(append), 2);
		}

        //! String concat.
		MashBaseString<T, TAlloc>& operator+=(const MashBaseString<T, TAlloc> &append)
		{
			return _Append(append.GetCString(), append.Size(), 2);
		}

        //! Returns an element at the given index.
		const T& operator[](uint32 index)const
		{
			return m_string[index];
		}

        //! Returns an element at the given index.
		T& operator[](uint32 index)
		{
			return m_string[index];
		}

        //! Returns a null terminated c string.
		const T* GetCString()const
		{
			return m_string;
		}

        //! Removes all elements, but doesn't delete memory.
		void Clear()
		{
			m_usedElements = 1;
			m_string[0] = 0;
		}

		//! Clears the string and frees memory.
		void FreeMemory()
		{
			if (m_reservedElements > INTERNAL_POOL_SIZE)
			{
				m_memoryPool.FreeMemory(m_string);
				m_string = &m_internalPool[0];
				m_string[0] = 0;
				m_reservedElements = INTERNAL_POOL_SIZE;
				m_usedElements = 1;
			}
			else
			{
				Clear();
			}
		}
        
        //! Shrinks ReservedSize() down to  Size() + 1.
        void ShrinkToFit()
		{
			if (m_reservedElements != m_usedElements)
			{
				_Reserve(m_usedElements);
			}
		}

        //! Returns true if this string is empty.
		bool Empty()const
		{
			return (m_usedElements == 1);
		}

        //! Returns the number of elements in this string, minius the null terminator.
		uint32 Size()const
		{
			return (m_usedElements - 1);
		}

        //! Returns the first character.
		const T Front()const
		{
			return m_string[0];
		}

        //! Returns the last character.
		const T Back()const
		{
			if (Empty())
				return m_string[0];

			return m_string[Size() - 1];
		}
        
        //! Returns the number of elements reserved.
        /*!
            Upto this amount can be accessed from a c style array.
        */
        uint32 ReservedSize()const
		{
			return m_reservedElements;
		}
        
        //! Returns the allocator.
        const TAlloc& GetAllocator()const
        {
            return m_memoryPool;
        }
        
        //! Returns the allocator.
        TAlloc& GetAllocator()
        {
            return m_memoryPool;
        }
	};

	//global operator
	template<class T, class TAlloc>
	MashBaseString<T, TAlloc> operator+(const T *left, const MashBaseString<T, TAlloc> &right)
	{
		MashBaseString<T, TAlloc> newString(left, right.GetAllocator());
		newString += right;
		return newString;
	}

	//global operator
	template<class T, class TAlloc>
	bool operator==(const T *left, const MashBaseString<T, TAlloc> &right)
	{
		return (right == left);
	}
	
	template<class T, class TAlloc>
	const uint32 MashBaseString<T, TAlloc>::npos = -1;

	typedef MashBaseString<int8, MashMemoryPoolSimple> MashStringc;
}

#endif
