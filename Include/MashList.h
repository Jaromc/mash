//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_LIST_H_
#define _MASH_LIST_H_

#include "MashDataTypes.h"
#include "MashMemory.h"
#include "MashMemoryPoolSimple.h"
#include "MashAlgorithms.h"
#include "MashHelper.h"

namespace mash
{
    /*!
        Templated doubly linked list.
    */
	template<class T, class TAlloc = MashMemoryPoolSimple, bool TSaveErasedNodes = false>
	class MashList
	{
	public:
		struct sItem
		{
			T data;

			sItem *previous;
			sItem *next;

			sItem():previous(0), next(0){}
		};

		struct sGetObjectData
		{
			const T& operator()(const sItem *lhs)const
			{
				return lhs->data;
			}
		};

		class ConstIterator;

		class Iterator
		{
			friend class MashList<T, TAlloc, TSaveErasedNodes>;
		private:
			sItem *item;
		public:
			Iterator():item(0){}
			Iterator(sItem *_item):item(_item){}
			Iterator(const Iterator &other):item(other.item){}
			~Iterator(){}

			uint32 Distance(const Iterator &end)const
			{
				uint32 count = 0;
				Iterator iter = *this;
				while(iter != end)
				{
					++iter;
					++count;
				}

				return count;
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
				if (val > 0)
				{
					while(val-- > 0)
						item = item->next;
				}
				else
				{
					while(val++ < 0)
						item = item->previous;
				}

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
				item = item->next;
				return *this;
			}

			Iterator& operator--()
			{
				item = item->previous;
				return *this;
			}

			Iterator operator++(int32 val)
			{
				Iterator t = *this;
				item = item->next;
				return t;
			}

			Iterator operator--(int32 val)
			{
				Iterator t = *this;
				item = item->previous;
				return t;
			}

			T& operator*()
			{
				return item->data;
			}

			T* operator->()
			{
				return &item->data;
			}
		};

		class ConstIterator
		{
			friend class MashList<T, TAlloc, TSaveErasedNodes>;
		private:
			sItem *item;
		public:
			ConstIterator():item(0){}
			ConstIterator(sItem *_item):item(_item){}
			ConstIterator(const ConstIterator &other):item(other.item){}
			~ConstIterator(){}

			uint32 Distance(const Iterator &end)const
			{
				uint32 count = 0;
				Iterator iter = *this;
				while(iter != end)
				{
					++iter;
					++count;
				}

				return count;
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

			ConstIterator& operator=(const Iterator &other)
			{
				item = other.item;
				return *this;
			}

			ConstIterator& operator+=(int32 val)
			{
				if (val > 0)
				{
					while(val-- > 0)
						item = item->next;
				}
				else
				{
					while(val++ < 0)
						item = item->previous;
				}

				return *this;
			}

			ConstIterator& operator-=(int32 val)
			{
				return ((*this) += -val);
			}

			ConstIterator operator+(int32 val)const
			{
				Iterator iter(*this);
				iter += val;
				return iter;
			}

			ConstIterator operator-(int32 val)const
			{
				Iterator iter(*this);
				iter -= val;
				return iter;
			}

			ConstIterator& operator++()
			{
				item = item->next;
				return *this;
			}

			ConstIterator& operator--()
			{
				item = item->previous;
				return *this;
			}

			ConstIterator operator++(int32 val)
			{
				ConstIterator t = *this;
				item = item->next;
				return t;
			}

			ConstIterator operator--(int32 val)
			{
				ConstIterator t = *this;
				item = item->previous;
				return t;
			}

			T& operator*()
			{
				return item->data;
			}

			T* operator->()
			{
				return &item->data;
			}
		};
	private:
		TAlloc m_memoryPool;
		sItem *m_listStart;
		sItem m_end;
		sItem *m_freeList;
		uint32 m_currentNodeCount;

		sItem* _GetEmptyNode()
		{
			sItem *emptyNode = 0;

			//reuse old elements if any are avaliable
			if (m_freeList)
			{
				emptyNode = m_freeList;
				m_freeList = m_freeList->next;
				if (m_freeList)
					m_freeList->previous = 0;
			}
			else
			{
				//new element needed
				emptyNode = (sItem*)m_memoryPool.GetMemory(sizeof(sItem));
			}

			return emptyNode;
		}

	public:
		MashList():m_listStart(0), m_freeList(0),
			m_currentNodeCount(0)
		{
			m_listStart = &m_end;
		}

        //! Copy constructor.
		MashList(const MashList<T, TAlloc> &other):m_memoryPool(other.m_memoryPool),
			m_listStart(0), m_freeList(0),m_currentNodeCount(0)
		{
			m_listStart = &m_end;
			*this = other;
		}

        //! Constructor.
        /*!
            \param alloc Allocator.
        */
		MashList(const TAlloc &alloc):m_memoryPool(alloc),
			m_listStart(0), m_freeList(0),m_currentNodeCount(0)
		{
			m_listStart = &m_end;
		}

		~MashList()
		{
			DeleteData();
		}

        //! Assignment operator.
		void operator=(const MashList<T, TAlloc> &other)
		{
			if (&other == this)
				return;

			DeleteData();

			ConstIterator iterBegin = other.Begin();
			ConstIterator iterEnd = other.End();
			for(; iterBegin != iterEnd; ++iterBegin)
				PushBack(*iterBegin);
		}

        //! Returns true if the given item is found within this list.
        /*!
            Just performs a linear search so this maybe slow.
        */
		bool Contains(const T &val)
		{
			if (Search(val) != End())
				return true;

			return false;
		}

        //! Searches this list for the given item.
		/*
			Just performs a linear search so this maybe slow.
         
            \return Iterator to the found item. End() if the item was not found.
		*/
		Iterator Search(const T &val)
		{
			Iterator begin = Begin();
			Iterator end = End();
			for(; begin != end; ++begin)
			{
				if (*begin == val)
					return begin;
			}

			return end;
		}

        //! Sorts the items in this list using the < operator.
		void Sort()
		{            
            Sort<mash::algorithms::sLess<T> >(mash::algorithms::sLess<T>());
		}

		//! Sorts the items in this list.
        /*!
            \param pred Predicate must use the () operator and do a < comparison.
        */
		template<class Pred>
		void Sort(const Pred &pred)
		{
			if (m_currentNodeCount < 2)
				return;

			m_end.previous->next = 0;
			sItem *endHint = 0;
			m_listStart = mash::algorithms::MergeSort<sItem, Pred, sGetObjectData>(m_listStart, &endHint, pred, sGetObjectData());

			while(endHint->next != 0)
				endHint = endHint->next;

			m_end.previous = endHint;	
			endHint->next = &m_end;
		}

        //! Returns an iterator to the first item in the list.
		ConstIterator Begin()const
		{
			return ConstIterator(m_listStart);
		}

        //! Returns an iterator to one past the last item in the list.
        /*!
            This iterator must not be dereferenced.
            This iterator can be decremented as long as Size() > 0.
        */
		ConstIterator End()const
		{
			return ConstIterator((sItem*)&m_end);
		}
        
        //! Returns an iterator to the last element in the list.
        ConstIterator BackIterator()const
		{
			return ConstIterator(m_end.previous);
		}

        //! Returns an iterator to the first item in the list.
		Iterator Begin()
		{
			return Iterator(m_listStart);
		}

        //! Returns an iterator to one past the last item in the list.
        /*!
            This iterator must not be dereferenced.
            This iterator can be decremented as long as Size() > 0.
         */
		Iterator End()
		{
			return Iterator(&m_end);
		}
        
        //! Returns an iterator to the last element in the list.
        Iterator BackIterator()
		{
			return Iterator(m_end.previous);
		}

        //! Returns a reference to the last item in this list.
		const T& Back()const
		{
			return m_end.previous->data;
		}

        //! Returns a reference to the first item in this list.
		const T& Front()const
		{
			return m_listStart->data;
		}

        //! Returns a reference to the last item in this list.
		T& Back()
		{
			return m_end.previous->data;
		}

        //! Returns a reference to the first item in this list.
		T& Front()
		{
			return m_listStart->data;
		}

        //! Inserts an item infront of the given position.
        /*!
            \param location Position to insert new item.
            \param item Item to insert.
            \return Iterator to inserted item.
        */
		Iterator Insert(const Iterator &location, const T &item = T())
		{
			if (!location.item->previous)
			{
				PushFront(item);
				return Begin();
			}
			else if (location.item == &m_end)
			{
				PushBack(item);
				return BackIterator();
			}
			else
			{
				sItem *newItem = _GetEmptyNode();
				MashMemoryAllocatorHelper<T>::Construct(&newItem->data, item);

				newItem->next = location.item;
				newItem->previous = location.item->previous;

				location.item->previous->next = newItem;
				location.item->previous = newItem;

				++m_currentNodeCount;

				return Iterator(newItem);
			}
		}

        //! Searches the list for an item and removes the first item if found. 
		void Erase(const T &item)
		{
			Iterator begin = Begin();
			Iterator end = End();
			for(; begin != end; ++begin)
			{
				if (*begin == item)
				{
					begin = Erase(begin);
					if (begin == end)
						break;
				}
			}
		}

        //! Removes an item.
        /*!
            \param iter Iterator to an item within this list to remove.
            \return Iterator to the next item in the list.
        */
		Iterator Erase(Iterator &iter)
		{
			MASH_ASSERT(iter != End());

			Iterator iterToReturn = iter.item->next;

			//remove item from list
			if (iter.item->previous)
				iter.item->previous->next = iter.item->next;
			if (iter.item->next)
				iter.item->next->previous = iter.item->previous;

			//validate the start node
			if (iter.item == m_listStart)
			{
				m_listStart = m_listStart->next;
				if (m_listStart == &m_end)
				{
					m_end.previous = 0;
					m_listStart->previous = 0;
				}
			}
			//validate the end
			else if (iter.item == m_end.previous)
			{
				m_end.previous = iter.item->previous;
			}

			//call destructor
			if (!helpers::MashIsFundamental<T>::value)
				MashMemoryAllocatorHelper<T>::Destruct(&(*iter));

			--m_currentNodeCount;

			if (TSaveErasedNodes)
			{
				//add to free list
				if (!m_freeList)
				{
					m_freeList = iter.item;
					m_freeList->previous = 0;
					m_freeList->next = 0;
				}
				else
				{
					//push front
					iter.item->previous = 0;
					iter.item->next = m_freeList;
					m_freeList->previous = iter.item;
					m_freeList = iter.item;
				}
			}
			else
			{
				m_memoryPool.FreeMemory(iter.item);
			}

			return iterToReturn;
		}
        
        //! Remove the first item.
        /*!
            It is safe to call this if the list is empty.
        */
        void PopFront()
		{
			if (m_listStart != &m_end)
            {
                Iterator iter = Begin();
				Erase(iter);
            }
		}
        
        //! Remove the last item.
        /*!
            It is safe to call this if the list is empty.
        */
		void PopBack()
		{
			if (m_listStart != &m_end)
            {
                Iterator iter = BackIterator();
				Erase(iter);
            }
		}

        //! Adds an item to the front of the list.
		void PushFront(const T &item)
		{
			sItem *newItem = _GetEmptyNode();

			MashMemoryAllocatorHelper<T>::Construct(&newItem->data, item);
			newItem->previous = 0;

			if (m_listStart != &m_end)
			{
				m_listStart->previous = newItem;
				newItem->next = m_listStart;
				
				m_listStart = newItem;
			}
			else
			{
				m_listStart = newItem;
				m_listStart->next = &m_end;
				m_end.previous = m_listStart;
			}

			++m_currentNodeCount;
		}

        //! Adds an item to the end of the list.
		void PushBack(const T &item)
		{
			sItem *newItem = _GetEmptyNode();
			newItem->previous = 0;
			newItem->next = 0;
			
			MashMemoryAllocatorHelper<T>::Construct(&newItem->data, item);

			if (m_listStart != &m_end)
			{
				m_end.previous->next = newItem;
				newItem->previous = m_end.previous;
				
				m_end.previous = newItem;
				newItem->next = &m_end;
			}
			else
			{
				m_listStart = newItem;
				m_listStart->next = &m_end;
				m_listStart->previous = 0;
				m_end.previous = m_listStart;
			}

			++m_currentNodeCount;
		}

        //! Removes all items from this list and destroys all memory.
		void DeleteData()
		{
			sItem *currentElement = m_listStart;
			sItem *next = 0;
			while(currentElement && (currentElement != &m_end))
			{
				next = currentElement->next;

				if (!helpers::MashIsFundamental<T>::value)
					MashMemoryAllocatorHelper<T>::Destruct(&currentElement->data);

				m_memoryPool.FreeMemory(currentElement);

				currentElement = next;
			}

			if (m_freeList)
			{
				sItem *currentElement = m_freeList;
				sItem *next = 0;
				while(currentElement)
				{
					next = currentElement->next;
					m_memoryPool.FreeMemory(currentElement);

					currentElement = next;
				}

				m_freeList = 0;
			}

			m_currentNodeCount = 0;

			m_listStart = &m_end;
			m_end.previous = 0;
		}

        //! Removes all items from this list.
        /*!
            Old nodes maybe saved if the template argument TSaveErasedNodes is
            set to true.
        */
		void Clear()
		{
			if (m_currentNodeCount > 0)
			{
				sItem *currentElement = m_listStart;
				sItem *currentFreeElement = m_freeList;
				sItem *next = 0;
				while(currentElement && (currentElement != &m_end))
				{
					next = currentElement->next;

					if (!helpers::MashIsFundamental<T>::value)
						MashMemoryAllocatorHelper<T>::Destruct(&currentElement->data);

					//add erased nodes to the free list
					if (TSaveErasedNodes)
					{
						if (!m_freeList)
						{
							m_freeList = currentElement;
							m_freeList->next = 0;
							m_freeList->previous = 0;
							currentFreeElement = m_freeList;
						}
						else
						{
							currentFreeElement->next = currentElement;
							currentElement->previous = currentFreeElement;
							currentElement->next = 0;
							currentFreeElement = currentElement;
						}
					}
					else
					{
						m_memoryPool.FreeMemory(currentElement);
					}

					currentElement = next;
				}

				m_listStart = &m_end;
				m_end.previous = 0;
				m_currentNodeCount = 0;
			}
		}

        //! Returns true if this list is empty.
		bool Empty()const
		{
			return (m_currentNodeCount == 0);
		}

        //! Returns the number of items in this list.
		uint32 Size()const
		{
			return m_currentNodeCount;
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