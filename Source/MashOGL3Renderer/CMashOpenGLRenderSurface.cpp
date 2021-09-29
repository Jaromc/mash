//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashOpenGLRenderSurface.h"
#include "CMashOpenGLRenderer.h"
#include "MashLog.h"
namespace mash
{
	CMashOpenGLRenderSurface::CMashOpenGLRenderSurface(CMashOpenGLRenderer *renderer,
			MashArray<eFORMAT> &targetFormats,
			bool useMipmaps,
			uint32 width,
			uint32 height,
			eDEPTH_BUFFER_OPTIONS depthOptions,
			bool fitToScreen):MashRenderSurface(), m_renderer(renderer), m_oglFBOIndex(-1),
			m_depthOptions(depthOptions), m_oglRBOIndex(-1),
			m_targetFormats(targetFormats), m_useMipmaps(useMipmaps), m_width(width), m_height(height),
			m_usesDefaultDepthBuffer(false), m_fitToScreen(fitToScreen), m_autoViewport(true)
	{

	}

	CMashOpenGLRenderSurface::~CMashOpenGLRenderSurface()
	{
		m_renderer->_RemoveRenderSurface(this);
        
        if (m_oglFBOIndex != (uint32)-1)
        {
            glDeleteFramebuffersPtr(1, &m_oglFBOIndex);
            m_oglFBOIndex = -1;
        }
        if ((m_oglRBOIndex != (uint32)-1) && !m_usesDefaultDepthBuffer)
        {
            glDeleteRenderbuffersPtr(1, &m_oglRBOIndex);
             m_oglRBOIndex = -1;
        }
        
        MashArray<CMashOpenGLTexture*>::Iterator texIter = m_textures.Begin();
        MashArray<CMashOpenGLTexture*>::Iterator texIterEnd = m_textures.End();
        for(; texIter != texIterEnd; ++texIter)
        {
			m_renderer->RemoveTextureFromCache(*texIter);
			(*texIter)->Drop();
        }
        
        m_textures.Clear();
	}

	MashRenderSurface* CMashOpenGLRenderSurface::Clone()const
	{
		if (m_targetFormats.Size() == 0)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
				"Failed to clone render surface.", 
				"CMashOpenGLRenderSurface::Clone");

			return 0;
		}

		MashRenderSurface *newSurface = m_renderer->CreateRenderSurface(m_width, m_height, &m_targetFormats[0], 
			m_targetFormats.Size(), m_useMipmaps, m_depthOptions);

		if (!newSurface)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
				"Failed to clone render surface.", 
				"CMashOpenGLRenderSurface::Clone");
		}

		return newSurface;
	}

	mash::MashVector2 CMashOpenGLRenderSurface::GetDimentions()const
	{
		if (m_textures.Empty())
			return mash::MashVector2(0, 0);

		uint32 width, height;
		m_textures[0]->GetSize(width, height);
		return mash::MashVector2(width, height);
	}

	eMASH_STATUS CMashOpenGLRenderSurface::Initialise()
	{
		for(uint32 i = 0; i < m_targetFormats.Size(); ++i)
		{
			CMashOpenGLTexture *tex = (CMashOpenGLTexture*)m_renderer->AddTexture("", m_width, m_height, m_useMipmaps, aUSAGE_RENDER_TARGET, m_targetFormats[i]);
			if (!tex)
			{
				MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Failed to create render surface texture.", 
					"CMashOpenGLRenderer::CreateRenderSurface");

				return aMASH_FAILED;
			}

			tex->Grab();

			glBindTexture(GL_TEXTURE_2D, tex->GetOpenGLIndex());
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			if (m_useMipmaps)
			{
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glGenerateMipmapPtr(GL_TEXTURE_2D);
			}
			else
			{
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			}

			glBindTexture(GL_TEXTURE_2D, 0);

			m_textures.PushBack(tex);
		}
        
        if (m_depthOptions == aDEPTH_OPTION_OWN_DEPTH)
		{
            glGenRenderbuffersPtr(1, &m_oglRBOIndex);
            glBindRenderbufferPtr(GL_RENDERBUFFER, m_oglRBOIndex);
            glRenderbufferStoragePtr(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_width, m_height);
            m_usesDefaultDepthBuffer = false;
		}
		else if (m_depthOptions == aDEPTH_OPTION_SHARE_MAIN_DEPTH)
		{
			m_oglRBOIndex = m_renderer->GetOGLDefaultDepthBuffer();
			m_usesDefaultDepthBuffer = true;
		}
        else
        {
            //no depth
        }
		
        glGenFramebuffersPtr(1, &m_oglFBOIndex);
        glBindFramebufferPtr(GL_FRAMEBUFFER, m_oglFBOIndex);

		for(uint32 i = 0; i < m_targetFormats.Size(); ++i)
		{
			GLenum att = GL_COLOR_ATTACHMENT0;

			switch(i)
			{
			case 0:
				att = GL_COLOR_ATTACHMENT0;
				break;
			case 1:
				att = GL_COLOR_ATTACHMENT1;
				break;
			case 2:
				att = GL_COLOR_ATTACHMENT2;
				break;
			case 3:
				att = GL_COLOR_ATTACHMENT3;
				break;
			case 4:
				att = GL_COLOR_ATTACHMENT4;
				break;
			case 5:
				att = GL_COLOR_ATTACHMENT5;
				break;
			case 6:
				att = GL_COLOR_ATTACHMENT6;
				break;
			case 7:
				att = GL_COLOR_ATTACHMENT7;
				break;
			case 8:
				att = GL_COLOR_ATTACHMENT8;
				break;
			case 9:
				att = GL_COLOR_ATTACHMENT9;
				break;
			case 10:
				att = GL_COLOR_ATTACHMENT10;
				break;
			default:
				{
					MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Failed to create render surface. A maximum of 10 targets are supported.", 
					"CMashOpenGLRenderer::CreateRenderSurface");

					return aMASH_FAILED;
				}
			}

			m_attachmentList.PushBack(att);

            glFramebufferTexture2DPtr(GL_FRAMEBUFFER, att, GL_TEXTURE_2D, m_textures[i]->GetOpenGLIndex(), 0);
		}

        if (m_depthOptions != aDEPTH_OPTION_NO_DEPTH)
        {
            glFramebufferRenderbufferPtr(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_oglRBOIndex);
        }

        glDrawBuffersPtr((GLsizei)m_attachmentList.Size(), &m_attachmentList[0]);

        GLenum status = glCheckFramebufferStatusPtr(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
				"Failed to create render surface. Failed final check.", 
				"CMashOpenGLRenderer::CreateRenderSurface");

			return aMASH_FAILED;
		}

        glBindFramebufferPtr(GL_FRAMEBUFFER, 0);
		

		return aMASH_OK;
	}

	void CMashOpenGLRenderSurface::GenerateMips(int32 iSurface)
	{
		if (m_useMipmaps)
		{
			uint32 targetCount = m_textures.Size();
			if (iSurface < 0)
			{
				for(uint32 i = 0; i < targetCount; ++i)
				{
					glBindTexture(GL_TEXTURE_2D, m_textures[i]->GetOpenGLIndex());
                    glGenerateMipmapPtr(GL_TEXTURE_2D);
					glBindTexture(GL_TEXTURE_2D, 0);
				}
			}
			else
			{
				if (iSurface < targetCount)
				{
					glBindTexture(GL_TEXTURE_2D, m_textures[iSurface]->GetOpenGLIndex());
                    glGenerateMipmapPtr(GL_TEXTURE_2D);
					glBindTexture(GL_TEXTURE_2D, 0);
				}
			}
		}
	}

	eMASH_STATUS CMashOpenGLRenderSurface::OnSet(int32 iSurface)
	{
        glBindFramebufferPtr(GL_FRAMEBUFFER, m_oglFBOIndex);

        if (iSurface == -1)
        {
            glDrawBuffersPtr((GLsizei)m_attachmentList.Size(), &m_attachmentList[0]);
        }
        else
        {
            if (m_attachmentList.Size() == 1)
            {
                glDrawBuffersPtr(1, &m_attachmentList[0]);
            }
            else
            {
                MashArray<GLenum> attachmentList(m_attachmentList.Size());
                for(int32 i = 0; i < m_attachmentList.Size(); ++i)
                {
                    if (i == iSurface)
                    {
                        attachmentList[i] = m_attachmentList[i];
                    }
                    else
                    {
                        attachmentList[i] == GL_NONE;
                    }
                }
            }
        }

        if (m_autoViewport)
		{
			sMashViewPort viewPort;
			viewPort.x = 0;
			viewPort.y = 0;
			viewPort.width = m_width;
			viewPort.height = m_height;
			viewPort.minZ = 0.0f;
			viewPort.maxZ = 1.0f;
			m_renderer->SetViewport(viewPort);
		}
        
		return aMASH_OK;
	}

	void CMashOpenGLRenderSurface::OnDismount()
	{
        glBindFramebufferPtr(GL_FRAMEBUFFER, 0);
	}

	void CMashOpenGLRenderSurface::_ClearTargets(uint32 iClearFlags, const sMashColour4 &colour, f32 fZDepth)
	{
		//done by the renderer
	}

	eMASH_STATUS CMashOpenGLRenderSurface::OnPreResize()
	{
		if (m_fitToScreen)
		{
			//clear old data
			if (m_oglFBOIndex != (uint32)-1)
			{
                glDeleteFramebuffersPtr(1, &m_oglFBOIndex);
				m_oglFBOIndex = -1;
			}
            
            if (m_depthOptions != aDEPTH_OPTION_NO_DEPTH)
            {
                if ((m_oglRBOIndex != (uint32)-1) && !m_usesDefaultDepthBuffer)
                {
                    glDeleteRenderbuffersPtr(1, &m_oglRBOIndex);
                }
            }

			m_oglRBOIndex = -1;

			for(uint32 i = 0; i < m_textures.Size(); ++i)
			{
				m_renderer->RemoveTextureFromCache(m_textures[i]);
				m_textures[i]->Drop();
			}

			m_textures.Clear();
			m_attachmentList.Clear();
		}

		return aMASH_OK;
	}

	eMASH_STATUS CMashOpenGLRenderSurface::OnPostResize(uint32 width, uint32 height)
	{
		//only resize if needed
		if (m_fitToScreen)
		{
			m_width = width;
			m_height = height;

			return Initialise();
		}

		return aMASH_OK;
	}
}
