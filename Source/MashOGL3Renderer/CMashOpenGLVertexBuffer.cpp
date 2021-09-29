//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashOpenGLVertexBuffer.h"
#include "CMashOpenGLRenderer.h"
#include "MashVertex.h"
#include "MashLog.h"

namespace mash
{
	CMashOpenGLVertexBuffer::CMashOpenGLVertexBuffer(CMashOpenGLRenderer *pRenderer,
			uint32 index,
			eUSAGE usage,
			uint32 size):MashVertexBuffer(),m_pRenderer(pRenderer),
			m_index(index), m_usage(usage), m_size(size)
	{
	}

	CMashOpenGLVertexBuffer::~CMashOpenGLVertexBuffer()
	{
		glDeleteBuffersPtr(1, &m_index);
	}

	eMASH_STATUS CMashOpenGLVertexBuffer::Resize(uint32 newSize, bool saveData)
	{
		m_size = newSize;

		void *dataToCopy = 0;
		int32 amountToCopy = 0;

		if (saveData)
		{
			int32 bufferSize = 0;

			glGetBufferParameterivPtr(m_index, GL_BUFFER_SIZE, &bufferSize);
			amountToCopy = (bufferSize > newSize)?newSize:bufferSize;
			void *mappedData = glMapBufferPtr(m_index, GL_READ_ONLY);
			dataToCopy = MASH_ALLOC_COMMON(amountToCopy);
			memcpy(dataToCopy, mappedData, amountToCopy);
		}

		uint32 flag = GL_STATIC_DRAW;
		if (m_usage == aUSAGE_DYNAMIC)
			flag = GL_DYNAMIC_DRAW;

		glBindBufferPtr(GL_ARRAY_BUFFER, m_index);	
		glBufferDataPtr(GL_ARRAY_BUFFER, m_size, 0, flag);
		
		if (saveData)
		{
			void *writeTo = glMapBufferPtr(m_index, GL_WRITE_ONLY);
			memcpy(writeTo, dataToCopy, amountToCopy);
			glUnmapBufferPtr(m_index);
			MASH_FREE(dataToCopy);
		}
		
		glBindBufferPtr(GL_ARRAY_BUFFER, 0);

		return aMASH_OK;
	}

	eMASH_STATUS CMashOpenGLVertexBuffer::Copy(const MashVertexBuffer *from)
	{
		CMashOpenGLVertexBuffer *oglFromBuffer = (CMashOpenGLVertexBuffer*)from;

		const uint32 fromSize = oglFromBuffer->GetBufferSize();
		const uint32 thisSize = GetBufferSize();
		const uint32 fromOgIndex = oglFromBuffer->GetOpenGLIndex();
		const uint32 amountToWrite = (thisSize > fromSize)?fromSize:thisSize;

		void *dataToCopy = glMapBufferPtr(fromOgIndex, GL_READ_ONLY);
		if (!dataToCopy)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
				"Failed to lock copyFrom buffer.", 
				"CMashOpenGLVertexBuffer::Copy");

			return aMASH_FAILED;
		}

		void *writeTo = glMapBufferPtr(m_index, GL_WRITE_ONLY);
		if (!writeTo)
		{
			glUnmapBufferPtr(fromOgIndex);

			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
				"Failed to lock copyTo buffer.", 
				"CMashOpenGLVertexBuffer::Copy");

			return aMASH_FAILED;
		}

		memcpy(writeTo, dataToCopy, amountToWrite);

		glUnmapBufferPtr(m_index);
		glUnmapBufferPtr(fromOgIndex);

		return aMASH_OK;
	}

	MashVertexBuffer* CMashOpenGLVertexBuffer::Clone()const
	{
		int32 bufferSize = 0;
		glGetBufferParameterivPtr(m_index, GL_BUFFER_SIZE, &bufferSize);
		void *dataToCopy = glMapBufferPtr(m_index, GL_READ_ONLY);
		MashVertexBuffer *newBuffer = m_pRenderer->CreateVertexBuffer(dataToCopy, bufferSize, m_usage);
		glUnmapBufferPtr(m_index);

		return newBuffer;
	}

	eMASH_STATUS CMashOpenGLVertexBuffer::Lock(eBUFFER_LOCK eType, void **pData)const
	{
		glBindBufferPtr(GL_ARRAY_BUFFER, m_index);

		if (eType == aLOCK_WRITE_DISCARD)
		{
			uint32 flag = GL_STATIC_DRAW;
			if (m_usage == aUSAGE_DYNAMIC)
				flag = GL_DYNAMIC_DRAW;

			/*
				Calling Map causes CPU stalls while it waits for the GPU
				to finish with the data. To avoid this, we can call glBufferDataPtr with a NULL
				pointer before hand to create a new chunk of memory for us to work with and
				returns it immediatly.
			*/
			glBufferDataPtr(GL_ARRAY_BUFFER, m_size, 0, flag);
		}

		*pData = glMapBufferPtr(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		glBindBufferPtr(GL_ARRAY_BUFFER, 0);

		return aMASH_OK;
	}

	eMASH_STATUS CMashOpenGLVertexBuffer::Unlock()const
	{
		glBindBufferPtr(GL_ARRAY_BUFFER, m_index);

		if (glUnmapBufferPtr(GL_ARRAY_BUFFER) != GL_TRUE)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
				"Failed to unmap vertex buffer.", 
				"CMashOpenGLVertexBuffer::Unlock");

			return aMASH_FAILED;
		}

		glBindBufferPtr(GL_ARRAY_BUFFER, 0);

		return aMASH_OK;
	}
}