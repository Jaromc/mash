//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_TECHNIQUE_H_
#define _C_MASH_TECHNIQUE_H_

#include "MashTechniqueInstance.h"
#include "MashTypes.h"

namespace mash
{
	
	class MashVideo;

	class CMashTechniqueInstance : public MashTechniqueInstance
	{
	private:
		mash::MashVideo *m_pRenderer;
		MashStringc m_sTechniqueName;
		MashTechnique *m_sharedTechniqueData;
		sTexture m_textures[aMAX_TEXTURE_COUNT];

		uint32 m_iRenderKey;
		bool m_bRenderKeyIsDirty;
	public:
		CMashTechniqueInstance(mash::MashVideo *pRenderer, MashTechnique *reference);
		virtual ~CMashTechniqueInstance();

		MashTechniqueInstance* CreateIndependentCopy(const MashStringc &name, bool copyTextures = true);
		MashTechniqueInstance* CreateInstance(const MashStringc &name, bool copyTextures = true);

		void SetTexture(uint32 iIndex, MashTexture *pTexture);
		void SetTextureState(uint32 iIndex, MashTextureState *pState);
		const mash::sTexture* GetTexture(uint32 iIndex)const;

		MashTechnique* GetTechnique()const;

		const MashStringc& GetTechniqueName()const;
		
		uint32 GetRenderKey();

		eMASH_STATUS _OnSet();
		void _OnUnload();

		void _SetTechniqueName(const MashStringc &name);
	};

	inline MashTechnique* CMashTechniqueInstance::GetTechnique()const
	{
		return m_sharedTechniqueData;
	}

	inline const MashStringc& CMashTechniqueInstance::GetTechniqueName()const
	{
		return m_sTechniqueName;
	}
}

#endif