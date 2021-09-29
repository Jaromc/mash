//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashSceneManager.h"
#include "MashVideo.h"
#include "CMashStaticMesh.h"
#include "CMashDynamicMesh.h"
#include "CMashMeshBuilder.h"
#include "CMashCamera.h"
#include "CMashBoneSceneNode.h"
#include "CMashModel.h"
#include "CMashEntityEx.h"
#include "CMashLight.h"
#include "MashMeshBuilder.h"
#include "CMashControllerManager.h"
#include "MashGeometryBatch.h"
#include "CMashCameraCull.h"
#include "CMashShadowCull.h"
#include "CMashDummy.h"
#include "CMashTriCollectionCached.h"
#include "CMashGPUParticleSystem.h"
#include "CMashMeshParticleSystem.h"
#include "CMashCPUParticleSystem.h"
#include "CMashSkin.h"
#include "CMashStaticDecal.h"
#include "CMashDynamicDecal.h"
#include "MashRenderInfo.h"
#include "MashGenericScriptReader.h"
#include "MashRenderSurface.h"

#include "CMashPrimitiveBatch.h"

#include "MashXMLWriter.h"

#include "CMashShadowDirCaster.h"
#include "CMashShadowSpotCaster.h"
#include "CMashShadowOmniCaster.h"
#include "CMashShadowCascadeCaster.h"
#include "MashMaterialManager.h"
#include "MashTexture.h"
#include "MashEffect.h"
#include "CMashTriangleBuffer.h"
#include "CMashColladaLoader.h"
#include "MashDevice.h"
#include "MashHelper.h"
#include "MashFileManager.h"
#include "CMashSceneLoader.h"
#include "CMashSceneWriter.h"
#include "MashRectangle2.h"
#include "CMashFreeCameraController.h"
#include "CMashCharacterMovementController.h"
#include "CMashEllipsoidColliderController.h"
#include "MashTimer.h"
#include "CMashTriColliderKDTree.h"

#include "CMashDirectionalCascadeCaster.h"
#include "CMashSpotShadowCaster.h"
#include "CMashPointShadowCaster.h"

#include "MashQuaternion.h"
#include "MashVector3.h"
#include "MashStringHelper.h"

namespace mash
{
	bool SortNodesByPointer(const MashSceneNode *a, const MashSceneNode *b)
	{
		return (a < b);
	}

	bool SortNodesByInternalID(const MashSceneNode *a, const MashSceneNode *b)
	{
		return (a->GetNodeID() < b->GetNodeID());
	}
    
    uint32 DefaultRenderKeyHashFunction(const MashTechniqueInstance *pTechnique)
	{
		uint32 hash;
		uint32 iTex0 = 0;
		uint32 iTex1 = 0;
		uint32 iTechniqueID = pTechnique->GetTechnique()->GetTechniqueId();
		const mash::sTexture *pTex = pTechnique->GetTexture(0);
		if (pTex && pTex->texture)
			iTex0 = pTex->texture->GetTextureID();
		pTex = pTechnique->GetTexture(1);
		if (pTex && pTex->texture)
			iTex1 = pTex->texture->GetTextureID();
        
		hash = (((iTex1)&0xff)<<24)|(((iTex0)&0xff)<<16)|((iTechniqueID)&0x0000ffff);
		return hash;
	}

	CMashSceneManager::CMashSceneManager(): 
		m_pRenderer(0), m_pInputManager(0),
		m_pActiveCamera(0),m_pCurrentSceneNode(0),
		m_pMeshBuilder(0),
		m_pControllerManager(0),
		m_iSceneNodeNameCounter(0),
		m_pGBufferRT(0),
		m_pGBufferLightRT(0),
		m_pFinalMaterial(0),
		m_pGBufferClearMaterial(0),
		m_eRenderPass(aRENDER_STAGE_SCENE),
		m_fDecalZBias(1.0f),
		m_bIsFogEnabled(false),
		m_reservedForwardLightBufferElements(0),
		m_forwardLightBuffer(0),
		m_pPrimitiveBatch(0),
		m_rebuildShaderState(0),
		m_deferredRendererLoadAttempted(false),
		m_deferredRendererValid(false),
		m_shadowEnabledDirLightCount(0),
		m_shadowEnabledSpotLightCount(0),
		m_shadowEnabledPointLightCount(0),
		m_activeSceneCullTechnique(0),
		m_activeShadowCullTechnique(0),
		m_castTransparentObjectShadows(false),
		m_isSceneInitializing(false),
		m_customViewportRT(0),
        m_pRenderKeyHashFunction(0)
	{
		m_pCurrentSceneNode = 0;

		for(uint32 i = 0; i < aLIGHT_TYPE_COUNT; ++i)
		{
			m_shadowMapDefaultSettings[i].bias = 0.005f;
			m_shadowMapDefaultSettings[i].drawDistance = 200;
			m_shadowMapDefaultSettings[i].textureFormat = aSHADOW_FORMAT_16;
			m_shadowMapDefaultSettings[i].textureSize = 512;
		}

		memset(m_pGBufferLighting, 0, sizeof(m_pGBufferLighting));

		memset(&m_sceneRenderInfo, 0, sizeof(sSceneRenderInfo));

		memset(m_shadowCasters, 0, sizeof(m_shadowCasters));
	}

	CMashSceneManager::~CMashSceneManager()
	{
		RemoveAllSceneObjects();

		if (m_pMeshBuilder)
		{
			MASH_DELETE m_pMeshBuilder;
			m_pMeshBuilder = 0;
		}

		if (m_pGBufferRT)
		{
			m_pGBufferRT->Drop();
			m_pGBufferRT = 0;
		}

		if (m_pGBufferLightRT)
		{
			m_pGBufferLightRT->Drop();
			m_pGBufferLightRT = 0;
		}

		for(uint32 i = 0; i < g_gbufferLightingType; ++i)
		{
			if (m_pGBufferLighting[i])
			{
				m_pGBufferLighting[i]->Drop();
				m_pGBufferLighting[i] = 0;
			}
		}

		if (m_pFinalMaterial)
		{
			m_pFinalMaterial->Drop();
			m_pFinalMaterial = 0;
		}

		if (m_pGBufferClearMaterial)
		{
			m_pGBufferClearMaterial->Drop();
			m_pGBufferClearMaterial = 0;
		}

		if (m_pPrimitiveBatch)
		{
			m_pPrimitiveBatch->Drop();
			m_pPrimitiveBatch = 0;
		}

		if (m_shadowCasters[aLIGHT_DIRECTIONAL].caster)
		{
			m_shadowCasters[aLIGHT_DIRECTIONAL].caster->Drop();
			m_shadowCasters[aLIGHT_DIRECTIONAL].caster = 0;
		}

		if (m_shadowCasters[aLIGHT_SPOT].caster)
		{
			m_shadowCasters[aLIGHT_SPOT].caster->Drop();
			m_shadowCasters[aLIGHT_SPOT].caster = 0;
		}

		if (m_shadowCasters[aLIGHT_POINT].caster)
		{
			m_shadowCasters[aLIGHT_POINT].caster->Drop();
			m_shadowCasters[aLIGHT_POINT].caster = 0;
		}

		if (m_activeSceneCullTechnique)
		{
			m_activeSceneCullTechnique->Drop();
			m_activeSceneCullTechnique = 0;
		}

		if (m_activeShadowCullTechnique)
		{
			m_activeShadowCullTechnique->Drop();
			m_activeShadowCullTechnique = 0;
		}

		if (m_pControllerManager)
		{
			m_pControllerManager->Drop();
			m_pControllerManager = 0;
		}
	}

	eMASH_STATUS CMashSceneManager::_Initialise(mash::MashVideo *pRenderer, mash::MashInputManager *pInputManager, const mash::sMashDeviceSettings &settings)
	{
		m_pRenderer = pRenderer;
		m_pInputManager = pInputManager;
        
		m_pMeshBuilder = MASH_NEW_COMMON CMashMeshBuilder(m_pRenderer);
		m_pControllerManager = MASH_NEW_COMMON CMashControllerManager();

		//load primitive batch
		MashMaterialManager *pSkinManager = pRenderer->GetMaterialManager();
		MashMaterial *primitiveBatchMaterial = pSkinManager->GetStandardMaterial(MashMaterialManager::aSTANDARD_MATERIAL_PRIMITIVE);

		if (!primitiveBatchMaterial)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
				"Failed to load primitive material",
				"CMashSceneManager::_Initialise");

			return aMASH_FAILED;
		}
		else
		{
			//init depth testing mode.
			primitiveBatchMaterial->SetActiveGroup("DepthTestingOn");
			m_lastPrimitiveDepthTest = true;

			m_pPrimitiveBatch = m_pRenderer->CreateGeometryBatch(primitiveBatchMaterial, aPRIMITIVE_LINE_LIST, MashGeometryBatch::aDYNAMIC);
			if (!m_pPrimitiveBatch)
			{
				MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
					"Failed to initialise line renderer",
					"CMashSceneManager::_Initialise");

				return aMASH_FAILED;
			}
		}

		m_pRenderer->_SetSceneManager(this);

		m_preferredLightingMode = settings.preferredLightingMode;

		//default to per pixel lighting
		if (m_preferredLightingMode == aLIGHT_TYPE_AUTO)
			m_preferredLightingMode = aLIGHT_TYPE_PIXEL;
		else if (m_preferredLightingMode == aLIGHT_TYPE_DEFERRED)
			_OnDeferredLightingModeEnabled();//creates the gbuffer
        
        MashCullTechnique *defaultCullTechnique = CreateCullTechnique(aCULL_TECH_CAMERA);
        SetCullTechnique(defaultCullTechnique);
		defaultCullTechnique->Drop();

		defaultCullTechnique = CreateCullTechnique(aCULL_TECH_SHADOW);
		SetCullTechniqueShadow(defaultCullTechnique);
		defaultCullTechnique->Drop();

		if (settings.enableDeferredRender)
			_OnDeferredLightingModeEnabled();

		return aMASH_OK;
	}

	void CMashSceneManager::RemoveAllSceneObjects()
	{
		//arnt grabbed
		m_lookatTrackers.Clear();
		m_forwardRenderedLightList.Clear();
        m_callbackNodes.Clear();

		if (m_forwardLightBuffer)
		{
			MASH_FREE(m_forwardLightBuffer);
			m_forwardLightBuffer = 0;
		}
		
		RemoveAllSceneNodes();

		MashArray<mash::MashMesh*>::Iterator initMeshIter = m_initialiseMeshLoadData.Begin();
		MashArray<mash::MashMesh*>::Iterator initMeshIterEnd = m_initialiseMeshLoadData.End();
		for(; initMeshIter != initMeshIterEnd; ++initMeshIter)
		{
			(*initMeshIter)->Drop();
		}

		m_initialiseMeshLoadData.Clear();		

		if (m_pActiveCamera)
		{
			m_pActiveCamera->Drop();
			m_pActiveCamera = 0;
		}

		if (m_pCurrentSceneNode)
		{
			m_pCurrentSceneNode->Drop();
			m_pCurrentSceneNode = 0;
		}
	}

	void CMashSceneManager::_SetCurrentScriptSceneNode(MashSceneNode *pNode)
	{
		if (pNode)
			pNode->Grab();

		if (m_pCurrentSceneNode)
			m_pCurrentSceneNode->Drop();

		m_pCurrentSceneNode = pNode;
	}

	MashLight* CMashSceneManager::GetFirstForwardRenderedLight()const
	{
		if (m_forwardRenderedLightList.Empty())
			return 0;

		return m_forwardRenderedLightList.Front();
	}
    
	void CMashSceneManager::_OnViewportChange()
	{
		if (m_pActiveCamera)
			m_pActiveCamera->_OnViewportChange();
	}

	void CMashSceneManager::_OnPostResolutionChange()
	{
		if (m_deferredRendererValid)
		{
			MashRenderInfo *pRenderInfo = m_pRenderer->GetRenderInfo();
			pRenderInfo->SetSceneDiffuseMap(GetDeferredDiffuseMap());
			pRenderInfo->SetSceneNormalMap(GetDeferredNormalMap());
			pRenderInfo->SetSceneSpecularMap(GetDeferredSpecularMap());
			pRenderInfo->SetSceneDepthMap(GetDeferredDepthMap());

			pRenderInfo->SetSceneLightMap(GetDeferredLightingMap());
			pRenderInfo->SetSceneLightSpecMap(GetDeferredLightingSpecularMap());
		}
	}

	eMASH_STATUS CMashSceneManager::CreateGBuffer()
	{
		if (!m_deferredRendererLoadAttempted)
		{
            MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_INFORMATION,
                             "Attempting to create the gbuffer",
                             "CMashSceneManager::CreateGBuffer");
            
			//set an init flag so we dont attempt to create the buffer many times on failure.
			m_deferredRendererLoadAttempted = true;
			m_deferredRendererValid = false;

			//build runtime shaders
			m_pRenderer->GetMaterialManager()->_RebuildDeferredLightingShaders();

			/*
				Setup Deffered renderer.

				Depth buffers are not needed cause they use the default buffer, which can then be
				used to continue rendering the forward rendered objects.
			*/
			mash::eFORMAT eRTFormats[4] = {aFORMAT_RGBA16_FLOAT, aFORMAT_RGBA16_FLOAT, aFORMAT_RGBA16_FLOAT, /*aFMT_R32F*/aFORMAT_R32_FLOAT};
			m_pGBufferRT = m_pRenderer->CreateRenderSurface(-1, -1, eRTFormats, 4, false, aDEPTH_OPTION_SHARE_MAIN_DEPTH, /*mash::aFMT_D24X8*/aFORMAT_DEPTH32_FLOAT);

			if (!m_pGBufferRT)
			{
                MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
                                 "Failed to create main gbuffer target.",
                                 "CMashSceneManager::CreateGBuffer");
                
				return aMASH_FAILED;
			}

			eRTFormats[0] = aFORMAT_RGBA16_FLOAT;
			eRTFormats[1] = aFORMAT_RGBA16_FLOAT;
			m_pGBufferLightRT = m_pRenderer->CreateRenderSurface(-1, -1, eRTFormats, 2,false, aDEPTH_OPTION_NO_DEPTH,aFORMAT_DEPTH32_FLOAT);

			if (!m_pGBufferLightRT)
			{
                MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
                                 "Failed to create lighting gbuffer target.",
                                 "CMashSceneManager::CreateGBuffer");
                
				return aMASH_FAILED;
			}

			MashMaterialManager *pSkinManager = m_pRenderer->GetMaterialManager();

			m_pGBufferLighting[mash::aLIGHT_DIRECTIONAL] = pSkinManager->GetStandardMaterial(MashMaterialManager::aSTANDARD_MATERIAL_GBUFFER_DIR_LIGHT);

			if (!m_pGBufferLighting[mash::aLIGHT_DIRECTIONAL])
			{
                MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
                                 "Failed to get gbuffer directional lighting material.",
                                 "CMashSceneManager::CreateGBuffer");
                
				return aMASH_FAILED;
			}

			m_pGBufferLighting[mash::aLIGHT_DIRECTIONAL]->Grab();

			m_pGBufferLighting[mash::aLIGHT_POINT] = pSkinManager->GetStandardMaterial(MashMaterialManager::aSTANDARD_MATERIAL_GBUFFER_POINT_LIGHT);

			if (!m_pGBufferLighting[mash::aLIGHT_POINT])
			{
                MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
                                 "Failed to get gbuffer point lighting material.",
                                 "CMashSceneManager::CreateGBuffer");
                
				return aMASH_FAILED;
			}

			m_pGBufferLighting[mash::aLIGHT_POINT]->Grab();

			m_pGBufferLighting[mash::aLIGHT_SPOT] = pSkinManager->GetStandardMaterial(MashMaterialManager::aSTANDARD_MATERIAL_GBUFFER_SPOT_LIGHT);

			if (!m_pGBufferLighting[mash::aLIGHT_SPOT])
			{
                MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
                                 "Failed to get gbuffer spot lighting material.",
                                 "CMashSceneManager::CreateGBuffer");
                
				return aMASH_FAILED;
			}

			m_pGBufferLighting[mash::aLIGHT_SPOT]->Grab();

			m_pFinalMaterial = pSkinManager->GetStandardMaterial(MashMaterialManager::aSTANDARD_MATERIAL_GBUFFER_COMBINE);

			if (!m_pFinalMaterial)
			{
                MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
                                 "Failed to get gbuffer combine material.",
                                 "CMashSceneManager::CreateGBuffer");
                
				return aMASH_FAILED;
			}
			
			m_pFinalMaterial->Grab();

			m_pGBufferClearMaterial = pSkinManager->GetStandardMaterial(MashMaterialManager::aSTANDARD_MATERIAL_GBUFFER_CLEAR);

			if (!m_pGBufferClearMaterial)
			{
                MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
                                 "Failed to get gbuffer clear material.",
                                 "CMashSceneManager::CreateGBuffer");
                
				return aMASH_FAILED;
			}
			m_pGBufferClearMaterial->Grab();

			//set up gbuffer texture targets
			MashRenderInfo *pRenderInfo = m_pRenderer->GetRenderInfo();

			pRenderInfo->SetSceneDiffuseMap(GetDeferredDiffuseMap());
			pRenderInfo->SetSceneNormalMap(GetDeferredNormalMap());
			pRenderInfo->SetSceneSpecularMap(GetDeferredSpecularMap());
			pRenderInfo->SetSceneDepthMap(GetDeferredDepthMap());

			pRenderInfo->SetSceneLightMap(GetDeferredLightingMap());
			pRenderInfo->SetSceneLightSpecMap(GetDeferredLightingSpecularMap());

			MashFileManager *fileManager = MashDevice::StaticDevice->GetFileManager();

			/*
				If the game loop has already been initialised then we need
				to force compile these
			*/
			if (MashDevice::StaticDevice->IsGameLoopInitialised())
			{
				m_pRenderer->GetMaterialManager()->_BeginBatchMaterialCompile();
				//compile materials.
				if (m_pGBufferLighting[mash::aLIGHT_SPOT]->CompileTechniques(fileManager, this, aMATERIAL_COMPILER_EVERYTHING) == aMASH_FAILED)
					return aMASH_FAILED;
				if (m_pGBufferLighting[mash::aLIGHT_POINT]->CompileTechniques(fileManager, this, aMATERIAL_COMPILER_EVERYTHING) == aMASH_FAILED)
					return aMASH_FAILED;
				if (m_pGBufferLighting[mash::aLIGHT_DIRECTIONAL]->CompileTechniques(fileManager, this, aMATERIAL_COMPILER_EVERYTHING) == aMASH_FAILED)
					return aMASH_FAILED;
				if (m_pFinalMaterial->CompileTechniques(fileManager, this, aMATERIAL_COMPILER_EVERYTHING) == aMASH_FAILED)
					return aMASH_FAILED;
				if (m_pGBufferClearMaterial->CompileTechniques(fileManager, this, aMATERIAL_COMPILER_EVERYTHING) == aMASH_FAILED)
					return aMASH_FAILED;
				m_pRenderer->GetMaterialManager()->_EndBatchMaterialCompile();
			}

			//we got this far, success!
			m_deferredRendererValid = true;
            
            MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_INFORMATION,
                             "gbuffer successfully created",
                             "CMashSceneManager::CreateGBuffer");
		}

		return aMASH_OK;
	}

	mash::MashTexture* CMashSceneManager::GetDeferredDiffuseMap()const
	{
		if (!m_pGBufferRT)
			return 0;

		return m_pGBufferRT->GetTexture(0);
	}

	mash::MashTexture* CMashSceneManager::GetDeferredNormalMap()const
	{
		if (!m_pGBufferRT)
			return 0;

		return m_pGBufferRT->GetTexture(1);
	}

	mash::MashTexture* CMashSceneManager::GetDeferredSpecularMap()const
	{
		if (!m_pGBufferRT)
			return 0;

		return m_pGBufferRT->GetTexture(2);
	}

	mash::MashTexture* CMashSceneManager::GetDeferredDepthMap()const
	{
		if (!m_pGBufferRT)
			return 0;

		return m_pGBufferRT->GetTexture(3);
	}

	mash::MashTexture* CMashSceneManager::GetDeferredLightingMap()const
	{
		if (!m_pGBufferLightRT)
			return 0;

		return m_pGBufferLightRT->GetTexture(0);
	}

	mash::MashTexture* CMashSceneManager::GetDeferredLightingSpecularMap()const
	{
		if (!m_pGBufferLightRT)
			return 0;

		return m_pGBufferLightRT->GetTexture(1);
	}

	void CMashSceneManager::_OnDeferredLightingModeEnabled()
	{
		CreateGBuffer();
	}

	bool CMashSceneManager::GetForwardRenderedShadowsEnabled()const
	{
		MashLight *light = GetFirstForwardRenderedLight();
		return (light && light->IsShadowsEnabled())?true:false;
	}

	bool CMashSceneManager::GetDeferredDirShadowsEnabled()const
	{
		return (m_shadowEnabledDirLightCount > 0)?true:false;
	}

	bool CMashSceneManager::GetDeferredSpotShadowsEnabled()const
	{
		return (m_shadowEnabledSpotLightCount > 0)?true:false;
	}

	bool CMashSceneManager::GetDeferredPointShadowsEnabled()const
	{
		return (m_shadowEnabledPointLightCount > 0)?true:false;
	}

	void CMashSceneManager::_OnShadowsEnabled(MashLight *light)
	{
		/*
			If the main light for forward rendering is changed then
			the shaders must be updated
		*/
		if (GetFirstForwardRenderedLight() == light)
			m_rebuildShaderState |= aREBUILD_FORWARD_RENDERED_SCENE;

        {
            switch(light->GetLightType())
            {
            case aLIGHT_DIRECTIONAL:
                {
                    /*
                        for deferred rendering, if one light has shadows enabled
                        then the deferred shadows must be rebuilt. Note, this
                        only happens once for all lights
                    */
                    if (m_shadowEnabledDirLightCount == 0)
                        m_rebuildShaderState |= aREBUILD_DEFERRED_DIR_SHADER;

                    ++m_shadowEnabledDirLightCount;

                    break;
                }
            case aLIGHT_SPOT:
                {
                    if (m_shadowEnabledSpotLightCount == 0)
                        m_rebuildShaderState |= aREBUILD_DEFERRED_SPOT_SHADER;

                    ++m_shadowEnabledSpotLightCount;

                    break;
                }
            case aLIGHT_POINT:
                {
                    if (m_shadowEnabledPointLightCount == 0)
                        m_rebuildShaderState |= aREBUILD_DEFERRED_POINT_SHADER;

                    ++m_shadowEnabledPointLightCount;

                    break;
                }
            };
        }
	}

	void CMashSceneManager::_OnShadowsDisabled(MashLight *light)
	{
		if (GetFirstForwardRenderedLight() == light)
			m_rebuildShaderState |= aREBUILD_FORWARD_RENDERED_SCENE;

        {
            switch(light->GetLightType())
            {
            case aLIGHT_DIRECTIONAL:
                {
                    //sanity checks
                    if (m_shadowEnabledDirLightCount > 0)
                    {
                        --m_shadowEnabledDirLightCount;

                        //shaders will be rebuilt with shadow code removed
                        if (m_shadowEnabledDirLightCount == 0)
                            m_rebuildShaderState |= aREBUILD_DEFERRED_DIR_SHADER;
                    }

                    break;
                }
            case aLIGHT_SPOT:
                {
                    if (m_shadowEnabledSpotLightCount > 0)
                    {
                        --m_shadowEnabledSpotLightCount;

                        if (m_shadowEnabledSpotLightCount == 0)
                            m_rebuildShaderState |= aREBUILD_DEFERRED_SPOT_SHADER;
                    }

                    break;
                }
            case aLIGHT_POINT:
                {
                    if (m_shadowEnabledPointLightCount > 0)
                    {
                        --m_shadowEnabledPointLightCount;

                        if (m_shadowEnabledPointLightCount == 0)
                            m_rebuildShaderState |= aREBUILD_DEFERRED_POINT_SHADER;
                    }

                    break;
                }
            };
        }
	}

	void CMashSceneManager::OnShadowCasterMaterialRebuildNeeded(MashShadowCaster *caster)
	{
		if (!caster)
			return;

		/*
			Only rebuild materials if this is the active caster for a
			light type. If it's not then materials will be rebuilt
			later when that caster is set.
		*/
		if (!m_shadowCasters[caster->GetShadowType()].caster || 
			(m_shadowCasters[caster->GetShadowType()].caster != caster))
		{
			return;
		}

		switch(caster->GetShadowType())
		{
		case aLIGHT_DIRECTIONAL:
			{
				m_rebuildShaderState |= aREBUILD_DIRECTIONAL_SHADOW_CASTERS;
				break;
			}
		case aLIGHT_SPOT:
			{
				m_rebuildShaderState |= aREBUILD_DIRECTIONAL_SPOT_CASTERS;
				break;
			}
		case aLIGHT_POINT:
			{
				m_rebuildShaderState |= aREBUILD_DIRECTIONAL_POINT_CASTERS;
				break;
			}
		};
	}

	void CMashSceneManager::OnShadowReceiverMaterialRebuildNeeded(MashShadowCaster *caster)
	{
		if (!caster)
			return;

		/*
			Only rebuild materials if this is the active caster for a
			light type. If it's not then materials will be rebuilt
			later when that caster is set.
		*/
		if (!m_shadowCasters[caster->GetShadowType()].caster || 
			(m_shadowCasters[caster->GetShadowType()].caster != caster))
		{
			return;
		}

		if (GetForwardRenderedShadowsEnabled())
			m_rebuildShaderState |= aREBUILD_FORWARD_RENDERED_SCENE;

		switch(caster->GetShadowType())
		{
		case aLIGHT_DIRECTIONAL:
			{
				if (m_shadowEnabledDirLightCount > 0)
				{
					m_rebuildShaderState |= aREBUILD_DEFERRED_DIR_SHADER;
				}

				break;
			}
		case aLIGHT_SPOT:
			{
				if (m_shadowEnabledSpotLightCount > 0)
				{
					m_rebuildShaderState |= aREBUILD_DEFERRED_SPOT_SHADER;
				}

				break;
			}
		case aLIGHT_POINT:
			{
				if (m_shadowEnabledPointLightCount > 0)
				{
					m_rebuildShaderState |= aREBUILD_DEFERRED_POINT_SHADER;
				}

				break;
			}
		};
	}

    MashDirectionalShadowCascadeCaster* CMashSceneManager::CreateDirectionalCascadeShadowCaster(MashDirectionalShadowCascadeCaster::eCASTER_TYPE casterType)
	{
		return MASH_NEW_COMMON CMashDirectionalCascadeCaster(m_pRenderer, casterType);
	}

	MashSpotShadowCaster* CMashSceneManager::CreateSpotShadowCaster(MashSpotShadowCaster::eCASTER_TYPE casterType)
	{
		return MASH_NEW_COMMON CMashSpotShadowCaster(m_pRenderer, casterType);
	}

	MashPointShadowCaster* CMashSceneManager::CreatePointShadowCaster(MashPointShadowCaster::eCASTER_TYPE casterType)
	{
		return MASH_NEW_COMMON CMashPointShadowCaster(m_pRenderer, casterType);
	}

	void CMashSceneManager::SetShadowCaster(mash::eLIGHTTYPE type, MashShadowCaster *caster)
	{
		/*
			If the caster pointers are the same then do nothing.
			Note if the caster is the same but different instances a
			change still occurs as settings may be different.
		*/
		if (m_shadowCasters[type].isLoaded && 
			(m_shadowCasters[type].caster == caster))
		{
			return;
		}

		//only call init if the scene has already been initialised.
        if (!m_isSceneInitializing && caster)
        {
            if (!caster->IsInitialised())
                caster->Initialise();
            
            if (!caster->IsValid())
                return;
        }

		/*
			Runtime scene shaders may need to be rebuilt when
			different casters are set.
		*/
		if (GetForwardRenderedShadowsEnabled())
			m_rebuildShaderState |= aREBUILD_FORWARD_RENDERED_SCENE;

		//if no caster then we are removing shadows
		if (caster)
		{
			switch(type)
			{
			case aLIGHT_DIRECTIONAL:
				{
					m_rebuildShaderState |= aREBUILD_DIRECTIONAL_SHADOW_CASTERS;

					if (GetDeferredDirShadowsEnabled())
						m_rebuildShaderState |= aREBUILD_DEFERRED_DIR_SHADER;

					break;
				}
			case aLIGHT_SPOT:
				{
					m_rebuildShaderState |= aREBUILD_DIRECTIONAL_SPOT_CASTERS;

					if (GetDeferredSpotShadowsEnabled())
						m_rebuildShaderState |= aREBUILD_DEFERRED_SPOT_SHADER;

					break;
				}
			case aLIGHT_POINT:
				{
					m_rebuildShaderState |= aREBUILD_DIRECTIONAL_POINT_CASTERS;

					if (GetDeferredPointShadowsEnabled())
						m_rebuildShaderState |= aREBUILD_DEFERRED_POINT_SHADER;

					break;
				}
			};
		}

		if (caster)
			caster->Grab();

		/*
			If another caster has been set and loaded, then unload it
			then drop it
		*/
		if (m_shadowCasters[type].caster)
		{
			if (m_shadowCasters[type].isLoaded)
			{
				m_shadowCasters[type].caster->OnUnload();
				m_shadowCasters[type].isLoaded = false;
			}

			m_shadowCasters[type].caster->Drop();
		}

		//assign the new caster
		m_shadowCasters[type].caster = caster;

		//call load only if the scene has finished initialising
		if (m_shadowCasters[type].caster && !m_isSceneInitializing)
		{
			m_shadowCasters[type].caster->OnLoad();
			m_shadowCasters[type].isLoaded = true;
		}
	}

	MashShadowCaster* CMashSceneManager::GetShadowCaster(mash::eLIGHTTYPE type)const
	{
		switch(type)
		{
		case aLIGHT_DIRECTIONAL:
			{
				return m_shadowCasters[aLIGHT_DIRECTIONAL].caster;
			}
		case aLIGHT_SPOT:
			{
				return m_shadowCasters[aLIGHT_SPOT].caster;
			}
		case aLIGHT_POINT:
			{
				return m_shadowCasters[aLIGHT_POINT].caster;
			}
		};

		return 0;
	}

	void CMashSceneManager::SetPreferredLightingMode(eLIGHTING_TYPE type)
	{
		if (type != m_preferredLightingMode)
		{
			m_preferredLightingMode = type;

			if (m_preferredLightingMode == aLIGHT_TYPE_DEFERRED)
				_OnDeferredLightingModeEnabled();

			m_rebuildShaderState |= aREBUILD_AUTO_SHADERS;
		}
	}

	void CMashSceneManager::_AddForwardRenderedLight(MashLight *light, bool setAsMain)
	{
		bool found = false;
		const uint32 lightCount = m_forwardRenderedLightList.Size();
		for(uint32 i = 0; i < lightCount; ++i)
		{
			if (m_forwardRenderedLightList[i] == light)
			{
				/*
					this light has already been enabled as a forward rendering
					light. Maybe the 'main' setting has changed so we just re-arrange
					the list a bit.
				*/

				//remove light as main?
				if ((lightCount > 1) && (i == 0) && !setAsMain)
				{
					m_forwardRenderedLightList.Erase(m_forwardRenderedLightList.Begin());
					m_forwardRenderedLightList.PushBack(light);

					m_rebuildShaderState |= aREBUILD_FORWARD_RENDERED_SCENE;
				}
				//set light as main?
				else if ((i > 0) && setAsMain)
				{
					m_forwardRenderedLightList.Erase(m_forwardRenderedLightList.Begin() + i);
					m_forwardRenderedLightList.Insert(m_forwardRenderedLightList.Begin(), light);

					m_rebuildShaderState |= aREBUILD_FORWARD_RENDERED_SCENE;
				}

				found = true;
				break;
			}
		}

		if (!found)
		{
			if (setAsMain)
				m_forwardRenderedLightList.Insert(m_forwardRenderedLightList.Begin(), light);
			else
				m_forwardRenderedLightList.PushBack(light);

			m_rebuildShaderState |= aREBUILD_FORWARD_RENDERED_SCENE;

			uint32 count = m_forwardRenderedLightList.Size();
			if (count > m_reservedForwardLightBufferElements)
			{
				if (m_forwardLightBuffer)
					MASH_FREE(m_forwardLightBuffer);

				m_reservedForwardLightBufferElements = count * 2;
				m_forwardLightBuffer = MASH_ALLOC_T_COMMON(sMashLight, m_reservedForwardLightBufferElements);
			}

		}
	}

	void CMashSceneManager::_RemoveForwardRenderedLight(MashLight *light)
	{
		const uint32 lightCount = m_forwardRenderedLightList.Size();
		for(uint32 i = 0; i < lightCount; ++i)
		{
			if (m_forwardRenderedLightList[i] == light)
			{
				m_forwardRenderedLightList.Erase(m_forwardRenderedLightList.Begin() + i);
				m_rebuildShaderState |= aREBUILD_FORWARD_RENDERED_SCENE;
				break;
			}
		}
	}

	uint32 CMashSceneManager::GetForwardRenderedLightCount()const
	{
		return m_forwardRenderedLightList.Size();
	}

	void CMashSceneManager::SetFogEnabled(bool bValue)
	{
		if (m_bIsFogEnabled != bValue)
		{
			m_bIsFogEnabled = bValue;
			m_rebuildShaderState |= aREBUILD_SCENE_WITH_FOG;
		}
	}
    
    void CMashSceneManager::_AddLightToCurrentRenderScene(MashLight *light)
    {
        m_currentRenderSceneLightList.PushBack(light);
    }

	void CMashSceneManager::_OnLightTypeChange(MashLight *light)
	{
        /*
            Only forward render shaders need recompiling.
        */
		if (light->IsForwardRenderedLight())
		{
			m_rebuildShaderState |= aREBUILD_FORWARD_RENDERED_SCENE;
		}
	}

	void CMashSceneManager::_AddCustomRenderPathToFlushList(MashCustomRenderPath *batch)
	{
		if (!batch->_IsRegisteredForFlush())
		{
			m_batchFlushList.PushBack(batch);
			batch->_SetRegisteredForFlushFlag(true);
		}
	}

	MashStaticMesh* CMashSceneManager::CreateStaticMesh()
	{
		MashStaticMesh *mesh = MASH_NEW_COMMON CMashStaticMesh(m_pRenderer, this);

		if (m_isSceneInitializing)
		{
			m_initialiseMeshLoadData.PushBack(mesh);
			mesh->Grab();
		}

		return mesh;
	}

	MashDynamicMesh* CMashSceneManager::CreateDynamicMesh()
	{
		MashDynamicMesh *mesh =  MASH_NEW_COMMON CMashDynamicMesh(m_pRenderer, this);

		if (m_isSceneInitializing)
		{
			m_initialiseMeshLoadData.PushBack(mesh);
			mesh->Grab();
		}

		return mesh;
	}

	void CMashSceneManager::_OnBeginUserInitialise()
	{
		m_isSceneInitializing = true;
	}

	void CMashSceneManager::_OnEndUserInitialise()
	{
		//needs to be at the top so that all objects are built
		m_isSceneInitializing = false;

		uint32 meshCount = m_initialiseMeshLoadData.Size();
		for(uint32 i = 0; i < meshCount; ++i)
		{
			m_initialiseMeshLoadData[i]->DeleteInitialiseData();
			m_initialiseMeshLoadData[i]->Drop();
		}

		m_initialiseMeshLoadData.Clear();
        
		/*
			The SetShadowCaster() function will initialse and load the casters
			if it has not already been done. Otherwise the function will do nothing.
		*/
        if (m_shadowCasters[aLIGHT_DIRECTIONAL].caster)
			SetShadowCaster(aLIGHT_DIRECTIONAL, m_shadowCasters[aLIGHT_DIRECTIONAL].caster);
        
        if (m_shadowCasters[aLIGHT_POINT].caster)
			SetShadowCaster(aLIGHT_POINT, m_shadowCasters[aLIGHT_POINT].caster);
        
        if (m_shadowCasters[aLIGHT_SPOT].caster)
			SetShadowCaster(aLIGHT_SPOT, m_shadowCasters[aLIGHT_SPOT].caster);

		//compile all materials after the scene is set up
		CompileAllMaterials(aMATERIAL_COMPILER_NON_COMPILED);
	}

	MashModel* CMashSceneManager::CreateModel()
	{
		CMashModel *pNewModel = MASH_NEW_COMMON CMashModel(this);
        
        MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_INFORMATION, "New model created", "CMashSceneManager::CreateModel");

		return pNewModel;
	}

	MashModel* CMashSceneManager::CreateModel(MashArray<MashArray<MashMesh*> > &meshLodList)
	{
		if (meshLodList.Empty())
		{
            MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
                             "Mesh list is empty. No model was created.", 
                             "CMashSceneManager::CreateModel");

			return 0;
		}

		MashModel *pNewModel = CreateModel();

		for(int32 i = 0; i < meshLodList.Size(); ++i)
		{
			if (!meshLodList[i].Empty())
				pNewModel->Append(&meshLodList[i][0], meshLodList[i].Size());
		}

		return pNewModel;
	}

	MashEllipsoidColliderController* CMashSceneManager::CreateEllipsoidColliderController(MashSceneNode *character, 
			MashSceneNode *collisionScene,
			const MashVector3 &radius,
			const MashVector3 &gravity)
	{
		MashEllipsoidColliderController *controller = MASH_NEW_COMMON CMashEllipsoidColliderController(character, collisionScene, radius, gravity);

		MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_INFORMATION, 
                         "Ellipsoid collision controller created.", 
                         "CMashSceneManager::CreateEllipsoidColliderController");
        
        return controller;
	}

	MashCharacterMovementController* CMashSceneManager::CreateCharacterMovementController(uint32 playerId, const MashStringc &inputContext, f32 rotationSpeed, f32 linearSpeed, const MashCharacterMovementController::sInputActionMovement *customActions)
    {
        MashCharacterMovementController *controller = MASH_NEW_COMMON CMashCharacterMovementController(playerId, m_pInputManager, inputContext, rotationSpeed, linearSpeed, customActions);
        
        MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_INFORMATION, 
                         "Character movement controller created.", 
                         "CMashSceneManager::CreateCharacterMovementController");
        
        return controller;
    }
    
    MashFreeMovementController* CMashSceneManager::CreateFreeMovementController(uint32 playerId, const MashStringc &inputContext, f32 rotationSpeed, f32 linearSpeed, const MashFreeMovementController::sInputActionMovement *customActions)
    {
        MashFreeMovementController *controller = MASH_NEW_COMMON CMashFreeCameraController(playerId, m_pInputManager, inputContext, rotationSpeed, linearSpeed, customActions);
        
        MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_INFORMATION, 
                         "Free movement controller created.", 
                         "CMashSceneManager::CreateFreeMovementController");
        
        return controller;
    }

	MashDummy* CMashSceneManager::AddDummy(MashSceneNode *parent, const MashStringc &sUserName)
	{
		CMashDummy *pNewDummy = MASH_NEW_COMMON CMashDummy(parent, this, sUserName);

		m_nodeList.PushBack(pNewDummy);

		MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_INFORMATION,
					"CMashSceneManager::AddDummy",
					"New dummy created '%s'.",
					sUserName.GetCString());

		return pNewDummy;
	}

	MashLight* CMashSceneManager::AddLight(MashSceneNode *parent, const MashStringc &sUserName, mash::eLIGHTTYPE lightType, eLIGHT_RENDERER_TYPE lightRendererType, bool mainLight)
	{
		CMashLight *pNewLight = MASH_NEW_COMMON CMashLight(parent, this, sUserName, lightType, lightRendererType, mainLight);
		m_nodeList.PushBack(pNewLight);

		MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_INFORMATION,
					"CMashSceneManager::AddLight",
					"New light created '%s'.",
					sUserName.GetCString());
		return pNewLight;
	}

	MashCamera* CMashSceneManager::AddCamera(MashSceneNode *parent, const MashStringc &sUserName)
	{
		CMashCamera *pNewCamera = MASH_NEW_COMMON CMashCamera(parent, this, m_pRenderer, sUserName);

		m_nodeList.PushBack(pNewCamera);

		MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_INFORMATION,
					"CMashSceneManager::AddCamera",
					"New camera created '%s'.",
					sUserName.GetCString());

		if (!m_pActiveCamera)
			SetActiveCamera(pNewCamera);

		return pNewCamera;
	}

    eMASH_STATUS CMashSceneManager::SetActiveCamera(MashCamera *pCamera)
	{
		if (!pCamera)
			return aMASH_FAILED;

		if (m_pActiveCamera == pCamera)
			return aMASH_OK;

		if (pCamera)
			pCamera->Grab();

		//drop the old one
		if (m_pActiveCamera)
		{
			m_pActiveCamera->_SetActiveCamera(false);
			m_pActiveCamera->Drop();
		}

		m_pActiveCamera = (MashCamera*)pCamera;
		
		if (m_pActiveCamera)
			m_pActiveCamera->_SetActiveCamera(true);

		m_pRenderer->GetRenderInfo()->SetCamera(m_pActiveCamera);

		return aMASH_OK;
	}

	eMASH_STATUS CMashSceneManager::SetActiveCameraByName(const MashStringc &name)
	{
		MashList<MashSceneNode*>::Iterator iter = m_nodeList.Begin();
		MashList<MashSceneNode*>::Iterator end = m_nodeList.End();
		for(; iter != end; ++iter)
		{
			if ((*iter)->GetNodeType() == aNODETYPE_CAMERA)
			{
                if ((*iter)->GetNodeName() == name)
				{
					SetActiveCamera((MashCamera*)(*iter));

					return aMASH_OK;
				}
			}
		}

		MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_ERROR,
					"CMashSceneManager::SetActiveCameraByName",
					"No camera found matching name '%s'.",
					name.GetCString());

		return aMASH_FAILED;
	}

	MashCullTechnique* CMashSceneManager::CreateCullTechnique(eCULL_TECHNIQUE tech)
	{
		switch(tech)
		{
		case aCULL_TECH_CAMERA:
			return MASH_NEW_COMMON CMashCameraCull(this);
		case aCULL_TECH_SHADOW:
			return MASH_NEW_COMMON CMashShadowCull(this);
		default:
			return 0;
		};
	}

	void CMashSceneManager::SetCullTechnique(MashCullTechnique *tech)
	{
		if (tech)
			tech->Grab();

		if (m_activeSceneCullTechnique)
			m_activeSceneCullTechnique->Drop();

		m_activeSceneCullTechnique = tech;
	}

	void CMashSceneManager::SetCullTechniqueShadow(MashCullTechnique *tech)
	{
		if (tech)
			tech->Grab();

		if (m_activeShadowCullTechnique)
			m_activeShadowCullTechnique->Drop();

		m_activeShadowCullTechnique = tech;
	}

	MashEntity* CMashSceneManager::AddEntity(MashSceneNode *parent, MashArray<MashArray<MashMesh*> > &meshLodList, const MashStringc &sNodeName)
	{
		if (meshLodList.Empty())
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
					"Mesh list is empty",
					"CMashSceneManager::AddEntity");

			return 0;
		}

		MashModel *pNewModel = CreateModel(meshLodList);

		if (!pNewModel)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
					"Entity creation failed",
					"CMashSceneManager::AddEntity");

			return 0;
		}

		return AddEntity(parent, pNewModel, sNodeName);
	}

	MashParticleSystem* CMashSceneManager::AddParticleSystem(MashSceneNode *parent, 
			const MashStringc &userName, 
			const sParticleSettings &settings,
			ePARTICLE_TYPE particleType, 
			eLIGHTING_TYPE lightingType,
			bool createMaterialInstance,
			MashModel *model)
	{
		if ((particleType == aPARTICLE_CPU_SOFT_DEFERRED || 
			particleType == aPARTICLE_GPU_SOFT_DEFERRED) &&
			(lightingType == aLIGHT_TYPE_DEFERRED))
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_WARNING,
					"Soft particles must use a lighting type other than deferred. The lighting\
													type has been set to the vertex lighting type",
					"CMashSceneManager::AddParticleSystem");
            
			lightingType = aLIGHT_TYPE_VERTEX;
		}

		MashMaterial *material = 0;

		/*
			We use the same material for each instance. There is no need at this point
			to create a new instance per particle system.
		*/
		bool wasMaterialLoaded = false;
		switch(particleType)
		{
		case aPARTICLE_CPU:
			{
				material = m_pRenderer->GetMaterialManager()->GetStandardMaterial(MashMaterialManager::aSTANDARD_MATERIAL_PARTICLE_CPU, &wasMaterialLoaded);
				break;
			}
		case aPARTICLE_CPU_SOFT_DEFERRED:
			{
				material = m_pRenderer->GetMaterialManager()->GetStandardMaterial(MashMaterialManager::aSTANDARD_MATERIAL_PARTICLE_CPU_SOFT, &wasMaterialLoaded);
				break;
			}
		case aPARTICLE_GPU:
			{
				material = m_pRenderer->GetMaterialManager()->GetStandardMaterial(MashMaterialManager::aSTANDARD_MATERIAL_PARTICLE_GPU, &wasMaterialLoaded);
				break;
			}
		case aPARTICLE_GPU_SOFT_DEFERRED:
			{
				material = m_pRenderer->GetMaterialManager()->GetStandardMaterial(MashMaterialManager::aSTANDARD_MATERIAL_PARTICLE_GPU_SOFT, &wasMaterialLoaded);
				break;
			}
		case aPARTICLE_MESH:
			{
				material = m_pRenderer->GetMaterialManager()->GetStandardMaterial(MashMaterialManager::aSTANDARD_MATERIAL_PARTICLE_MESH, &wasMaterialLoaded);
				break;
			}
		};

		if (!material)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
					"Failed to load particle system material",
					"CMashSceneManager::AddParticleSystem");

			return 0;
		}

		/*
			This just saves writing a material for each particle type per lighting type.
			We generate a material with the users lighting type on the fly if it hasnt
			already been loaded.
		*/
		if (material->GetFirstTechnique()->GetTechnique()->GetLightingType() != lightingType)
		{
			MashStringc customMaterialName;
			switch(lightingType)
			{
			case aLIGHT_TYPE_NONE:
				{
					customMaterialName = material->GetMaterialName() + "_NoLighting";
					break;
				}
			case aLIGHT_TYPE_VERTEX:
				{
					customMaterialName = material->GetMaterialName() + "_VertexLighting";
					break;
				}
			case aLIGHT_TYPE_PIXEL:
				{
					customMaterialName = material->GetMaterialName() + "_PixelLighting";
					break;
				}
			case aLIGHT_TYPE_DEFERRED:
				{
					customMaterialName = material->GetMaterialName() + "_VertexLighting";
					break;
				}
			case aLIGHT_TYPE_AUTO:
				{
					customMaterialName = material->GetMaterialName() + "_AutoLighting";
					break;
				}
			}

			MashMaterial *prevMaterial = 0;

			/*
				If this material was just loaded for the first time, then we set the lighting type
				of it to whats needed. Otherwise we create a new one with the new lighting type.
			*/
			if (wasMaterialLoaded)
				prevMaterial = material;
			else
				prevMaterial = m_pRenderer->GetMaterialManager()->FindMaterial(customMaterialName.GetCString());

			if (!prevMaterial)
			{
				//this light material has not yet been created so create it
				material = material->CreateIndependentCopy(customMaterialName.GetCString(), false);
				if (!material)
				{
					MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
						"Failed to load particle material",
						"CMashSceneManager::AddParticleSystem");

					return 0;
				}

				//a new material was created
				wasMaterialLoaded = true;
			}
			else
			{
				material = prevMaterial;
			}

			if (material->GetFirstTechnique()->GetTechnique()->GetLightingType() != lightingType)
			{
				//and set the new lighting type
				std::map<MashStringc, MashArray<MashTechniqueInstance*> >::const_iterator groupStart = material->GetTechniqueList().begin();
				std::map<MashStringc, MashArray<MashTechniqueInstance*> >::const_iterator groupEnd = material->GetTechniqueList().end();
				for(; groupStart != groupEnd; ++groupStart)
				{
					const uint32 iTechCount = groupStart->second.Size();
					for(uint32 iTech = 0; iTechCount != iTech; ++iTech)
					{
						groupStart->second[iTech]->GetTechnique()->SetLightingType(lightingType);
					}
				}
			}
		}

		/*
			Create a new instance if needed . If it was just loaded for the first time
			then there is no need to create an instance
		*/
		if (!wasMaterialLoaded && createMaterialInstance)
		{
			static uint32 particleMaterialNameCounter = 0;
			int8 buffer[10];
			mash::helpers::PrintToBuffer(buffer, 10, "%d", particleMaterialNameCounter++);
			material = material->CreateInstance((material->GetMaterialName() + buffer).GetCString());

			if (!material)
			{
				MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
						"Failed to load particle material",
						"CMashSceneManager::AddParticleSystem");

				return 0;
			}
		}

		//compile the new material if this was create after the init.
		if (!material->IsCompiled() && MashDevice::StaticDevice->IsGameLoopInitialised())
		{
			material->CompileTechniques(MashDevice::StaticDevice->GetFileManager(), this, aMATERIAL_COMPILER_NON_COMPILED);
			if (!material->IsValid())
			{
				MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_INFORMATION,
						"Failed to compile default particle material",
						"CMashSceneManager::AddParticleSystem");

				return 0;
			}
		}

		MashParticleSystem *newParticleSystem = 0;
		switch(particleType)
		{
		case aPARTICLE_CPU:
		case aPARTICLE_CPU_SOFT_DEFERRED:
			{
				newParticleSystem = MASH_NEW_COMMON CMashCPUParticleSystem(parent, this, m_pRenderer, particleType, material, userName, false, createMaterialInstance, settings);
				break;
			}
		case aPARTICLE_GPU:
		case aPARTICLE_GPU_SOFT_DEFERRED:
			{
				newParticleSystem = MASH_NEW_COMMON CMashGPUParticleSystem(parent, this, m_pRenderer, particleType, material, userName, false, createMaterialInstance, settings);
				break;
			}
		case aPARTICLE_MESH:
			{
				newParticleSystem = MASH_NEW_COMMON CMashMeshParticleSystem(parent, this, m_pRenderer, particleType, material, userName, false, createMaterialInstance, settings);
				newParticleSystem->SetModel(model);
				break;
			}
		};

		m_nodeList.PushBack(newParticleSystem);

		MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_INFORMATION,
						"CMashSceneManager::AddParticleSystem",
						"New particle system created '%s'.",
						userName.GetCString());

		return newParticleSystem;
	}

	MashParticleSystem* CMashSceneManager::AddParticleSystemCustom(MashSceneNode *parent, 
			const MashStringc &userName, 
			const sParticleSettings &settings,
			MashMaterial *material)
	{
		if (!material)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
					"Custom particles must have a custom material",
					"CMashSceneManager::AddParticleSystemCustom");

			return 0;
		}

		MashParticleSystem *newParticleSystem = MASH_NEW_COMMON CMashCPUParticleSystem(parent, this, m_pRenderer, aPARTICLE_CPU, material, userName, true, false, settings);

		m_nodeList.PushBack(newParticleSystem);

		MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_INFORMATION,
					"CMashSceneManager::AddParticleSystemCustom",
					"New particle system created '%s'.",
					userName.GetCString());

		return newParticleSystem;
	}


	MashEntity* CMashSceneManager::AddEntity(MashSceneNode *parent, const MashStringc &sUserName)
	{
		return AddEntity(parent, 0, sUserName);
	}

	MashEntity* CMashSceneManager::AddEntity(MashSceneNode *parent, MashModel *pModel, const MashStringc &sUserName)
	{
		CMashEntityEx *pEntity = MASH_NEW_COMMON CMashEntityEx(parent, this, m_pRenderer, pModel, sUserName);

		m_nodeList.PushBack(pEntity);

		MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_INFORMATION,
					"CMashSceneManager::AddEntity",
					"New entity created '%s'.",
					sUserName.GetCString());

		return pEntity;
	}

	eMASH_STATUS CMashSceneManager::SaveShadowCastersToFile(const MashStringc &filename)
	{
		MashXMLWriter *fileStream = MashDevice::StaticDevice->GetFileManager()->CreateXMLWriter(filename.GetCString(), "scene");
		fileStream->WriteChild("shadowcasters");
		for(uint32 i = 0; i < aLIGHT_TYPE_COUNT; ++i)
		{
			if (m_shadowCasters[i].caster)
			{
				fileStream->WriteChild("caster");
				fileStream->WriteAttributeInt("lighttype", i);
				fileStream->WriteAttributeInt("castertype", m_shadowCasters[i].caster->GetShadowCasterType());
				m_shadowCasters[i].caster->Serialise(fileStream);

				fileStream->PopChild();
			}
		}

		fileStream->PopChild();
		fileStream->SaveAndDestroy();
		return aMASH_OK;
	}

	eMASH_STATUS CMashSceneManager::LoadShadowCastersFromFile(const MashStringc &filename, MashLoadCasterFunctor customFunctor)
	{
		MashXMLReader *fileStream = MashDevice::StaticDevice->GetFileManager()->CreateXMLReader(filename.GetCString());
		if (fileStream && fileStream->MoveToFirstChild("shadowcasters"))
		{
			if (fileStream->MoveToFirstChild("caster"))
			{
				do
				{
					int32 lightType = 0;
					MashShadowCaster *caster = 0;
					int32 shadowCasterType = 0;
					if (fileStream->GetAttributeInt("lighttype", lightType) && fileStream->GetAttributeInt("castertype", shadowCasterType))
					{
						switch(shadowCasterType)
						{
						case aSHADOW_CASTER_DIRECTIONAL_CASCADE_STANDARD:
							{
								caster = CreateDirectionalCascadeShadowCaster(MashDirectionalShadowCascadeCaster::aCASTER_TYPE_STANDARD);
								break;
							}
						case aSHADOW_CASTER_DIRECTIONAL_CASCADE_ESM:
							{
								caster = CreateDirectionalCascadeShadowCaster(MashDirectionalShadowCascadeCaster::aCASTER_TYPE_ESM);
								break;
							}
						case aSHADOW_CASTER_SPOT_STANDARD:
							{
								caster = CreateSpotShadowCaster(MashSpotShadowCaster::aCASTER_TYPE_STANDARD);
								break;
							}
						case aSHADOW_CASTER_SPOT_ESM:
							{
								caster = CreateSpotShadowCaster(MashSpotShadowCaster::aCASTER_TYPE_ESM);
								break;
							}
						case aSHADOW_CASTER_POINT_STANDARD:
							{
								caster = CreatePointShadowCaster(MashPointShadowCaster::aCASTER_TYPE_STANDARD);
								break;
							}
						case aSHADOW_CASTER_POINT_STANDARD_FILTERED:
							{
								caster = CreatePointShadowCaster(MashPointShadowCaster::aCASTER_TYPE_STANDARD_FILTERED);
								break;
							}
						case aSHADOW_CASTER_POINT_ESM:
							{
								caster = CreatePointShadowCaster(MashPointShadowCaster::aCASTER_TYPE_ESM);
								break;
							}
						default:
							{
								if (customFunctor.IsValid())
								{
									sMashCasterLoader customData;
									customData.casterType = shadowCasterType;
									customData.returnVal = 0;
									customFunctor.Call(customData);
									caster = customData.returnVal;
								}
							}
						}

						if (caster)
						{
							caster->Deserialise(fileStream);
							SetShadowCaster((eLIGHTTYPE)lightType, caster);
							caster->Drop();
						}
					}
				}while(fileStream->MoveToNextSibling("caster"));

				fileStream->PopChild();
			}

			fileStream->Destroy();
		}

		return aMASH_OK;
	}

	eMASH_STATUS CMashSceneManager::LoadSceneFile(const MashArray<MashStringc> &filenames, MashList<mash::MashSceneNode*> &rootNodes, const sLoadSceneSettings &loadSettings)
	{
		MashStringc fileExt;
		CMashSceneLoader *setFileStream = 0;
		CMashColladaLoader *colladaLoader = 0;

		eMASH_STATUS status = aMASH_OK;
		const uint32 filenameCount = filenames.Size();
		for(uint32 i = 0; i < filenameCount; ++i)
		{
			fileExt.Clear();
			GetFileExtention(filenames[i].GetCString(), fileExt);

			if (scriptreader::CompareStrings(fileExt.GetCString(), "dae"))
			{
				if (!colladaLoader)
					colladaLoader = MASH_NEW_COMMON CMashColladaLoader();

				if (colladaLoader->Load(MashDevice::StaticDevice, filenames[i], rootNodes, loadSettings) == aMASH_FAILED)
					status = aMASH_FAILED;
			}
			else if(scriptreader::CompareStrings(fileExt.GetCString(), "nss"))
			{
				if (!setFileStream)
					setFileStream = MASH_NEW_T_COMMON(CMashSceneLoader)();

				if (setFileStream->LoadSETFile(MashDevice::StaticDevice, filenames[i].GetCString(), rootNodes, loadSettings) == aMASH_FAILED)
					status = aMASH_FAILED;
			}
		}

		if (setFileStream)
			MASH_DELETE_T(CMashSceneLoader, setFileStream);

		if (colladaLoader)
			MASH_DELETE_T(CMashColladaLoader, colladaLoader);

		return status;
	}

	eMASH_STATUS CMashSceneManager::LoadSceneFile(const MashStringc &filename, MashList<mash::MashSceneNode*> &rootNodes, const sLoadSceneSettings &loadSettings)
	{        
		MashStringc fileExt;
        GetFileExtention(filename.GetCString(), fileExt);

		if (scriptreader::CompareStrings(fileExt.GetCString(), "dae"))
		{
			CMashColladaLoader colladaLoader;
			return colladaLoader.Load(MashDevice::StaticDevice, filename, rootNodes, loadSettings);
		}
		else if(scriptreader::CompareStrings(fileExt.GetCString(), "nss"))
		{
			CMashSceneLoader setFileStream;
			return setFileStream.LoadSETFile(MashDevice::StaticDevice, filename.GetCString(), rootNodes, loadSettings);
		}

		return aMASH_FAILED;
	}

	eMASH_STATUS CMashSceneManager::SaveSceneFile(const MashStringc &fileName, const MashList<mash::MashSceneNode*> &rootNodes, const sSaveSceneSettings &saveData)
	{
        MashStringc absPath;
        MashDevice::StaticDevice->GetFileManager()->GetAbsoutePath(fileName.GetCString(), absPath);
        
		CMashSceneWriter fileWriter;
		return fileWriter.SaveScene(MashDevice::StaticDevice, absPath, rootNodes, saveData);
	}

	void CMashSceneManager::RemoveAllSceneNodes()
	{
		while(!m_nodeList.Empty())
		{
			m_nodeList.Front()->Drop();
			m_nodeList.PopFront();
		}
		m_iSceneNodeNameCounter = 0;
	}

	void CMashSceneManager::RemoveSceneNode(MashSceneNode *pNode)
	{
		MashList<MashSceneNode*>::Iterator iter = m_nodeList.Begin();
		MashList<MashSceneNode*>::Iterator end = m_nodeList.End();
		for(; iter != end; ++iter)
		{
			if (pNode == *iter)
			{
				pNode->Detach();
				m_nodeList.Erase(iter);
				pNode->Drop();
				break;
			}
		}
	}
    
    uint32 CMashSceneManager::GenerateRenderKeyForTechnique(const MashTechniqueInstance *pTechnique)
    {
        if (!m_pRenderKeyHashFunction)
            m_pRenderKeyHashFunction = DefaultRenderKeyHashFunction;
        
        return m_pRenderKeyHashFunction(pTechnique);
    }

	MashSceneNode* CMashSceneManager::GetSceneNodeByName(const MashStringc &sName)const
	{
		/*
			TODO : sort the list first for speed.
			Then do a binary search.
		*/

		MashList<MashSceneNode*>::ConstIterator iter = m_nodeList.Begin();
		MashList<MashSceneNode*>::ConstIterator end = m_nodeList.End();
		for(; iter != end; ++iter)
		{
            if((*iter)->GetNodeName() == sName)
			{
				return *iter;
			}
		}

		return 0;
	}
    
    MashSceneNode* CMashSceneManager::GetSceneNodeByUserID(int32 id)const
    {
        if (m_pCurrentSceneNode && (m_pCurrentSceneNode->GetUserID() == id))
			return m_pCurrentSceneNode;
        
		MashList<MashSceneNode*>::ConstIterator iter = m_nodeList.Begin();
		MashList<MashSceneNode*>::ConstIterator end = m_nodeList.End();
		for(; iter != end; ++iter)
		{
			if ((*iter)->GetUserID() == id)
			{
				return *iter;
			}
		}
        
		return 0;
    }

	MashSceneNode* CMashSceneManager::GetSceneNodeByID(uint32 id)const
	{
		/*
			This will usually be called by a script, and the current node
			will be stored in this pointer. So we first check this node
			because it's the most likley node being selected.
		*/
		if (m_pCurrentSceneNode && (m_pCurrentSceneNode->GetNodeID() == id))
			return m_pCurrentSceneNode;

		/*
			TODO : sort the list first for speed.
			Then do a binary search.
		*/
		MashList<MashSceneNode*>::ConstIterator iter = m_nodeList.Begin();
		MashList<MashSceneNode*>::ConstIterator end = m_nodeList.End();
		for(; iter != end; ++iter)
		{
			if ((*iter)->GetNodeID() == id)
			{
				return *iter;
			}
		}

		return 0;
	}

	uint32 CMashSceneManager::GetSceneNodeCount()const
	{
		return m_nodeList.Size();
	}

	void CMashSceneManager::GenerateUniqueSceneNodeName(MashStringc &out)
	{
		int8 buffer[100];
		int32 iCounter = 0;//stop inifinate loops
		do
		{
			out = "__sceneNodeObj";
            mash::helpers::PrintToBuffer(buffer, 10, "%d", m_iSceneNodeNameCounter++);
			out += buffer;
		}while(GetSceneNodeByName(out) && (iCounter++ < 1000));
	}

	MashTriangleCollider* CMashSceneManager::CreateTriangleCollider(MashTriangleBuffer **buffer, uint32 bufferCount, eTRIANGLE_COLLIDER_TYPE type, bool waitForDeserialize)
	{
		if (type == aTRIANGLE_COLLIDER_AUTO)
		{
			uint32 totalTriangleCount = 0;
			for(uint32 i = 0; i < bufferCount; ++i)
				totalTriangleCount += buffer[i]->GetTriangleCount();

			if (totalTriangleCount <= 6)
				type = aTRIANGLE_COLLIDER_STANDARD;
			else
				type = aTRIANGLE_COLLIDER_KD_TREE;
		}

		switch(type)
		{
		case aTRIANGLE_COLLIDER_STANDARD:
			{
				CMashTriCollectionCached *pNewCollider = MASH_NEW_COMMON CMashTriCollectionCached();

				pNewCollider->SetTriangleBuffers(buffer, bufferCount);
				return pNewCollider;

				break;
			}
		case aTRIANGLE_COLLIDER_KD_TREE:
			{
				CMashTriColliderKDTree *pNewCollider = MASH_NEW_COMMON CMashTriColliderKDTree();

				pNewCollider->SetTriangleBuffers(buffer, bufferCount);

				if (!waitForDeserialize)
					pNewCollider->GenerateSpacialData();

				return pNewCollider;

				break;
			}
		default:
			{
				MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
					"Not yet implimented collider methods",
					"CMashSceneManager::CreateTriangleCollider");
			}
		};

		MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_INFORMATION,
					"Triangle collider created.",
					"CMashSceneManager::CreateTriangleCollider");

		return 0;
	}

	MashTriangleCollider* CMashSceneManager::CreateTriangleCollider(MashModel *model, uint32 lod, eTRIANGLE_COLLIDER_TYPE type, bool generateTriangleBufferIfNull)
	{
		uint32 lodCount = model->GetLodCount();
		if ((lodCount  == 0) || (lod >= lodCount))
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
					"Lod greater than avaliable lods",
					"CMashSceneManager::CreateTriangleCollider");

			return 0;
		}

		MashArray<MashTriangleBuffer*> buffers(lodCount, 0);
		uint32 meshCount = model->GetMeshCount(lod);
		for(uint32 i = 0; i < meshCount; ++i)
		{
			MashMesh *mesh = model->GetMesh(i, lod);
			MashTriangleBuffer *triangleBuffer = mesh->GetTriangleBuffer();
			if (!triangleBuffer && generateTriangleBufferIfNull)
			{
				triangleBuffer = CreateTriangleBuffer(mesh);
				mesh->SetTriangleBuffer(triangleBuffer);
			}

			if (triangleBuffer)
			{
				buffers[i] = triangleBuffer;
			}
		}

		MashTriangleCollider *triangleCollection = CreateTriangleCollider(&buffers[0], lodCount, type, false);
		model->SetTriangleCollider(triangleCollection);
		return triangleCollection;
	}

	MashTriangleBuffer* CMashSceneManager::CreateTriangleBuffer()
	{
		return MASH_NEW_COMMON CMashTriangleBuffer();
	}

	MashTriangleBuffer* CMashSceneManager::CreateTriangleBuffer(MashMesh *pMesh)
	{
		CMashTriangleBuffer *newBuffer = (CMashTriangleBuffer*)CreateTriangleBuffer();

		if (newBuffer->Set(pMesh->GetRawVertices().Pointer(),
			pMesh->GetVertexCount(),
			pMesh->GetVertexDeclaration(),
			pMesh->GetRawIndices().Pointer(),
			pMesh->GetIndexCount(),
			pMesh->GetIndexFormat()) == aMASH_FAILED)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
					"Failed to create triangle buffer",
					"CMashSceneManager::CreateTriangleBuffer");

			MASH_DELETE newBuffer;

			return 0;
		}

		MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_INFORMATION,
					"CMashSceneManager::CreateTriangleBuffer",
					"Triangle buffer created with triangle count of '%d'.", newBuffer->GetTriangleCount());

		return newBuffer;
	}
    
    MashSkin* CMashSceneManager::_CreateSkinInstance(MashSkin *instanceFrom)
    {
        if (!instanceFrom)
            return instanceFrom;
        
        if (!m_createSkinInstances)
        {
            instanceFrom->Grab();
            return instanceFrom;
        }
        
        MashSkin *newSkin = CreateSkin();
        
        sInstancedSkin newSkinInstance;
        newSkinInstance.newSkin = newSkin;
        newSkinInstance.oldSkin = instanceFrom;
        m_instancedSkins.PushBack(newSkinInstance);
        
        //one for the manager
        instanceFrom->Grab();
        newSkin->Grab();
        
        return newSkin;
    }
    
    MashSceneNode* CMashSceneManager::AddInstance(MashSceneNode *instanceFrom, MashSceneNode *parent, const MashStringc &instanceName, bool createSkinInstances)
    {
        m_instancedSkins.Clear();
        m_createSkinInstances = createSkinInstances;
        
        MashSceneNode *newInstanceRoot = instanceFrom->_CreateInstance(parent, instanceName);
        
        MashArray<sInstancedSkin>::Iterator iter = m_instancedSkins.Begin();
        MashArray<sInstancedSkin>::Iterator iterEnd = m_instancedSkins.End();
        for(; iter != iterEnd; ++iter)
        {
            iter->newSkin->FillFrom(iter->oldSkin, newInstanceRoot);
            iter->oldSkin->Drop();
            iter->newSkin->Drop();
        }
        
        m_instancedSkins.Clear();

		MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_INFORMATION,
					"CMashSceneManager::AddInstance",
					"New scene node instance created '%s'.", instanceName.GetCString());
        
        return newInstanceRoot;
    }

	MashSkin* CMashSceneManager::CreateSkin()
	{
		return MASH_NEW_COMMON CMashSkin(m_pRenderer);
	}

	MashBone* CMashSceneManager::AddBone(MashSceneNode *parent,
		const MashStringc &name)
	{
		/*
			This is used when cloning skins
		*/
		static uint32 uniqueBoneId = 0;

		CMashBoneSceneNode *newBone = MASH_NEW_COMMON CMashBoneSceneNode(this, parent, name.GetCString(), uniqueBoneId++);
		m_nodeList.PushBack(newBone);

		MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_INFORMATION,
					"CMashSceneManager::AddBone",
					"New bone created '%s'.",
					name.GetCString());

		return newBone;
	}

	MashDecal* CMashSceneManager::AddDecal(MashSceneNode *parent,
		const MashStringc &sName,
		eDECAL_TYPE decalType,
		int32 decalLimit,
		eLIGHTING_TYPE lightingType,
		bool createMaterialInstance)
	{
		MashMaterial *material = 0;

		bool wasMaterialLoaded = false;
		switch(decalType)
		{
		case aDECAL_STANDARD:
			{
				material = m_pRenderer->GetMaterialManager()->GetStandardMaterial(MashMaterialManager::aSTANDARD_MATERIAL_DECAL_STANDARD, &wasMaterialLoaded);
				break;
			}
		}

		if (!material)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
					"Failed to load decal material",
					"CMashSceneManager::AddDecal");

			return 0;
		}

		/*
			This just saves writing a material for each node type per lighting type.
			We generate a material with the users lighting type on the fly if it hasnt
			already been loaded.
		*/
		if (material->GetFirstTechnique()->GetTechnique()->GetLightingType() != lightingType)
		{
			MashStringc customMaterialName;
			switch(lightingType)
			{
			case aLIGHT_TYPE_NONE:
				{
					customMaterialName = material->GetMaterialName() + "_NoLighting";
					break;
				}
			case aLIGHT_TYPE_VERTEX:
				{
					customMaterialName = material->GetMaterialName() + "_VertexLighting";
					break;
				}
			case aLIGHT_TYPE_PIXEL:
				{
					customMaterialName = material->GetMaterialName() + "_PixelLighting";
					break;
				}
			case aLIGHT_TYPE_DEFERRED:
				{
					customMaterialName = material->GetMaterialName() + "_VertexLighting";
					break;
				}
			case aLIGHT_TYPE_AUTO:
				{
					customMaterialName = material->GetMaterialName() + "_AutoLighting";
					break;
				}
			}

			MashMaterial *prevMaterial = 0;

			/*
				If this material was just loaded for the first time, then we set the lighting type
				of it to whats needed. Otherwise we create a new one with the new lighting type.
			*/
			if (wasMaterialLoaded)
				prevMaterial = material;
			else
				prevMaterial = m_pRenderer->GetMaterialManager()->FindMaterial(customMaterialName.GetCString());

			if (!prevMaterial)
			{
				//this light material has not yet been created so create it
				material = material->CreateIndependentCopy(customMaterialName.GetCString());
				if (!material)
				{
					MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_ERROR,
						"CMashSceneManager::AddDecal",
						"Failed to load decal material : %s",
						customMaterialName.GetCString());

					return 0;
				}

				//a new material was created
				wasMaterialLoaded = true;

				//and set the new lighting type
				std::map<MashStringc, MashArray<MashTechniqueInstance*> >::const_iterator groupStart = material->GetTechniqueList().begin();
				std::map<MashStringc, MashArray<MashTechniqueInstance*> >::const_iterator groupEnd = material->GetTechniqueList().end();
				for(; groupStart != groupEnd; ++groupStart)
				{
					const uint32 iTechCount = groupStart->second.Size();
					for(uint32 iTech = 0; iTechCount != iTech; ++iTech)
					{
						groupStart->second[iTech]->GetTechnique()->SetLightingType(lightingType);
					}
				}
			}
			else
			{
				material = prevMaterial;
			}
		}

		/*
			Create a new instance if needed. We only need to create an instance
			if the material has already been laoded.
		*/
		if (!wasMaterialLoaded && createMaterialInstance)
		{
			static uint32 decalMaterialNameCounter = 0;
			int8 buffer[10];
            mash::helpers::PrintToBuffer(buffer, 10, "%d", decalMaterialNameCounter++);
			material = material->CreateInstance((material->GetMaterialName() + buffer).GetCString());

			if (!material)
			{
				MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
						"Failed to load decal material",
						"CMashSceneManager::AddDecal");

				return 0;
			}
		}

		//compile the new material if this was create after the init.
		if (!material->IsCompiled() && MashDevice::StaticDevice->IsGameLoopInitialised())
		{
			material->CompileTechniques(MashDevice::StaticDevice->GetFileManager(), this, aMATERIAL_COMPILER_NON_COMPILED);
			if (!material->IsValid())
			{
				MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
						"Failed to compile default decal material",
						"CMashSceneManager::AddDecal");

				return 0;
			}
		}

		return AddDecalCustom(parent, sName, material, decalLimit, 0);
	}

	MashDecal* CMashSceneManager::AddDecalCustom(MashSceneNode *parent,
		const MashStringc &sName,
		MashMaterial *pMaterial,
		int32 decalLimit,
		MashSkin *skin)
	{
		MashDecal *pNewDecal = 0;
		
		if (decalLimit < 0)
			pNewDecal = MASH_NEW_COMMON CMashStaticDecal(parent, this, m_pRenderer, sName, pMaterial, skin);
		else
			pNewDecal = MASH_NEW_COMMON CMashDynamicDecal(parent, this, m_pRenderer, sName, pMaterial, skin, decalLimit);

		m_nodeList.PushBack(pNewDecal);

		MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_INFORMATION,
						"CMashSceneManager::AddDecalCustom",
						"New decal created '%s'.",
						sName.GetCString());

		return pNewDecal;
	}

	void CMashSceneManager::CompileAllMaterials(uint32 compileFlags)
	{
		if (!compileFlags && !m_rebuildShaderState)
			return;

		MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_INFORMATION,
					"CMashSceneManager::CompileAllMaterials",
					"Beginning to compile all materials at time '%u'.", MashDevice::StaticDevice->GetTimer()->GetTimeSinceProgramStart());

        m_pRenderer->GetMaterialManager()->_BeginBatchMaterialCompile();

		/*
			generate common shader code based on current settings.
		*/
		if (m_rebuildShaderState)
		{
			if (m_deferredRendererValid)
			{
				MashFileManager *fileManager = MashDevice::StaticDevice->GetFileManager();

				if (m_rebuildShaderState & aREBUILD_DEFERRED_DIR_SHADER ||
					m_rebuildShaderState & aREBUILD_DEFERRED_POINT_SHADER ||
					m_rebuildShaderState & aREBUILD_DEFERRED_SPOT_SHADER)
				{
					m_pRenderer->GetMaterialManager()->_RebuildDeferredLightingShaders();
				}

				/*
					Update gbuffer lighting materials. These are not autos so we
					compile them manually.
				*/
				if (m_rebuildShaderState & aREBUILD_DEFERRED_DIR_SHADER)
					m_pGBufferLighting[mash::aLIGHT_DIRECTIONAL]->CompileTechniques(fileManager, this, aMATERIAL_COMPILER_EVERYTHING, 0, 0);

				if (m_rebuildShaderState & aREBUILD_DEFERRED_POINT_SHADER)
					m_pGBufferLighting[mash::aLIGHT_POINT]->CompileTechniques(fileManager, this, aMATERIAL_COMPILER_EVERYTHING, 0, 0);

				if (m_rebuildShaderState & aREBUILD_DEFERRED_SPOT_SHADER)
					m_pGBufferLighting[mash::aLIGHT_SPOT]->CompileTechniques(fileManager, this, aMATERIAL_COMPILER_EVERYTHING, 0, 0);
			}

			if (m_rebuildShaderState & aREBUILD_FORWARD_RENDERED_SCENE)
			{
				compileFlags |= aMATERIAL_COMPILER_FORWARD_RENDERED;
				m_pRenderer->GetMaterialManager()->_RebuildCommonSceneShaders();
			}
			
			if (m_rebuildShaderState & aREBUILD_AUTO_SHADERS)
				compileFlags |= aMATERIAL_COMPILER_AUTOS;

			if (m_rebuildShaderState & aREBUILD_DIRECTIONAL_SHADOW_CASTERS)
				compileFlags |= aMATERIAL_COMPILER_DIRECTIONAL_SHADOW_CASTERS;
			if (m_rebuildShaderState & aREBUILD_DIRECTIONAL_SPOT_CASTERS)
				compileFlags |= aMATERIAL_COMPILER_SPOT_SHADOW_CASTERS;
			if (m_rebuildShaderState & aREBUILD_DIRECTIONAL_POINT_CASTERS)
				compileFlags |= aMATERIAL_COMPILER_POINT_SHADOW_CASTERS;
			
			m_rebuildShaderState = 0;
		}

		/*
			compile all loaded shaders.
		*/
		if (compileFlags != 0)
			m_pRenderer->GetMaterialManager()->_CompileAllMaterials(this, compileFlags);

		if (compileFlags || m_rebuildShaderState)
			m_pRenderer->GetMaterialManager()->_EndBatchMaterialCompile();

		MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_INFORMATION,
					"CMashSceneManager::CompileAllMaterials",
					"Finished compiling all materials at time '%u'.", MashDevice::StaticDevice->GetTimer()->GetTimeSinceProgramStart());
	}

	void CMashSceneManager::_LateUpdate()
	{
		const uint32 count = m_lookatTrackers.Size();
		for(uint32 i = 0; i < count; ++i)
		{
			m_lookatTrackers[i]->_LookAt();
		}
	}

	void CMashSceneManager::_Update(f32 dt)
	{
		/*
			Update all managed effects if changes have been made
			to the scene. Changes might include additions or removal
			to forward rendered lighting and/or fog
		*/
		CompileAllMaterials();
        
        const uint32 count = m_callbackNodes.Size();
		for(uint32 i = 0; i < count; ++i)
		{
			m_callbackNodes[i]->_UpdateCallbacks(dt);
		}
        
        m_pControllerManager->Update(dt);
	}
	
	eMASH_STATUS CMashSceneManager::UpdateScene(f32 dt, MashSceneNode *pScene)
	{
		/*
			Update the scene nodes
		*/
		pScene->Update();
		return aMASH_OK;
	}

	void CMashSceneManager::FlushGeometryBuffers()
	{
		if (m_pPrimitiveBatch)
			m_pPrimitiveBatch->Flush();
	}

	eMASH_STATUS CMashSceneManager::DrawLine(const MashVector3 &start, const MashVector3 &end, const mash::sMashColour &colour, bool depthTest)
	{
		MashVertexColour::sMashVertexColour linePoints[2] = {MashVertexColour::sMashVertexColour(start, colour), MashVertexColour::sMashVertexColour(end, colour)};
		return DrawLines(linePoints, 2, depthTest);
	}

	eMASH_STATUS CMashSceneManager::DrawLines(const mash::MashVertexColour::sMashVertexColour *pLines, uint32 iVertexCount, bool depthTest)
	{
		if (m_lastPrimitiveDepthTest != depthTest)
		{
			FlushGeometryBuffers();
			MashMaterial *primitiveBatchMaterial = m_pRenderer->GetMaterialManager()->GetStandardMaterial(MashMaterialManager::aSTANDARD_MATERIAL_PRIMITIVE);
			
			if (depthTest)
			{
				if (primitiveBatchMaterial->SetActiveGroup("DepthTestingOn") == aMASH_OK)
					m_lastPrimitiveDepthTest = depthTest;
			}
			else
			{
				if (primitiveBatchMaterial->SetActiveGroup("DepthTestingOff") == aMASH_OK)
					m_lastPrimitiveDepthTest = depthTest;
			}
		}

		m_pPrimitiveBatch->AddPoints((const uint8*)pLines, iVertexCount);
		return aMASH_OK;
	}

	eMASH_STATUS CMashSceneManager::DrawAABB(const mash::MashAABB &box, const sMashColour &colour, bool depthTest)
	{
		mash::MashVector3 points[8];
		box.GetVerticies(points);

		mash::MashVertexColour::sMashVertexColour boxVerts[24] = {
			//bottom
			mash::MashVertexColour::sMashVertexColour(points[0], colour),
			mash::MashVertexColour::sMashVertexColour(points[1], colour),
			mash::MashVertexColour::sMashVertexColour(points[1], colour),
			mash::MashVertexColour::sMashVertexColour(points[2], colour),
			mash::MashVertexColour::sMashVertexColour(points[2], colour),
			mash::MashVertexColour::sMashVertexColour(points[3], colour),
			mash::MashVertexColour::sMashVertexColour(points[3], colour),
			mash::MashVertexColour::sMashVertexColour(points[0], colour),

			//top
			mash::MashVertexColour::sMashVertexColour(points[4], colour),
			mash::MashVertexColour::sMashVertexColour(points[5], colour),
			mash::MashVertexColour::sMashVertexColour(points[5], colour),
			mash::MashVertexColour::sMashVertexColour(points[6], colour),
			mash::MashVertexColour::sMashVertexColour(points[6], colour),
			mash::MashVertexColour::sMashVertexColour(points[7], colour),
			mash::MashVertexColour::sMashVertexColour(points[7], colour),
			mash::MashVertexColour::sMashVertexColour(points[4], colour),

			//sides
			mash::MashVertexColour::sMashVertexColour(points[0], colour),
			mash::MashVertexColour::sMashVertexColour(points[4], colour),
			mash::MashVertexColour::sMashVertexColour(points[1], colour),
			mash::MashVertexColour::sMashVertexColour(points[5], colour),
			mash::MashVertexColour::sMashVertexColour(points[2], colour),
			mash::MashVertexColour::sMashVertexColour(points[6], colour),
			mash::MashVertexColour::sMashVertexColour(points[3], colour),
			mash::MashVertexColour::sMashVertexColour(points[7], colour),
		};

		return DrawLines(boxVerts, 24, depthTest);
	}

	eMASH_STATUS CMashSceneManager::CreateTriangleBufferAdjacencyRenderBuffer(const MashTriangleBuffer *buffer, const MashMatrix4 &worldTransform, MashArray<MashVertexColour::sMashVertexColour> &out)
	{
		if (!buffer)
			return aMASH_OK;

		MashVertexColour::sMashVertexColour linePoints;
		uint32 currentTri = 0;

		eMASH_STATUS status = aMASH_OK;
		MashArray<sTriangleRecord>::ConstIterator iter = buffer->GetTriangleList().Begin();
		MashArray<sTriangleRecord>::ConstIterator iterEnd = buffer->GetTriangleList().End();
		for(; iter != iterEnd; ++iter, ++currentTri)
		{
			for(int32 i = 0; i < 3; ++i)
			{
				if (iter->adjacencyEdgeList[i] != mash::math::MaxUInt32())
					linePoints.colour = sMashColour(0,255,0,255);
				else
					linePoints.colour = sMashColour(255,0,0,255);

				int32 pointIndex = i;
				linePoints.position = worldTransform.TransformVector(buffer->GetPoint(currentTri, pointIndex));
				out.PushBack(linePoints);

				++pointIndex;
				if (pointIndex == 3)
					pointIndex = 0;

				linePoints.position = worldTransform.TransformVector(buffer->GetPoint(currentTri, pointIndex));
				out.PushBack(linePoints);
			}
		}

		return status;
	}

	eMASH_STATUS CMashSceneManager::CreateTriangleBufferAdjacencyRenderBuffer(const MashModel *model, const MashMatrix4 &worldTransform, MashArray<MashVertexColour::sMashVertexColour> &out)
	{
		eMASH_STATUS status = aMASH_OK;
		if (model->GetTriangleCollider())
		{
			MashTriangleCollider *triCollider = model->GetTriangleCollider();
			for(uint32 i = 0; i < triCollider->GetTriangleBufferCount(); ++i)
			{
				if (CreateTriangleBufferAdjacencyRenderBuffer(triCollider->GetTriangleBuffer(i), worldTransform, out) == aMASH_FAILED)
						status = aMASH_FAILED;
			}
		}
		else
		{
			for(uint32 lod = 0; lod < model->GetLodCount(); ++lod)
			{
				for(uint32 mesh = 0; mesh < model->GetMeshCount(lod); ++mesh)
				{
					MashTriangleBuffer *triBuffer = model->GetMesh(mesh, lod)->GetTriangleBuffer();
					if (triBuffer)
					{
						if (CreateTriangleBufferAdjacencyRenderBuffer(triBuffer, worldTransform, out) == aMASH_FAILED)
							status = aMASH_FAILED;
					}
				}
			}
		}
		
		return status;
	}
    
    void CMashSceneManager::_AddCallbackNode(mash::MashSceneNode *node)
    {
        m_callbackNodes.PushBack(node);
    }
    
    void CMashSceneManager::_RemoveCallbackNode(mash::MashSceneNode *node)
	{
		const uint32 count = m_callbackNodes.Size();
		for(uint32 i = 0; i < count; ++i)
		{
			if (m_callbackNodes[i] == node)
			{
				m_callbackNodes.Erase(m_callbackNodes.Begin() + i);
				return;
			}
		}
	}

	void CMashSceneManager::_AddLookAtTracker(mash::MashSceneNode *node)
	{
		m_lookatTrackers.PushBack(node);
	}

	void CMashSceneManager::_RemoveLookAtTracker(mash::MashSceneNode *node)
	{
		const uint32 count = m_lookatTrackers.Size();
		for(uint32 i = 0; i < count; ++i)
		{
			if (m_lookatTrackers[i] == node)
			{
				m_lookatTrackers.Erase(m_lookatTrackers.Begin() + i);
				return;
			}
		}
	}

	void CMashSceneManager::_FlushRenderableBatches()
	{
		if (!m_batchFlushList.Empty())
		{
			MashArray<MashCustomRenderPath*>::Iterator batchIter = m_batchFlushList.Begin();
			MashArray<MashCustomRenderPath*>::Iterator batchEnd = m_batchFlushList.End();
			for(; batchIter != batchEnd; ++batchIter)
			{
				(*batchIter)->Flush();
				(*batchIter)->_SetRegisteredForFlushFlag(false);
			}

			m_batchFlushList.Clear();
		}
	}

	eMASH_STATUS CMashSceneManager::RenderShadowMap(mash::MashLight *pLight, MashShadowCaster *pShadowCaster)
	{
		if (!pShadowCaster)
			return aMASH_OK;

		m_pRenderer->GetRenderInfo()->SetShadowCaster(pShadowCaster);

		if (pShadowCaster->OnPassSetup(pLight, 
				m_pActiveCamera,
				m_shadowSceneBounds) == aMASH_FAILED)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
						"Shadow caster failed pass setup.",
						"CMashSceneManager::RenderShadowMap");

			return aMASH_FAILED;
		}

		const int32 iShadowPassCount = pShadowCaster->GetNumPasses();

		/*
			Some casters require multiple passes to generate a shadow map.
			For instance, an omni light must make 6 passes to genertae
			a cube map.
		*/
		for(int32 iShadowPass = 0; iShadowPass < iShadowPassCount; ++iShadowPass)
		{
			if (pShadowCaster->OnPass(iShadowPass, 
				pLight, 
				m_pActiveCamera,
				m_shadowSceneBounds) == aMASH_FAILED)
			{
				MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
						"Shadow caster pass failed.",
						"CMashSceneManager::RenderShadowMap");

				return aMASH_FAILED;
			}
            
			MashList<sSolidRenderable>::Iterator iter = m_shadowRenderables.Begin();
			MashList<sSolidRenderable>::Iterator end = m_shadowRenderables.End();
			for(; iter != end; ++iter)
				iter->pRenderable->Draw();

			_FlushRenderableBatches();
		}

		m_eRenderPass = aRENDER_STAGE_SCENE;
		pShadowCaster->OnPassEnd();
		m_eRenderPass = aRENDER_STAGE_SHADOW;

		return aMASH_OK;
	}

	void CMashSceneManager::AddRenderableToRenderQueue(mash::MashRenderable *pRenderable, eHLRENDER_PASS pass, eRENDER_STAGE stage)
	{
		MashTechniqueInstance *techniqueInstance = pRenderable->GetMaterial()->GetActiveTechnique();
		if (!techniqueInstance)
			return;

		MashTechnique *activeTechnique = techniqueInstance->GetTechnique();

		if (!activeTechnique)
			return;

		/*
			We collect some information about the renderable scene so that
			we can pass it on to the shadow casters.
			The scene bounds is important so that a tight view frustum
			can be created to improve shadow map quality.
		*/
		m_shadowSceneBounds.Merge(pRenderable->GetTotalWorldBoundingBox());

		if (stage == aRENDER_STAGE_SHADOW)
		{
			if (activeTechnique->IsTransparent())
			{
				if (IsTransparentObjectShadowCastingEnabled())
				{
					m_shadowRenderables.PushBack(sSolidRenderable(pRenderable));
					++m_sceneRenderInfo.shadowObjectCount;
				}
			}
			else
			{
				m_shadowRenderables.PushBack(sSolidRenderable(pRenderable));
				++m_sceneRenderInfo.shadowObjectCount;
			}
			
		}
		else
		{
			switch(pass)
			{
			case aHLPASS_SCENE:
				switch(activeTechnique->GetRenderPass(this))
				{
					case aPASS_SOLID:
						{
							m_solidRenderables.PushBack(sSolidRenderable(pRenderable));
							++m_sceneRenderInfo.forwardRenderedSolidObjectCount;
							break;
						}
					case aPASS_TRANSPARENT:
						{
							m_transparentRenderables.PushBack(sTransparentRenderable(pRenderable, m_pActiveCamera));
							++m_sceneRenderInfo.forwardRenderedTransparentObjectCount;
							break;
						}
					case aPASS_DEFERRED:
						{
							m_deferredRenderables.PushBack(sSolidRenderable(pRenderable));
							++m_sceneRenderInfo.deferredObjectSolidCount;
							break;
						}
				};
				break;
			case aHLPASS_PARTICLES:
					switch(activeTechnique->GetRenderPass(this))
					{
					case aPASS_SOLID:
						{
							m_solidParticles.PushBack(sSolidRenderable(pRenderable));
							++m_sceneRenderInfo.forwardRenderedSolidObjectCount;
							break;
						}
					case aPASS_TRANSPARENT:
						{
							m_transparentParticles.PushBack(sTransparentRenderable(pRenderable, m_pActiveCamera));
							++m_sceneRenderInfo.forwardRenderedTransparentObjectCount;
							break;
						}
					case aPASS_DEFERRED:
						{
							m_deferredParticles.PushBack(sSolidRenderable(pRenderable));
							++m_sceneRenderInfo.deferredObjectSolidCount;
							break;
						}
					};
				break;
			case aHLPASS_DECAL:
				switch(activeTechnique->GetRenderPass(this))
					{
					case aPASS_SOLID:
						{
							m_solidDecals.PushBack(sSolidRenderable(pRenderable));
							++m_sceneRenderInfo.forwardRenderedSolidObjectCount;
							break;
						}
					case aPASS_TRANSPARENT:
						{
							m_transparentDecals.PushBack(sTransparentRenderable(pRenderable, m_pActiveCamera));
							++m_sceneRenderInfo.forwardRenderedTransparentObjectCount;
							break;
						}
					case aPASS_DEFERRED:
						{
							m_deferredDecals.PushBack(sSolidRenderable(pRenderable));
							++m_sceneRenderInfo.deferredObjectSolidCount;
							break;
						}
					};
				break;
			};
		}
	}

	eMASH_STATUS CMashSceneManager::CullScene(MashSceneNode *scene)
	{
		m_sceneRenderInfo.deferredObjectSolidCount = 0;
		m_sceneRenderInfo.forwardRenderedSolidObjectCount = 0;
		m_sceneRenderInfo.forwardRenderedTransparentObjectCount = 0;
		m_sceneRenderInfo.shadowObjectCount = 0;

		if (!m_pActiveCamera)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
						"Culling failed. An active camera must be set",
						"CMashSceneManager::CullScene");

			return aMASH_FAILED;
		}

		//make sure the camera is fully updated
        /*
            This is done here so that any internal data waiting on cull pass
            is updated before culling.
        */
        m_pActiveCamera->OnCullPass();

		if (scene)
		{
			if (!m_activeSceneCullTechnique)
			{
				MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
						"Culling failed. A culling technique must be set",
						"CMashSceneManager::CullScene");

				return aMASH_FAILED;
			}

			/*
				TODO : Calculate this when changes occur
			*/
			bool shadowsEnabled = GetForwardRenderedShadowsEnabled() ||
				GetDeferredDirShadowsEnabled() ||
				GetDeferredSpotShadowsEnabled() ||
				GetDeferredPointShadowsEnabled();

			m_activeSceneCullTechnique->CullScene(scene);

			if (shadowsEnabled)
			{
				if (m_activeShadowCullTechnique)
					m_activeShadowCullTechnique->CullScene(scene);
				else
				{
					MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
						"Shadow culling failed. Shadows are enabled but no shadow culling technique is set",
						"CMashSceneManager::CullScene");
				}
			}
		}

		return aMASH_OK;
	}

	eMASH_STATUS CMashSceneManager::DrawScene()
	{
		if (m_rebuildShaderState == 0)
		{
			bool isForwardRendererEmpty = (m_sceneRenderInfo.forwardRenderedSolidObjectCount + m_sceneRenderInfo.forwardRenderedTransparentObjectCount) == 0;
			bool isDeferredRendererEmpty = m_sceneRenderInfo.deferredObjectSolidCount == 0;

			if (!isDeferredRendererEmpty)
				DrawDeferredScene();

			if (!isForwardRendererEmpty)
				DrawForwardRenderedScene();

			//clear render buckets.
			m_solidRenderables.Clear();
			m_transparentRenderables.Clear();
			m_solidDecals.Clear();
			m_transparentDecals.Clear();
			m_solidParticles.Clear();
			m_transparentParticles.Clear();
			m_deferredRenderables.Clear();
			m_deferredDecals.Clear();
			m_deferredParticles.Clear();
			m_batchFlushList.Clear();
			m_shadowRenderables.Clear();
			m_currentRenderSceneLightList.Clear();

			m_shadowSceneBounds.min = MashVector3(mash::math::MaxInt32(), mash::math::MaxInt32(), mash::math::MaxInt32());
			m_shadowSceneBounds.max = MashVector3(mash::math::MinInt32(), mash::math::MinInt32(), mash::math::MinInt32());
		}

		return aMASH_OK;
	}

	eMASH_STATUS CMashSceneManager::DrawForwardRenderedScene()
	{
		//sort render buckets
		if (!m_solidRenderables.Empty())
			m_solidRenderables.Sort();

		if (!m_transparentRenderables.Empty())
			m_transparentRenderables.Sort();

		if (!m_solidDecals.Empty())
			m_solidDecals.Sort();

		if (!m_transparentDecals.Empty())
			m_transparentDecals.Sort();

		if (!m_solidParticles.Empty())
			m_solidParticles.Sort();

		if (!m_transparentParticles.Empty())
			m_transparentParticles.Sort();

		//TODO : Can this be done on changes only?
		const uint32 lightDataSize = sizeof(sMashLight);
		const uint32 forwardLightCount = m_forwardRenderedLightList.Size();
		for(uint32 i = 0; i < forwardLightCount; ++i)
		{
			const MashLight *pLight = m_forwardRenderedLightList[i];
			if (pLight->IsLightEnabled())
			{
				memcpy(&m_forwardLightBuffer[i], pLight->GetLightData(), lightDataSize);
			}
			else
			{
				/*
					Turn the light off
				*/
				sMashLight emptyData;
				memcpy(&m_forwardLightBuffer[i], &emptyData, lightDataSize);
			}
		}

		m_pRenderer->GetRenderInfo()->SetLightBuffer(m_forwardLightBuffer, forwardLightCount);
		
		/*
			Set up the main light and do shadow map pass.

			Each technique will later grab only the lights they need and
			add them to a buffer.
		*/
		
		if (!m_forwardRenderedLightList.Empty())
		{
			MashLight *pMainLight = m_forwardRenderedLightList[0];

			//set main light
			m_pRenderer->GetRenderInfo()->SetLight(pMainLight);

			if (m_forwardRenderedLightList[0]->IsShadowsEnabled())
			{
				/*
					Store this so that we can render the final scene
					to the original surface. This may change if shadows
					are enabled
				*/
				MashRenderSurface *pOriginalTarget = m_pRenderer->GetRenderSurface();
				mash::sMashViewPort originalViewport = m_pRenderer->GetViewport();

				/*
					Create shadow map only for the light at index 0
				*/
				m_eRenderPass = aRENDER_STAGE_SHADOW;

				switch(pMainLight->GetLightType())
				{
				case aLIGHT_DIRECTIONAL:
					RenderShadowMap(pMainLight, m_shadowCasters[aLIGHT_DIRECTIONAL].caster);
					break;
				case aLIGHT_POINT:
					RenderShadowMap(pMainLight, m_shadowCasters[aLIGHT_POINT].caster);
					break;
				case aLIGHT_SPOT:
					RenderShadowMap(pMainLight, m_shadowCasters[aLIGHT_SPOT].caster);
					break;
				default:
					{
						MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
							"Invalid shadow caster type",
							"CMashSceneManager::DrawForwardRenderedScene");
					}	
				}

				if (pOriginalTarget)
					m_pRenderer->SetRenderTarget(pOriginalTarget);
				else
					m_pRenderer->SetRenderTargetDefault();
                
                m_pRenderer->SetViewport(originalViewport);
			}
			else
			{
				m_pRenderer->GetRenderInfo()->SetShadowCaster(0);
			}
		}
		else
		{
			//no main light to set
			m_pRenderer->GetRenderInfo()->SetLight(0);
			
		}

		m_eRenderPass = aRENDER_STAGE_SCENE;

		/*
			TODO : wrap each render block in...
				if (!m_solidRenderables.empty())
		*/

		//draw solid objects
		MashList<sSolidRenderable>::Iterator iter = m_solidRenderables.Begin();
		MashList<sSolidRenderable>::Iterator end = m_solidRenderables.End();
		for(; iter != end; ++iter)
			iter->pRenderable->Draw();

		_FlushRenderableBatches();
		
		//draw solid particles
		iter = m_solidParticles.Begin();
		end = m_solidParticles.End();
		for(; iter != end; ++iter)
			iter->pRenderable->Draw();

		_FlushRenderableBatches();

		//draw decals
		if (!m_solidDecals.Empty() || !m_transparentDecals.Empty())
		{
			//render decals
			MashList<sSolidRenderable>::Iterator solidDecalIter = m_solidDecals.Begin();
			MashList<sSolidRenderable>::Iterator solidDecalEnd = m_solidDecals.End();
			for(; solidDecalIter != solidDecalEnd; ++solidDecalIter)
				solidDecalIter->pRenderable->Draw();

			_FlushRenderableBatches();

			MashList<sTransparentRenderable>::Iterator transDecalIter = m_transparentDecals.Begin();
			MashList<sTransparentRenderable>::Iterator transDecalEnd = m_transparentDecals.End();
			for(; transDecalIter != transDecalEnd; ++transDecalIter)
				transDecalIter->pRenderable->Draw();

			_FlushRenderableBatches();
		}

		/*
			TODO : All transparent objects will need to be sorted together
			by depth together!
		*/

		//draw transparent objects
		MashList<sTransparentRenderable>::Iterator transparentIter = m_transparentRenderables.Begin();
		MashList<sTransparentRenderable>::Iterator transparentEnd = m_transparentRenderables.End();
		for(; transparentIter != transparentEnd; ++transparentIter)
			transparentIter->pRenderable->Draw();

		_FlushRenderableBatches();

		//draw transparent particles
		transparentIter = m_transparentParticles.Begin();
		transparentEnd = m_transparentParticles.End();
		for(; transparentIter != transparentEnd; ++transparentIter)
			transparentIter->pRenderable->Draw();

		_FlushRenderableBatches();

		return aMASH_OK;
	}

	eMASH_STATUS CMashSceneManager::DrawDeferredScene()
	{
		if (!m_deferredRendererValid)
			return aMASH_OK;

		if (!m_deferredRenderables.Empty())
			m_deferredRenderables.Sort();

		if (!m_deferredDecals.Empty())
			m_deferredDecals.Sort();

		if (!m_deferredParticles.Empty())
			m_deferredParticles.Sort();

		/*
			Store this so that we can render the final scene
			to the original surface.
		*/
		MashRenderSurface *pOriginalTarget = m_pRenderer->GetRenderSurface();
		/*
			Also make a copy of the original viewport
		*/
		mash::sMashViewPort originalViewport = m_pRenderer->GetViewport();

		//do solid pass
		m_eRenderPass = aRENDER_STAGE_SCENE;

		/*
			Note this target has AutoViewport set to false so depth is rendered correctly. 
			This is done when the gbuffer is created.
		*/
		m_pRenderer->SetRenderTarget(m_pGBufferRT);

		//z buffer only (some ogl vresions wants the whole thing cleared)
        /*
            Note this means any following calls to DrawScene will be using a diferent depth buffer.
            This is needed for ogls shared depth buffer.
        */
		m_pRenderer->ClearTarget(mash::aCLEAR_DEPTH | mash::aCLEAR_TARGET, mash::sMashColour4(0.0f, 0.0f, 0.0f, 0.0f), 1.0f);
		
		//clear the gbuffer
		if (m_pGBufferClearMaterial->OnSet() == aMASH_OK)
			m_pRenderer->DrawFullScreenQuad();
        
        /*
            We want to maintain the same viewport as the user has set to keep
            the depth buffer correct.
            We originally set the render target viewport so the entire surface
            is cleared to remove an artifacts.
        */
        m_pRenderer->SetViewport(originalViewport);

		//draw solid objects
		MashList<sSolidRenderable>::Iterator iter = m_deferredRenderables.Begin();
		MashList<sSolidRenderable>::Iterator end = m_deferredRenderables.End();
		for(; iter != end; ++iter)
			iter->pRenderable->Draw();

		_FlushRenderableBatches();
		
		//draw solid particles
		iter = m_deferredParticles.Begin();
		end = m_deferredParticles.End();
		for(; iter != end; ++iter)
			iter->pRenderable->Draw();

		_FlushRenderableBatches();

		//draw decals
		if (!m_deferredDecals.Empty())
		{
			//render decals
			MashList<sSolidRenderable>::Iterator solidDecalIter = m_deferredDecals.Begin();
			MashList<sSolidRenderable>::Iterator solidDecalEnd = m_deferredDecals.End();
			for(; solidDecalIter != solidDecalEnd; ++solidDecalIter)
				solidDecalIter->pRenderable->Draw();

			_FlushRenderableBatches();
		}

		//do lighting
		m_pRenderer->SetRenderTarget(m_pGBufferLightRT);
		m_pRenderer->ClearTarget(mash::aCLEAR_TARGET, mash::sMashColour4(0.0f, 0.0f, 0.0f, 0.0f), 1.0f);
        m_pRenderer->SetViewport(originalViewport);

		for(int32 i = 0 ; i < m_currentRenderSceneLightList.Size(); ++i)
		{
			//set main light
			MashLight *pLight = m_currentRenderSceneLightList[i];
			m_pRenderer->GetRenderInfo()->SetLight(pLight);
			m_pRenderer->GetRenderInfo()->SetLightBuffer(pLight->GetLightData(), 1);

			m_eRenderPass = aRENDER_STAGE_SHADOW;

			/*
				Do shadow map pass
			*/
			if (pLight->IsShadowsEnabled())
			{
				switch(pLight->GetLightType())
				{
				case aLIGHT_DIRECTIONAL:
					RenderShadowMap(pLight, m_shadowCasters[aLIGHT_DIRECTIONAL].caster);
					break;
				case aLIGHT_POINT:
					RenderShadowMap(pLight, m_shadowCasters[aLIGHT_POINT].caster);
					break;
				case aLIGHT_SPOT:
					RenderShadowMap(pLight, m_shadowCasters[aLIGHT_SPOT].caster);
					break;
				default:
					{
						MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
								"Invalid shadow caster type",
								"CMashSceneManager::DrawDeferredScene");
					}	
				}
                
                //swap from shadow rt to light rt
                m_pRenderer->SetRenderTarget(m_pGBufferLightRT);
                m_pRenderer->SetViewport(originalViewport);
			}

			m_eRenderPass = aRENDER_STAGE_SCENE;

			/*
				Calculate lighting on GBuffer data
			*/
			MashMaterial *pActiveSkin = 0;
			switch(pLight->GetLightType())
			{
			case mash::aLIGHT_DIRECTIONAL:
				pActiveSkin = m_pGBufferLighting[mash::aLIGHT_DIRECTIONAL];
				break;
			case mash::aLIGHT_SPOT:
				pActiveSkin = m_pGBufferLighting[mash::aLIGHT_SPOT];
				break;
			case mash::aLIGHT_POINT:
				pActiveSkin = m_pGBufferLighting[mash::aLIGHT_POINT];
				break;
			default:
				{
					MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
								"Invalid light type",
								"CMashSceneManager::DrawDeferredScene");

					continue;
				}
            };

			if (pActiveSkin->OnSet() == aMASH_OK)
			{
				MashVector2 rtDim = m_pGBufferRT->GetDimentions();
				mash::MashRectangle2 zoomRegion((f32)originalViewport.x / rtDim.x, (f32)originalViewport.y / rtDim.y, 
					((f32)originalViewport.x + originalViewport.width) / rtDim.x, ((f32)originalViewport.y + originalViewport.height) / rtDim.y);

				m_pRenderer->DrawFullScreenQuadTexCoords(zoomRegion);
			}
		}

		if (pOriginalTarget)
			m_pRenderer->SetRenderTarget(pOriginalTarget);
		else
			m_pRenderer->SetRenderTargetDefault();

		m_pRenderer->SetViewport(originalViewport);


		if (m_pFinalMaterial->OnSet() == aMASH_OK)
		{
			/*
				If a custom viewport has been set, we clip the final image to that area.
				Squeezing it into the vp will result in a destorted final image.
				The deferred scene is also rendered in such a way as to preserve the depth buffer data
				for forward rendered objects.
			*/
			/*MashVector2 rtDim = m_pGBufferRT->GetDimentions();
			if ((originalViewport.x != 0.0f) || 
				(originalViewport.y != 0.0f) || 
				(originalViewport.width != rtDim.x) || 
				(originalViewport.height != rtDim.y))
			{
				MashRectangle2 region(originalViewport.x, originalViewport.y, 
					originalViewport.width + originalViewport.x, originalViewport.height + originalViewport.y);
				m_pRenderer->DrawClippedRegionFullScreen((uint32)rtDim.x, (uint32)rtDim.y, region);
			}
			else*/
			{
				MashVector2 rtDim = m_pGBufferRT->GetDimentions();
				mash::MashRectangle2 zoomRegion((f32)originalViewport.x / rtDim.x, (f32)originalViewport.y / rtDim.y, 
					((f32)originalViewport.x + originalViewport.width) / rtDim.x, ((f32)originalViewport.y + originalViewport.height) / rtDim.y);

				m_pRenderer->DrawFullScreenQuadTexCoords(zoomRegion);
			}
			
		}
		return aMASH_OK;
	}
}