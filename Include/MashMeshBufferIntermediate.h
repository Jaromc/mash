//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_MESH_BUFFER_INTERMEDIATE_H_
#define _MASH_MESH_BUFFER_INTERMEDIATE_H_

#include "MashMeshBuffer.h"
#include "MashArray.h"

namespace mash
{
	class MashVideo;

    /*!
        Intermediate class for API data.
        This class shouldn't be accessed directly. Instead use MashMeshBuffer.
    */
	class MashMeshBufferIntermediate : public MashMeshBuffer
	{
	protected:
		MashVideo *m_renderer;
		MashArray<MashVertexBuffer*> m_vertexBuffers;
		MashIndexBuffer *m_indexBuffer;
		MashVertex *m_vertexDeclaration;
	protected:
		MashMeshBuffer* CloneMembers(MashMeshBufferIntermediate *from);
	public:
		MashMeshBufferIntermediate(MashVideo *renderer,
			MashArray<MashVertexBuffer*> &vertexBuffers, 
			MashIndexBuffer *indexBuffer,
			MashVertex *vertexDeclaration);
        
        MashMeshBufferIntermediate(MashVideo *renderer);

		virtual ~MashMeshBufferIntermediate();

		uint32 GetVertexBufferCount()const;
		MashVertexBuffer** GetVertexBufferArray()const;
		MashVertexBuffer* GetVertexBuffer(uint32 stream = 0)const;
		MashIndexBuffer* GetIndexBuffer()const;
		MashVertex* GetVertexDeclaration()const;

		virtual eMASH_STATUS ResizeVertexBuffers(uint32 bufferIndex, uint32 bufferSize, eUSAGE usage, bool saveData = false);
		virtual eMASH_STATUS ResizeIndexBuffer(uint32 bufferSize, bool saveData = false);
		virtual void _SetVertexDeclaration(MashVertex *vertex);
	};

	inline MashVertex* MashMeshBufferIntermediate::GetVertexDeclaration()const
	{
		return m_vertexDeclaration;
	}

	inline MashIndexBuffer* MashMeshBufferIntermediate::GetIndexBuffer()const
	{
		return m_indexBuffer;
	}

	inline uint32 MashMeshBufferIntermediate::GetVertexBufferCount()const
	{
		return m_vertexBuffers.Size();
	}
}

#endif