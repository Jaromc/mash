//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_D3D10_RENDER_SURFACE_H_
#define _C_MASH_D3D10_RENDER_SURFACE_H_

#include "MashDataTypes.h"

#ifdef MASH_WINDOWS
#include "MashRenderSurface.h"
#include <d3d10_1.h>
#include "CMashD3D10Texture.h"
#include "MashArray.h"

namespace mash
{
	class CMashD3D10Renderer;

	class CMashD3D10RenderSurface : public MashRenderSurface
	{
	public:
		ID3D10DepthStencilView *m_pDepthStencilView;
	private:
		CMashD3D10Renderer *m_pRenderer;
		
		CMashD3D10Texture *m_pDepthStencilTexture;

		MashArray<ID3D10RenderTargetView*> m_RenderTargetViews;
		MashArray<CMashD3D10Texture*> m_RenderTargetTextures;

		bool m_fitToScreen;
		uint32 m_iWidth;
		uint32 m_iHeight;
		bool m_useMipmaps;
		eDEPTH_BUFFER_OPTIONS m_depthOption;
		eFORMAT m_eDepthFormat;
		int32 m_iTargetCount;
		MashArray<eFORMAT> m_eTargetFormats;
		bool m_autoViewport;
	public:
		CMashD3D10RenderSurface(bool fitToScreen);
		~CMashD3D10RenderSurface();

		eMASH_STATUS CreateSurface(CMashD3D10Renderer *pRenderer, uint32 iWidth, uint32 iHeight, const eFORMAT *pFormats,
			uint32 iTargetCount, bool useMipmaps, eDEPTH_BUFFER_OPTIONS depthOption, eFORMAT eDepthFormat);

		void GenerateMips(int32 iSurface = -1);

		MashRenderSurface* Clone()const;
		eMASH_STATUS OnSet(int32 iSurface = -1);
		void OnDismount();
		MashTexture* GetTexture(uint32 iTexture)const;
		uint32 GetTextureCount()const;
		eUSAGE GetUsageType()const;

		mash::MashVector2 GetDimentions()const;

		void _ClearTargets(uint32 iClearFlags, const sMashColour4 &colour, float fZDepth);
		eMASH_STATUS OnPreResize();
		eMASH_STATUS OnPostResize(uint32 width, uint32 height);

		void SetAutoViewport(bool state);
		bool GetAutoViewport()const;
	};

	inline void CMashD3D10RenderSurface::SetAutoViewport(bool state)
	{
		m_autoViewport = state;
	}

	inline bool CMashD3D10RenderSurface::GetAutoViewport()const
	{
		return m_autoViewport;
	}

	inline eUSAGE CMashD3D10RenderSurface::GetUsageType()const
	{
		return aUSAGE_RENDER_TARGET;
	}

	inline MashTexture* CMashD3D10RenderSurface::GetTexture(uint32 iTexture)const
	{
		return m_RenderTargetTextures[iTexture];
	}

	inline uint32 CMashD3D10RenderSurface::GetTextureCount()const
	{
		return m_RenderTargetTextures.Size();
	}
}

#endif

#endif