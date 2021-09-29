//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_OGL_CUBIC_RENDER_SURFACE_H_
#define _C_MASH_OGL_CUBIC_RENDER_SURFACE_H_

#include "MashRenderSurface.h"
#include "CMashOpenGLHeader.h"
#include "CMashOpenGLCubeTexture.h"
#include "MashArray.h"

namespace mash
{
	class CMashOpenGLRenderer;

	class CMashOpenGLCubicRenderSurface : public MashRenderSurface
	{
	private:
		CMashOpenGLRenderer *m_renderer;
		uint32 m_oglFBOIndex;
		bool m_useDepthBuffer;
		uint32 m_oglRBOIndex;
		CMashOpenGLCubeTexture *m_texture;
		eFORMAT m_depthFormat;
		eFORMAT m_targetFormat;
		bool m_useMipmaps;
		MashArray<GLenum> m_attachmentList;

		/*
			Maybe set to -1 to fit the current viewport.
		*/
		int32 m_width;
		int32 m_height;
		bool m_autoViewport;
	public:
		CMashOpenGLCubicRenderSurface(CMashOpenGLRenderer *renderer,
			eFORMAT targetFormat,
			bool useMipmaps,
			int32 width,
			int32 height,
			bool useDepthBuffer,
			eFORMAT depthFormat);

		~CMashOpenGLCubicRenderSurface();

		eMASH_STATUS Initialise();

		void GenerateMips(int32 iSurface);
		MashRenderSurface* Clone()const;
		eMASH_STATUS OnSet(int32 iSurface = -1);
		void OnDismount();
		MashTexture* GetTexture(uint32 iTexture)const;
		uint32 GetTextureCount()const;
		eUSAGE GetUsageType()const;

		mash::MashVector2 GetDimentions()const;

		eMASH_STATUS OnPreResize();
		eMASH_STATUS OnPostResize(uint32 width, uint32 height);
		void _ClearTargets(uint32 iClearFlags, const sMashColour4 &colour, f32 fZDepth);

		void SetAutoViewport(bool state);
		bool GetAutoViewport()const;
	};

	inline void CMashOpenGLCubicRenderSurface::SetAutoViewport(bool state)
	{
		m_autoViewport = state;
	}

	inline bool CMashOpenGLCubicRenderSurface::GetAutoViewport()const
	{
		return m_autoViewport;
	}

	inline eUSAGE CMashOpenGLCubicRenderSurface::GetUsageType()const
	{
		return aUSAGE_RENDER_TARGET;
	}

	inline MashTexture* CMashOpenGLCubicRenderSurface::GetTexture(uint32 iTexture)const
	{
		return m_texture;
	}

	inline uint32 CMashOpenGLCubicRenderSurface::GetTextureCount()const
	{
		return 1;
	}
}

#endif