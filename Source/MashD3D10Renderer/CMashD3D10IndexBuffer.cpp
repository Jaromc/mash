//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashD3D10IndexBuffer.h"
#include "CMashD3D10Renderer.h"
#include "CMashD3D10Helper.h"
#include "MashLog.h"
namespace mash
{
	CMashD3D10IndexBuffer::CMashD3D10IndexBuffer(CMashD3D10Renderer *pVideo,
			ID3D10Buffer *pIndexBuffer,
			eFORMAT eFormat,
			const D3D10_BUFFER_DESC &desc):MashIndexBuffer(),m_pVideo(pVideo),
			m_pIndexBuffer(pIndexBuffer), m_desc(desc), m_eFormat(eFormat)
	{
		if (eFormat == aFORMAT_R16_UINT)
			m_eD3D10Format = DXGI_FORMAT_R16_UINT;
		else
			m_eD3D10Format = DXGI_FORMAT_R32_UINT;
	}

	CMashD3D10IndexBuffer::~CMashD3D10IndexBuffer()
	{
		if (m_pIndexBuffer)
		{
			m_pIndexBuffer->Release();
			m_pIndexBuffer = 0;
		}
	}

	eMASH_STATUS CMashD3D10IndexBuffer::Resize(uint32 newSize, bool saveData)
	{
		m_desc.ByteWidth = newSize;

		ID3D10Buffer *oldData = m_pIndexBuffer;

		HRESULT hr = m_pVideo->GetD3D10Device()->CreateBuffer(&m_desc, 0, &m_pIndexBuffer);
		if (FAILED(hr))
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Failed to resize vertex buffer.", 
					"CMashD3D10IndexBuffer::Resize");
			return aMASH_FAILED;
		}

		eMASH_STATUS status = aMASH_OK;
		if (saveData)
		{
			status = CopyData(oldData);
		}

		oldData->Release();

		return status;
	}

	eMASH_STATUS CMashD3D10IndexBuffer::CopyData(ID3D10Buffer *from)
	{
		ID3D10Buffer *newBuffer = 0;
		m_pVideo->GetD3D10Device()->CopyResource(newBuffer, from);

		if (!newBuffer)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
				"Failed to get valid pointer from D3D::CopyResource().", 
				"CMashD3D10IndexBuffer::CopyData");

			return aMASH_FAILED;
		}

		if (m_pIndexBuffer)
			m_pIndexBuffer->Release();

		m_pIndexBuffer = newBuffer;

		return aMASH_OK;
	}

	eMASH_STATUS CMashD3D10IndexBuffer::Copy(const MashIndexBuffer *from)
	{
		CMashD3D10IndexBuffer *fromD3D10Buffer = (CMashD3D10IndexBuffer*)from;
		m_desc = fromD3D10Buffer->m_desc;
		m_eFormat = fromD3D10Buffer->m_eFormat;
		m_eD3D10Format = fromD3D10Buffer->m_eD3D10Format;
		return CopyData(fromD3D10Buffer->GetD3D10Buffer());
	}

	MashIndexBuffer* CMashD3D10IndexBuffer::Clone()const
	{
		ID3D10Buffer *pBuffer = 0;
		m_pVideo->GetD3D10Device()->CopyResource(pBuffer, m_pIndexBuffer);

		if (!pBuffer)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
				"Failed to get valid pointer from D3D::CopyResource().", 
				"CMashD3D10IndexBuffer::Clone");

			return 0;
		}

		CMashD3D10IndexBuffer *pNewBuffer = (CMashD3D10IndexBuffer*)m_pVideo->CreateIndexBuffer(pBuffer, m_eFormat, m_desc);

		if (!pNewBuffer)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
				"Failed to get valid index buffer pointer.", 
				"CMashD3D10IndexBuffer::Clone");

			pBuffer->Release();
			return 0;
		}

		return pNewBuffer;
	}

	eUSAGE CMashD3D10IndexBuffer::GetUsageType()const
	{
		if (m_desc.CPUAccessFlags == 0)
			return aUSAGE_STATIC;

		return aUSAGE_DYNAMIC;
	}

	eMASH_STATUS CMashD3D10IndexBuffer::Lock(eBUFFER_LOCK eType, void **pData)const
	{
		/*
			Make sure this resource is not currently mapped as a shader input.
		*/
		m_pVideo->ResetUsedBuffers();

		if (FAILED(m_pIndexBuffer->Map(MashToD3D10Lock(eType), 0, pData)))
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Index buffer failed to lock.", 
					"CMashD3D10IndexBuffer::Lock");
			return aMASH_FAILED;
		}

		return aMASH_OK;
	}

	eMASH_STATUS CMashD3D10IndexBuffer::Unlock()const
	{
		m_pIndexBuffer->Unmap();

		return aMASH_OK;
	}
}