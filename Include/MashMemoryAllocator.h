//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_MEMORY_ALLOCATOR_H_
#define _MASH_MEMORY_ALLOCATOR_H_

#include "MashMemoryTypes.h"

namespace mash
{
    /*!
        Used for allocating dynamic memory. The user must provide one of
        these at load time.

		Custom allocators can be created for memory pools or specialised operations.
     
        The memory categories can be used to determine which memory pool the memory
        should be allocated from. These don't have to be considered though, a simple
        implimentation would simply allocate all memory from the one pool.
     
        The allocate and deallocate functions are not normally acessed directly from
        here, instead go through the memory manager. The memory manager also logs
        memory debug info, so this class does not need to impliment that.
    */
	class MashMemoryAllocator
	{
	public:
		MashMemoryAllocator(){}
		virtual ~MashMemoryAllocator(){}

        //! Returns dynamic memory.
        /*!
            \param size Memory size to allocate in bytes.
            \param alignment Requested memory byte alignment.
            \param memCategory Hint as to how the memory will be used.
        */
		virtual void* Allocate(size_t size, size_t alignment, eMEMORY_CATEGORY memCategory) = 0;
        
        //! Frees a block of memory.
        /*!
            \param ptr Memory block to free.
        */
		virtual void Deallocate(void *ptr) = 0;

        //! This is called by the engine when the engine is being destroyed.
		/*!
			You can destroy the allocator at this point.
		*/
		virtual void Destroy() = 0;
	};
}

#endif