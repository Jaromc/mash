//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_VERTEX_BUFFER_H_
#define _MASH_VERTEX_BUFFER_H_

#include "MashResource.h"

namespace mash
{
	class MashVertex;

    /*!
        Vertex buffers are used as streams for sending data to the GPU. They are mainly
        used for mesh geometry but can be filled with any data needed on the GPU.
    */
	class MashVertexBuffer : public MashResource
	{
	public:
		MashVertexBuffer():MashResource(){}

		virtual ~MashVertexBuffer(){}

        //! Copies data from this buffer into a new buffer.
		virtual MashVertexBuffer* Clone()const = 0;
        
        //! Copies data from one buffer into this buffer.
        /*!
            \param from Data to copy from.
            \return Ok on success, failed otherwise.
        */
		virtual eMASH_STATUS Copy(const MashVertexBuffer *from) = 0;
        
        //! Resizes this buffer.
        /*!
            If this buffer is apart of a mesh buffer then consider using the resize methods
            in MashMeshBuffer.
         
            \param newSize New size of this buffer in bytes.
            \param saveData Set to true if the current data should be copied over.
            \return Ok on success, failed otherwise.
        */
		virtual eMASH_STATUS Resize(uint32 newSize, bool saveData = false) = 0;

        //! Gets the current size of this buffer in bytes.
		virtual uint32 GetBufferSize()const = 0;

        //! Locks this buffer for reading or writing.
        /*!
            This is only valid for dynamic buffers. Be sure to call Unlock() when
            done with the buffer.
         
            \param type Lock type.
            \param data Returns a pointer to the buffers data.
            \return Ok on success, failed otherwise.
        */
		virtual eMASH_STATUS Lock(eBUFFER_LOCK type, void **data)const = 0;
        
        //! Unlocks this buffer for rendering.
        /*!
            Called after calling Lock().
         
            \return Ok on success, failed otherwise.
        */
		virtual eMASH_STATUS Unlock()const = 0;
	};
}

#endif