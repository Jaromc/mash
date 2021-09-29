//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_D3D10_TEXTURE_H_
#define _C_MASH_D3D10_TEXTURE_H_

#include "MashDataTypes.h"

#ifdef MASH_WINDOWS
#include "MashTexture.h"
#include <d3d10_1.h>
#include <D3DX10tex.h>

namespace mash
{
	class CMashD3D10Renderer;

	class CMashD3D10Texture : public MashTexture
	{
	private:
		mash::CMashD3D10Renderer *m_pVideo;
		ID3D10Texture2D *m_pTexture;
		ID3D10ShaderResourceView *m_pTextureResourceView;
		D3D10_TEXTURE2D_DESC m_textureDesc;//TODO : Remove this because its keeped inside the texture
		MashStringc m_sName;
		uint32 m_UID;
		bool m_useMipmaps;
		uint32 m_mipmapCount;

	public:
		CMashD3D10Texture(mash::CMashD3D10Renderer *pVideo, 
			ID3D10Texture2D *pTexture,
			ID3D10ShaderResourceView *pResourceView,
			const D3D10_TEXTURE2D_DESC &desc, 
			uint32 iUID, 
			bool useMipmaps,
			const MashStringc &sName);
		~CMashD3D10Texture();

		MashTexture* Clone(const MashStringc &sName)const;
		eMASH_STATUS Lock(eBUFFER_LOCK eType, void **pData, uint32 iLevel = 0, uint32 iFace = 0);
		eMASH_STATUS Unlock(uint32 iLevel = 0, uint32 iFace = 0);
		eRESOURCE_TYPE GetType()const;
		const MashStringc& GetName()const;
		void GetSize(uint32 &iWidth, uint32 &iHeight)const;
		uint32 GetTextureID()const;
		uint32 GetMipmapCount()const;

		ID3D10Texture2D* GetD3D10Buffer()const;
		ID3D10ShaderResourceView* GetD3D10ResourceView()const;
		eUSAGE GetUsageType()const;
	};

	inline uint32 CMashD3D10Texture::GetMipmapCount()const
	{
		return m_mipmapCount;
	}

	inline ID3D10ShaderResourceView* CMashD3D10Texture::GetD3D10ResourceView()const
	{
		return m_pTextureResourceView;
	}

	inline ID3D10Texture2D* CMashD3D10Texture::GetD3D10Buffer()const
	{
		return m_pTexture;
	}

	inline uint32 CMashD3D10Texture::GetTextureID()const
	{
		return m_UID;
	}

	inline const MashStringc& CMashD3D10Texture::GetName()const
	{
		return m_sName;
	}

	inline eRESOURCE_TYPE CMashD3D10Texture::GetType()const
	{
        return aRESOURCE_TEXTURE;
	}
}

#endif

#endif
