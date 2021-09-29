//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_D3D10_VERTEX_BUFFER_H_
#define _C_MASH_D3D10_VERTEX_BUFFER_H_

#include "MashDataTypes.h"

#ifdef MASH_WINDOWS
#include "MashVertexBuffer.h"
#include <d3d10_1.h>

namespace mash
{
	class CMashD3D10Renderer;

	class CMashD3D10VertexBuffer : public MashVertexBuffer
	{
	private:
		CMashD3D10Renderer *m_pVideo;
		ID3D10Buffer *m_pVertexBuffer;
		D3D10_BUFFER_DESC m_desc;

		eMASH_STATUS CopyData(ID3D10Buffer *from);
	public:
		CMashD3D10VertexBuffer(CMashD3D10Renderer *pVideo,
			ID3D10Buffer *pVertexBuffer,
			const D3D10_BUFFER_DESC &desc);

		~CMashD3D10VertexBuffer();

		MashVertexBuffer* Clone()const;
		eMASH_STATUS Copy(const MashVertexBuffer *from);

		eMASH_STATUS Lock(eBUFFER_LOCK eType, void **pData)const;
		eMASH_STATUS Unlock()const;
		eRESOURCE_TYPE GetType()const;
		eMASH_STATUS Resize(uint32 newSize, bool saveData = false);

		ID3D10Buffer* GetD3D10Buffer()const;

		eUSAGE GetUsageType()const;
		uint32 GetBufferSize()const;
	};

	inline uint32 CMashD3D10VertexBuffer::GetBufferSize()const
	{
		return m_desc.ByteWidth;
	}

	inline ID3D10Buffer* CMashD3D10VertexBuffer::GetD3D10Buffer()const
	{
		return m_pVertexBuffer;
	}

	inline eRESOURCE_TYPE CMashD3D10VertexBuffer::GetType()const
	{
        return aRESOURCE_VERTEX;
	}

}

#endif

#endif
