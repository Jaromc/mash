//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashOpenGLTexture.h"
#include "CMashOpenGLRenderer.h"
#include "MashLog.h"
namespace mash
{
	CMashOpenGLTexture::CMashOpenGLTexture(CMashOpenGLRenderer *renderer, 
			uint32 openGLID,
			uint32 engineID, 
			const MashStringc &name,
			bool useMipmaps,
			eUSAGE usage,
			uint32 width,
			uint32 height,
			GLenum pixelLayout,
			GLenum pixelDataType,
			uint32 pixelSizeInBytes):MashTexture(),m_renderer(renderer),
			m_name(name), m_openGLID(openGLID), m_engineID(engineID), m_sizeInBytes(0),
			m_usage(usage), m_width(width), m_height(height), m_PBOOpenGLID(mash::math::MaxUInt32()), m_pixelSizeInBytes(pixelSizeInBytes),
			m_pixelLayout(pixelLayout), m_pixelDataType(pixelDataType), m_useMipmaps(useMipmaps)
	{
		m_sizeInBytes = width * height * pixelSizeInBytes;

		if (m_useMipmaps)
		{
			m_mipmapCount = (log((f32)math::Max<int32>(m_width,m_height))/log(2.0f)+1.0f+0.5f);
		}
		else
		{
			m_mipmapCount = 1;
		}
	}

	CMashOpenGLTexture::~CMashOpenGLTexture()
	{
		glDeleteTextures(1, &m_openGLID);
	}

	void CMashOpenGLTexture::GetSize(uint32 &iWidth, uint32 &iHeight)const
	{
		iWidth = m_width;
		iHeight = m_height;
	}

	MashTexture* CMashOpenGLTexture::Clone(const MashStringc &sName)const
	{
		//TODO
		CMashOpenGLTexture *pNewTexture = 0;//(CMashOpenGLTexture*)m_pVideo->AddTexture(sName, m_width, m_height, 1, m_usage, m_format);

		MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
				"Clone texture not currently supported in OGL.", 
				"CMashOpenGLTexture::Clone");

		if (!pNewTexture)
			return 0;

		uint32 newTexGLId = pNewTexture->GetOpenGLIndex();

		glBindTexture(GL_TEXTURE_2D, m_openGLID);

		//get pixel data
		void *pixelData = (void*)MASH_ALLOC_COMMON(m_sizeInBytes);
		glGetTexImage(GL_TEXTURE_2D, 0, m_pixelLayout, m_pixelDataType, pixelData);

		//copy over pixel data
		glBindTexture(GL_TEXTURE_2D, newTexGLId);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height, m_pixelLayout, m_pixelDataType, pixelData);

		MASH_FREE(pixelData);
		
		glBindTexture(GL_TEXTURE_2D, 0);

		return 0;
	}

	eMASH_STATUS CMashOpenGLTexture::Lock(eBUFFER_LOCK eType, void **pData, uint32 iLevel, uint32 iFace)
	{
		if (m_PBOOpenGLID == mash::math::MaxUInt32())
		{
			/*
				For dynamic buffers this will only be done once. For static it
				will be done on each Lock call. Ff this is being called
				frequently then dynamic buffers are the preferred method.
			*/
			glGenBuffersPtr(1, &m_PBOOpenGLID);

			/*
				Note : at this point there is no need to fill the PBO with
				initial data because the user doesn't have the option of
				lock read, only lock write.
			*/
		}

		glBindBufferPtr(GL_PIXEL_UNPACK_BUFFER, m_PBOOpenGLID);

		if (eType == aLOCK_WRITE_DISCARD)
		{
			uint32 flag = GL_STATIC_DRAW;
			if (m_usage == aUSAGE_DYNAMIC)
				flag = GL_DYNAMIC_DRAW;

			/*
				Calling Map causes CPU stalls while it waits for the GPU
				to finish with the data. To avoid this, we can call glBufferDataPtr with a NULL
				pointer before hand to create a new chunk of memory for us to work with and
				returns it immediatly.
			*/
			glBufferDataPtr(GL_PIXEL_UNPACK_BUFFER, m_sizeInBytes, 0, flag);
		}
		
		*pData = glMapBufferPtr(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);

		return aMASH_OK;
	}

	eMASH_STATUS CMashOpenGLTexture::Unlock(uint32 iLevel, uint32 iFace)
	{
		if (glUnmapBufferPtr(GL_PIXEL_UNPACK_BUFFER) != GL_TRUE)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
				"Failed to unmap texture buffer.", 
				"CMashOpenGLTexture::Unlock");

			return aMASH_FAILED;
		}

		glBindTexture(GL_TEXTURE_2D, m_openGLID);
		glBindBufferPtr(GL_PIXEL_UNPACK_BUFFER, m_PBOOpenGLID);

		/*
			copy data from the PBO to the texture. The last param
			is set to NULL because in this case it is used as
			an offset.
		*/
		glTexSubImage2D(GL_TEXTURE_2D, iLevel, 0, 0, m_width, m_height, /*m_pixelLayout, m_pixelDataType*/GL_RGBA, GL_FLOAT, 0);

		glBindBufferPtr(GL_PIXEL_UNPACK_BUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
		

		/*
			Only keep the buffers around if they are dynamic. In theory, they shouldn't
			be needed if static buffers have been selected.
		*/
		if (m_usage != aUSAGE_DYNAMIC)
		{
			glDeleteBuffersPtr(1, &m_PBOOpenGLID);
			m_PBOOpenGLID = mash::math::MaxUInt32();
		}

		return aMASH_OK;
	}
}