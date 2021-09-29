//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_OPENGL_TEXTURE_H_
#define _C_MASH_OPENGL_TEXTURE_H_

#include "MashTexture.h"
#include "CMashOpenGLHeader.h"
namespace mash
{
	class CMashOpenGLRenderer;

	class CMashOpenGLTexture : public MashTexture
	{
	private:
		CMashOpenGLRenderer *m_renderer;
		MashStringc m_name;
		uint32 m_openGLID;
		uint32 m_engineID;
		eUSAGE m_usage;

		bool m_useMipmaps;
		//TODO : Remove all this. Just fetch the data if needed.
		uint32 m_width;
		uint32 m_height;
		uint32 m_pixelSizeInBytes;
		uint32 m_sizeInBytes;
		GLenum m_pixelLayout;
		GLenum m_pixelDataType;
		uint32 m_mipmapCount;
		uint32 m_PBOOpenGLID;
	public:
		CMashOpenGLTexture(CMashOpenGLRenderer *renderer, 
			uint32 openGLID,
			uint32 engineID, 
			const MashStringc &name,
			bool useMipmaps,
			eUSAGE usage,
			uint32 width,
			uint32 height,
			GLenum pixelLayout,
			GLenum pixelDataType,
			uint32 pixelSizeInBytes);

		~CMashOpenGLTexture();

		MashTexture* Clone(const MashStringc &sName)const;
		eMASH_STATUS Lock(eBUFFER_LOCK eType, void **pData, uint32 iLevel = 0, uint32 iFace = 0);
		eMASH_STATUS Unlock(uint32 iLevel = 0, uint32 iFace = 0);
		eRESOURCE_TYPE GetType()const;
		const MashStringc& GetName()const;
		void GetSize(uint32 &iWidth, uint32 &iHeight)const;
		uint32 GetTextureID()const;
		uint32 GetMipmapCount()const;

		eUSAGE GetUsageType()const;

		uint32 GetOpenGLIndex()const;
	};

	inline uint32 CMashOpenGLTexture::GetMipmapCount()const
	{
		return m_mipmapCount;
	}

	inline uint32 CMashOpenGLTexture::GetOpenGLIndex()const
	{
		return m_openGLID;
	}

	inline eUSAGE CMashOpenGLTexture::GetUsageType()const
	{
		return m_usage;
	}

	inline uint32 CMashOpenGLTexture::GetTextureID()const
	{
		return m_engineID;
	}

	inline const MashStringc& CMashOpenGLTexture::GetName()const
	{
		return m_name;
	}

	inline eRESOURCE_TYPE CMashOpenGLTexture::GetType()const
	{
		return aRESOURCE_TEXTURE;
	}
}

#endif