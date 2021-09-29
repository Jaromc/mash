//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_MESH_BUFFER_H_
#define _MASH_MESH_BUFFER_H_

#include "MashReferenceCounter.h"
#include "MashEnum.h"

namespace mash
{
	class MashVertex;
	class MashIndexBuffer;
	class MashVertexBuffer;

    /*!
        Mesh buffers are compiled API versions of the mesh class. These can be
        created via MashVideo::CreateMeshBuffer().
    */
	class MashMeshBuffer : public MashReferenceCounter
	{
	public:
		MashMeshBuffer():MashReferenceCounter(){}
		virtual ~MashMeshBuffer(){}

        //! Creates an independent copy of this buffer.
        /*!
            \return A new mesh buffer.
        */
		virtual MashMeshBuffer* Clone() = 0;
        
        //! Number of vertex buffers in this mesh.
        /*!
            A mesh buffer may contain many vertex buffers to support multiple streams
            for advanced rendering techniques such instancing.
            \return Vertex buffer count.
        */
		virtual uint32 GetVertexBufferCount()const = 0;
        
        //! Array of vertex buffers.
        /*!
            Returns an array of vertex buffers. Buffers contents can be modified from this function, but not resized.
            \return Array of vertex buffers.
        */
		virtual MashVertexBuffer** GetVertexBufferArray()const = 0;

		//! Single vertex buffer stream.
		/*!
			Buffers contents can be modified from this function, but not resized.

			\param stream Vertex buffer to return.
			\return Vertex buffer.
		*/
		virtual MashVertexBuffer* GetVertexBuffer(uint32 stream = 0)const = 0;
        
        //! Index buffer.
        /*!
            Mesh buffers support multiple vertex buffers, but only 1 index buffer.
            The returned buffer contents may be modified, but not resized.
            \return Index buffer.
        */
		virtual MashIndexBuffer* GetIndexBuffer()const = 0;
        
        //! Vertex declaration.
        /*!
            \return Vertex declaration.
        */
		virtual MashVertex* GetVertexDeclaration()const = 0;

        //! Resize the vertex buffers.
        /*!
			If a vertex buffer is not located at bufferIndex and the vertex declaration stream count
			is greater than or equal to this number then a buffer will be created.

			\param bufferIndex Vertex buffer to resize.
            \param bufferSize New buffer size in bytes.
			\param usage Only used when creating a new buffer
            \param saveData True if the old data should be copied into the new buffers.
            \return Function state.
        */
		virtual eMASH_STATUS ResizeVertexBuffers(uint32 bufferIndex, uint32 bufferSize, eUSAGE usage = aUSAGE_STATIC, bool saveData = false) = 0;
        
        //! Resize the index buffer.
        /*!
            \param bufferSize New buffer size in bytes.
            \param saveData True if the old data should be copied into the new buffer.
            \return Function state.
         */
		virtual eMASH_STATUS ResizeIndexBuffer(uint32 bufferSize, bool saveData = false) = 0;

        //! Sets the vertex declaration.
		/*!
			Used internally.
            
            \param vertex Vertex declaration for this buffer.
		*/
		virtual void _SetVertexDeclaration(MashVertex *vertex) = 0;
	};
}

#endif