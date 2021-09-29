//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "MashMaterialManagerIntermediate.h"
#include "CMashMaterial.h"
#include "MashDevice.h"
#include "MashVideo.h"
#include "MashFileManager.h"
#include "MashSceneManager.h"
#include "CMashTechniqueInstance.h"
#include "CMashTechnique.h"
#include "MashLog.h"
#include "MashMaterialBuilder.h"
#include "MashHelper.h"
#include "CMashSceneWriter.h"
#include "CMashSceneLoader.h"
#include "MashGenericScriptReader.h"
#include "MashAutoEffectParameter.h"
#include "MashXMLWriter.h"

namespace mash
{
	MashMaterialManagerIntermediate::MashMaterialManagerIntermediate(mash::MashVideo *pRenderer):MashMaterialManager(),
		m_renderer(pRenderer), m_materialBuilder(0), m_materialNameCounter(0)
	{
	}

	MashMaterialManagerIntermediate::~MashMaterialManagerIntermediate()
	{
		MashArray<MashAutoEffectParameter*>::Iterator autoParamIter = m_autoShaderParameters.Begin();
		MashArray<MashAutoEffectParameter*>::Iterator autoParamIterEnd = m_autoShaderParameters.End();
		for(; autoParamIter != autoParamIterEnd; ++autoParamIter)
		{
			(*autoParamIter)->Drop();
		}

		m_autoShaderParameters.Clear();

		if (m_materialBuilder)
		{
			m_materialBuilder->Drop();
			m_materialBuilder = 0;
		}

		MashList<MashMaterial*>::Iterator matIter = m_materials.Begin();
		MashList<MashMaterial*>::Iterator matIterEnd = m_materials.End();
		for(; matIter != matIterEnd; ++matIter)
		{
			(*matIter)->Drop();
		}

		m_materials.Clear();
	}

	eMASH_STATUS MashMaterialManagerIntermediate::_Initialise(const sMashDeviceSettings &creationParameters)
    {
		m_autoShaderParameters.PushBack(MASH_NEW_COMMON MashParameterWVP());
		m_autoShaderParameters.PushBack(MASH_NEW_COMMON MashParameterViewProj());
		m_autoShaderParameters.PushBack(MASH_NEW_COMMON MashParameterWorld());
		m_autoShaderParameters.PushBack(MASH_NEW_COMMON MashParameterView());
		m_autoShaderParameters.PushBack(MASH_NEW_COMMON MashParameterProj());
		m_autoShaderParameters.PushBack(MASH_NEW_COMMON MashParameterWorldInvTrans());
		m_autoShaderParameters.PushBack(MASH_NEW_COMMON MashParameterViewInvTrans());
		m_autoShaderParameters.PushBack(MASH_NEW_COMMON MashParameterInvViewProj());
		m_autoShaderParameters.PushBack(MASH_NEW_COMMON MashParameterInvView());
		m_autoShaderParameters.PushBack(MASH_NEW_COMMON MashParameterInvProj());
		m_autoShaderParameters.PushBack(MASH_NEW_COMMON MashParameterTexture(m_renderer));
		m_autoShaderParameters.PushBack(MASH_NEW_COMMON MashParameterWorldViewInvTrans());
		m_autoShaderParameters.PushBack(MASH_NEW_COMMON MashParameterCameraNearFar());
		m_autoShaderParameters.PushBack(MASH_NEW_COMMON MashParameterShadowMap(m_renderer));
		m_autoShaderParameters.PushBack(MASH_NEW_COMMON MashParameterLight());

		m_autoShaderParameters.PushBack(MASH_NEW_COMMON MashParameterGBufferDiffuseSampler(m_renderer));
		m_autoShaderParameters.PushBack(MASH_NEW_COMMON MashParameterGBufferSpecualrSampler(m_renderer));
		m_autoShaderParameters.PushBack(MASH_NEW_COMMON MashParameterGBufferDepthSampler(m_renderer));
		m_autoShaderParameters.PushBack(MASH_NEW_COMMON MashParameterGBufferNormalSampler(m_renderer));

		m_autoShaderParameters.PushBack(MASH_NEW_COMMON MashParameterGBufferLightSampler(m_renderer));
		m_autoShaderParameters.PushBack(MASH_NEW_COMMON MashParameterGBufferLightSpecualrSampler(m_renderer));

		m_autoShaderParameters.PushBack(MASH_NEW_COMMON MashParamBonePaletteArray());

		m_autoShaderParameters.PushBack(MASH_NEW_COMMON MashParamLightWorldPosition());

		m_autoShaderParameters.PushBack(MASH_NEW_COMMON MashParamWorldView());

		m_autoShaderParameters.PushBack(MASH_NEW_COMMON MashParamWorldPosition());

		m_autoShaderParameters.PushBack(MASH_NEW_COMMON MashParamShadowsEnabled());

		m_compiledEffectOutputDirectory = creationParameters.compiledShaderOutputDirectory;
		m_intermediateEffectOutputDirectory = creationParameters.intermediateShaderOutputDirectory;

		m_materialBuilder = CreateMashMaterialBuilder(MashDevice::StaticDevice);

		return aMASH_OK;
	}

	MashMaterial* MashMaterialManagerIntermediate::GetStandardMaterial(eSTANDARD_MATERIAL materialType, bool *wasLoaded)
	{
		MashMaterial *loadedMaterial = 0;

		if (wasLoaded)
			*wasLoaded = false;

		//Load any special autos needed
		switch(materialType)
		{
		case aSTANDARD_MATERIAL_PARTICLE_CPU_SOFT:
		case aSTANDARD_MATERIAL_PARTICLE_GPU_SOFT:
			{
				uint32 tempParamIndex = 0;
				//make sure it hasn't already been loaded
				if (!IsAutoParameter(g_additionalEffectAutoNames[aADDITIONAL_EFFECT_SOFT_PARTICLE_SCALE], tempParamIndex))
					m_autoShaderParameters.PushBack(MASH_NEW_COMMON MashParamSoftParticleScale());

				//no break
			}
		case aSTANDARD_MATERIAL_PARTICLE_CPU:
		case aSTANDARD_MATERIAL_PARTICLE_GPU:
		case aSTANDARD_MATERIAL_PARTICLE_MESH:
			{
				uint32 tempParamIndex = 0;
				if (!IsAutoParameter(g_additionalEffectAutoNames[aADDITIONAL_EFFECT_PARTICLE_BUFFER], tempParamIndex))
					m_autoShaderParameters.PushBack(MASH_NEW_COMMON MashParameterParticleBuffer());

				break;
			}
		case aSTANDARD_MATERIAL_GUI_LINE:
		case aSTANDARD_MATERIAL_GUI_SPRITE:
		case aSTANDARD_MATERIAL_GUI_FONT:
			{
				uint32 tempParamIndex = 0;
				if (!IsAutoParameter(g_additionalEffectAutoNames[aADDITIONAL_EFFECT_GUI_ALPHAMASK_THRESHOLD], tempParamIndex))
					m_autoShaderParameters.PushBack(MASH_NEW_COMMON MashParamGUIAlphaMaskThreshold());

				if (!IsAutoParameter(g_additionalEffectAutoNames[aADDITIONAL_EFFECT_GUI_BASE_COLOUR], tempParamIndex))
					m_autoShaderParameters.PushBack(MASH_NEW_COMMON MashParamGUIBaseColour());

				if (!IsAutoParameter(g_additionalEffectAutoNames[aADDITIONAL_EFFECT_GUI_FONT_COLOUR], tempParamIndex))
					m_autoShaderParameters.PushBack(MASH_NEW_COMMON MashParamGUIFontColour());

				break;
			}
		}

		switch(materialType)
		{
		case aSTANDARD_MATERIAL_DEFAULT_MESH:
			{
				loadedMaterial = GetMaterial("MashDefaultExporterMaterial", "MashDefaultExporterMaterial.mtl", 0, 0, wasLoaded);
				break;
			}
		case aSTANDARD_MATERIAL_DECAL_STANDARD:
			{
				loadedMaterial = GetMaterial("MashStaticDecalMaterial", "MashStaticDecalMaterial.mtl", 0, 0, wasLoaded);
				break;
			}
		case aSTANDARD_MATERIAL_DECAL_SKINNED:
			{
				loadedMaterial = GetMaterial("MashDecalSkinnedMaterial", "MashDecalSkinnedMaterial.mtl", 0, 0, wasLoaded);
				break;
			}
		case aSTANDARD_MATERIAL_DRAW_TEXTURE:
			{
				loadedMaterial = GetMaterial("MashDrawTextureMaterial", "MashDrawTextureMaterial.mtl", 0, 0, wasLoaded);
				break;
			}
        case aSTANDARD_MATERIAL_DRAW_TRANS_TEXTURE:
			{
				loadedMaterial = GetMaterial("MashDrawTextureTransMaterial", "MashDrawTextureTransMaterial.mtl", 0, 0, wasLoaded);
				break;
			}
		case aSTANDARD_MATERIAL_PRIMITIVE:
			{
				loadedMaterial = GetMaterial("MashPrimitiveMaterial", "MashPrimitiveMaterial.mtl", 0, 0, wasLoaded);
				break;
			}
		case aSTANDARD_MATERIAL_GUI_LINE:
			{
				loadedMaterial = GetMaterial("MashGUILineMaterial", "MashGUIMaterials.mtl", 0, 0, wasLoaded);
				break;
			}
		case aSTANDARD_MATERIAL_GUI_SPRITE:
			{
				loadedMaterial = GetMaterial("MashGUISpriteMaterial", "MashGUIMaterials.mtl", 0, 0, wasLoaded);
				break;
			}
		case aSTANDARD_MATERIAL_GUI_FONT:
			{
				loadedMaterial = GetMaterial("MashGUIFontMaterial", "MashGUIMaterials.mtl", 0, 0, wasLoaded);
				break;
			}			
		case aSTANDARD_MATERIAL_GBUFFER_DIR_LIGHT:
			{
				loadedMaterial = GetMaterial("MashGBufferDirectionalLight", "MashGBufferMaterial.mtl", 0, 0, wasLoaded);
				break;
			}
		case aSTANDARD_MATERIAL_GBUFFER_SPOT_LIGHT:
			{
				loadedMaterial = GetMaterial("MashGBufferSpotLight", "MashGBufferMaterial.mtl", 0, 0, wasLoaded);
				break;
			}
		case aSTANDARD_MATERIAL_GBUFFER_POINT_LIGHT:
			{
				loadedMaterial = GetMaterial("MashGBufferPointLight", "MashGBufferMaterial.mtl", 0, 0, wasLoaded);
				break;
			}
		case aSTANDARD_MATERIAL_GBUFFER_COMBINE:
			{
				loadedMaterial = GetMaterial("MashGBufferCombine", "MashGBufferMaterial.mtl", 0, 0, wasLoaded);
				break;
			}
		case aSTANDARD_MATERIAL_GBUFFER_CLEAR:
			{
				loadedMaterial = GetMaterial("MashGBufferClear", "MashGBufferMaterial.mtl", 0, 0, wasLoaded);
				break;
			}
		case aSTANDARD_MATERIAL_PARTICLE_CPU:
			{
				loadedMaterial = GetMaterial("MashCPUParticleMaterial", "MashCPUParticleMaterial.mtl", 0, 0, wasLoaded);
				break;
			}
		case aSTANDARD_MATERIAL_PARTICLE_GPU:
			{
				loadedMaterial = GetMaterial("MashGPUParticleMaterial", "MashGPUParticleMaterial.mtl", 0, 0, wasLoaded);
				break;
			}
		case aSTANDARD_MATERIAL_PARTICLE_CPU_SOFT:
			{
				loadedMaterial = GetMaterial("MashCPUSoftParticleMaterial", "MashCPUSoftParticleMaterial.mtl", 0, 0, wasLoaded);
				break;
			}
		case aSTANDARD_MATERIAL_PARTICLE_GPU_SOFT:
			{
				loadedMaterial = GetMaterial("MashGPUSoftParticleMaterial", "MashGPUSoftParticleMaterial.mtl", 0, 0, wasLoaded);
				break;
			}
		case aSTANDARD_MATERIAL_PARTICLE_MESH:
			{
				loadedMaterial = GetMaterial("MashMeshParticleMaterial", "MashMeshParticleMaterial.mtl", 0, 0, wasLoaded);
				break;
			}
		}

		return loadedMaterial;
	}

	void MashMaterialManagerIntermediate::SetCompiledEffectOutputDirectory(const MashStringc &dir)
	{
		m_compiledEffectOutputDirectory = dir;
	}

	void MashMaterialManagerIntermediate::SetIntermediateEffectOutputDirectory(const MashStringc &dir)
	{
		m_intermediateEffectOutputDirectory = dir;
	}

	void MashMaterialManagerIntermediate::RegisterAutoParameterHandler(MashAutoEffectParameter *autoParamHandler, bool overWrite)
	{
		if (!autoParamHandler)
			return;

		uint32 autoParamHandlerIndex = 0;
		if (IsAutoParameter(autoParamHandler->GetParameterName(), autoParamHandlerIndex))
		{
			if (overWrite)
			{
				m_autoShaderParameters[autoParamHandlerIndex]->Drop();
				m_autoShaderParameters[autoParamHandlerIndex] = autoParamHandler;
				autoParamHandler->Grab();
			}
		}
		else
		{
			m_autoShaderParameters.PushBack(autoParamHandler);
            autoParamHandler->Grab();
		}
	}

	bool MashMaterialManagerIntermediate::IsAutoParameter(const int8 *simpleName, uint32 &autoParamHandlerIndex)const
	{
		const uint32 autoParamCount = m_autoShaderParameters.Size();
		for(uint32 i = 0; i < autoParamCount; ++i)
		{
			if (scriptreader::CompareStrings(simpleName, m_autoShaderParameters[i]->GetParameterName()))
			{
				autoParamHandlerIndex = i;
				return true;
			}
		}

		autoParamHandlerIndex = 0;
		return false;
	}

	MashAutoEffectParameter* MashMaterialManagerIntermediate::GetAutoParameterByName(const int8 *simpleName)const
	{
		const uint32 autoParamCount = m_autoShaderParameters.Size();
		for(uint32 i = 0; i < autoParamCount; ++i)
		{
			if (scriptreader::CompareStrings(simpleName, m_autoShaderParameters[i]->GetParameterName()))
			{
				return m_autoShaderParameters[i];
			}
		}

		return 0;
	}

	eMASH_STATUS MashMaterialManagerIntermediate::GenerateUniqueMaterialName(const MashStringc &orig, MashStringc &out)
	{
		MashMaterial *prevMaterial = 0;

		if (orig.Empty())
		{
			int8 buffer[100];
			int32 iCounter = 0;//stop inifinate loops
			do
			{
				out = "__material";
				out += mash::helpers::NumberToString(buffer, 100, m_materialNameCounter++);

			}while((prevMaterial = FindMaterial(out.GetCString())) && (iCounter++ < 1000));
		}
		else
		{
			out = orig;
			prevMaterial = FindMaterial(out.GetCString());
		}

		if (prevMaterial)
		{
			MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_ERROR, 
							"MashMaterialManagerIntermediate::GenerateUniqueMaterialName",
							"Failed to create material '%s'. Names must be unique",
							out.GetCString());

			return aMASH_FAILED;
		}

		return aMASH_OK;
	}

	void MashMaterialManagerIntermediate::RemoveMaterial(MashMaterial *pMaterial)
	{
		MashList<MashMaterial*>::Iterator iter = m_materials.Begin();
		MashList<MashMaterial*>::Iterator end = m_materials.End();
		for(; iter != end; ++iter)
		{
			if (*iter == pMaterial)
			{
				m_materials.Erase(iter);
                pMaterial->Drop();
				break;
			}
		}
	}

	void MashMaterialManagerIntermediate::RemoveAllMaterials()
	{
		MashList<MashMaterial*>::Iterator iter = m_materials.Begin();
		for(; iter != m_materials.End(); ++iter)
		{
			MashMaterial *pMaterial = *iter;
			iter = m_materials.Erase(iter);
            pMaterial->Drop();
		}

		m_materials.Clear();
	}

	eMASH_STATUS MashMaterialManagerIntermediate::SaveAllSkinsToFile(const int8 *sFileName)
	{
		MashXMLWriter *pWriter = MashDevice::StaticDevice->GetFileManager()->CreateXMLWriter(sFileName, "Skins");

		pWriter->Destroy();
		return aMASH_OK;
	}

	MashMaterial* MashMaterialManagerIntermediate::FindMaterial(const int8 *sName)const
	{
        if (!sName)
            return 0;
        
		MashList<MashMaterial*>::ConstIterator iter = m_materials.Begin();
		MashList<MashMaterial*>::ConstIterator end = m_materials.End();
		for(; iter != end; ++iter)
		{
			if (strcmp((*iter)->GetMaterialName().GetCString(), sName) == 0)
				return (*iter);
		}
        
		return 0;
	}

	MashTechniqueInstance* MashMaterialManagerIntermediate::_CreateTechniqueInstance(MashTechnique *refTechnique)
	{
		bool dropTech = false;
		if (!refTechnique)
		{
			refTechnique = _CreateTechnique();
			dropTech = true;
		}

		MashTechniqueInstance *pNewTechnique = MASH_NEW_COMMON CMashTechniqueInstance(m_renderer, refTechnique);

		/*
			If the refTech was just created, we drop a copy so the instance
			owns the tech
		*/
		if (dropTech)
			refTechnique->Drop();

		return pNewTechnique;
	}

	MashTechnique* MashMaterialManagerIntermediate::_CreateTechnique()
	{
		MashTechnique *pNewTechnique = MASH_NEW_COMMON CMashTechnique(m_renderer);

		return pNewTechnique;
	}

	MashMaterial* MashMaterialManagerIntermediate::GetMaterial(const int8 *sMaterialName, 
		const int8 *sFilePath, 
		const sEffectMacro *pCompileArgs, 
		uint32 argCount,
		bool *wasLoaded, bool reload)
	{
		if (wasLoaded)
			*wasLoaded = false;

		MashMaterial *pMaterial = FindMaterial(sMaterialName);
		if (pMaterial)
		{
			if (reload)
			{
				if (LoadMaterialFile(sFilePath, pCompileArgs, argCount, 0, reload) == aMASH_FAILED)
				{
					MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_ERROR, 
								"MashMaterialManagerIntermediate::GetMaterial",
								"Failed to reload material file : %s",
								sMaterialName);

					return 0;
				}
			}
		}
		else
		{
			if (LoadMaterialFile(sFilePath, pCompileArgs, argCount, 0) == aMASH_FAILED)
			{
				MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_ERROR, 
							"MashMaterialManagerIntermediate::GetMaterial",
							"Failed to load material file : %s",
							sMaterialName);

				return 0;
			}
			
			pMaterial = FindMaterial(sMaterialName);

			if (!pMaterial)
			{
				MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_ERROR, 
							"MashMaterialManagerIntermediate::GetMaterial",
							"Failed to load material : %s. File load was fine however it does not contain the named material",
							sMaterialName);

				return 0;
			}

			if (wasLoaded)
				*wasLoaded = true;
		}

		return pMaterial;
	}

	eMASH_STATUS MashMaterialManagerIntermediate::LoadMaterialFile(const int8 *sFilePath, 
			const sEffectMacro *pCompileArgs, 
			uint32 argCount, 
			MashArray<mash::MashMaterial*> *materialsOut,
            bool reloadMaterial)
	{
		return m_materialBuilder->LoadMaterialFile(sFilePath, pCompileArgs, argCount, materialsOut, reloadMaterial);
	}

	eMASH_STATUS MashMaterialManagerIntermediate::BuildRunTimeEffect(MashEffect *effect, 
			const sEffectCompileArgs &compileArgs)
	{
		return m_materialBuilder->_BuildRunTimeEffect(effect, compileArgs);
	}

	MashMaterial* MashMaterialManagerIntermediate::_CreateMaterial(const int8 *sName, MashMaterial *reference)
	{
		if (!reference)
			return 0;

		MashStringc sUniqueName;
		MashMaterial *pMaterial = 0;

		if (GenerateUniqueMaterialName(sName, sUniqueName) == aMASH_FAILED)
			return 0;
		
		pMaterial = MASH_NEW_COMMON CMashMaterial(m_renderer, sUniqueName.GetCString(), reference);
		m_materials.PushBack(pMaterial);

		return pMaterial;
	}

	MashMaterial* MashMaterialManagerIntermediate::AddMaterial(const int8 *sName, const sMashVertexElement *vertexElements, uint32 elementCount)
	{
		MashStringc sUniqueName;
		if (GenerateUniqueMaterialName(sName, sUniqueName) == aMASH_FAILED)
			return 0;

		MashMaterial *newMaterial = MASH_NEW_COMMON CMashMaterial(m_renderer, sUniqueName.GetCString(), 0);
		m_materials.PushBack(newMaterial);

		MashVertex *vertexDeclaration = m_renderer->_CreateVertexType(newMaterial, vertexElements, elementCount);
		if (!vertexDeclaration)
		{
			MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_ERROR, 
							"MashMaterialManagerIntermediate::CreateMaterial",
							"Failed to create vertex declaration for material '%s'",
							newMaterial->GetMaterialName().GetCString());

			RemoveMaterial(newMaterial);
			newMaterial = 0;
		}
		else
		{
			newMaterial->_SetVertexDeclaration(vertexDeclaration);
		}

		return newMaterial;
	}

	void MashMaterialManagerIntermediate::_BeginBatchMaterialCompile()
	{
		return m_materialBuilder->_BeginBatchMaterialCompile();
	}

	void MashMaterialManagerIntermediate::_EndBatchMaterialCompile()
	{
		return m_materialBuilder->_EndBatchMaterialCompile();
	}

	eMASH_STATUS MashMaterialManagerIntermediate::_RebuildDeferredLightingShaders()
	{
		return m_materialBuilder->_RebuildDeferredLightingShaders();
	}

	eMASH_STATUS MashMaterialManagerIntermediate::_RebuildCommonSceneShaders()
	{
		return m_materialBuilder->_RebuildCommonSceneShaders();
	}

	eMASH_STATUS MashMaterialManagerIntermediate::_CompileAllMaterials(MashSceneManager *sceneManager, uint32 compileFlags)
	{
		MashList<MashMaterial*>::Iterator iter = m_materials.Begin();
		MashList<MashMaterial*>::Iterator end = m_materials.End();
		for(; iter != end; ++iter)
		{
			if ((*iter)->CompileTechniques(MashDevice::StaticDevice->GetFileManager(), sceneManager, compileFlags/*true*/) == aMASH_FAILED)
				return aMASH_FAILED;
		}

		return aMASH_OK;
	}

	void MashMaterialManagerIntermediate::_SetProgramAutoParameter(MashEffect *pEffect, 
		MashEffectParamHandle *pParameter, 
		uint32 parameterType,
		uint32 index)
	{
		//TODO : Some bounds checking would be nice
		m_autoShaderParameters[parameterType]->OnSet(m_renderer->GetRenderInfo(), pEffect, pParameter, index);
	}
}