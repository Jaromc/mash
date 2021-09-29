//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _CMASH_STL_MAP_ALLOCATOR_H_
#define _CMASH_STL_MAP_ALLOCATOR_H_

#include <map>
#include "MashMemoryPool.h"

namespace mash
{
	template<class _Ty, class TPoolType>
	class CMashSTLMapAllocator
		{
	public:
        typedef _Ty value_type;
		typedef value_type *pointer;
		typedef value_type &reference;
		typedef const value_type *const_pointer;
		typedef const value_type &const_reference;

		typedef size_t size_type;
		typedef ptrdiff_t difference_type;

		TPoolType *m_mashPool;

		template<class _Other>
			struct rebind
			{	// convert an allocator<_Ty> to an allocator <_Other>
			typedef CMashSTLMapAllocator<_Other, TPoolType> other;
			};

		pointer address(reference _Val) const
			{	// return address of mutable _Val
			return (&_Val);
			}

		const_pointer address(const_reference _Val) const
			{	// return address of nonmutable _Val
			return (&_Val);
			}

		CMashSTLMapAllocator(TPoolType *mashPool):m_mashPool(mashPool)
			{	// construct default allocator (do nothing)
			}

		CMashSTLMapAllocator(const CMashSTLMapAllocator<_Ty, TPoolType>&c):m_mashPool(c.m_mashPool)
			{	// construct by copying (do nothing)
			}

		template<class _Other>
			CMashSTLMapAllocator(const CMashSTLMapAllocator<_Other, TPoolType>&c):m_mashPool(c.m_mashPool)
			{	// construct from a related allocator (do nothing)
			}

		template<class _Other>
			CMashSTLMapAllocator<_Ty, TPoolType>& operator=(const CMashSTLMapAllocator<_Other, TPoolType>&c)
			{	// assign from a related allocator (do nothing)
                m_mashPool = c.m_mashPool;
                return (*this);
			}

		void deallocate(pointer _Ptr, size_type)
			{	// deallocate object at _Ptr, ignore size
			//::operator delete(_Ptr);
				m_mashPool->FreeMemory(_Ptr);
			}

		pointer allocate(size_type _Count)
			{	// allocate array of _Count elements
			//return (_Allocate(_Count, (pointer)0));
				return (pointer)m_mashPool->GetMemory(sizeof(_Ty) * _Count);
			}

		pointer allocate(size_type _Count, const void *)
			{	// allocate array of _Count elements, ignore hint
			//return (allocate(_Count));
				return (pointer)m_mashPool->GetMemory(sizeof(_Ty) * _Count);
			}

		void construct(pointer _Ptr, const _Ty& _Val)
			{	// construct object at _Ptr with value _Val
			//_Construct(_Ptr, _Val);
				MashMemoryAllocatorHelper<_Ty>::Construct(_Ptr, _Val);
			}

		void destroy(pointer _Ptr)
			{	// destroy object at _Ptr
			//_Destroy(_Ptr);
				MashMemoryAllocatorHelper<_Ty>::Destruct(_Ptr);
			}

		size_type max_size() const
			{	// estimate maximum array size
			size_type _Count = (size_type)(-1) / sizeof (_Ty);
			return (0 < _Count ? _Count : 1);
			}
		};

			// allocator TEMPLATE OPERATORS
	template<class _Ty,
		class _Other,
		class _AllocTy,
		class _AllocOther> inline
		bool operator==(const CMashSTLMapAllocator<_Ty, _AllocTy>&, const CMashSTLMapAllocator<_Other, _AllocOther>&)
		{	// test for allocator equality (always true)
		return (true);
		}

	template<class _Ty,
		class _Other,
		class _AllocTy,
		class _AllocOther> inline
		bool operator!=(const CMashSTLMapAllocator<_Ty, _AllocTy>&, const CMashSTLMapAllocator<_Other, _AllocOther>&)
		{	// test for allocator inequality (always false)
		return (false);
		}
}

#endif