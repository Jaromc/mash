//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_ARRAY_H_
#define _MASH_ARRAY_H_

#include "MashDataTypes.h"
#include "MashMemory.h"
#include "MashMemoryPoolSimple.h"
#include "MashAlgorithms.h"
#include "MashHelper.h"
#include <cstring>
#include <algorithm>

namespace mash
{
    /*!
        Templated dynamic array. 
    */
	template<class T, class TAlloc = MashMemoryPoolSimple>
	class MashArray
	{
	public:
		class ConstIterator;

		class Iterator
		{
			friend class MashArray<T, TAlloc>;
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
			friend class MashArray<T, TAlloc>;
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

			bool operator==(const ConstIterator &other)const
			{
				return (item == other.item);
			}

			bool operator!=(const ConstIterator &other)const
			{
				return (item != other.item);
			}

			bool operator==(const Iterator &other)const
			{
				return (item == other.item);
			}

			bool operator!=(const Iterator &other)const
			{
				return (item != other.item);
			}

			ConstIterator& operator=(const ConstIterator &other)
			{
				item = other.item;
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
		T *m_data;
		uint32 m_reservedElements;
		uint32 m_currentElements;

		void _Reserve(uint32 elementCount)
		{
			T *newMemory = (T*)m_memoryPool.GetMemory(sizeof(T) * elementCount);

			//copy over old data
			if (m_data)
			{
				if (m_currentElements > 0)
				{
					//reconstruct old data
					if (!helpers::MashIsFundamental<T>::value)
					{
						for(uint32 i = 0; i < m_currentElements; ++i)
						{
							MashMemoryAllocatorHelper<T>::Construct(&newMemory[i], m_data[i]);
							MashMemoryAllocatorHelper<T>::Destruct(&m_data[i]);
						}
					}
					else
					{
						//memcpy primitive types
						memcpy(newMemory, m_data, sizeof(T) * m_currentElements);
					}
				}

				m_memoryPool.FreeMemory(m_data);
				m_data = 0;
			}

			m_data = newMemory;
			m_reservedElements = elementCount;
		}
	public:
		MashArray():m_data(0), m_reservedElements(0), m_currentElements(0)
		{
		}

		MashArray(const MashArray<T, TAlloc> &other):m_memoryPool(other.m_memoryPool), 
			m_data(0), m_reservedElements(0), m_currentElements(0)
		{
			*this = other;
		}

		MashArray(const TAlloc &alloc):m_memoryPool(alloc), 
			m_data(0), m_reservedElements(0), m_currentElements(0)
		{
		}

        //! Constructor.
        /*!
            \param items A C array of items to fill this array with.
            \param count Number of elements in items.
        */
		MashArray(const T *items, uint32 count):m_data(0), m_reservedElements(0), m_currentElements(0)
		{
			Append(items, count);
		}

        //! Constructor.
        /*!
            \param items A C array of items to fill this array with.
            \param count Number of elements in items.
            \param alloc Allocator.
         */
		MashArray(const T *items, uint32 count, const TAlloc &alloc):m_memoryPool(alloc), 
			m_data(0), m_reservedElements(0), m_currentElements(0)
		{
			Append(items, count);
		}

        //! Constructor.
        /*!
            \param size Number of elements this array will be resized to.
            \param val Default value for all newly constructed elements.
        */
		MashArray(uint32 size, const T &val = T()):m_data(0), m_reservedElements(0), m_currentElements(0)
		{
			Resize(size, val);
		}

        //! Constructor.
        /*!
            \param size Number of elements this array will be resized to.
            \param val Default value for all newly constructed elements.
            \param alloc Allocator.
         */
		MashArray(uint32 size, const T &val, const TAlloc &alloc):m_memoryPool(alloc), 
			m_data(0), m_reservedElements(0), m_currentElements(0)
		{
			Resize(size, val);
		}

		~MashArray()
		{
			DeleteData();
		}

        //! Deleted all internal allocated data.
		void DeleteData()
		{
			if (m_data)
			{
				//call destructors
				if (!helpers::MashIsFundamental<T>::value)
				{
					for(uint32 i = 0; i < m_currentElements; ++i)
						MashMemoryAllocatorHelper<T>::Destruct(&m_data[i]);
				}

				m_memoryPool.FreeMemory(m_data);
				m_data = 0;
			}

			m_reservedElements = 0;
			m_currentElements = 0;
		}

        //! Returns true if this array contains the given value.
		bool Contains(const T &val)
		{
			if (Search(val) != End())
				return true;

			return false;
		}
        
        //! Performs a binary search for the given value.
        /*!
            Uses the template arguments < comparison for searching.
         
            \return Found iterator or End() if not found.
        */
        Iterator Search(const T &val)
		{
			return SearchPred< mash::algorithms::sLess<T> >(val,  mash::algorithms::sLess<T>());
		}

        //! Performs a binary search for the given value.
        /*!
            \param val Item to search for.
            \param pred Predicate that overloads the () operator to perform a "less than" comparison.
            \return Found iterator or End() if not found.
         */
		template<class TPred>
		Iterator SearchPred(const T &val, const TPred &pred)
		{
			return mash::algorithms::BinarySearchPred<Iterator, T, TPred, mash::algorithms::sEquals<T> >(Begin(), End(), val, pred, mash::algorithms::sEquals<T>());
		}

        //! Sorts the elements in this array.
        /*!
            Sorting is done using an items < operator.
        */
		void Sort()
		{
			std::sort(m_data, m_data + m_currentElements);
		}

        //! Sorts the elements in this array.
        /*!
            \param pred Predicate that overloads the () operator to perform a "less than" comparison.
         */
		template<class TPred>
		void Sort(const TPred &pred)
		{
			std::sort(m_data, m_data + m_currentElements, pred);
		}

        //! Clears this array and assigns new values to this container.
        /*!
            \param data C array of data.
            \param elementCount Number of elements to copy over from data.
        */
		void Assign(const T *data, uint32 elementCount)
		{
			Clear();
			Append(data, elementCount);
		}

        //! Returns an iterator to the first element in this container.
		ConstIterator Begin()const
		{
			return ConstIterator(m_data);
		}

        //! Returns an iterator to one past the last element in this container.
        /*!
            This iterator can be decremented if Size() > 0.
            This iterator must not be dereferenced.
        */
		ConstIterator End()const
		{
			return ConstIterator(m_data + m_currentElements);
		}

        //! Returns an iterator to the first element in this container.
		Iterator Begin()
		{
			return Iterator(m_data);
		}

        //! Returns an iterator to one past the last element in this container.
        /*!
            This iterator can be decremented if Size() > 0.
            This iterator must not be dereferenced.
         */
		Iterator End()
		{
			return Iterator(m_data + m_currentElements);
		}

        //! Returns the last item in this array.
		T& Back()const
		{
			return m_data[m_currentElements-1];
		}

        //! Returns the first item in this array.
		T& Front()const
		{
			return m_data[0];
		}

        //! Returns the last item in this array.
		T& Back()
		{
			return m_data[m_currentElements-1];
		}

        //! Returns the first item in this array.
		T& Front()
		{
			return m_data[0];
		}

        //! Reserves memory.
        /*!
            This can be used to make adding elements more memory efficient.
            This does not change the Size() of this array.
         
            This will not reduce the number of reserved elements in
            this container. Use DeleteData() or ShinkToFit() intead.
         
            \param elementCount New reserved elements.
        */
		void Reserve(uint32 elementCount)
		{
			if (elementCount < m_reservedElements)
				return;

			_Reserve(elementCount);
		}

        //! Changes the Size() of this array.
        /*!
            This will not reduce the number of reserved elements in
            this container. Use DeleteData() or ShinkToFit() instead.
         
            If the new number of elements is less than the current value
            then those extra elements will have their destructors called.
            Char types are optimised to use memset and set to 0.
         
            \param elementCount New Size().
            \param val New elements will be initialised to this value.
        */
		void Resize(uint32 elementCount, const T &val = T())
		{
			if (elementCount == m_currentElements)
				return;

			if (elementCount > m_reservedElements)
			{
				_Reserve(elementCount);
			
				//construct new elements
				if (helpers::MashIsInt8<T>::value || helpers::MashIsUInt8<T>::value)
				{
					//use memset on char types
					memset(&m_data[m_currentElements], 0, (elementCount - m_currentElements));
				}
				else
				{
					for(uint32 i = m_currentElements; i < elementCount; ++i)
						MashMemoryAllocatorHelper<T>::Construct(&m_data[i], val);
				}
			}
			else
			//if there are remaining elements then call their destructors
			{
				//don't call fundamental destructors 
				if (!helpers::MashIsFundamental<T>::value)
				{
					for(uint32 i = elementCount; i < m_currentElements; ++i)
						MashMemoryAllocatorHelper<T>::Destruct(&m_data[i]);
				}
			}

			m_currentElements = elementCount;
		}

        //! Inserts new elements infront of the given location.
        /*!
            \param location Location of the new elements.
            \param item C array of items to insert.
            \param count Number of items to add to this container.
            \return Iterator to the first inserted item.
        */
		Iterator Insert(uint32 location, const T *item, uint32 count)
		{
			if (count == 0)
				return End();

			//resize the buffer if necessary
			const uint32 newSize = count + m_currentElements;
			if (newSize > m_reservedElements)
				_Reserve(newSize * 2);

			if (m_currentElements > 0)
			{
				int32 loc = location;
				for(int32 i = m_currentElements-1; i >= loc; --i)
				{
					MashMemoryAllocatorHelper<T>::Construct(&m_data[i+count], m_data[i]);
					MashMemoryAllocatorHelper<T>::Destruct(&m_data[i]);
				}
			}

			for(uint32 i = 0; i < count; ++i)
			{
				MashMemoryAllocatorHelper<T>::Construct(&m_data[location + i], item[i]);
			}

			m_currentElements += count;

			return Iterator(m_data + location);
		}

        //! Inserts a new element infront of the given location.
        /*!
            \param location Location of the new elements.
            \param item Item to insert.
            \return Iterator to the first inserted item.
         */ 
		Iterator Insert(uint32 location, const T &item)
		{
			return Insert(location, &item, 1);
		}

        //! Inserts a new element infront of the given location.
        /*!
            \param location Location of the new elements.
            \param item Item to insert.
            \return Iterator to the first inserted item.
         */ 
		Iterator Insert(const Iterator &location, const T &item)
		{
			return Insert(location.item - m_data, &item, 1);
		}

        //! Inserts new elements infront of the given location.
        /*!
            \param location Location of the new elements.
            \param first Iterator to the first item to insert.
            \param last Iterator to the last item to insert. This must be one past the last element.
            \return Iterator to the first inserted item.
         */
		Iterator Insert(const Iterator &location, const Iterator &first, const Iterator &last)
		{
			return Insert(location.item - m_data, first.item, last.item - first.item);
		}
        
        //! Inserts new elements infront of the given location.
        /*!
            \param location Location of the new elements.
            \param item C array of items to insert.
            \param count Number of items to add to this container.
            \return Iterator to the first inserted item.
         */
        Iterator Insert(const Iterator &location, const T *item, uint32 count)
		{
			return Insert(location.item - m_data, item, count);
		}

        //! Adds a new item to the end of this array.
		void PushBack(const T &item)
		{
			Append(&item, 1);
		}
        
        //! Adds a new item to the front of this array.
        /*!
            Note, this is not an efficient operation for arrays.
        */
        void PushFront(const T &item)
        {
            Insert(Begin(), item);
        }

        //! Appends new data to the end of this array.
        /*!
            \param data C array of items to append.
            \param elementCount Number of elements to append from data.
        */
		void Append(const T *data, uint32 elementCount)
		{
			//resize the buffer if necessary
			const uint32 newSize = elementCount + m_currentElements;
			if (newSize > m_reservedElements)
				_Reserve(newSize * 2);

			if (!helpers::MashIsFundamental<T>::value)
			{
				for(uint32 i = 0; i < elementCount; ++i)
				{
					MashMemoryAllocatorHelper<T>::Construct(&m_data[m_currentElements + i], data[i]);
				}
			}
			else
			{
				memcpy(&(m_data[m_currentElements]), data, sizeof(T) * elementCount);
			}

			m_currentElements += elementCount;
		}

        //! Removes an item from this array and reduces its size by one. 
		Iterator Erase(const Iterator &location)
		{
			return Erase(location.item - m_data);
		}

        //! Removes an item from this array and reduces its size by one.
		Iterator Erase(uint32 index)
		{
            MASH_ASSERT(index < m_currentElements);
            
			if (m_currentElements > 0)
			{
				if (!helpers::MashIsFundamental<T>::value)
				{
					for(uint32 i = index+1; i < m_currentElements; ++i)
					{
						MashMemoryAllocatorHelper<T>::Destruct(&m_data[i-1]);
						MashMemoryAllocatorHelper<T>::Construct(&m_data[i-1], m_data[i]);
					}

					//destruct the last element
					MashMemoryAllocatorHelper<T>::Destruct(&m_data[m_currentElements-1]);
				}
				else
				{
					for(uint32 i = index+1; i < m_currentElements; ++i)
						m_data[i-1] = m_data[i];
				}
				
				--m_currentElements;
			}

			if (index >= m_currentElements)
				return End();
			else
				return (Begin() + index);
		}

        //! Removes the last item and reduces this arrays size by one.
        /*!
            Size() must be > 0.
        */
		void PopBack()
		{
			MASH_ASSERT(m_currentElements > 0);
            
            Erase(m_currentElements - 1);
		}
        
        //! Removes the first item and reduces this arrays size by one.
        /*!
            Size() must be > 0.
         */
        void PopFront()
		{
            Erase(0);
		}

        //! Clears this array and copies data from another array. 
		void operator=(const MashArray<T, TAlloc> &other)
		{
			if (&other == this)
				return;

			Clear();
			Append(other.Pointer(), other.Size());
		}

        //! Appends an item to this array.
		MashArray<T, TAlloc>& operator+=(const T &other)
		{
			Append(&other, 1);
			return *this;
		}

        //! Appends an array to the end of this array.
		MashArray<T, TAlloc>& operator+=(const MashArray<T, TAlloc> &other)
		{
			Append(other.Pointer(), other.Size());
			return *this;
		}

        //! Returns an item at the given location.
        /*!
            index must be less than ReservedSize().
        */
		const T& operator[](uint32 index)const
		{
			return m_data[index];
		}

        //! Returns an item at the given location.
        /*!
            index must be less than ReservedSize().
         */
		T& operator[](uint32 index)
		{
			return m_data[index];
		}

        //! Returns true if Size() is zero.
		bool Empty()const
		{
			return (m_currentElements == 0);
		}

        //! Clears and destructs all elements, but doesn't destroy any reserved memory.
		void Clear()
		{
			//call destructors
			if (!helpers::MashIsFundamental<T>::value)
			{
				for(uint32 i = 0; i < m_currentElements; ++i)
					MashMemoryAllocatorHelper<T>::Destruct(&m_data[i]);
			}

			m_currentElements = 0;
		}

        //! Shrinks the reserved memory to fit Size().
		void ShrinkToFit()
		{
			if (m_reservedElements != m_currentElements)
			{
				if (m_currentElements > 0)
					_Reserve(m_currentElements);
				else if (m_data)
				{
					m_memoryPool.FreeMemory(m_data);
					m_data = 0;
					m_reservedElements = 0;
				}
			}
		}

        //! Returns a raw pointer to the first element in this array.
		T* Pointer()
		{
			return m_data;
		}
        
        //! Returns a raw pointer to the first element in this array.
        const T* Pointer()const
		{
			return m_data;
		}

        //! Returns the number of used elements in this array.
		uint32 Size()const
		{
			return m_currentElements;
		}
        
        //! Returns the number of reserved elements in this array.
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
}

#endif