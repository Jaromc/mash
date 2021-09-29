//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashOpenGLCubicRenderSurface.h"
#include "CMashOpenGLRenderer.h"
#include "CMashOpenGLHelper.h"
#include "MashLog.h"
namespace mash
{
	CMashOpenGLCubicRenderSurface::CMashOpenGLCubicRenderSurface(CMashOpenGLRenderer *renderer,
			eFORMAT targetFormat,
			bool useMipmaps,
			int32 width,
			int32 height,
			bool useDepthBuffer,
			eFORMAT depthFormat):MashRenderSurface(), m_renderer(renderer), m_oglFBOIndex(-1),
			m_useDepthBuffer(useDepthBuffer), m_oglRBOIndex(-1), m_depthFormat(depthFormat),
			m_targetFormat(targetFormat), m_useMipmaps(useMipmaps), m_width(width), m_height(height),
			m_texture(0), m_autoViewport(true)/*, m_usesDefaultDepthBuffer(false)*/
	{

	}

	CMashOpenGLCubicRenderSurface::~CMashOpenGLCubicRenderSurface()
	{
		m_renderer->_RemoveRenderSurface(this);

		if (m_texture)
		{
			m_renderer->RemoveTextureFromCache(m_texture);
			m_texture->Drop();
		}
        
        if (m_oglFBOIndex != (uint32)-1)
        {
            glDeleteFramebuffersPtr(1, &m_oglFBOIndex);
            m_oglFBOIndex = -1;
        }
        if ((m_oglRBOIndex != (uint32)-1))
        {
            glDeleteRenderbuffersPtr(1, &m_oglRBOIndex);
        }
	}

	MashRenderSurface* CMashOpenGLCubicRenderSurface::Clone()const
	{
		MashRenderSurface *newSurface = m_renderer->CreateCubicRenderSurface(m_width, m_useMipmaps, m_targetFormat, 
			m_useDepthBuffer, m_depthFormat);

		if (!newSurface)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
				"Failed to clone render surface.", 
				"CMashOpenGLCubicRenderSurface::Clone");
		}

		return newSurface;
	}

	mash::MashVector2 CMashOpenGLCubicRenderSurface::GetDimentions()const
	{
		if (!m_texture)
			return mash::MashVector2(0, 0);

		uint32 width, height;
		m_texture->GetSize(width, height);
		return mash::MashVector2(width, height);
	}

	eMASH_STATUS CMashOpenGLCubicRenderSurface::Initialise()
	{
		//clear old data
		if (m_oglFBOIndex != (uint32)-1)
        {
            glDeleteFramebuffersPtr(1, &m_oglFBOIndex);
            m_oglFBOIndex = -1;
        }
        if ((m_oglRBOIndex != (uint32)-1))
        {
            glDeleteRenderbuffersPtr(1, &m_oglRBOIndex);
        }

		if (m_texture)
		{
			m_renderer->RemoveTextureFromCache(m_texture);
			m_texture = 0;
		}

		int32 bufferSize = m_width;

		if (bufferSize == -1)
			bufferSize = m_renderer->GetBackBufferSize(false).x;

		m_texture = (CMashOpenGLCubeTexture*)m_renderer->AddCubeTexture("", bufferSize, m_useMipmaps, aUSAGE_RENDER_TARGET, m_targetFormat);
		if (!m_texture)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
				"Failed to create render surface texture.", 
				"CMashOpenGLCubicRenderSurface::CreateRenderSurface");

			return aMASH_FAILED;
		}

		m_texture->Grab();

		glBindTexture(GL_TEXTURE_CUBE_MAP, m_texture->GetOpenGLIndex());
		glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		
		glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		if (m_useMipmaps)
		{
			glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glGenerateMipmapPtr(GL_TEXTURE_CUBE_MAP);
		}
		else
		{
			glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		}

		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		
		glGenFramebuffersPtr(/*m_targetFormats.Size()*/1, &m_oglFBOIndex);
		glBindFramebufferPtr(GL_FRAMEBUFFER, m_oglFBOIndex);
		glFramebufferTexture2DPtr(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, m_texture->GetOpenGLIndex(), 0);
		glFramebufferTexture2DPtr(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_NEGATIVE_X, m_texture->GetOpenGLIndex(), 0);
		glFramebufferTexture2DPtr(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_Y, m_texture->GetOpenGLIndex(), 0);
		glFramebufferTexture2DPtr(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, m_texture->GetOpenGLIndex(), 0);
		glFramebufferTexture2DPtr(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_Z, m_texture->GetOpenGLIndex(), 0);
		glFramebufferTexture2DPtr(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, m_texture->GetOpenGLIndex(), 0);

		if (m_useDepthBuffer)
		{
			glGenRenderbuffersPtr(1, &m_oglRBOIndex);
			glBindRenderbufferPtr(GL_RENDERBUFFER, m_oglRBOIndex);
			glRenderbufferStoragePtr(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, bufferSize, bufferSize);
		}

		glFramebufferRenderbufferPtr(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_oglRBOIndex);
		glBindRenderbufferPtr(GL_RENDERBUFFER, 0);

		glDrawBuffer(GL_COLOR_ATTACHMENT0);

		GLenum status = glCheckFramebufferStatusPtr(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
				"Failed to create render surface. Failed final check.", 
				"CMashOpenGLCubicRenderSurface::CreateRenderSurface");

			return aMASH_FAILED;
		}

		glBindFramebufferPtr(GL_FRAMEBUFFER, 0);

		return aMASH_OK;
	}

	void CMashOpenGLCubicRenderSurface::GenerateMips(int32 iSurface)
	{
		if (m_useMipmaps)
		{
			glBindTexture(GL_TEXTURE_CUBE_MAP, m_texture->GetOpenGLIndex());
			glGenerateMipmapPtr(GL_TEXTURE_CUBE_MAP);
			glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		}
	}

	eMASH_STATUS CMashOpenGLCubicRenderSurface::OnSet(int32 iSurface)
	{
		glBindFramebufferPtr(GL_FRAMEBUFFER, m_oglFBOIndex);
		glFramebufferTexture2DPtr(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, MashToOpenGLCubeFace((eCUBEMAP_FACE)iSurface), m_texture->GetOpenGLIndex(), 0);

		if (m_autoViewport)
		{
			int32 bufferSize = m_width;

			if (bufferSize == -1)
				bufferSize = m_renderer->GetBackBufferSize(false).x;

		
			sMashViewPort viewPort;
			viewPort.x = 0;
			viewPort.y = 0;
			viewPort.width = bufferSize;
			viewPort.height = bufferSize;
			viewPort.minZ = 0.0f;
			viewPort.maxZ = 1.0f;

			m_renderer->SetViewport(viewPort);
		}

		return aMASH_OK;
	}

	void CMashOpenGLCubicRenderSurface::OnDismount()
	{
		glBindFramebufferPtr(GL_FRAMEBUFFER, 0);
	}

	void CMashOpenGLCubicRenderSurface::_ClearTargets(uint32 iClearFlags, const sMashColour4 &colour, f32 fZDepth)
	{
		//done by the renderer
	}

	eMASH_STATUS CMashOpenGLCubicRenderSurface::OnPreResize()
	{
		return aMASH_OK;
	}

	eMASH_STATUS CMashOpenGLCubicRenderSurface::OnPostResize(uint32 width, uint32 height)
	{
		return aMASH_OK;
	}
}