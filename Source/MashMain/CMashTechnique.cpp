//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashTechnique.h"
#include "MashVideo.h"
#include "MashSceneManager.h"
#include "MashLight.h"
#include "MashMaterialManager.h"
#include "MashEffect.h"
#include "MashVertex.h"
#include "MashDevice.h"

namespace mash
{
	uint32 CMashTechnique::m_staticTechniqueCounter = 0;

	CMashTechnique::CMashTechnique(MashVideo *renderer):MashTechnique(), m_iBlendStateIndex(-1), m_iRasterizerStateIndex(-1),
			m_bIsTransparent(false), m_techniqueId(m_staticTechniqueCounter++), m_bIsValid(false), 
			m_bIsCompiled(false), m_pVertexDeclaration(0), m_lightingType(aLIGHT_TYPE_NONE),
			m_effect(0), m_renderPassNeedsUpdate(true), m_activeRenderPass(aPASS_SOLID), m_activeRenderStage(aRENDER_STAGE_SCENE),
			m_activePassEffect(0), m_containsValidShadowCaster(false),
			m_onSetCallback(0), m_renderer(renderer), m_overrideLightShadingFile(""), m_stateChangedSinceLastRender(false)
		{
			m_effect = m_renderer->GetMaterialManager()->CreateEffect();
			m_lods.PushBack(0);//every valid technique supports 0
		}

	CMashTechnique::~CMashTechnique()
	{
		if (m_effect)
		{
			m_effect->Drop();
			m_effect = 0;
		}

		if (m_onSetCallback)
		{
			m_onSetCallback->Drop();
			m_onSetCallback = 0;
		}

		for(uint32 i = 0; i < aLIGHT_TYPE_COUNT; ++i)
		{
			if (m_shadowCasters[i].shadowEffect)
			{
				m_shadowCasters[i].shadowEffect->Drop();
				m_shadowCasters[i].shadowEffect = 0;
			}
		}

		if (m_pVertexDeclaration)
		{
			m_pVertexDeclaration->Drop();
			m_pVertexDeclaration = 0;
		}
	}

	MashTechnique* CMashTechnique::CreateIndependentCopy()
	{
		CMashTechnique *newTechnique = (CMashTechnique*)m_renderer->GetMaterialManager()->_CreateTechnique();

		newTechnique->m_effect = m_effect->CreateIndependentCopy();
		for(uint32 i = 0; i < aLIGHT_TYPE_COUNT; ++i)
		{
			if (m_shadowCasters[i].shadowEffect)
			{
				newTechnique->m_shadowCasters[i].shadowEffect = m_shadowCasters[i].shadowEffect->CreateIndependentCopy();
				newTechnique->m_shadowCasters[i].bIsShadowCompiled = m_shadowCasters[i].bIsShadowCompiled;
				newTechnique->m_shadowCasters[i].bIsShadowValid = m_shadowCasters[i].bIsShadowValid;
			}
		}

		newTechnique->m_overrideLightShadingFile = m_overrideLightShadingFile;
		newTechnique->m_onSetCallback = m_onSetCallback;
		newTechnique->m_iBlendStateIndex = m_iBlendStateIndex;
		newTechnique->m_iRasterizerStateIndex = m_iRasterizerStateIndex;
		newTechnique->m_bIsTransparent = m_bIsTransparent;
		newTechnique->m_lightingType = m_lightingType;
		newTechnique->m_pVertexDeclaration = m_pVertexDeclaration;
		newTechnique->m_containsValidShadowCaster = m_containsValidShadowCaster;
		newTechnique->m_lods = m_lods;
		newTechnique->m_overrideLightShadingFile = m_overrideLightShadingFile;

		m_renderPassNeedsUpdate = true;

		m_activePassEffect = 0;

		return newTechnique;
	}

	MashEffect* CMashTechnique::InitialiseShadowEffect(eLIGHTTYPE lightType)
	{
		if (!m_shadowCasters[lightType].shadowEffect)
			m_shadowCasters[lightType].shadowEffect = m_renderer->GetMaterialManager()->CreateEffect();

		return m_shadowCasters[lightType].shadowEffect;
	}

	void CMashTechnique::SetCustomLightShadingEffect(const MashStringc &effectPath)
	{
		m_overrideLightShadingFile = effectPath;
	}

	void CMashTechnique::SetTechniqueCallback(TechniqueCallback *callback)
	{
		if (callback)
			callback->Grab();

		if (m_onSetCallback)
			m_onSetCallback->Drop();

		m_onSetCallback = callback;
	}

	void CMashTechnique::OnSetActiveEffect(MashRenderInfo *renderInfo)
	{
		MashEffect *lastEffect = renderInfo->GetEffect();
		m_activePassEffect = 0;

		/*
			reset state change flag if needed. Setting the lastEffect to null
			forces an update.
		*/
		if (m_stateChangedSinceLastRender)
		{
			lastEffect = 0;
			m_stateChangedSinceLastRender = false;
		}

		switch(MashDevice::StaticDevice->GetSceneManager()->GetActivePass())
		{
		case aRENDER_STAGE_SHADOW:
			{
				eLIGHTTYPE activeLightType = renderInfo->GetLight()->GetLightType();
				switch(activeLightType)
				{
				case aLIGHT_DIRECTIONAL:
					{
						if (m_shadowCasters[aLIGHT_DIRECTIONAL].bIsShadowValid)
						{
							m_activePassEffect = m_shadowCasters[aLIGHT_DIRECTIONAL].shadowEffect;
						}
						break;
					}
				case aLIGHT_SPOT:
					{
						if (m_shadowCasters[aLIGHT_SPOT].bIsShadowValid)
						{
							m_activePassEffect = m_shadowCasters[aLIGHT_SPOT].shadowEffect;
						}
						break;
					}
				case aLIGHT_POINT:
					{
						if (m_shadowCasters[aLIGHT_POINT].bIsShadowValid)
						{
							m_activePassEffect= m_shadowCasters[aLIGHT_POINT].shadowEffect;
						}
						break;
					}
				default:
					{
					}
				};

				break;
			}
		default:
			{
				if (m_bIsValid)
				{
					m_activePassEffect = m_effect;

					if (lastEffect != m_activePassEffect)
					{
						//set render states. Note this is only done for the main effect, not the shadow effects.
						//Shadow effect states are set by caster implimentations.
						m_renderer->SetRasteriserState(m_iRasterizerStateIndex);
						m_renderer->SetBlendState(m_iBlendStateIndex);
					}
				}
			}
		};

		if (m_activePassEffect && (lastEffect != m_activePassEffect))
		{
			if (lastEffect)
				lastEffect->_OnUnload(m_renderer->GetMaterialManager(), m_renderer->GetRenderInfo());

			//set shaders
			m_activePassEffect->_OnLoad(m_renderer->GetMaterialManager(), m_renderer->GetRenderInfo());

			m_renderer->_IncrementCurrentFrameTechniqueChanges();
		}
	}

	eMASH_STATUS CMashTechnique::_OnSet(MashTechniqueInstance *techniqueInstance)
	{
		OnSetActiveEffect(m_renderer->GetRenderInfo());

		if (m_activePassEffect)
		{
			m_renderer->GetRenderInfo()->SetTechnique(techniqueInstance);

			if (m_onSetCallback)
				m_onSetCallback->OnSet(techniqueInstance);

			m_activePassEffect->_OnUpdate(m_renderer->GetMaterialManager(), m_renderer->GetRenderInfo());

			return aMASH_OK;
		}

		return aMASH_FAILED;
	}

	void CMashTechnique::AddLodLevelSupport(uint16 iLodLevel)
	{
		const uint32 iLodSupportCount = m_lods.Size();
		for(uint32 i = 0; i < iLodSupportCount; ++i)
		{
			//make sure this lod has not already been added
			if (m_lods[i] == iLodLevel)
				return;
		}

		m_lods.PushBack(iLodLevel);
	}

	bool CMashTechnique::IsLodLevelSupported(uint16 iLodLevel)const
	{
		const uint32 iLodSupportCount = m_lods.Size();
		for(uint32 i = 0; i < iLodSupportCount; ++i)
		{
			if (m_lods[i] == iLodLevel)
				return true;
		}

		return false;
	}

	
	void CMashTechnique::SetLightingType(eLIGHTING_TYPE type)
	{
		//update pass info if needed
		if (m_lightingType != type)
			m_renderPassNeedsUpdate = true;

		m_lightingType = type;
	}

	eRENDER_PASS CMashTechnique::GetRenderPass(MashSceneManager *sceneManager)
	{
		//note this is only performed once for all instances
		if (m_renderPassNeedsUpdate == true)
		{
			eLIGHTING_TYPE lightingType = m_lightingType;

			//get scene lighting type if set to auto
			if (m_lightingType == aLIGHT_TYPE_AUTO)
				lightingType = sceneManager->GetPreferredLightingMode();

			if (m_bIsTransparent)
			{
				//no deferred option for transparent objects
				m_activeRenderPass = aPASS_TRANSPARENT;
			}
			else
			{
				if (lightingType == aLIGHT_TYPE_DEFERRED)
					m_activeRenderPass = aPASS_DEFERRED;
				else
					m_activeRenderPass = aPASS_SOLID;
			}

			//update complete for all instances
			m_renderPassNeedsUpdate = false;
		}

		return m_activeRenderPass;
	}

	void CMashTechnique::_SetVertexDeclaration(MashVertex *pVertexDeclaration)
	{
		if (pVertexDeclaration)
			pVertexDeclaration->Grab();

		if (m_pVertexDeclaration)
			m_pVertexDeclaration->Drop();

		m_pVertexDeclaration = pVertexDeclaration;
	}

	eMASH_STATUS CMashTechnique::CompileTechnique(MashFileManager *pFileManager, MashSceneManager *sceneManager, uint32 compileFlags, const sEffectMacro *args, uint32 argCount)
	{
		eLIGHTING_TYPE lightingType = m_lightingType;

		//set the preferred lighting type if the auto flag is set
		if (lightingType == aLIGHT_TYPE_AUTO)
			lightingType = sceneManager->GetPreferredLightingMode();

		sEffectCompileArgs compileArgs;
		/*
			Check if this technique should be compiled
			based on the flags given.

			Forward rendered effects need to be compiled on every lighting change.
			A scene may be made up from deferred and forward rendered effects.
			The aMATERIAL_COMPILER_FORWARD_RENDERED flags make sure we dont waste time building effects that dont need
			to be rebuilt.
		*/
		if (((compileFlags & aMATERIAL_COMPILER_EVERYTHING) || 
			((compileFlags & aMATERIAL_COMPILER_NON_COMPILED) && !m_bIsCompiled) ||
			((compileFlags & aMATERIAL_COMPILER_FORWARD_RENDERED) && ((lightingType == aLIGHT_TYPE_VERTEX) || (lightingType == aLIGHT_TYPE_PIXEL))) ||
			((compileFlags & aMATERIAL_COMPILER_AUTOS) && (m_lightingType == aLIGHT_TYPE_AUTO))))
		{
			if (lightingType != m_lightingType)
				m_renderPassNeedsUpdate = true;

			//////////////Normal effect start////////////////////////////
			//invalidate effect status
			m_bIsValid = false;
			m_bIsCompiled = false;

			MashMaterialManager *skinManager = m_renderer->GetMaterialManager();

			
			compileArgs.isShadowEffect = false;
			compileArgs.lightingType = lightingType;
			compileArgs.macros = args;
			compileArgs.macroCount = argCount;
			compileArgs.overrideLightShadingFile = m_overrideLightShadingFile;
			compileArgs.shadowEffectType = aLIGHT_TYPE_COUNT;

			if (m_effect->_Compile(pFileManager, skinManager, compileArgs) == aMASH_FAILED)
				return aMASH_FAILED;

			//compilation was successfull (no errors)
			m_bIsCompiled = true;

			//were they all valid?
			if (m_effect->IsValid())
				m_bIsValid = true;
			else
				m_bIsValid = false;
			//////////////Normal effect end////////////////////////////
		}
		//////////////Shadow effect start////////////////////////////
		for(uint32 i = 0; i < aLIGHT_TYPE_COUNT; ++i)
		{
			bool compileCaster = false;

			switch(i)
			{
			case aLIGHT_DIRECTIONAL:
				compileCaster = compileFlags & aMATERIAL_COMPILER_DIRECTIONAL_SHADOW_CASTERS;
				break;
			case aLIGHT_SPOT:
				compileCaster = compileFlags & aMATERIAL_COMPILER_SPOT_SHADOW_CASTERS;
				break;
			case aLIGHT_POINT:
				compileCaster = compileFlags & aMATERIAL_COMPILER_POINT_SHADOW_CASTERS;
				break;
			}

			if (compileCaster)
			{
				//invalidate effect status
				m_shadowCasters[i].bIsShadowValid = false;
				m_shadowCasters[i].bIsShadowCompiled = false;
				m_containsValidShadowCaster = false;

				if (m_shadowCasters[i].shadowEffect)
				{
					compileArgs.isShadowEffect = true;
					compileArgs.shadowEffectType = (eLIGHTTYPE)i;

					if (m_shadowCasters[i].shadowEffect->_Compile(pFileManager, m_renderer->GetMaterialManager(), compileArgs) == aMASH_FAILED)
						return aMASH_FAILED;

					//compilation was successfull (no errors)
					m_shadowCasters[i].bIsShadowCompiled = true;

					//were they all valid?
					if (m_shadowCasters[i].shadowEffect->IsValid())
					{
						m_shadowCasters[i].bIsShadowValid = true;

						if (!m_containsValidShadowCaster)
							m_containsValidShadowCaster = true;
					}
					else
						m_shadowCasters[i].bIsShadowValid = false;
				}
			}
		}
		//////////////Shadow effect end////////////////////////////

		return aMASH_OK;
	}

	void CMashTechnique::SetRasteriserStateIndex(int32 iIndex)
	{
		m_iRasterizerStateIndex = iIndex;
		m_stateChangedSinceLastRender = true;
	}

	void CMashTechnique::SetBlendStateIndex(int32 iIndex)
	{
		m_iBlendStateIndex = iIndex;
		const sBlendStates *blendState = m_renderer->GetBlendState(iIndex);

		/*
			Set transparent state so this techniques gets rendered
			in the correct pass
		*/
		if (blendState)
		{
			bool newTransparentState = false;
			if (blendState->blendingEnabled)
				newTransparentState = true;
			else
				newTransparentState = false;

			/*
				Render pass state will need updating if transparent
				state is changed
			*/
			if (newTransparentState != m_bIsTransparent)
			{
				m_renderPassNeedsUpdate = true;
				m_stateChangedSinceLastRender = true;
			}

			m_bIsTransparent = newTransparentState;

		}
	}

}