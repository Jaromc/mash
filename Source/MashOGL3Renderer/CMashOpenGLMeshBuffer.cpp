//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashOpenGLMeshBuffer.h"
#include "CMashOpenGLVertex.h"
#include "CMashOpenGLVertexBuffer.h"
#include "CMashOpenGLIndexBuffer.h"
#include "CMashOpenGLHeader.h"
#include "MashVideo.h"
#include "MashLog.h"
namespace mash
{
	CMashOpenGLMeshBuffer::CMashOpenGLMeshBuffer(MashVideo *renderer,
			MashArray<MashVertexBuffer*> &vertexBuffers, 
			MashIndexBuffer *indexBuffer,
			MashVertex *vertexDeclaration):MashMeshBufferIntermediate(renderer, vertexBuffers,
				indexBuffer, vertexDeclaration), m_openGLvao(0),
				m_isCompiled(false)
	{

	}
    
    CMashOpenGLMeshBuffer::CMashOpenGLMeshBuffer(MashVideo *renderer):MashMeshBufferIntermediate(renderer)
    {
        
    }

	CMashOpenGLMeshBuffer::~CMashOpenGLMeshBuffer()
	{
		if (m_openGLvao)
			glDeleteVertexArraysPtr(1, &m_openGLvao);
	}

	MashMeshBuffer* CMashOpenGLMeshBuffer::Clone()
	{
		MashMeshBuffer *newBuffer = MashMeshBufferIntermediate::CloneMembers(this);
        if (newBuffer)
        {
            m_renderer->_AddCompileDependency((CMashOpenGLVertex*)newBuffer->GetVertexDeclaration(), (CMashOpenGLMeshBuffer*)newBuffer);
        }
        
        return newBuffer;
	}

	eMASH_STATUS CMashOpenGLMeshBuffer::ResizeVertexBuffers(uint32 bufferIndex, uint32 bufferSize, eUSAGE usage, bool saveData)
	{
		if (MashMeshBufferIntermediate::ResizeVertexBuffers(bufferIndex, bufferSize, usage, saveData) == aMASH_FAILED)
			return aMASH_FAILED;

		return aMASH_OK;
	}

	eMASH_STATUS CMashOpenGLMeshBuffer::ResizeIndexBuffer(uint32 bufferSize, bool saveData)
	{
		if (MashMeshBufferIntermediate::ResizeIndexBuffer(bufferSize, saveData) == aMASH_FAILED)
			return aMASH_FAILED;

		return aMASH_OK;
	}

	void CMashOpenGLMeshBuffer::_SetVertexDeclaration(MashVertex *vertex)
	{
		MashMeshBufferIntermediate::_SetVertexDeclaration(vertex);
		
		if (m_isCompiled == true)
		{
			_CommitBufferChange();
		}
	}

	void CMashOpenGLMeshBuffer::OnDependencyCompiled(MashVideo *renderer, MashMaterialDependentResourceBase *dependency)
	{
		if (!m_isCompiled)
		{
			/*
				This gets around an issue where a mesh is loaded with a partiular vertex type. Then
				the material (and therefore the vertex decl) is changed then the vertex types no longer
				match. The new vertex decal may not yet be compiled. So we re-add the mesh buffer for
				compiling till the new vertex is compiled.
			*/
			if (((CMashOpenGLVertex*)m_vertexDeclaration != dependency) && !((CMashOpenGLVertex*)m_vertexDeclaration)->IsValid())
			{
				renderer->_AddCompileDependency((CMashOpenGLVertex*)m_vertexDeclaration, this);
				return;
			}

			if (_CommitBufferChange() == aMASH_OK)
			{
				m_isCompiled = true;

				/*
					Build anything relying on this.
				*/
				renderer->_OnDependencyCompiled(this);
			}
		}
	}

	eMASH_STATUS CMashOpenGLMeshBuffer::_CommitBufferChange()
	{

		if (m_vertexBuffers.Empty())
			return aMASH_FAILED;

		/*
			This simply sorts a vertex elm list so that the elements are ordered by stream,
			from lowest to highest
		*/
		MashArray<sMashOpenGLVertexData> openGLElements;
		MashArray<sMashOpenGLVertexData>::ConstIterator elmStartIter = ((CMashOpenGLVertex*)m_vertexDeclaration)->GetOpenGLVertexElements().Begin();
		MashArray<sMashOpenGLVertexData>::ConstIterator elmEndIter = ((CMashOpenGLVertex*)m_vertexDeclaration)->GetOpenGLVertexElements().End();
		for(; elmStartIter != elmEndIter; ++elmStartIter)
		{
			if (openGLElements.Empty())
				openGLElements.PushBack(*elmStartIter);
			else
			{
				MashArray<sMashOpenGLVertexData>::Iterator tIter = openGLElements.Begin();
				MashArray<sMashOpenGLVertexData>::Iterator tEndIter = openGLElements.End();
				for(; tIter != tEndIter; ++tIter)
				{
					if (tIter->stream >= elmStartIter->stream)
					{
						tIter = openGLElements.Insert(tIter, *elmStartIter);
						break;
					}
				}
				
				if (tIter == openGLElements.End())
				{
					openGLElements.PushBack(*elmStartIter);
				}
			}
		}
		const uint32 openGLElementCount = openGLElements.Size();

		if (!((CMashOpenGLVertex*)m_vertexDeclaration)->IsValid() || (openGLElementCount == 0))
		{
			return aMASH_FAILED;
		}

		uint32 lastStream = openGLElements[0].stream;

		uint32 vaoID = 0;
		glGenVertexArraysPtr(1, &vaoID);
		glBindVertexArrayPtr(vaoID);

		glBindBufferPtr(GL_ARRAY_BUFFER, ((CMashOpenGLVertexBuffer*)m_vertexBuffers[0])->GetOpenGLIndex());
        
		for(uint32 i = 0; i < openGLElementCount; ++i)
		{
			if (lastStream != openGLElements[i].stream)
			{
				lastStream = openGLElements[i].stream;
				if (m_vertexBuffers.Size() <= openGLElements[i].stream)
				{
					MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
						"Vertex declaration has more streams than this mesh buffer. Mesh buffers must be created with the same number of streams as the vertex declaration.", 
						"CMashOpenGLMeshBuffer::_CommitBufferChange");
					
					//TODO : Clean up
					return aMASH_FAILED;
				}
				
				glBindBufferPtr(GL_ARRAY_BUFFER, ((CMashOpenGLVertexBuffer*)m_vertexBuffers[openGLElements[i].stream])->GetOpenGLIndex());
			}

			glEnableVertexAttribArrayPtr(openGLElements[i].attribute);

			glVertexAttribPointerPtr(openGLElements[i].attribute,
				openGLElements[i].size,
				openGLElements[i].type,
				openGLElements[i].normalized,
				openGLElements[i].stride,
				openGLElements[i].pointer);

			if (openGLElements[i].stream > 0)
				glVertexAttribDivisorPtr(openGLElements[i].attribute, openGLElements[i].stepRate);
			else
				glVertexAttribDivisorPtr(openGLElements[i].attribute, 0);     

		}
		
		if (m_indexBuffer)
			glBindBufferPtr(GL_ELEMENT_ARRAY_BUFFER, ((CMashOpenGLIndexBuffer*)m_indexBuffer)->GetOpenGLIndex());

		glBindVertexArrayPtr(0);
		glBindBufferPtr(GL_ARRAY_BUFFER, 0);
		glBindBufferPtr(GL_ELEMENT_ARRAY_BUFFER, 0);
		for(uint32 i = 0; i < openGLElementCount; ++i)
		{
			glDisableVertexAttribArrayPtr(openGLElements[i].attribute);

			if (openGLElements[i].stream > 0)
				glVertexAttribDivisorPtr(openGLElements[i].attribute, 0);
		}

		m_openGLvao = vaoID;

		return aMASH_OK;
	}
}