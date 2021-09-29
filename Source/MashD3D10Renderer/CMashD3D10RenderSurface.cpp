//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashD3D10RenderSurface.h"
#include "CMashD3D10Renderer.h"
#include "CMashD3D10Helper.h"
#include "MashLog.h"
namespace mash
{
	CMashD3D10RenderSurface::CMashD3D10RenderSurface(bool fitToScreen):m_pDepthStencilView(0), m_pRenderer(0),
		m_iWidth(0), m_iHeight(0), m_useMipmaps(false), m_depthOption(aDEPTH_OPTION_OWN_DEPTH), m_iTargetCount(0), 
		m_eDepthFormat(aFORMAT_DEPTH32_FLOAT), m_fitToScreen(fitToScreen), m_pDepthStencilTexture(0), m_autoViewport(true)

	{
		
	}

	CMashD3D10RenderSurface::~CMashD3D10RenderSurface()
	{
		m_pRenderer->_RemoveRenderSurface(this);

		if (m_pDepthStencilView)
		{
			m_pDepthStencilView->Release();
			m_pDepthStencilView = 0;
		}

		if (m_pDepthStencilTexture)
		{
			m_pRenderer->RemoveTextureFromCache(m_pDepthStencilTexture);

			m_pDepthStencilTexture->Drop();
			m_pDepthStencilTexture = 0;
		}

		MashArray<ID3D10RenderTargetView*>::Iterator rtvIter = m_RenderTargetViews.Begin();
		MashArray<ID3D10RenderTargetView*>::Iterator rtvIterEnd = m_RenderTargetViews.End();
		for(; rtvIter != rtvIterEnd; ++rtvIter)
		{
			if (*rtvIter)
				(*rtvIter)->Release();
		}

		m_RenderTargetViews.Clear();

		MashArray<CMashD3D10Texture*>::Iterator rtTextureIter = m_RenderTargetTextures.Begin();
		MashArray<CMashD3D10Texture*>::Iterator rtTextureIterEnd = m_RenderTargetTextures.End();
		for(; rtTextureIter != rtTextureIterEnd; ++rtTextureIter)
		{
			m_pRenderer->RemoveTextureFromCache(*rtTextureIter);

			if (*rtTextureIter)
				(*rtTextureIter)->Drop();
		}

		m_RenderTargetTextures.Clear();
	}

	void CMashD3D10RenderSurface::GenerateMips(int32 iSurface)
	{
		if (m_useMipmaps)
		{
			ID3D10Device *d3d10Device = m_pRenderer->GetD3D10Device();
			uint32 targetCount = m_RenderTargetTextures.Size();
			if (iSurface < 0)
			{
				for(uint32 i = 0; i < targetCount; ++i)
					d3d10Device->GenerateMips(m_RenderTargetTextures[i]->GetD3D10ResourceView());
			}
			else
			{
				if (iSurface < targetCount)
					d3d10Device->GenerateMips(m_RenderTargetTextures[iSurface]->GetD3D10ResourceView());
			}
		}
	}

	MashRenderSurface* CMashD3D10RenderSurface::Clone()const
	{
		CMashD3D10RenderSurface *pNewSurface = (CMashD3D10RenderSurface*)m_pRenderer->CreateRenderSurface(m_iWidth,
			m_iHeight, 
			&m_eTargetFormats[0],
			m_eTargetFormats.Size(),
			m_useMipmaps,
			m_depthOption,
			m_eDepthFormat);

		return pNewSurface;
	}

	mash::MashVector2 CMashD3D10RenderSurface::GetDimentions()const
	{
		return mash::MashVector2(m_iWidth, m_iHeight);
	}

	eMASH_STATUS CMashD3D10RenderSurface::CreateSurface(CMashD3D10Renderer *pRenderer, uint32 iWidth, uint32 iHeight, const eFORMAT *pFormats,
			uint32 iTargetCount, bool useMipmaps, eDEPTH_BUFFER_OPTIONS depthOption, eFORMAT eDepthFormat)
	{
		m_pRenderer = pRenderer;

		m_eTargetFormats.Clear();
		m_useMipmaps = useMipmaps;
		m_depthOption = depthOption;
		m_eDepthFormat = eDepthFormat;
		m_iTargetCount = iTargetCount;

		m_iHeight = iHeight;
		m_iWidth = iWidth;

		for(int32 i = 0; i < m_iTargetCount; ++i)
			m_eTargetFormats.PushBack(pFormats[i]);

		//create depth stencil texture
		D3D10_TEXTURE2D_DESC textureDesc;
		textureDesc.Width = m_iWidth;
		textureDesc.Height = m_iHeight;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = MashToD3D10Format(eDepthFormat);
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Usage = D3D10_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D10_BIND_DEPTH_STENCIL;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = 0;

		if (m_depthOption == eDEPTH_BUFFER_OPTIONS::aDEPTH_OPTION_OWN_DEPTH)
		{
			m_pDepthStencilTexture = (CMashD3D10Texture*)m_pRenderer->CreateTexture("", textureDesc);

			if (!m_pDepthStencilTexture)
			{
					MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Failed to create render target depth texture.", 
					"CMashD3D10RenderSurface::CreateSurface");
				return aMASH_FAILED;
			}

			m_pDepthStencilTexture->Grab();

			m_pDepthStencilView = 0;

			//create depth stencil view
			D3D10_DEPTH_STENCIL_VIEW_DESC viewDesc;
			viewDesc.Format = textureDesc.Format;
			viewDesc.ViewDimension = D3D10_DSV_DIMENSION_TEXTURE2D;
			viewDesc.Texture2D.MipSlice = 0;
			if (FAILED(m_pRenderer->GetD3D10Device()->CreateDepthStencilView(m_pDepthStencilTexture->GetD3D10Buffer(), &viewDesc, &m_pDepthStencilView)))
			{
				MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Failed to create render target depth view.", 
					"CMashD3D10RenderSurface::CreateSurface");
				return aMASH_FAILED;
			}
		}
		else if (m_depthOption == eDEPTH_BUFFER_OPTIONS::aDEPTH_OPTION_SHARE_MAIN_DEPTH)
		{
			//use the default depth target
			m_pDepthStencilView = m_pRenderer->GetD3D10DefaultDepthStencilView();
			if (m_pDepthStencilView)
			{
				m_pDepthStencilView->AddRef();
			}
		}
		else
		{
			//no depth
			m_pDepthStencilView = 0;
		}
		
		if (m_useMipmaps)
		{
			textureDesc.MiscFlags = D3D10_RESOURCE_MISC_GENERATE_MIPS;
			textureDesc.MipLevels = 0;
		}
		else
		{
			textureDesc.MiscFlags = 0;
			textureDesc.MipLevels = 1;
		}
		textureDesc.BindFlags = D3D10_BIND_RENDER_TARGET | D3D10_BIND_SHADER_RESOURCE;

		for(int32 i = 0; i < iTargetCount; ++i)
		{
			textureDesc.Format = MashToD3D10Format(pFormats[i]);
			
			CMashD3D10Texture *pTargetTexture = (CMashD3D10Texture*)m_pRenderer->CreateTexture("", textureDesc);

			if (!pTargetTexture)
			{
				MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Failed to create render target texture.", 
					"CMashD3D10RenderSurface::CreateSurface");
				return aMASH_FAILED;
			}

			pTargetTexture->Grab();

			ID3D10RenderTargetView *pTargetView = 0;
			if (FAILED(m_pRenderer->GetD3D10Device()->CreateRenderTargetView(pTargetTexture->GetD3D10Buffer(), NULL, &pTargetView)))
			{
				MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Failed to create render target view.", 
					"CMashD3D10RenderSurface::CreateSurface");
				return aMASH_FAILED;
			}

			m_RenderTargetTextures.PushBack(pTargetTexture);
			m_RenderTargetViews.PushBack(pTargetView);
		}

		return aMASH_OK;
	}

	eMASH_STATUS CMashD3D10RenderSurface::OnSet(int32 iSurface)
	{
		/*
			Removes textures from shader use so they can be mounted as render targets
		*/
		m_pRenderer->ResetUsedShaderResources();

		const int32 iTargetCount = m_RenderTargetViews.Size();
		if (iTargetCount > 0)
		{
			if (m_autoViewport)
			{
				sMashViewPort viewPort;
				viewPort.x = 0;
				viewPort.y = 0;
				viewPort.width = m_iWidth;
				viewPort.height = m_iHeight;
				viewPort.minZ = 0.0f;
				viewPort.maxZ = 1.0f;
				
				m_pRenderer->SetViewport(viewPort);
			}

			if (iSurface == -1)
			{
				m_pRenderer->GetD3D10Device()->OMSetRenderTargets(iTargetCount, &m_RenderTargetViews[0], m_pDepthStencilView);
				return aMASH_OK;
			}
			else if (iSurface < iTargetCount)
			{
				m_pRenderer->GetD3D10Device()->OMSetRenderTargets(1, &m_RenderTargetViews[iSurface], m_pDepthStencilView);
				return aMASH_OK;
			}
		}

		MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Failed to set render surface for rendering. No render target views were created.", 
					"CMashD3D10RenderSurface::OnSet");

		return aMASH_FAILED;
	}

	void CMashD3D10RenderSurface::OnDismount()
	{
		/*
			Removes resource views so the final textures can be set as textures, if needed
		*/
		m_pRenderer->ResetUsedShaderResources();
	}

	eMASH_STATUS CMashD3D10RenderSurface::OnPreResize()
	{
		if (m_fitToScreen)
		{
			if (m_pDepthStencilView)
			{
				m_pDepthStencilView->Release();
				m_pDepthStencilView = 0;
			}

			if (m_pDepthStencilTexture)
			{
				m_pRenderer->RemoveTextureFromCache(m_pDepthStencilTexture);
				m_pDepthStencilTexture->Drop();
				m_pDepthStencilTexture = 0;
			}

			for(uint32 i = 0; i < m_RenderTargetViews.Size(); ++i)
			{
				m_RenderTargetViews[i]->Release();
			}
			m_RenderTargetViews.Clear();

			for(uint32 i = 0; i < m_RenderTargetTextures.Size(); ++i)
			{
				m_pRenderer->RemoveTextureFromCache(m_RenderTargetTextures[i]);
				m_RenderTargetTextures[i]->Drop();
			}
			m_RenderTargetTextures.Clear();
		}

		return aMASH_OK;
	}

	eMASH_STATUS CMashD3D10RenderSurface::OnPostResize(uint32 width, uint32 height)
	{
		if (m_fitToScreen)
		{
			MashArray<eFORMAT> targetFormats = m_eTargetFormats;
			return CreateSurface(m_pRenderer, width, height, &targetFormats[0], targetFormats.Size(), m_useMipmaps, m_depthOption, m_eDepthFormat);
		}

		return aMASH_OK;	
	}

	void CMashD3D10RenderSurface::_ClearTargets(uint32 iClearFlags, const sMashColour4 &colour, float fZDepth)
	{
		if (iClearFlags & aCLEAR_TARGET)
		{
			const uint32 iTargetCount = m_RenderTargetViews.Size();
			for(uint32 i = 0; i < iTargetCount; ++i)
				m_pRenderer->GetD3D10Device()->ClearRenderTargetView(m_RenderTargetViews[i], colour.v);
		}

		if (m_pDepthStencilView && ((iClearFlags & aCLEAR_DEPTH) || (iClearFlags & aCLEAR_STENCIL)))
			m_pRenderer->GetD3D10Device()->ClearDepthStencilView(m_pDepthStencilView, MashToD3D10ClearFlags(iClearFlags), fZDepth, 0);
	}
}