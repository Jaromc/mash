//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashD3D10CubicRenderSurface.h"
#include "CMashD3D10Renderer.h"
#include "CMashD3D10CubeTexture.h"
#include "CMashD3D10Helper.h"
#include "MashLog.h"
namespace mash
{
	CMashD3D10CubicRenderSurface::CMashD3D10CubicRenderSurface():m_currentSurface(0), m_pDepthStencilTexture(0),
		m_RenderTargetTexture(0), m_iSize(0), m_useMipmaps(false), m_bUseDepth(false), m_iTargetCount(0),
		m_eDepthFormat(aFORMAT_DEPTH32_FLOAT), m_eTargetFormat(aFORMAT_RGBA32_FLOAT), m_autoViewport(true)
	{
		memset(m_pDepthStencilView, 0, sizeof(m_pDepthStencilView));
		memset(m_RenderTargetView, 0, sizeof(m_RenderTargetView));
	}

	CMashD3D10CubicRenderSurface::~CMashD3D10CubicRenderSurface()
	{
		for(uint32 i = 0; i < g_cubicRenderTargetCount; ++i)
		{
			if (m_RenderTargetView[i])
			{
				m_RenderTargetView[i]->Release();
				m_RenderTargetView[i] = 0;
			}
		}

		for(uint32 i = 0; i < g_cubicRenderTargetCount; ++i)
		{
			if (m_pDepthStencilView[i])
			{
				m_pDepthStencilView[i]->Release();
				m_pDepthStencilView[i] = 0;
			}
		}

		if (m_RenderTargetTexture)
		{
			m_pRenderer->RemoveTextureFromCache(m_RenderTargetTexture);

			m_RenderTargetTexture->Drop();
			m_RenderTargetTexture = 0;
		}

		if (m_pDepthStencilTexture)
		{
			m_pRenderer->RemoveTextureFromCache(m_pDepthStencilTexture);

			m_pDepthStencilTexture->Drop();
			m_pDepthStencilTexture = 0;
		}
	}

	MashRenderSurface* CMashD3D10CubicRenderSurface::Clone()const
	{
		CMashD3D10CubicRenderSurface *pNewSurface = (CMashD3D10CubicRenderSurface*)m_pRenderer->CreateCubicRenderSurface(m_iSize,
			m_useMipmaps, 
			m_eTargetFormat,
			m_bUseDepth,
			m_eDepthFormat);

		return pNewSurface;
	}

	mash::MashVector2 CMashD3D10CubicRenderSurface::GetDimentions()const
	{
		return mash::MashVector2(m_iSize, m_iSize);
	}

	eMASH_STATUS CMashD3D10CubicRenderSurface::CreateSurface(CMashD3D10Renderer *pRenderer, uint32 iSize, bool useMipmaps,
			eFORMAT eFormat, bool bUseDepth, eFORMAT eDepthFormat)
	{
		m_pRenderer = pRenderer;

		m_iSize = iSize;
		m_useMipmaps = useMipmaps;
		m_bUseDepth = bUseDepth;
		m_eDepthFormat = eDepthFormat;
		m_iTargetCount = 1;
		m_eTargetFormat = eFormat;

		//create depth stencil texture
		D3D10_TEXTURE2D_DESC textureDesc;
		textureDesc.Width = iSize;
		textureDesc.Height = iSize;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 6;
		textureDesc.Format = MashToD3D10Format(eDepthFormat);
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Usage = D3D10_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D10_BIND_DEPTH_STENCIL;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = D3D10_RESOURCE_MISC_TEXTURECUBE;

		if (bUseDepth)
		{
			m_pDepthStencilTexture = (CMashD3D10CubeTexture*)m_pRenderer->CreateCubeTexture("", textureDesc);

			if (!m_pDepthStencilTexture)
			{
				MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Failed to create render target depth texture.", 
					"CMashD3D10CubicRenderSurface::CreateSurface");
				return aMASH_FAILED;
			}

			m_pDepthStencilTexture->Grab();

			for(uint32 i = 0 ; i < 6; ++i)
			{
				//create depth stencil view
				D3D10_DEPTH_STENCIL_VIEW_DESC viewDesc;
				viewDesc.Format = textureDesc.Format;
				viewDesc.ViewDimension = D3D10_DSV_DIMENSION_TEXTURE2DARRAY;
				viewDesc.Texture2DArray.FirstArraySlice = i;
				viewDesc.Texture2DArray.ArraySize = 1;
				viewDesc.Texture2DArray.MipSlice = 0;
				if (FAILED(m_pRenderer->GetD3D10Device()->CreateDepthStencilView(m_pDepthStencilTexture->GetD3D10Buffer(), &viewDesc, &m_pDepthStencilView[i])))
				{
					MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
						"Failed to create render target depth view.", 
						"CMashD3D10CubicRenderSurface::CreateSurface");
					return aMASH_FAILED;
				}
			}
		}
		
		if (m_useMipmaps)
		{
			textureDesc.MiscFlags = D3D10_RESOURCE_MISC_GENERATE_MIPS | D3D10_RESOURCE_MISC_TEXTURECUBE;
			textureDesc.MipLevels = 0;
		}
		else
		{
			textureDesc.MiscFlags = D3D10_RESOURCE_MISC_TEXTURECUBE;
			textureDesc.MipLevels = 1;
		}
		
		textureDesc.BindFlags = D3D10_BIND_RENDER_TARGET | D3D10_BIND_SHADER_RESOURCE;

		textureDesc.Format = MashToD3D10Format(eFormat);
		
		m_RenderTargetTexture = (CMashD3D10CubeTexture*)m_pRenderer->CreateCubeTexture("", textureDesc);

		if (!m_RenderTargetTexture)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Failed to create render target texture.", 
					"CMashD3D10CubicRenderSurface::CreateSurface");
			return aMASH_FAILED;
		}

		m_RenderTargetTexture->Grab();

		for(uint32 i = 0 ; i < 6; ++i)
		{
			D3D10_RENDER_TARGET_VIEW_DESC viewDesc;
			viewDesc.Format = textureDesc.Format;
			viewDesc.ViewDimension = D3D10_RTV_DIMENSION_TEXTURE2DARRAY;
			viewDesc.Texture2DArray.FirstArraySlice = i;
			viewDesc.Texture2DArray.ArraySize = 1;
			viewDesc.Texture2DArray.MipSlice = 0;
			if (FAILED(m_pRenderer->GetD3D10Device()->CreateRenderTargetView(m_RenderTargetTexture->GetD3D10Buffer(), &viewDesc, &m_RenderTargetView[i])))
			{
				MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
						"Failed to create render target view.", 
						"CMashD3D10CubicRenderSurface::CreateSurface");
				return aMASH_FAILED;
			}
		}

		return aMASH_OK;
	}

	void CMashD3D10CubicRenderSurface::GenerateMips(int32 iSurface)
	{
		if (m_useMipmaps)
		{
			ID3D10Device *d3d10Device = m_pRenderer->GetD3D10Device();
			d3d10Device->GenerateMips(m_RenderTargetTexture->GetD3D10ResourceView());
		}
	}

	void CMashD3D10CubicRenderSurface::OnDismount()
	{
		/*
			Removes resource views so the final textures can be set as textures, if needed
		*/
		m_pRenderer->ResetUsedShaderResources();
	}

	eMASH_STATUS CMashD3D10CubicRenderSurface::OnSet(int32 iSurface)
	{
		/*
			Removes textures from shader use so they can be mounted as render targets
		*/
		m_pRenderer->ResetUsedShaderResources();

		if (m_autoViewport)
		{
			sMashViewPort viewPort;
			viewPort.x = 0;
			viewPort.y = 0;
			viewPort.width = m_iSize;
			viewPort.height = m_iSize;
			viewPort.minZ = 0.0f;
			viewPort.maxZ = 1.0f;

			m_pRenderer->SetViewport(viewPort);
		}

		m_currentSurface = iSurface;

		ID3D10RenderTargetView *rtViews[1] = {m_RenderTargetView[m_currentSurface]};
		m_pRenderer->GetD3D10Device()->OMSetRenderTargets(1, rtViews, m_pDepthStencilView[m_currentSurface]);

		return aMASH_OK;
	}

	eMASH_STATUS CMashD3D10CubicRenderSurface::OnPreResize()
	{
		/*
			Resizing not needed because cubic surfaces don't fit to screen
		*/
		return aMASH_OK;	
	}

	eMASH_STATUS CMashD3D10CubicRenderSurface::OnPostResize(uint32 width, uint32 height)
	{
		return aMASH_OK;	
	}

	void CMashD3D10CubicRenderSurface::_ClearTargets(uint32 iClearFlags, const sMashColour4 &colour, float fZDepth)
	{
		m_pRenderer->GetD3D10Device()->ClearRenderTargetView(m_RenderTargetView[m_currentSurface], colour.v);

		if (m_pDepthStencilView[m_currentSurface] && ((iClearFlags & aCLEAR_DEPTH) || (iClearFlags & aCLEAR_STENCIL)))
			m_pRenderer->GetD3D10Device()->ClearDepthStencilView(m_pDepthStencilView[m_currentSurface], MashToD3D10ClearFlags(iClearFlags), fZDepth, 0);
	}
}