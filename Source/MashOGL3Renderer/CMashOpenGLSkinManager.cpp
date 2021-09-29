//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashOpenGLSkinManager.h"
#include "CMashOpenGLRenderer.h"
#include "CMashOpenGLEffect.h"
#include "MashDevice.h"
#include "MashLog.h"
#include "MashHelper.h"

namespace mash
{
	CMashOpenGLSkinManager::CMashOpenGLSkinManager(mash::CMashOpenGLRenderer *pRenderer):MashMaterialManagerIntermediate(pRenderer),
		m_oglRenderer(pRenderer), m_nextUboBindingIndex(1)
	{
	}

	CMashOpenGLSkinManager::~CMashOpenGLSkinManager()
	{
        m_sharedUniformBuffers.clear();
	}

    void CMashOpenGLSkinManager::_OnSharedUniformBufferDelete(CMashOglSharedUniformBuffer *buffer)
    {
        m_sharedUniformBuffers.erase(buffer->bufferName);
        m_freedUniformBufferBindings.push(buffer->bindingIndex);
    }

	CMashOglSharedUniformBuffer* CMashOpenGLSkinManager::GetSharedUniformBuffer(const int8 *name, uint32 bufferSize)
	{
		std::map<MashStringc, CMashOglSharedUniformBuffer*>::iterator iter = m_sharedUniformBuffers.find(name);
		if (iter != m_sharedUniformBuffers.end())
		{
			if (iter->second->bufferSize < bufferSize)
			{
				/*
					Another effect may use a uniform buffer of the same name, and of a different
					size. This check makes sure the shared buffer is big enough for anything that will
					use it.
				*/
				glBindBufferPtr(GL_UNIFORM_BUFFER, iter->second->uboIndex);
				glBufferDataPtr (GL_UNIFORM_BUFFER, bufferSize, 0, GL_DYNAMIC_DRAW);
				glBindBufferPtr(GL_UNIFORM_BUFFER, 0);

				iter->second->bufferSize = bufferSize;
			}
            
            iter->second->Grab();
            
			return iter->second;
		}

		//create a new buffer
		GLuint bindingIndex = 0;
		if (!m_freedUniformBufferBindings.empty())
		{
			bindingIndex = m_freedUniformBufferBindings.top();
			m_freedUniformBufferBindings.pop();
		}
		else
		{
			int32 maxUboBindings = 0;
			glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &maxUboBindings);
			if (m_nextUboBindingIndex > maxUboBindings)
			{
				int8 buffer[256];
				sprintf(buffer, "The maximum number of uniform buffers '%i' has been exceeded.", maxUboBindings);
				MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					buffer, 
					"CMashOpenGLSkinManager::GetSharedUniformBuffer");

				return 0;
			}

			bindingIndex = m_nextUboBindingIndex++;
		}
		
		GLuint uboIndex = 0;
		glGenBuffersPtr(1, &uboIndex);

		glBindBufferPtr(GL_UNIFORM_BUFFER, uboIndex);
		glBufferDataPtr (GL_UNIFORM_BUFFER, bufferSize, 0, GL_DYNAMIC_DRAW);
		
        glBindBufferBasePtr (GL_UNIFORM_BUFFER, bindingIndex, uboIndex);
		glBindBufferPtr(GL_UNIFORM_BUFFER, 0);
        
		CMashOglSharedUniformBuffer *newParam = MASH_NEW_COMMON CMashOglSharedUniformBuffer(this, name, uboIndex, bindingIndex, bufferSize);
		m_sharedUniformBuffers.insert(std::make_pair(name, newParam));

		return newParam;
	}

	MashEffect* CMashOpenGLSkinManager::CreateEffect()
	{
		return MASH_NEW_COMMON CMashOpenGLEffect((CMashOpenGLRenderer*)m_renderer);
	}

	bool CMashOpenGLSkinManager::IsProfileSupported(eSHADER_PROFILE profile)const
	{
		switch(profile)
		{
		case aSHADER_PROFILE_VS_GLSL:
			return true;
		case aSHADER_PROFILE_PS_GLSL:
			return true;
		case aSHADER_PROFILE_GS_GLSL:
			return true;
		};

		return false;
	}

	eSHADER_PROFILE CMashOpenGLSkinManager::GetLatestVertexProfile()const
	{
		return aSHADER_PROFILE_VS_GLSL;
	}

	eSHADER_PROFILE CMashOpenGLSkinManager::GetLatestFragmentProfile()const
	{
		return aSHADER_PROFILE_PS_GLSL;
	}

	eSHADER_PROFILE CMashOpenGLSkinManager::GetLatestGeometryProfile()const
	{
		return aSHADER_PROFILE_GS_GLSL;
	}

	const int8* CMashOpenGLSkinManager::_GetAPIShaderHeader()
	{
		eMASH_OPENGL_VERSION version = m_oglRenderer->GetOGLVersion();
		return mash::helpers::GetGLSLVersionAsString(version);
	}
}