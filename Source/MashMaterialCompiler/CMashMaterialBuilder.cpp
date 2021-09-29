//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashMaterialBuilder.h"
#include "MaterialLoader.h"
#include "CMashShaderCompiler.h"
#include "MashDevice.h"
#include "MashLog.h"

namespace mash
{
	_MASH_EXPORT MashMaterialBuilder* CreateMashMaterialBuilder(MashDevice *device)
	{
		CMashMaterialBuilder *pMaterialBuilder = MASH_NEW_COMMON CMashMaterialBuilder(device->GetRenderer(),
			device->GetSceneManager(),
			device->GetFileManager());

		return pMaterialBuilder;
	}

	CMashMaterialBuilder::CMashMaterialBuilder(mash::MashVideo *pRenderer, 
			mash::MashSceneManager *pSceneManager,
			MashFileManager *pFileManager):MashMaterialBuilder(),m_pRenderer(pRenderer),
			m_pSceneManager(pSceneManager), m_pFileManager(pFileManager),
			m_pMaterialLoader(0), m_shaderCompiler(0)
	{
		m_shaderCompiler = MASH_NEW_COMMON CMashShaderCompiler(m_pRenderer);	
		m_pMaterialLoader = MASH_NEW_COMMON MaterialLoader(m_shaderCompiler, m_pRenderer, m_pSceneManager, m_pFileManager);
	}

	CMashMaterialBuilder::~CMashMaterialBuilder()
	{
		if (m_shaderCompiler)
		{
			MASH_DELETE m_shaderCompiler;
			m_shaderCompiler = 0;
		}

		if (m_pMaterialLoader)
		{
			MASH_DELETE m_pMaterialLoader;
			m_pMaterialLoader = 0;
		}
	}
    
    void CMashMaterialBuilder::SetCustomRuntimeIncludes(eSHADER_EFFECT_INCLUDES type, const int8 *includeString)
    {
        m_shaderCompiler->SetAlternateInclude(type, includeString);
    }

	void CMashMaterialBuilder::SetIncludeCallback(const int8 *includeString, MashEffectIncludeFunctor includeFunctor)
	{
		m_shaderCompiler->SetIncludeCallback(includeString, includeFunctor);
	}

	eMASH_STATUS CMashMaterialBuilder::LoadMaterialFile(const int8 *filePath, 
			const sEffectMacro *compileArgs, 
			uint32 argCount, 
			MashArray<mash::MashMaterial*> *materialsOut,
            bool reloadMaterial)
	{        
		return m_pMaterialLoader->LoadMaterialFile(filePath, compileArgs, argCount, materialsOut, reloadMaterial);
	}

	eMASH_STATUS CMashMaterialBuilder::_RebuildDeferredLightingShaders()
	{
		return m_shaderCompiler->RecompileDeferredLightingShaders(m_pSceneManager, m_pFileManager);
	}

	eMASH_STATUS CMashMaterialBuilder::_RebuildCommonSceneShaders()
	{
		return m_shaderCompiler->RecompileCommonRunTimeFunctions(m_pSceneManager, m_pFileManager);
	}

	eMASH_STATUS CMashMaterialBuilder::_BuildRunTimeEffect(MashEffect *effect,
			const sEffectCompileArgs &compileArgs)
	{
		return m_shaderCompiler->BuildRunTimeEffect(m_pFileManager, 
			effect, 
			compileArgs);
	}

	void CMashMaterialBuilder::_BeginBatchMaterialCompile()
	{
		m_shaderCompiler->BeginBatchCompile();
	}

	void CMashMaterialBuilder::_EndBatchMaterialCompile()
	{
		m_shaderCompiler->EndBatchCompile();
	}
}