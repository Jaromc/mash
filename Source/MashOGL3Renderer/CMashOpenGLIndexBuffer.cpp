//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashOpenGLIndexBuffer.h"
#include "CMashOpenGLRenderer.h"
#include "MashLog.h"
namespace mash
{
	CMashOpenGLIndexBuffer::CMashOpenGLIndexBuffer(CMashOpenGLRenderer *pRenderer,
			uint32 index,
			eUSAGE usage,
			eFORMAT format,
			uint32 size):MashIndexBuffer(),m_pRenderer(pRenderer),
			m_index(index), m_usage(usage), m_format(format), m_size(size)
	{
	}

	CMashOpenGLIndexBuffer::~CMashOpenGLIndexBuffer()
	{
        glDeleteBuffersPtr(1, &m_index);
	}

	eMASH_STATUS CMashOpenGLIndexBuffer::Resize(uint32 newSize, bool saveData)
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

        glBindBufferPtr(GL_ELEMENT_ARRAY_BUFFER, m_index);
        glBufferDataPtr(GL_ELEMENT_ARRAY_BUFFER, m_size, 0, flag);

		if (saveData)
		{
            void *writeTo = glMapBufferPtr(m_index, GL_WRITE_ONLY);
			memcpy(writeTo, dataToCopy, amountToCopy);
            glUnmapBufferPtr(m_index);
			MASH_FREE(dataToCopy);
		}

        glBindBufferPtr(GL_ELEMENT_ARRAY_BUFFER, 0);

		return aMASH_OK;
	}

	eMASH_STATUS CMashOpenGLIndexBuffer::Copy(const MashIndexBuffer *from)
	{
		const uint32 fromSize = from->GetBufferSize();
		const uint32 thisSize = GetBufferSize();
		const uint32 fromOgIndex = ((CMashOpenGLIndexBuffer*)from)->GetOpenGLIndex();
		const uint32 amountToWrite = (thisSize > fromSize)?fromSize:thisSize;

        void *dataToCopy = glMapBufferPtr(fromOgIndex, GL_READ_ONLY);
		if (!dataToCopy)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
				"Failed to lock copyFrom buffer.", 
				"CMashOpenGLIndexBuffer::Copy");

			return aMASH_FAILED;
		}

        void *writeTo = glMapBufferPtr(m_index, GL_WRITE_ONLY);
		if (!writeTo)
		{
            glUnmapBufferPtr(fromOgIndex);

			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
				"Failed to lock copyTo buffer.", 
				"CMashOpenGLIndexBuffer::Copy");

			return aMASH_FAILED;
		}

		memcpy(writeTo, dataToCopy, amountToWrite);

        glUnmapBufferPtr(m_index);
        glUnmapBufferPtr(fromOgIndex);

		return aMASH_OK;
	}

	MashIndexBuffer* CMashOpenGLIndexBuffer::Clone()const
	{
		int32 bufferSize = 0;
        glGetBufferParameterivPtr(m_index, GL_BUFFER_SIZE, &bufferSize);
        void *dataToCopy = glMapBufferPtr(m_index, GL_READ_ONLY);
		MashIndexBuffer *newBuffer = m_pRenderer->CreateIndexBufferBySize(dataToCopy, bufferSize, m_usage, m_format);
        glUnmapBufferPtr(m_index);

		return newBuffer;
	}

	eMASH_STATUS CMashOpenGLIndexBuffer::Lock(eBUFFER_LOCK eType, void **pData)const
	{
        glBindBufferPtr(GL_ELEMENT_ARRAY_BUFFER, m_index);

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
            glBufferDataPtr(GL_ELEMENT_ARRAY_BUFFER, m_size, 0, flag);
		}

        *pData = glMapBufferPtr(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);

		return aMASH_OK;
	}

	eMASH_STATUS CMashOpenGLIndexBuffer::Unlock()const
	{
        if (glUnmapBufferPtr(GL_ELEMENT_ARRAY_BUFFER) != GL_TRUE)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
				"Failed to unmap index buffer.", 
				"CMashOpenGLIndexBuffer::Unlock");

			return aMASH_FAILED;
		}

        glBindBufferPtr(GL_ELEMENT_ARRAY_BUFFER, 0);

		return aMASH_OK;
	}
}
