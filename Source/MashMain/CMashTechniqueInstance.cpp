//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashTechniqueInstance.h"
#include "MashVideo.h"
#include "MashSceneManager.h"
#include "MashTechnique.h"
#include "MashTexture.h"
#include "MashLog.h"
#include "MashDevice.h"
#include "MashMaterialManager.h"
#include "MashGenericArray.h"

namespace mash
{
	CMashTechniqueInstance::CMashTechniqueInstance(mash::MashVideo *pRenderer, MashTechnique *reference):m_sTechniqueName(""),
		m_sharedTechniqueData(0), m_pRenderer(pRenderer), m_bRenderKeyIsDirty(true)
	{
		m_sharedTechniqueData = reference;
		m_sharedTechniqueData->Grab();

		memset(m_textures, 0, sizeof(sTexture) * aMAX_TEXTURE_COUNT);
	}

	CMashTechniqueInstance::~CMashTechniqueInstance()
	{
		if (m_sharedTechniqueData)
		{
			m_sharedTechniqueData->Drop();
			m_sharedTechniqueData = 0;
		}

		for(uint32 i = 0; i < aMAX_TEXTURE_COUNT; ++i)
		{
			if (m_textures[i].state)
			{
				m_textures[i].state->Drop();
				m_textures[i].state = 0;
			}

			if (m_textures[i].texture)
			{
				m_textures[i].texture->Drop();
				m_textures[i].texture = 0;
			}
		}
	}

	MashTechniqueInstance* CMashTechniqueInstance::CreateIndependentCopy(const MashStringc &name, bool copyTextures)
	{
		MashTechnique *technique = m_sharedTechniqueData->CreateIndependentCopy();
		CMashTechniqueInstance *newTechniqueInstance = (CMashTechniqueInstance*)m_pRenderer->GetMaterialManager()->_CreateTechniqueInstance(technique);

		newTechniqueInstance->m_sTechniqueName = name;
		newTechniqueInstance->m_iRenderKey = m_iRenderKey;

		if (copyTextures)
		{
			for(uint32 i = 0; i < aMAX_TEXTURE_COUNT; ++i)
			{
				newTechniqueInstance->SetTextureState(i, m_textures[i].state);
				newTechniqueInstance->SetTexture(i, m_textures[i].texture);
			}
		}

		return newTechniqueInstance;
	}

	MashTechniqueInstance* CMashTechniqueInstance::CreateInstance(const MashStringc &name, bool copyTextures)
	{
		CMashTechniqueInstance *pNewTechnique = (CMashTechniqueInstance*)m_pRenderer->GetMaterialManager()->_CreateTechniqueInstance(this->GetTechnique());

		pNewTechnique->m_sTechniqueName = name;
		pNewTechnique->m_iRenderKey = m_iRenderKey;

		if (copyTextures)
		{
			for(uint32 i = 0; i < aMAX_TEXTURE_COUNT; ++i)
			{
				pNewTechnique->SetTextureState(i, m_textures[i].state);
				pNewTechnique->SetTexture(i, m_textures[i].texture);
			}
		}

		return pNewTechnique;
	}

	void CMashTechniqueInstance::_SetTechniqueName(const MashStringc &name)
	{
		m_sTechniqueName = name;
	}

	uint32 CMashTechniqueInstance::GetRenderKey()
	{
		if (m_bRenderKeyIsDirty)
		{
			m_iRenderKey = MashDevice::StaticDevice->GetSceneManager()->GenerateRenderKeyForTechnique(this);
            m_bRenderKeyIsDirty = false;
		}

		return m_iRenderKey;
	}

	void CMashTechniqueInstance::SetTexture(uint32 iIndex, MashTexture *pTexture)
	{
		MASH_DEBUG_IF (iIndex >= aMAX_TEXTURE_COUNT, 
			MASH_LOG_BOUNDS_ERROR(iIndex, 0, aMAX_TEXTURE_COUNT, "index", "CMashTechniqueInstance::SetTexture") 
			return;)

		if (pTexture)
			pTexture->Grab();

        if (pTexture != m_textures[iIndex].texture)
            m_bRenderKeyIsDirty = true;
        
		if (m_textures[iIndex].texture)
			m_textures[iIndex].texture->Drop();

		m_textures[iIndex].texture = pTexture;
	}

	void CMashTechniqueInstance::SetTextureState(uint32 iIndex, MashTextureState *pState)
	{
		MASH_DEBUG_IF (iIndex >= aMAX_TEXTURE_COUNT, 
			MASH_LOG_BOUNDS_ERROR(iIndex, 0, aMAX_TEXTURE_COUNT, "index", "CMashTechniqueInstance::SetTextureState") 
			return;)

		if (pState)
			pState->Grab();

		if (m_textures[iIndex].state)
			m_textures[iIndex].state->Drop();

		m_textures[iIndex].state = pState;
	}

	const mash::sTexture* CMashTechniqueInstance::GetTexture(uint32 iIndex)const
	{
		MASH_DEBUG_IF(iIndex >= aMAX_TEXTURE_COUNT, 
			MASH_LOG_BOUNDS_ERROR(iIndex, 0, aMAX_TEXTURE_COUNT, "index", "CMashTechniqueInstance::GetTexture")
			iIndex = aMAX_TEXTURE_COUNT - 1;)

		return &m_textures[iIndex];
	}

	eMASH_STATUS CMashTechniqueInstance::_OnSet()
	{
		return m_sharedTechniqueData->_OnSet(this);
	}

	void CMashTechniqueInstance::_OnUnload()
	{
	}
}
