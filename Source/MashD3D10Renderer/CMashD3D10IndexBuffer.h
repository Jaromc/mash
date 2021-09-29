//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_D3D10_INDEX_BUFFER_H_
#define _C_MASH_D3D10_INDEX_BUFFER_H_

#include "MashDataTypes.h"

#ifdef MASH_WINDOWS
#include "MashIndexBuffer.h"
#include <d3d10_1.h>

namespace mash
{
	class CMashD3D10Renderer;

	class CMashD3D10IndexBuffer : public MashIndexBuffer
	{
	private:
		CMashD3D10Renderer *m_pVideo;
		ID3D10Buffer *m_pIndexBuffer;
		D3D10_BUFFER_DESC m_desc;
		eFORMAT m_eFormat;
		DXGI_FORMAT m_eD3D10Format;

		eMASH_STATUS CopyData(ID3D10Buffer *from);
	public:
		CMashD3D10IndexBuffer(CMashD3D10Renderer *pVideo,
			ID3D10Buffer *pIndexBuffer,
			eFORMAT eFormat,
			const D3D10_BUFFER_DESC &desc);

		~CMashD3D10IndexBuffer();

		eMASH_STATUS Resize(uint32 newSize, bool saveData = false);
		MashIndexBuffer* Clone()const;
		eMASH_STATUS Copy(const MashIndexBuffer *from);

		eMASH_STATUS Lock(eBUFFER_LOCK eType, void **pData)const;
		eMASH_STATUS Unlock()const;
		eRESOURCE_TYPE GetType()const;
		eFORMAT GetFormat()const;

		ID3D10Buffer* GetD3D10Buffer()const;
		/*
			 DXGI_FORMAT is accessed reguarly so a special function is created for it rather than
			 converting from GetFormat() to D3D10
		*/
		DXGI_FORMAT GetD3D10Format()const;
		eUSAGE GetUsageType()const;

		uint32 GetBufferSize()const;
	};

	inline uint32 CMashD3D10IndexBuffer::GetBufferSize()const
	{
		return m_desc.ByteWidth;
	}

	inline DXGI_FORMAT CMashD3D10IndexBuffer::GetD3D10Format()const
	{
		return m_eD3D10Format;
	}

	inline eFORMAT CMashD3D10IndexBuffer::GetFormat()const
	{
		return m_eFormat;
	}

	inline ID3D10Buffer* CMashD3D10IndexBuffer::GetD3D10Buffer()const
	{
		return m_pIndexBuffer;
	}

	inline eRESOURCE_TYPE CMashD3D10IndexBuffer::GetType()const
	{
		return eRESOURCE_TYPE::aRESOURCE_INDEX;
	}
}

#endif

#endif