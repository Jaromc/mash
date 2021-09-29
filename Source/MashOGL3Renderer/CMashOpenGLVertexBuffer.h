//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_OPENGL_VERTEX_BUFFER_H_
#define _C_MASH_OPENGL_VERTEX_BUFFER_H_

#include "MashVertexBuffer.h"
#include "CMashOpenGLHeader.h"
namespace mash
{
	class CMashOpenGLRenderer;

	class CMashOpenGLVertexBuffer : public MashVertexBuffer
	{
	private:
		CMashOpenGLRenderer *m_pRenderer;
		GLuint m_index;
		eUSAGE m_usage;
		uint32 m_size;
	public:
		CMashOpenGLVertexBuffer(CMashOpenGLRenderer *pRenderer,
			uint32 index,
			eUSAGE usage,
			uint32 size);

		~CMashOpenGLVertexBuffer();

		eMASH_STATUS Copy(const MashVertexBuffer *from);
		MashVertexBuffer* Clone()const;

		eMASH_STATUS Resize(uint32 newSize, bool saveData = false);
		uint32 GetBufferSize()const;
		eMASH_STATUS Lock(eBUFFER_LOCK eType, void **pData)const;
		eMASH_STATUS Unlock()const;
		eRESOURCE_TYPE GetType()const;
		eUSAGE GetUsageType()const;
		uint32 GetOpenGLIndex()const;
	};

	inline uint32 CMashOpenGLVertexBuffer::GetBufferSize()const
	{
		return m_size;
	}

	inline uint32 CMashOpenGLVertexBuffer::GetOpenGLIndex()const
	{
		return m_index;
	}

	inline eUSAGE CMashOpenGLVertexBuffer::GetUsageType()const
	{
		return m_usage;
	}

	inline eRESOURCE_TYPE CMashOpenGLVertexBuffer::GetType()const
	{
		return aRESOURCE_VERTEX;
	}

}

#endif