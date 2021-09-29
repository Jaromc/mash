//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _CMASH_MATERIAL_BUILDER_H_
#define _CMASH_MATERIAL_BUILDER_H_

#include "MashMaterialBuilder.h"
#include "MaterialLoader.h"

namespace mash
{
	class CMashShaderCompiler;
	class CMashMaterialBuilder : public MashMaterialBuilder
	{
	private:
		mash::MashVideo *m_pRenderer;
		mash::MashSceneManager *m_pSceneManager;
		MashFileManager *m_pFileManager;
		MaterialLoader *m_pMaterialLoader;
		CMashShaderCompiler *m_shaderCompiler;
	public:
		CMashMaterialBuilder(mash::MashVideo *pRenderer, 
			mash::MashSceneManager *pSceneManager,
			MashFileManager *pFileManager);

		~CMashMaterialBuilder();
        
        void SetCustomRuntimeIncludes(eSHADER_EFFECT_INCLUDES type, const int8 *includeString);
		void SetIncludeCallback(const int8 *includeString, MashEffectIncludeFunctor includeFunctor);

		eMASH_STATUS LoadMaterialFile(const int8 *sFilePath, 
			const sEffectMacro *pCompileArgs = 0, 
			uint32 argCount = 0, 
			MashArray<mash::MashMaterial*> *materialsOut = 0,
            bool reloadMaterial = false);

		eMASH_STATUS _RebuildDeferredLightingShaders();
		eMASH_STATUS _RebuildCommonSceneShaders();

		eMASH_STATUS _BuildRunTimeEffect(MashEffect *effect, 
			const sEffectCompileArgs &compileArgs);

		void _BeginBatchMaterialCompile();
		void _EndBatchMaterialCompile();
	};
}

#endif