//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_MEMORY_MANAGER_H_
#define _MASH_MEMORY_MANAGER_H_

#include "MashDataTypes.h"
#include "MashMemory.h"

namespace mash
{
	class MashMemoryAllocator;

    /*!
        This is a singleton class and is only valid after the device has
        been created. It should be accesed from MashMemoryManager::Instance.
        The functions here don't normally need to be accessed directly. Instead, 
        macros can be used from MashMemory.h.
     
        All dynamic memory allocations should pass through here. The manager will
        log debug information about memory allocations so that memory leaks can
        be traced. Then it passes the actual memory allocation on to the custom
        allocator that is created by the user.
    */
	class MashMemoryManager
	{
	private:
		friend class MashMemoryAllocatorHelper<MashMemoryManager>;
		static MashMemoryManager *m_memoryManager;
		static MashMemoryAllocator *m_allocator;
        
        //! Logs memory (de)allocations.
        /*!
            \param p Pointer to the memory allocated.
            \param size Size in bytes of the memory allocated.
            \param file File where the allocation was made.
            \param line Line the allocation was made.
            \param func Function where the allcation was made.
        */
		void LogAllocation(void *p, int32 size, const int8 *file, int32 line, const int8 *func);
        
        //! Logs a deallocation.
        /*!
            \param p Pointer to the memory allocated.
        */
		void LogDeallocation(void *p);
	protected:
		MashMemoryManager();
		~MashMemoryManager();
	public:
		
        //! Allocate a block of dynamic memory.
		/*
            Passes the allocation onto the allocator and logs an allocation.
         
            \param size Size in bytes of the memory to allocate.
            \param file File where the allocation was made.
            \param line Line the allocation was made.
            \param func Function where the allcation was made.
            \param alignment Requested byte alignment for the new block of memory.
            \param memCategory Hint as to how the memory will be used.
            \return Pointer to the allocated block. NULL if there was an error.
		*/
		void* Allocate(size_t size, const int8 *file, int32 line, const int8 *func, size_t alignment, eMEMORY_CATEGORY memCategory);	
		
        //! Allocate a block of dynamic memory.
        /*!
            Passes the allocation onto the allocator. This function will still log the allocation
            if it's enabled.
         
            \param alignment Requested byte alignment for the new block of memory.
            \param memCategory Hint as to how the memory will be used.
            \return Pointer to the allocated block. NULL if there was an error.
        */
        void* Allocate(size_t size, size_t alignment, eMEMORY_CATEGORY memCategory);
        
        //! Deallocates a block of memory.
        /*!
            This will also log the deallocation if it's enabled.
         
            \param ptr Block of memory to deallocate.
        */
		void Deallocate(void *ptr);

        //! The memory manager should be accessed through here.
        /*!
            \return Memory manager instance.
        */
		static MashMemoryManager* Instance();
        
        //! Destroys the instance.
        /*!
            Used internally. Called when the engine id destroyed.
        */
		static void DestroyInstance();

		MashMemoryAllocator* GetAllocator()const;
	};	

	inline MashMemoryAllocator* MashMemoryManager::GetAllocator()const
	{
		return m_allocator;
	}

	_MASH_EXPORT MashMemoryAllocator* CreateMemoryAllocator();
}

#endif