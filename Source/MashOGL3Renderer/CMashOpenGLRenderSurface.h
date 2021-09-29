//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_OGL_RENDER_SURFACE_H_
#define _C_MASH_OGL_RENDER_SURFACE_H_

#include "MashRenderSurface.h"
#include "CMashOpenGLTexture.h"
#include "MashArray.h"
namespace mash
{
	class CMashOpenGLRenderer;

	class CMashOpenGLRenderSurface : public MashRenderSurface
	{
	private:
		CMashOpenGLRenderer *m_renderer;
		uint32 m_oglFBOIndex;
		eDEPTH_BUFFER_OPTIONS m_depthOptions;
		uint32 m_oglRBOIndex;
		MashArray<CMashOpenGLTexture*> m_textures;
		MashArray<eFORMAT> m_targetFormats;
		bool m_useMipmaps;
		MashArray<GLenum> m_attachmentList;
		bool m_usesDefaultDepthBuffer;

		bool m_fitToScreen;
		uint32 m_width;
		uint32 m_height;
		bool m_autoViewport;
	public:
		CMashOpenGLRenderSurface(CMashOpenGLRenderer *renderer,
			MashArray<eFORMAT> &targetFormats,
			bool useMipmaps,
			uint32 width,
			uint32 height,
			eDEPTH_BUFFER_OPTIONS depthOptions,
			bool fitToScreen);

		~CMashOpenGLRenderSurface();

		eMASH_STATUS Initialise();

		void GenerateMips(int32 iSurface);

		MashRenderSurface* Clone()const;
		eMASH_STATUS OnSet(int32 iSurface = -1);
		void OnDismount();
		MashTexture* GetTexture(uint32 iTexture)const;
		uint32 GetTextureCount()const;
		eUSAGE GetUsageType()const;

		mash::MashVector2 GetDimentions()const;

		uint32 GetOGLFrameBuffer()const;
        uint32 GetOGLDepthRenderBuffer()const;

		eMASH_STATUS OnPreResize();
		eMASH_STATUS OnPostResize(uint32 width, uint32 height);
		void _ClearTargets(uint32 iClearFlags, const sMashColour4 &colour, f32 fZDepth);

		void SetAutoViewport(bool state);
		bool GetAutoViewport()const;
	};
    
    inline uint32 CMashOpenGLRenderSurface::GetOGLDepthRenderBuffer()const
    {
        return m_oglRBOIndex;
    }

	inline void CMashOpenGLRenderSurface::SetAutoViewport(bool state)
	{
		m_autoViewport = state;
	}

	inline bool CMashOpenGLRenderSurface::GetAutoViewport()const
	{
		return m_autoViewport;
	}

	inline uint32 CMashOpenGLRenderSurface::GetOGLFrameBuffer()const
	{
		return m_oglFBOIndex;
	}

	inline eUSAGE CMashOpenGLRenderSurface::GetUsageType()const
	{
		return aUSAGE_RENDER_TARGET;
	}

	inline MashTexture* CMashOpenGLRenderSurface::GetTexture(uint32 iTexture)const
	{
		if (iTexture >= m_textures.Size())
			return 0;

		return m_textures[iTexture];
	}

	inline uint32 CMashOpenGLRenderSurface::GetTextureCount()const
	{
		return m_textures.Size();
	}
}

#endif