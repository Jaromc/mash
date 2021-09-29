//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashOpenGLCubeTexture.h"
#include "CMashOpenGLRenderer.h"
#include "CMashOpenGLHelper.h"
#include "MashLog.h"
namespace mash
{
	CMashOpenGLCubeTexture::CMashOpenGLCubeTexture(CMashOpenGLRenderer *renderer, 
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
			m_name(name), m_openGLID(openGLID), m_engineID(engineID),
			m_usage(usage), m_width(width), m_height(height),
			m_pixelSizeInBytes(pixelSizeInBytes),
			m_sizeInBytesPerFace(0),m_pixelLayout(pixelLayout), m_pixelDataType(pixelDataType), m_useMipmaps(useMipmaps)
	{
		//per face
		m_sizeInBytesPerFace = width * height * pixelSizeInBytes;

		if (m_useMipmaps)
		{
			m_mipmapCount = (log((f32)math::Max<int32>(m_width,m_height))/log(2.0f)+1.0f+0.5f);
		}
		else
		{
			m_mipmapCount = 1;
		}

		//pbo for each face, if needed
		for(uint32 i = 0; i < 6; ++i)
			m_PBOOpenGLID[i] = -1;
	}

	CMashOpenGLCubeTexture::~CMashOpenGLCubeTexture()
	{
		glDeleteTextures(1, &m_openGLID);
	}

	void CMashOpenGLCubeTexture::GetSize(uint32 &iWidth, uint32 &iHeight)const
	{
		//return size per face
		iWidth = m_width;
		iHeight = m_height;
	}

	MashTexture* CMashOpenGLCubeTexture::Clone(const MashStringc &sName)const
	{
		//TODO
		CMashOpenGLCubeTexture *pNewTexture = 0;//(CMashOpenGLCubeTexture*)m_pVideo->AddTexture(sName, m_width, m_height, 1, m_usage, m_format);

		MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
				"Clone texture not currently supported in OGL.", 
				"CMashOpenGLCubeTexture::Clone");

		if (!pNewTexture)
			return 0;

		uint32 newTexGLId = pNewTexture->GetOpenGLIndex();
		
		void *pixelData = (void*)MASH_ALLOC_COMMON(m_sizeInBytesPerFace);

		//get pixel data
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_openGLID);
		glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, m_pixelLayout, m_pixelDataType, pixelData);
		//copy over pixel data
		glBindTexture(GL_TEXTURE_CUBE_MAP, newTexGLId);
		glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, 0, 0, m_width, m_height, m_pixelLayout, m_pixelDataType, pixelData);

		//get pixel data
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_openGLID);
		glGetTexImage(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, m_pixelLayout, m_pixelDataType, pixelData);
		//copy over pixel data
		glBindTexture(GL_TEXTURE_CUBE_MAP, newTexGLId);
		glTexSubImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, 0, 0, m_width, m_height, m_pixelLayout, m_pixelDataType, pixelData);

		//get pixel data
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_openGLID);
		glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, m_pixelLayout, m_pixelDataType, pixelData);
		//copy over pixel data
		glBindTexture(GL_TEXTURE_CUBE_MAP, newTexGLId);
		glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, 0, 0, m_width, m_height, m_pixelLayout, m_pixelDataType, pixelData);

		//get pixel data
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_openGLID);
		glGetTexImage(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, m_pixelLayout, m_pixelDataType, pixelData);
		//copy over pixel data
		glBindTexture(GL_TEXTURE_CUBE_MAP, newTexGLId);
		glTexSubImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, 0, 0, m_width, m_height, m_pixelLayout, m_pixelDataType, pixelData);

		//get pixel data
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_openGLID);
		glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, m_pixelLayout, m_pixelDataType, pixelData);
		//copy over pixel data
		glBindTexture(GL_TEXTURE_CUBE_MAP, newTexGLId);
		glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, 0, 0, m_width, m_height, m_pixelLayout, m_pixelDataType, pixelData);

		//get pixel data
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_openGLID);
		glGetTexImage(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, m_pixelLayout, m_pixelDataType, pixelData);
		//copy over pixel data
		glBindTexture(GL_TEXTURE_CUBE_MAP, newTexGLId);
		glTexSubImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, 0, 0, m_width, m_height, m_pixelLayout, m_pixelDataType, pixelData);

		MASH_FREE(pixelData);
		
		glBindTexture(GL_TEXTURE_2D, 0);

		return 0;
	}

	eMASH_STATUS CMashOpenGLCubeTexture::Lock(eBUFFER_LOCK eType, void **pData, uint32 iLevel, uint32 iFace)
	{
		if (m_PBOOpenGLID[iFace] == -1)
		{
			/*
				For dynamic buffers this will only be done once. For static it
				will be done on each Lock call. Ff this is being called
				frequently then dynamic buffers are the preferred method.
			*/
			glGenBuffersPtr(1, (GLuint*)&m_PBOOpenGLID[iFace]);

			/*
				Note : at this point there is no need to fill the PBO with
				initial data because the user doesn't have the option of
				lock read, only lock write.
			*/
		}

		glBindBufferPtr(GL_PIXEL_UNPACK_BUFFER, m_PBOOpenGLID[iFace]);

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
			glBufferDataPtr(GL_PIXEL_UNPACK_BUFFER, m_sizeInBytesPerFace, 0, flag);
		}
		
		*pData = glMapBufferPtr(m_PBOOpenGLID[iFace], GL_WRITE_ONLY);

		return aMASH_OK;
	}

	eMASH_STATUS CMashOpenGLCubeTexture::Unlock(uint32 iLevel, uint32 iFace)
	{
		if (glUnmapBufferPtr(m_PBOOpenGLID[iFace]) != GL_TRUE)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
				"Failed to unmap texture buffer.", 
				"CMashOpenGLCubeTexture::Unlock");

			return aMASH_FAILED;
		}

		glBindTexture(GL_TEXTURE_CUBE_MAP, m_openGLID);
		glBindBufferPtr(GL_PIXEL_UNPACK_BUFFER, m_PBOOpenGLID[iFace]);

		/*
			copy data from the PBO to the texture. The last param
			is set to NULL because in this case it is used as
			an offset.
		*/
		glTexSubImage2D(MashToOpenGLCubeFace((eCUBEMAP_FACE)iFace), iLevel, 0, 0, m_width, m_height, m_pixelLayout, m_pixelDataType, 0);

		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		glBindBufferPtr(GL_PIXEL_UNPACK_BUFFER, 0);

		/*
			Only keep the buffers around if they are dynamic. In theory, they shouldn't
			be needed if static buffers have been selected.
		*/
		if (m_usage != aUSAGE_DYNAMIC)
		{
			glDeleteBuffersPtr(1, (GLuint*)&m_PBOOpenGLID[iFace]);
			m_PBOOpenGLID[iFace] = -1;
		}

		return aMASH_OK;
	}
}