//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashD3D10VertexBuffer.h"
#include "CMashD3D10Renderer.h"
#include "CMashD3D10Helper.h"
#include "MashVertex.h"
#include "MashLog.h"
namespace mash
{
	CMashD3D10VertexBuffer::CMashD3D10VertexBuffer(CMashD3D10Renderer *pVideo,
			ID3D10Buffer *pVertexBuffer,
			const D3D10_BUFFER_DESC &desc):MashVertexBuffer(),m_pVideo(pVideo),
			m_pVertexBuffer(pVertexBuffer), m_desc(desc)
	{
	}

	CMashD3D10VertexBuffer::~CMashD3D10VertexBuffer()
	{
		if (m_pVertexBuffer)
		{
			m_pVertexBuffer->Release();
			m_pVertexBuffer = 0;
		}
	}

	eMASH_STATUS CMashD3D10VertexBuffer::Resize(uint32 newSize, bool saveData)
	{
		m_desc.ByteWidth = newSize;

		ID3D10Buffer *oldData = m_pVertexBuffer;

		HRESULT hr = m_pVideo->GetD3D10Device()->CreateBuffer(&m_desc, 0, &m_pVertexBuffer);
		if (FAILED(hr))
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
				"Failed to create vertex buffer using D3D::CreateBuffer().", 
					"CMashD3D10VertexBuffer::Resize");

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

	eMASH_STATUS CMashD3D10VertexBuffer::CopyData(ID3D10Buffer *from)
	{
		ID3D10Buffer *newBuffer = 0;
		m_pVideo->GetD3D10Device()->CopyResource(newBuffer, from);

		if (!newBuffer)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
				"Failed to get valid pointer from D3D::CopyResource().", 
				"CMashD3D10VertexBuffer::Clone");

			return aMASH_FAILED;
		}

		if (m_pVertexBuffer)
			m_pVertexBuffer->Release();

		m_pVertexBuffer = newBuffer;

		return aMASH_OK;
	}

	eMASH_STATUS CMashD3D10VertexBuffer::Copy(const MashVertexBuffer *from)
	{
		CMashD3D10VertexBuffer *fromD3D10Buffer = (CMashD3D10VertexBuffer*)from;
		m_desc = fromD3D10Buffer->m_desc;
		return CopyData(fromD3D10Buffer->GetD3D10Buffer());
	}

	MashVertexBuffer* CMashD3D10VertexBuffer::Clone()const
	{
		ID3D10Buffer *pBuffer = 0;
		m_pVideo->GetD3D10Device()->CopyResource(pBuffer, m_pVertexBuffer);

		if (!pBuffer)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
				"Failed to get valid pointer from D3D::CopyResource().", 
				"CMashD3D10VertexBuffer::Clone");

			return 0;
		}

		CMashD3D10VertexBuffer *pNewBuffer = (CMashD3D10VertexBuffer*)m_pVideo->CreateVertexBuffer(pBuffer, m_desc);

		if (!pNewBuffer)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
				"Failed to get valid vertex buffer pointer.", 
				"CMashD3D10VertexBuffer::Clone");

			pBuffer->Release();
			return 0;
		}

		return pNewBuffer;
	}

	eUSAGE CMashD3D10VertexBuffer::GetUsageType()const
	{
		if (m_desc.CPUAccessFlags == 0)
			return aUSAGE_STATIC;

		return aUSAGE_DYNAMIC;
	}

	eMASH_STATUS CMashD3D10VertexBuffer::Lock(eBUFFER_LOCK eType, void **pData)const
	{
		/*
			Make sure this resource is not currently mapped as a shader input.
		*/
		m_pVideo->ResetUsedBuffers();

		if (FAILED(m_pVertexBuffer->Map(MashToD3D10Lock(eType), 0, pData)))
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Vertex buffer failed to lock.", 
					"CMashD3D10VertexBuffer::Lock");
			return aMASH_FAILED;
		}

		return aMASH_OK;
	}

	eMASH_STATUS CMashD3D10VertexBuffer::Unlock()const
	{
		m_pVertexBuffer->Unmap();

		return aMASH_OK;
	}
}