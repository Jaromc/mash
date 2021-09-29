//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_SHARED_TECHNIQUE_H_
#define _C_MASH_SHARED_TECHNIQUE_H_

#include "MashTechnique.h"

namespace mash
{
	class MashVideo;
	class MashRenderInfo;

	class CMashTechnique : public MashTechnique
	{
	private:
		struct sShadowEffect
		{
			MashEffect *shadowEffect;
			bool bIsShadowCompiled;
			bool bIsShadowValid;

			sShadowEffect():shadowEffect(0), bIsShadowCompiled(false), bIsShadowValid(false){}
		};
	private:
		MashVideo *m_renderer;
		static uint32 m_staticTechniqueCounter;
		uint32 m_techniqueId;
		bool m_bIsValid;
		bool m_bIsCompiled;
		MashEffect *m_effect;

		/*
			overrideLightingShading, This holds the file name of an effect that will override
			the default shading. Useful for different objects such
			as wood, metal, etc...
		*/
		MashStringc m_overrideLightShadingFile;

		TechniqueCallback *m_onSetCallback;

		sShadowEffect m_shadowCasters[aLIGHT_TYPE_COUNT];
		MashEffect *m_activePassEffect;
		eRENDER_STAGE m_activeRenderStage;

		int32 m_iBlendStateIndex;
		int32 m_iRasterizerStateIndex;
		bool m_bIsTransparent;

		eLIGHTING_TYPE m_lightingType;

		MashVertex *m_pVertexDeclaration;

		eRENDER_PASS m_activeRenderPass;
		bool m_renderPassNeedsUpdate;
		bool m_stateChangedSinceLastRender;

		bool m_containsValidShadowCaster;

		MashArray<uint16> m_lods;
		void OnSetActiveEffect(MashRenderInfo *renderInfo);
	public:
		CMashTechnique(MashVideo *renderer);
		~CMashTechnique();

		MashTechnique* CreateIndependentCopy();
		MashEffect* InitialiseShadowEffect(eLIGHTTYPE lightType);
		eMASH_STATUS CompileTechnique(MashFileManager *pFileManager, MashSceneManager *sceneManager, uint32 compileFlags, const sEffectMacro *args, uint32 argCount);
		
		void AddLodLevelSupport(uint16 iLodLevel);
		bool IsLodLevelSupported(uint16 iLodLevel)const;

		eLIGHTING_TYPE GetLightingType()const;
		void SetLightingType(eLIGHTING_TYPE type);
		bool IsTransparent()const;
		bool ContainsValidShadowCaster()const;
		uint32 GetTechniqueId()const;

		eRENDER_PASS GetRenderPass(MashSceneManager *sceneManager);

		void _SetVertexDeclaration(MashVertex *pVertexDeclaration);
		const MashVertex* GetVertexDeclaration()const;
		void SetBlendStateIndex(int32 iIndex);
		int32 GetBlendStateIndex()const;

		void SetTechniqueCallback(TechniqueCallback *pCallback);
		TechniqueCallback* GetTechniqueCallback()const;

		void SetCustomLightShadingEffect(const MashStringc &effectPath);
		const MashStringc& GetCustomLightShadingEffect()const;
		
		void SetRasteriserStateIndex(int32 iIndex);
		int32 GetRasterizerStateIndex()const;

		MashEffect* GetShadowEffect(eLIGHTTYPE lightType)const;

		MashEffect* GetEffect()const;
		bool IsValid()const;
		bool IsCompiled()const;

		MashEffect* GetActiveEffect()const;

		const MashArray<uint16>& GetSupportedLodList()const;

		eMASH_STATUS _OnSet(MashTechniqueInstance *techniqueInstance);
	};

	inline const MashStringc& CMashTechnique::GetCustomLightShadingEffect()const
	{
		return m_overrideLightShadingFile;
	}

	inline MashEffect* CMashTechnique::GetActiveEffect()const
	{
		return m_activePassEffect;
	}

	inline const MashArray<uint16>& CMashTechnique::GetSupportedLodList()const
	{
		return m_lods;
	}

	inline MashTechnique::TechniqueCallback* CMashTechnique::GetTechniqueCallback()const
	{
		return m_onSetCallback;
	}

	inline bool CMashTechnique::ContainsValidShadowCaster()const
	{
		return m_containsValidShadowCaster;
	}

	inline MashEffect* CMashTechnique::GetShadowEffect(eLIGHTTYPE lightType)const
	{
		return m_shadowCasters[lightType].shadowEffect;
	}

	inline MashEffect* CMashTechnique::GetEffect()const
	{
		return m_effect;
	}

	inline uint32 CMashTechnique::GetTechniqueId()const
	{
		return m_techniqueId;
	}

	inline bool CMashTechnique::IsValid()const
	{
		return m_bIsValid;
	}

	inline bool CMashTechnique::IsCompiled()const
	{
		return m_bIsCompiled;
	}

	inline eLIGHTING_TYPE CMashTechnique::GetLightingType()const
	{
		return m_lightingType;
	}

	inline const MashVertex* CMashTechnique::GetVertexDeclaration()const
	{
		return m_pVertexDeclaration;
	}
    
	inline bool CMashTechnique::IsTransparent()const
	{
		return m_bIsTransparent;
	}

	inline int32 CMashTechnique::GetRasterizerStateIndex()const
	{
		return m_iRasterizerStateIndex;
	}

	inline int32 CMashTechnique::GetBlendStateIndex()const
	{
		return m_iBlendStateIndex;
	}
}

#endif