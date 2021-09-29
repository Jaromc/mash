//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashD3D10Texture.h"
#include "CMashD3D10Renderer.h"
#include "CMashD3D10Helper.h"
#include "MashLog.h"
namespace mash
{

	CMashD3D10Texture::CMashD3D10Texture(mash::CMashD3D10Renderer *pVideo, ID3D10Texture2D *pTexture, ID3D10ShaderResourceView *pResourceView,
		const D3D10_TEXTURE2D_DESC &desc, uint32 iUID, bool useMipmaps, const MashStringc &sName):MashTexture(),m_pVideo(pVideo),
		m_pTexture(pTexture), m_textureDesc(desc), m_sName(sName), m_UID(iUID), m_pTextureResourceView(pResourceView), m_useMipmaps(useMipmaps)
	{
		m_pTexture->AddRef();

		D3D10_TEXTURE2D_DESC texDesc;
		m_pTexture->GetDesc(&texDesc);
		m_mipmapCount = texDesc.MipLevels;
	}

	CMashD3D10Texture::~CMashD3D10Texture()
	{
		if (m_pTextureResourceView)
		{
			m_pTextureResourceView->Release();
			m_pTextureResourceView = 0;
		}

		if (m_pTexture)
		{
			m_pTexture->Release();
			m_pTexture = 0;
		}
	}

	void CMashD3D10Texture::GetSize(uint32 &iWidth, uint32 &iHeight)const
	{
		iWidth = m_textureDesc.Width;
		iHeight = m_textureDesc.Height;
	}

	eUSAGE CMashD3D10Texture::GetUsageType()const
	{
		if (m_textureDesc.CPUAccessFlags == 0)
			return aUSAGE_STATIC;

		return aUSAGE_DYNAMIC;
	}

	MashTexture* CMashD3D10Texture::Clone(const MashStringc &sName)const
	{
		CMashD3D10Texture *pNewTexture = (CMashD3D10Texture*)m_pVideo->CreateTexture(sName, m_textureDesc);

		if (!pNewTexture)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Failed to create new texture resource for texture copy.", 
					"CMashD3D10Texture::Clone");

			return 0;
		}

		D3DX10_TEXTURE_LOAD_INFO textureInfo;
		textureInfo.pSrcBox = 0;
		textureInfo.pDstBox = 0;
		textureInfo.SrcFirstMip = 0;
		textureInfo.DstFirstMip = 0;
		textureInfo.NumMips = m_textureDesc.MipLevels;
		textureInfo.SrcFirstElement = 0;
		textureInfo.DstFirstElement = 0;
		textureInfo.NumElements = 0;
		textureInfo.Filter = D3DX10_FILTER_LINEAR;
		textureInfo.MipFilter = D3DX10_FILTER_LINEAR;
		if (FAILED(D3DX10LoadTextureFromTexture(m_pTexture, &textureInfo, pNewTexture->GetD3D10Buffer())))
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Failed to copy texture using D3D.", 
					"CMashD3D10Texture::Clone");

			pNewTexture->Drop();
			return 0;
		}

		return pNewTexture;
	}

	eMASH_STATUS CMashD3D10Texture::Lock(eBUFFER_LOCK eType, void **pData, uint32 iLevel, uint32 iFace)
	{
		/*
			Make sure this resource is not currently mapped as a shader input.
		*/
		m_pVideo->ResetUsedShaderResources();

		D3D10_MAPPED_TEXTURE2D lockedTextureData;
		if (FAILED(m_pTexture->Map(D3D10CalcSubresource(iLevel, 0, m_textureDesc.MipLevels), MashToD3D10Lock(eType), 0, &lockedTextureData)))
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Texture buffer failed to lock.", 
					"CMashD3D10Texture::Lock");
			return aMASH_FAILED;
		}

		(*pData) = lockedTextureData.pData;

		return aMASH_OK;
	}

	eMASH_STATUS CMashD3D10Texture::Unlock(uint32 iLevel, uint32 iFace)
	{
		m_pTexture->Unmap(D3D10CalcSubresource(iLevel, 0, m_textureDesc.MipLevels));

		return aMASH_OK;
	}


}