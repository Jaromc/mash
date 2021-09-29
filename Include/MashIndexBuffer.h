//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_INDEX_BUFFER_H_
#define _MASH_INDEX_BUFFER_H_

#include "MashResource.h"

namespace mash
{
    /*!
        Index buffers are used to reduce the size of vertex buffers.
    */
	class MashIndexBuffer : public MashResource
	{
	public:
		MashIndexBuffer():MashResource(){}

		virtual ~MashIndexBuffer(){}

		virtual eMASH_STATUS Resize(uint32 newSize, bool saveData = false) = 0;

        //! Creates an independent copy of this buffer.
        /*!
            \return A new index buffer.
        */
		virtual MashIndexBuffer* Clone()const = 0;
        
        //! Copies the data from one buffer to another.
        /*!
            \param from Source data.
            \return Status of this function.
         */
		virtual eMASH_STATUS Copy(const MashIndexBuffer *from) = 0;

        //! Buffer format.
        /*!
            \return Buffer format.
        */
		virtual eFORMAT GetFormat()const = 0;
        
        //! Buffer size.
        /*!
            \return Buffer size in bytes.
         */
		virtual uint32 GetBufferSize()const = 0;

        //! Locks this buffer for data access.
        /*!
            A locked buffer must be unlocked when finished.
         
            \param type Locking type.
            \param data Data held in this buffer. This must note be deleted.
            \return Status of this function.
         */
		virtual eMASH_STATUS Lock(eBUFFER_LOCK type, void **data)const = 0;
        
        //! Unlocks this buffer.
        /*!
            This must be called after a lock.
         
            \return Status of this function.
        */
		virtual eMASH_STATUS Unlock()const = 0;
	};
}

#endif