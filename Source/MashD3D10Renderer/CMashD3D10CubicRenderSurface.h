//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_D3D10_CUBIC_RENDER_SURFACE_H_
#define _C_MASH_D3D10_CUBIC_RENDER_SURFACE_H_

#include "MashDataTypes.h"

#ifdef MASH_WINDOWS
#include "MashRenderSurface.h"
#include "CMashD3D10CubeTexture.h"
#include <d3d10_1.h>

namespace mash
{
	class CMashD3D10Renderer;
	class CMashD3D10CubeTexture;

	const uint32 g_cubicRenderTargetCount = 6;

	class CMashD3D10CubicRenderSurface : public MashRenderSurface
	{
	private:
		CMashD3D10Renderer *m_pRenderer;
		ID3D10DepthStencilView *m_pDepthStencilView[g_cubicRenderTargetCount];
		CMashD3D10CubeTexture *m_pDepthStencilTexture;
		CMashD3D10CubeTexture *m_RenderTargetTexture;
		ID3D10RenderTargetView *m_RenderTargetView[g_cubicRenderTargetCount];

		uint32 m_iSize;
		bool m_useMipmaps;
		bool m_bUseDepth;
		eFORMAT m_eDepthFormat;
		int32 m_iTargetCount;
		eFORMAT m_eTargetFormat;
		uint32 m_currentSurface;
		bool m_autoViewport;
	public:
		CMashD3D10CubicRenderSurface();
		~CMashD3D10CubicRenderSurface();

		eMASH_STATUS CreateSurface(CMashD3D10Renderer *pRenderer, uint32 iSize, bool useMipmaps,
			eFORMAT eFormat, bool bUseDepth, eFORMAT eDepthFormat);

		void GenerateMips(int32 iSurface = -1);

		MashRenderSurface* Clone()const;
		eMASH_STATUS OnSet(int32 iSurface = -1);
		void OnDismount();
		MashTexture* GetTexture(uint32 iTexture)const;
		uint32 GetTextureCount()const;

		eUSAGE GetUsageType()const;

		mash::MashVector2 GetDimentions()const;
		eMASH_STATUS OnPreResize();
		eMASH_STATUS OnPostResize(uint32 width, uint32 height);

		void _ClearTargets(uint32 iClearFlags, const sMashColour4 &colour, float fZDepth);

		void SetAutoViewport(bool state);
		bool GetAutoViewport()const;
	};

	inline void CMashD3D10CubicRenderSurface::SetAutoViewport(bool state)
	{
		m_autoViewport = state;
	}

	inline bool CMashD3D10CubicRenderSurface::GetAutoViewport()const
	{
		return m_autoViewport;
	}

	inline eUSAGE CMashD3D10CubicRenderSurface::GetUsageType()const
	{
		return aUSAGE_RENDER_TARGET;
	}

	inline MashTexture* CMashD3D10CubicRenderSurface::GetTexture(uint32 iTexture)const
	{
		return m_RenderTargetTexture;
	}

	inline uint32 CMashD3D10CubicRenderSurface::GetTextureCount()const
	{
		return 1;
	}
}

#endif

#endif