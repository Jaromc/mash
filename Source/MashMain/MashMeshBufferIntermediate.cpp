//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "MashMeshBufferIntermediate.h"
#include "MashVideo.h"
#include "MashVertex.h"
#include "MashIndexBuffer.h"
#include "MashVertexBuffer.h"
#include "MashLog.h"

namespace mash
{
	MashMeshBufferIntermediate::MashMeshBufferIntermediate(MashVideo *renderer,
			MashArray<MashVertexBuffer*> &vertexBuffers, 
			MashIndexBuffer *indexBuffer,
			MashVertex *vertexDeclaration):MashMeshBuffer(),m_vertexBuffers(vertexBuffers),
			m_indexBuffer(indexBuffer), m_vertexDeclaration(vertexDeclaration),
			m_renderer(renderer)
	{
		if (vertexDeclaration)
			vertexDeclaration->Grab();
	}
    
    MashMeshBufferIntermediate::MashMeshBufferIntermediate(MashVideo *renderer):MashMeshBuffer(),m_renderer(renderer)
    {
    }

	MashMeshBufferIntermediate::~MashMeshBufferIntermediate()
	{
		MashArray<MashVertexBuffer*>::Iterator vbIter = m_vertexBuffers.Begin();
		MashArray<MashVertexBuffer*>::Iterator vbIterEnd = m_vertexBuffers.End();
		for(; vbIter != vbIterEnd; ++vbIter)
		{
			(*vbIter)->Drop();
		}

		m_vertexBuffers.Clear();

		if (m_indexBuffer)
		{
			m_indexBuffer->Drop();
			m_indexBuffer = 0;
		}

		if (m_vertexDeclaration)
		{
			m_vertexDeclaration->Drop();
			m_vertexDeclaration = 0;
		}
	}

	MashMeshBuffer* MashMeshBufferIntermediate::CloneMembers(MashMeshBufferIntermediate *from)
	{
        if (!from)
            return 0;
        
        MashMeshBufferIntermediate *newBuffer = (MashMeshBufferIntermediate*)m_renderer->_CreateMeshBuffer();
        
		MashArray<MashVertexBuffer*>::Iterator vbIter = from->m_vertexBuffers.Begin();
		MashArray<MashVertexBuffer*>::Iterator vbIterEnd = from->m_vertexBuffers.End();
		for(; vbIter != vbIterEnd; ++vbIter)
		{
            MashVertexBuffer *nb = (*vbIter)->Clone();
            if (!nb)
            {
                MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
                                 "Failed to create vertex buffer.", 
                                 "MashMeshBufferIntermediate::CloneMembers");
            }
            else
            {
                newBuffer->m_vertexBuffers.PushBack(nb);
            }
        }
        
        if (from->m_indexBuffer)
        {
            MashIndexBuffer *ni = from->m_indexBuffer->Clone();
            if (!ni)
            {
                MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
                                 "Failed to create index buffer.", 
                                 "MashMeshBufferIntermediate::CloneMembers");
            }
            else
            {
                newBuffer->m_indexBuffer = ni;
            }
        }
        
        newBuffer->m_vertexDeclaration = from->m_vertexDeclaration;
        if (newBuffer->m_vertexDeclaration)
        {
            newBuffer->m_vertexDeclaration->Grab();
        }
        
        return newBuffer;
	}

	MashVertexBuffer** MashMeshBufferIntermediate::GetVertexBufferArray()const
	{
		if (!m_vertexBuffers.Empty())
			return (MashVertexBuffer**)&m_vertexBuffers[0];

		return 0;
	}

	MashVertexBuffer* MashMeshBufferIntermediate::GetVertexBuffer(uint32 stream)const
	{
		if (stream < m_vertexBuffers.Size())
			return m_vertexBuffers[stream];

		return 0;
	}

	void MashMeshBufferIntermediate::_SetVertexDeclaration(MashVertex *vertex)
	{
		if (vertex)
			vertex->Grab();

		if (m_vertexDeclaration)
			m_vertexDeclaration->Drop();

		m_vertexDeclaration = vertex;
	}

	eMASH_STATUS MashMeshBufferIntermediate::ResizeVertexBuffers(uint32 bufferIndex, uint32 bufferSize, eUSAGE usage, bool saveData)
	{
		/*
			Do we need to create a new buffer for a vertex stream?
		*/
		if ((bufferIndex >= m_vertexBuffers.Size()) && (bufferIndex <= m_vertexDeclaration->GetStreamCount()))
		{
			MashVertexBuffer *newBuffer = m_renderer->CreateVertexBuffer(0, bufferSize, usage);
			if (!newBuffer)
            {
                MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
                                 "Failed to create vertex buffer.", 
                                 "MashMeshBufferIntermediate::ResizeVertexBuffers");
                
				return aMASH_FAILED;
            }

			m_vertexBuffers.PushBack(newBuffer);
			return aMASH_OK;
			
		}
		else if (bufferIndex >= m_vertexBuffers.Size())
        {
            MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
                             "The buffer index to resize is greater than the vertex buffer count.", 
                             "MashMeshBufferIntermediate::ResizeVertexBuffers");
            
			return aMASH_FAILED;
        }

		const uint32 currentSize = m_vertexBuffers[bufferIndex]->GetBufferSize();
		if (currentSize == bufferSize)
			return aMASH_OK;

		return m_vertexBuffers[bufferIndex]->Resize(bufferSize, saveData);
	}

	eMASH_STATUS MashMeshBufferIntermediate::ResizeIndexBuffer(uint32 bufferSize, bool saveData)
	{
		if (!m_indexBuffer)
			return aMASH_OK;

		const uint32 currentSize = m_indexBuffer->GetBufferSize();
		if (currentSize == bufferSize)
			return aMASH_OK;

		return m_indexBuffer->Resize(bufferSize, saveData);
	}
}