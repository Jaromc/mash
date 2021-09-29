//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_RENDER_INFO_H_
#define _C_MASH_RENDER_INFO_H_

#include "MashRenderInfo.h"
#include "MashMatrix4.h"
#include "MashMaterial.h"
#include "MashTechniqueInstance.h"
#include "CMashLight.h"
#include "MashTexture.h"
#include "MashCamera.h"
#include "MashShadowCaster.h"
#include "MashSceneManager.h"
#include "MashParticleSystem.h"
#include "MashEffect.h"
#include "MashSkin.h"
#include "MashTechnique.h"
namespace mash
{
	class CMashRenderInfo : public MashRenderInfo
	{
	private:
		MashTechniqueInstance *m_pTechnique;
		MashEffect *m_pEffect;
		void* m_pRenderObject;
		mash::MashMatrix4 m_mWorldTransform;
		mash::MashCamera *m_pCamera;
		int32 m_vertexType;
		mash::MashMatrix4 *m_pBonePalette;
		uint32 m_iBonePaletteMaxCount;
		uint32 m_currentBonePaletteCount;
		MashSkin *m_skin;
		MashParticleSystem *m_particleSystem;

		uint32 m_iLightBufferSizeInBytes;
		const sMashLight *m_pLightBuffer;
		MashLight *m_pLight;
		MashShadowCaster *m_pShadowCaster;

		/*
			TODO : Is there a way we can just access these
			from the renderer?
			It would save the hassle of resetting them
			when the device is lost
		*/
		MashTexture *m_pSceneDiffuseMap;
		MashTexture *m_pSceneLightMap;
		MashTexture *m_pSceneLightSpecMap;
		MashTexture *m_pSceneSpeculareMap;
		MashTexture *m_pSceneNormalMap;
		MashTexture *m_pSceneDepthMap;

		MashVertex *m_vertex;
	public:
		CMashRenderInfo():m_pTechnique(0),
			m_pRenderObject(0),
			m_vertexType(-1),
			m_pBonePalette(0),
			m_iBonePaletteMaxCount(0),
			m_currentBonePaletteCount(0),
			m_pCamera(0),
			m_pSceneDiffuseMap(0),
			m_pSceneLightMap(0),
			m_pSceneLightSpecMap(0),
			m_pSceneSpeculareMap(0),
			m_pSceneNormalMap(0),
			m_pSceneDepthMap(0),
			m_mWorldTransform(),
			m_pLight(0),
			m_pEffect(0),
			m_iLightBufferSizeInBytes(0),
			m_pShadowCaster(0),
			m_particleSystem(0),
			m_skin(0),
			m_vertex(0)
		{
		}

		virtual ~CMashRenderInfo()
		{
			if (m_pBonePalette)
				MASH_FREE(m_pBonePalette);

			m_pRenderObject = 0;
		}

		virtual MashTechniqueInstance* GetTechnique()const;
		MashEffect* GetEffect()const;
		virtual const void* GetRenderObject()const;
		virtual int32 GetVertexType()const;
		virtual mash::MashMatrix4* GetBonePalette()const;
		virtual uint32 GetMaximumBoneCount()const;
		uint32 GetCurrentBoneCount()const;
		virtual mash::MashCamera* GetCamera()const;
		const mash::MashMatrix4& GetWorldTransform()const;

		void SetSkin(MashSkin *skin);
		MashSkin* GetSkin()const;

		void SetCurrentBonePaletteSize(uint32 boneCount);
		void SetBonePaletteMinimumSize(uint32 boneCount);

		MashLight* GetLight()const;
		void SetLight(MashLight *pLight);

		MashTexture* GetSceneDiffuseMap()const;
		MashTexture* GetSceneLightMap()const;
		MashTexture* GetSceneLightSpecMap()const;
		MashTexture* GetSceneSpecularMap()const;
		MashTexture* GetSceneNormalMap()const;
		MashTexture* GetSceneDepthMap()const;

		void SetSceneDiffuseMap(MashTexture *pTexture){m_pSceneDiffuseMap = pTexture;}
		void SetSceneLightMap(MashTexture *pTexture){m_pSceneLightMap = pTexture;}
		void SetSceneLightSpecMap(MashTexture *pTexture){m_pSceneLightSpecMap = pTexture;}
		void SetSceneSpecularMap(MashTexture *pTexture){m_pSceneSpeculareMap = pTexture;}
		void SetSceneNormalMap(MashTexture *pTexture){m_pSceneNormalMap = pTexture;}
		void SetSceneDepthMap(MashTexture *pTexture){m_pSceneDepthMap = pTexture;}

		void SetWorldTransform(const mash::MashMatrix4 &mWorld);
		void SetTechnique(MashTechniqueInstance *pTechnique);
		void SetVertexType(int32 vertexType);
		void SetRenderObject(void *pObject);
		void SetCamera(mash::MashCamera *pCamera);

		void* GetLightBuffer(uint32 *sizeInBytesOut)const;

		const mash::MashParticleSystem* GetParticleSystem()const;
		void SetParticleSystem(MashParticleSystem *particleSystem);

		void SetShadowCaster(MashShadowCaster *pCaster);
		MashShadowCaster* GetShadowCaster()const;

		void SetLightBuffer(const sMashLight *lightArray, uint32 count);

		void SetVertex(MashVertex *vertexDecl);
		MashVertex* GetVertex()const;
	};

	inline MashVertex* CMashRenderInfo::GetVertex()const
	{
		return m_vertex;
	}

	inline void CMashRenderInfo::SetVertex(MashVertex *vertexDecl)
	{
		m_vertex = vertexDecl;
	}

	inline void CMashRenderInfo::SetSkin(MashSkin *skin)
	{
		m_skin = skin;
	}

	inline MashSkin* CMashRenderInfo::GetSkin()const
	{
		return m_skin;	
	}

	inline void CMashRenderInfo::SetParticleSystem(MashParticleSystem *particleSystem)
	{
		m_particleSystem = particleSystem;
	}

	inline const mash::MashParticleSystem* CMashRenderInfo::GetParticleSystem()const
	{
		return m_particleSystem;
	}

	inline void CMashRenderInfo::SetLightBuffer(const sMashLight *lightArray, uint32 count)
	{
		m_pLightBuffer = lightArray;
		m_iLightBufferSizeInBytes = sizeof(sMashLight) * count;
	}

	inline void CMashRenderInfo::SetShadowCaster(MashShadowCaster *pCaster)
	{
		m_pShadowCaster = pCaster;
	}

	inline MashShadowCaster* CMashRenderInfo::GetShadowCaster()const
	{
		return m_pShadowCaster;
	}

	inline MashLight* CMashRenderInfo::GetLight()const
	{
		return m_pLight;
	}

	inline void CMashRenderInfo::SetLight(MashLight *pLight)
	{
		m_pLight = pLight;
	}

	inline void* CMashRenderInfo::GetLightBuffer(uint32 *sizeInBytesOut)const
	{
		if (sizeInBytesOut)
			*sizeInBytesOut = m_iLightBufferSizeInBytes;

		return (void*)m_pLightBuffer;
	}

	inline MashTexture* CMashRenderInfo::GetSceneDiffuseMap()const
	{
		return m_pSceneDiffuseMap;
	}
	inline MashTexture* CMashRenderInfo::GetSceneLightMap()const
	{
		return m_pSceneLightMap;
	}
	inline MashTexture* CMashRenderInfo::GetSceneLightSpecMap()const
	{
		return m_pSceneLightSpecMap;
	}
	inline MashTexture* CMashRenderInfo::GetSceneSpecularMap()const
	{
		return m_pSceneSpeculareMap;
	}
	inline MashTexture* CMashRenderInfo::GetSceneNormalMap()const
	{
		return m_pSceneNormalMap;
	}
	inline MashTexture* CMashRenderInfo::GetSceneDepthMap()const
	{
		return m_pSceneDepthMap;
	}

	inline const mash::MashMatrix4& CMashRenderInfo::GetWorldTransform()const
	{
		return m_mWorldTransform;
	}

	inline void CMashRenderInfo::SetWorldTransform(const mash::MashMatrix4 &mWorld)
	{
		m_mWorldTransform = mWorld;
	}

	inline mash::MashCamera* CMashRenderInfo::GetCamera()const
	{
		return m_pCamera;
	}

	inline void CMashRenderInfo::SetCamera(mash::MashCamera *pCamera)
	{
		m_pCamera = pCamera;
	}

	inline mash::MashMatrix4* CMashRenderInfo::GetBonePalette()const
	{
		return m_pBonePalette;
	}

	inline uint32 CMashRenderInfo::GetMaximumBoneCount()const
	{
		return m_iBonePaletteMaxCount;
	}

	inline uint32 CMashRenderInfo::GetCurrentBoneCount()const
	{
		return m_currentBonePaletteCount;
	}

	inline void CMashRenderInfo::SetCurrentBonePaletteSize(uint32 boneCount)
	{
		m_currentBonePaletteCount = boneCount;
	}

	inline void CMashRenderInfo::SetBonePaletteMinimumSize(uint32 boneCount)
	{
		if (m_iBonePaletteMaxCount < boneCount)
		{
			MASH_FREE(m_pBonePalette);
			m_pBonePalette = MASH_ALLOC_T_COMMON(mash::MashMatrix4, boneCount);
			m_iBonePaletteMaxCount = boneCount;
		}
	}

	inline int32 CMashRenderInfo::GetVertexType()const
	{
		return m_vertexType;
	}

	inline void CMashRenderInfo::SetVertexType(int32 vertexType)
	{
		m_vertexType = vertexType;
	}

	inline void CMashRenderInfo::SetRenderObject(void *pObject)
	{
		m_pRenderObject = pObject;
	}

	inline MashEffect* CMashRenderInfo::GetEffect()const
	{
		return m_pEffect;
	}

	inline void CMashRenderInfo::SetTechnique(MashTechniqueInstance *pTechnique)
	{
		m_pTechnique = pTechnique;

		if (pTechnique)
			m_pEffect = m_pTechnique->GetTechnique()->GetActiveEffect();
		else
			m_pEffect = 0;
	}

	inline const void* CMashRenderInfo::GetRenderObject()const
	{
		return m_pRenderObject;
	}

	inline MashTechniqueInstance* CMashRenderInfo::GetTechnique()const
	{
		return m_pTechnique;
	}
}

#endif