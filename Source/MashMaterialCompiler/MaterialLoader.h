//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MATERIAL_LOADER_H_
#define _MATERIAL_LOADER_H_

#include "MashEnum.h"
#include "Material.h"
#include "MashVideo.h"
#include "MashFileManager.h"
#include "MashArray.h"

namespace mash
{
	class CMashShaderCompiler;
	class MaterialLoader : public MashReferenceCounter
	{
	private:
		mash::MashVideo *m_pRenderer;
		mash::MashSceneManager *m_pSceneManager;
		MashFileManager *m_pFileManager;
		CMashShaderCompiler *m_shaderCompiler;

	private:
		mash::eFILTER MaterialSamplerFilterToEngineState(eTEXTURE_FILTERS minMag, eTEXTURE_FILTERS mip);
		MashMaterial* CreateMaterial(mash::MashVideo *pRenderer, sMaterial *pMaterial, 
			const sEffectMacro *compileArgs, 
			unsigned int argCount, bool reloadMaterial);

		MashMaterial* CreateReferenceMaterial(mash::MashVideo *pRenderer, sMaterial *pMaterial, bool reloadFile);

		eMASH_STATUS GetVertex(mash::MashVideo *pRenderer,
			const MashArray<sVertexElement> &vertexDeclaration, 
			MashArray<sMashVertexElement> &elmsOut);
	public:
		MaterialLoader(CMashShaderCompiler *shaderCompiler,
			mash::MashVideo *pRenderer, 
			mash::MashSceneManager *pSceneManager,
			MashFileManager *pFileManager);

		~MaterialLoader(){}

		eMASH_STATUS LoadMaterialFile(const char *sMaterialFileName, 
			const sEffectMacro *compileArgs, 
			unsigned int argCount, //TODO : Are these needed anymore?
			MashArray<mash::MashMaterial*> *materialsOut = 0,
            bool reloadFile = false);
	};
}

#endif