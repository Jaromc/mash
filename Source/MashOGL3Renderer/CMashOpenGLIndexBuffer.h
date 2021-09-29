//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_OPENGL_INDEX_BUFFER_H_
#define _C_MASH_OPENGL_INDEX_BUFFER_H_

#include "MashIndexBuffer.h"
#include "CMashOpenGLHeader.h"
namespace mash
{
	class CMashOpenGLRenderer;

	class CMashOpenGLIndexBuffer : public MashIndexBuffer
	{
	private:
		CMashOpenGLRenderer *m_pRenderer;
		GLuint m_index;
		eUSAGE m_usage;
		eFORMAT m_format;
		uint32 m_size;
	public:
		CMashOpenGLIndexBuffer(CMashOpenGLRenderer *pRenderer,
			uint32 index,
			eUSAGE usage,
			eFORMAT format,
			uint32 size);

		~CMashOpenGLIndexBuffer();

		MashIndexBuffer* Clone()const;
		eMASH_STATUS Copy(const MashIndexBuffer *from);

		eMASH_STATUS Resize(uint32 newSize, bool saveData);
		uint32 GetBufferSize()const;
		eMASH_STATUS Lock(eBUFFER_LOCK eType, void **pData)const;
		eMASH_STATUS Unlock()const;
		eRESOURCE_TYPE GetType()const;
		eFORMAT GetFormat()const;
		eUSAGE GetUsageType()const;
		GLuint GetOpenGLIndex()const;
	};

	inline uint32 CMashOpenGLIndexBuffer::GetBufferSize()const
	{
		return m_size;
	}

	inline GLuint CMashOpenGLIndexBuffer::GetOpenGLIndex()const
	{
		return m_index;
	}

	inline eUSAGE CMashOpenGLIndexBuffer::GetUsageType()const
	{
		return m_usage;
	}

	inline eFORMAT CMashOpenGLIndexBuffer::GetFormat()const
	{
		return m_format;
	}

	inline eRESOURCE_TYPE CMashOpenGLIndexBuffer::GetType()const
	{
		return aRESOURCE_INDEX;
	}
}

#endif