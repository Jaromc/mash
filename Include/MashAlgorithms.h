//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_ALGORITHMS_H_
#define _MASH_ALGORITHMS_H_

#include "MashDataTypes.h"

namespace mash
{
	namespace algorithms
	{
		template<class T>
		struct sLess
		{
			bool operator()(const T &lhs, const T &rhs)const
			{
				return (lhs < rhs);
			}
		};
        
        template<class T>
		struct sEquals
		{
			bool operator()(const T &lhs, const T &rhs)const
			{
				return (lhs == rhs);
			}
		};

		template<class T>
		struct sNot
		{
			T operator()(const T &val)const
			{
				return !val;
			}
		};

		template<class T>
		struct sPassThrough
		{
			const T& operator()(const T &val)const
			{
				return val;
			}
		};

		template<class TIn, class TOut>
		struct sGetObjectData
		{
			const TOut& operator()(const TIn &val)const
			{
				return val;
			}
		};

        //! This returns an item that is LESS then OR EQUAL to the value.
		/*!
			TPred is a struct that has the () operator to do a less than comparison. Eg,
            \code
            template<class T>
            struct sLess
            {
                bool operator()(const T &lhs, const T &rhs)const
                {
                    return (lhs < rhs);
                }
            };
            \endcode
         
			A default implementation of TPredFlip is sPassThrough<T>. This is used to manipulate
            the boolean value that is returned from the comparison. This is used so that only
            the < operator needs to be implemented (and not > as well) by the user and the algorithms can
            manipulate the result based on what it needs. Eg,
            \code
            template<class T>
            struct sPassThrough
            {
                const T& operator()(const T &val)const
                {
                    return val;
                }
            };
            \endcode
         
            \param start Position to start searching.
            \param end One past the last element in the search.
            \param val Element to search for.
            \param pred less than comparison.
            \param predFlip Manipulates the result of pred.
            \return Iterator to the lower bound.
		*/
		template<class Iterator, class T, class TPred, class TPredFlip >
		Iterator LowerBoundPred(const Iterator &start, const Iterator &end, const T &val, const TPred &pred, const TPredFlip &predFlip)
		{
			Iterator iter;
			Iterator front(start);
			int32 distance = start.Distance(end);

			while(distance > 0)
			{
				iter = front;
				uint32 mid = distance / 2;
				iter += mid;

				if (predFlip(pred(*iter, val)))
				{
					//continue searching the upper bound
					front = ++iter;
					distance -= mid+1;
				}
				else
				{
					//the lower value lies in the lower bound
					distance = mid;
				}
			}

			return front;
		}
        
        //! This returns an item that is GREATER to the value.
		/*!
            TPred is a struct that has the () operator to do a less than comparison. Eg,
            \code
            template<class T>
            struct sLess
            {
                bool operator()(const T &lhs, const T &rhs)const
                {
                    return (lhs < rhs);
                }
            };
            \endcode

            \param start Position to start searching.
            \param end One past the last element in the search.
            \param val Element to search for.
            \param pred less than comparison.
            \return Iterator to the upper bound.
         */
		template<class Iterator, class T, class TPred>
		Iterator UpperBoundPred(const Iterator &start, const Iterator &end, const T &val, const TPred &pred)
		{
			//uses sNot to flip the comparison. 
			return LowerBoundPred<Iterator, T, TPred, sNot<bool> >(start, end, val, pred, sNot<bool>());
		}

        //! Searches a range of values for an item matching the given value. Best used for random access iterators.
        /*!
            \param start Position to start searching.
            \param end One past the last element in the elements to search.
            \param val Element to search for.
            \param lessThanPred less than comparison. Eg, sLess.
            \param equalsToPred equals to comparison. Eg, sEquals.
            \return Iterator to the matching element or end if it wasn't found.
         */
		template<class Iterator, class T, class TLessThanPred, class TEqualsToPred>
		Iterator BinarySearchPred(const Iterator &start, 
                                  const Iterator &end, 
                                  const T &val, 
                                  const TLessThanPred &lessThanPred, 
                                  const TEqualsToPred &equalsToPred)
		{
			Iterator iter = LowerBoundPred<Iterator, T, TLessThanPred, sPassThrough<bool> >(start, end, val, lessThanPred, sPassThrough<bool>());

			if (iter == end)
				return end;

			if (!equalsToPred(val, *iter))
				return end;

			return iter;
		}

		/*
			Do not use directly. Instead call MergeSort().
		*/
		template<class TNode, class TPred, class TGetData>
		TNode* _Merge(TNode *a, TNode *b, TNode **outEndHint, const TPred &pred, const TGetData &getData)
		{
			TNode *newStart = 0;
			TNode *next = 0;
			while((a != 0) && (b != 0))
			{
				TNode **i = pred(getData(a), getData(b)) ? &a : &b;

				if (!newStart)
				{
					newStart = *i;
					next = *i;
					next->previous = 0;
				}
				else
				{
					next->next = *i;
					(*i)->previous = next;

					next = next->next;
				}

				*i = (*i)->next;
			}

			next->next = a != 0 ? a : b;
			if (next->next)
			{
				next->next->previous = next;
			}

			*outEndHint = next->next;
			return newStart;
		}

        //! Generic merge sort for linked lists.
		/*!
			This does not use iterators. Instead it takes any bidirectional object with "next" and "previous"
			pointers linking the list.

			TPred must be a binary predicate (less than operator) returning true or false based on some comparision. Eg, Less than.
         
			Returns the new start of the list.
         
            outEndHint can be used as a close approximation to the end node so you don't need to search
            through the entire list looking for the new end. It could be used as follows:
            \code
                 sItem *endHint = 0;
                 sItem *start = MergeSort(..., &endHint ,...);
                 
                 while(endHint->next != 0)
                    endHint = endHint->next;
         
                sItem *end = endHint;
            \endcode
         
            TGetData returns the internal data from within a TNode.
            \code
            struct sGetObjectData
            {
                const SomeData& operator()(const sItem *lhs)const
                {
                    return lhs->data;
                }
            };   
            \endcode
         
            \param start First node in the list. "previous" must be null.
            \param outEndHint Gives a close approximation to the end node.
            \param pred Less than comparison. Eg, sLess.
            \param getData Returns the internal value from a TNode. Eg, sGetObjectData.
		*/
		template<class TNode, class TPred, class TGetData>
		TNode* MergeSort(TNode *start, TNode **outEndHint, const TPred &pred, const TGetData &getData)
		{
			if (start->next == 0)
				return start;

			TNode *e = start;
			TNode *mid = start;

			while(1)
			{
				e = e->next;
				if (e->next == 0)
					break;

				e = e->next;
				if (e->next == 0)
					break;

				mid = mid->next;
			}

			e = mid->next;
			mid->next = 0;

			return _Merge(MergeSort(start, outEndHint, pred, getData), MergeSort(e, outEndHint, pred, getData), outEndHint, pred, getData);
		}
	}
}

#endif