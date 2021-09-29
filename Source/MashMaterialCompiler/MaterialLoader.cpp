//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "MaterialLoader.h"
#include "Material.h"
#include "MashFileStream.h"
#include "MashEffectProgram.h"
#include "MashEffect.h"
#include "MashTechnique.h"
#include "MashTechniqueInstance.h"
#include "MashMaterialManager.h"
#include "MashMaterial.h"
#include "CMashShaderCompiler.h"
#include "MashTechniqueInstance.h"
#include "MashHelper.h"
#include "MashLog.h"
#include "MashArray.h"

void MaterialParser(const char *sString);

int mtrl_parse(void);
void SetLexerString(const char *sString);
void DeleteLexerString();

namespace mash
{
int g_lineno = 0;

sShaderCompilerData *g_shaderCompilerData = 0;

}

const char *g_currentMaterialFileName = 0;

#if defined (MASH_LINUX) || defined (MASH_APPLE)
void mtrl_error(const char *msg)
#else
void mtrl_error(char *msg)
#endif
{
	if (!g_currentMaterialFileName)
		g_currentMaterialFileName = "noname";

	MASH_WRITE_TO_LOG_EX(mash::MashLog::aERROR_LEVEL_ERROR, 
		"Material Parser Error",
		"Material parse error in file '%s' line '%i'. %s",
		g_currentMaterialFileName, mash::g_lineno, msg);
}

extern "C" int mtrl_wrap()
{
	return 1;
}

namespace mash
{
	MaterialLoader::MaterialLoader(CMashShaderCompiler *shaderCompiler,
			mash::MashVideo *pRenderer, 
			mash::MashSceneManager *pSceneManager,
			MashFileManager *pFileManager):MashReferenceCounter(), m_pRenderer(pRenderer),
			m_pSceneManager(pSceneManager), m_pFileManager(pFileManager)
	{
	}

	mash::eFILTER MaterialLoader::MaterialSamplerFilterToEngineState(eTEXTURE_FILTERS minMag, eTEXTURE_FILTERS mip)
	{
		if (minMag == TEX_FILTER_LINEAR && mip == TEX_FILTER_LINEAR)
			return aFILTER_MIN_MAG_MIP_LINEAR;
		if (minMag == TEX_FILTER_POINT && mip == TEX_FILTER_POINT)
			return aFILTER_MIN_MAG_MIP_POINT;

		if (minMag == TEX_FILTER_LINEAR && mip == TEX_FILTER_MIPNONE)
			return aFILTER_MIN_MAG_LINEAR;
		if (minMag == TEX_FILTER_POINT && mip == TEX_FILTER_MIPNONE)
			return aFILTER_MIN_MAG_POINT;

		//default
		return aFILTER_MIN_MAG_POINT;
	}

	eMASH_STATUS MaterialLoader::LoadMaterialFile(const char *sMaterialFileName, 
		const sEffectMacro *compileArgs,
		unsigned int argCount,
		MashArray<mash::MashMaterial*> *materialsOut,
        bool reloadFile)
	{
		g_lineno = 0;

		MashFileStream *pFileStream = m_pFileManager->CreateFileStream();
		if (pFileStream->LoadFile(sMaterialFileName, aFILE_IO_TEXT) == aMASH_FAILED)
		{
			MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_ERROR, 
				"MaterialLoader::LoadMaterialFile", 
				"Failed to read material file '%s'.",
				sMaterialFileName);

			pFileStream->Destroy();
			return aMASH_FAILED;
		}

		//create memory pool for lexer strings.
		g_shaderCompilerData = MASH_NEW_T_COMMON(sShaderCompilerData);

		//for debuging
		g_currentMaterialFileName = sMaterialFileName;

		SetLexerString((const char*)pFileStream->GetData());
		mtrl_parse();
		DeleteLexerString();
		g_currentMaterialFileName = 0;

		pFileStream->Destroy();

		/*
			Materials need to be to be created in order so that any materials being
			created from a reference reference find their base
		*/
		MashArray<mash::sMaterial*> mainMaterials;
		MashArray<mash::sMaterial*> refMaterials;

		const unsigned int iMaterialCount = g_shaderCompilerData->g_materials.Size();
		for(unsigned int i = 0; i < iMaterialCount; ++i)
		{
			if (g_shaderCompilerData->g_materials[i].sRefName)
				refMaterials.PushBack(&g_shaderCompilerData->g_materials[i]);
			else
				mainMaterials.PushBack(&g_shaderCompilerData->g_materials[i]);
		}

		//load base materials
		const unsigned int iMainMaterialCount = mainMaterials.Size();
		for(unsigned int i = 0; i < iMainMaterialCount; ++i)
		{
			MashMaterial *pMaterial = CreateMaterial(m_pRenderer, mainMaterials[i], compileArgs, argCount, reloadFile);

			if (pMaterial && materialsOut)
				materialsOut->PushBack(pMaterial);
		}

		//load any ref materials
		const unsigned int iRefMaterialCount = refMaterials.Size();
		for(unsigned int i = 0; i < iRefMaterialCount; ++i)
		{
			MashMaterial *pMaterial = CreateReferenceMaterial(m_pRenderer, refMaterials[i], reloadFile);

			if (pMaterial && materialsOut)
				materialsOut->PushBack(pMaterial);
		}

		if (g_shaderCompilerData)
		{
			MASH_DELETE_T(sShaderCompilerData, g_shaderCompilerData);
			g_shaderCompilerData = 0;
		}
        
        MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_INFORMATION, 
							"MaterialLoader::LoadMaterialFile",
							"Material '%s' loaded.",
							sMaterialFileName);

		return aMASH_OK;
	}

	eMASH_STATUS MaterialLoader::GetVertex(mash::MashVideo *pRenderer,
		const MashArray<sVertexElement> &vertexDeclaration, 
		MashArray<sMashVertexElement> &elmsOut)						
	{
		int iCurrentStride = 0;
		int iUsageIndex[aVERTEX_DECLUSAGE_COUNT];
		memset(iUsageIndex, 0, sizeof(iUsageIndex));

		//position
		const unsigned int iVertexElementCount = vertexDeclaration.Size();
		unsigned int lastStream = 0;
		for(unsigned int i = 0; i < iVertexElementCount; ++i)
		{
			//handles changing streams
			if (vertexDeclaration[i].stream > lastStream)
			{
				lastStream = vertexDeclaration[i].stream;
				iCurrentStride = 0;
			}

			sMashVertexElement newElm;
			newElm.stride = iCurrentStride;
			newElm.stream = vertexDeclaration[i].stream;
			newElm.usage = vertexDeclaration[i].declUsage;
			newElm.type = vertexDeclaration[i].declType;
			newElm.classification = (vertexDeclaration[i].stepRate == 0)?mash::aCLASSIFICATION_VERTEX_DATA:mash::aCLASSIFICATION_INSTANCE_DATA;
			newElm.instanceStepRate = vertexDeclaration[i].stepRate;
			newElm.usageIndex = iUsageIndex[newElm.usage];

			++iUsageIndex[newElm.usage];
			iCurrentStride += helpers::GetVertexDeclTypeSize(newElm.type);
			elmsOut.PushBack(newElm);
		}

		if (elmsOut.Empty())
			return aMASH_FAILED;

		return aMASH_OK;
	}

	MashMaterial* MaterialLoader::CreateReferenceMaterial(mash::MashVideo *pRenderer, sMaterial *pMaterial, bool reloadMaterial)
	{
		MashMaterialManager *pSkinManager = pRenderer->GetMaterialManager();

		//has it already been loaded?
		MashMaterial *pNewMaterial = pSkinManager->FindMaterial(pMaterial->sMaterialName);
		if (pNewMaterial)
        {
            if (reloadMaterial)
                pNewMaterial->_OnReload();
            else
                return pNewMaterial;
        }

		MashMaterial *pReferenceMaterial = pSkinManager->FindMaterial(pMaterial->sRefName);
		if (!pReferenceMaterial)
		{
			MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_ERROR, 
				"MaterialLoader::CreateReferenceMaterial", 
				"Reference material '%s' doesn't exist.",
				pMaterial->sRefName);
			
			return 0;
		}

        if (!pNewMaterial)
            pNewMaterial = pReferenceMaterial->CreateInstance(pMaterial->sMaterialName);
        
		if (!pNewMaterial)
		{
			MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_ERROR, 
				"MaterialLoader::CreateReferenceMaterial", 
				"Failed to create material instance '%s'.", pMaterial->sMaterialName);
			
			return 0;
		}

		/*
			There's alot of code here for something fairly simple!
			We loop through all the techniques and apply new textures
			and samplers if needed.
		*/
		const unsigned int iNewMatTechniqueCount = pMaterial->techniques.Size();

		const std::map<MashStringc, MashArray<MashTechniqueInstance*> > &materialTechniques = pReferenceMaterial->GetTechniqueList();
		std::map<MashStringc, MashArray<MashTechniqueInstance*> >::const_iterator groupStart = materialTechniques.begin();
		std::map<MashStringc, MashArray<MashTechniqueInstance*> >::const_iterator groupEnd = materialTechniques.end();
		for(; groupStart != groupEnd; ++groupStart)
		{
			const unsigned int iTechCount = groupStart->second.Size();
			for(unsigned int iTech = 0; iTechCount != iTech; ++iTech)
			{
				MashTechniqueInstance *pSceneTechnique = groupStart->second[iTech];

				MashArray<sSampler> *samplerToUse = 0;
				/*
					If the user has stated techniques they wish to override then
					we find them here
				*/
				for(unsigned int iNewMatTechnique = 0; iNewMatTechnique < iNewMatTechniqueCount; ++iNewMatTechnique)
				{
					if (strcmp(pMaterial->techniques[iNewMatTechnique].sName, pSceneTechnique->GetTechniqueName().GetCString()) == 0)
					{
						samplerToUse = &pMaterial->techniques[iNewMatTechnique].samplers;
						break;
					}

					//override declared but the technique was not found in the reference
					if (!samplerToUse)
					{
						MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_ERROR, 
							"MaterialLoader::CreateReferenceMaterial",
							"Override technique declared '%s' in material '%s' but was not found in the reference.",
							pMaterial->techniques[iNewMatTechnique].sName, pMaterial->sMaterialName);

						return 0;
					}
				}
			
				//no override found so use the reference material sampler if set
				if ((!samplerToUse || (samplerToUse && samplerToUse->Empty())) && !pMaterial->samplers.Empty())
					samplerToUse = &pMaterial->samplers;

				if (samplerToUse)
				{
					MashArray<unsigned int> samplerIndexes;
					for(unsigned int iTex = 0; iTex < samplerToUse->Size(); ++iTex)
					{
						if (samplerIndexes.Search((*samplerToUse)[iTex].index) != samplerIndexes.End())
						{
							MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_WARNING, 
								"MaterialLoader::CreateReferenceMaterial",
								"Sampler indexes are not unique in technique '%s'. This will cause sampler information to be overwritten.",
								pSceneTechnique->GetTechniqueName().GetCString());
						}
						else
						{
							samplerIndexes.PushBack((*samplerToUse)[iTex].index);
						}

						mash::MashTexture *pTexture = 0;
						if ((*samplerToUse)[iTex].sTextureFile)
							pTexture = pRenderer->GetTexture((*samplerToUse)[iTex].sTextureFile);

						sSamplerState samplerState;
						samplerState.type = (*samplerToUse)[iTex].type;
						samplerState.uMode = (*samplerToUse)[iTex].addressU;
						samplerState.vMode = (*samplerToUse)[iTex].addressV;
						samplerState.filter = MaterialSamplerFilterToEngineState((*samplerToUse)[iTex].minMagFilter,
							(*samplerToUse)[iTex].mipFilter);

						MashTextureState *pTextureState = pRenderer->AddSamplerState(samplerState);

						if (pTexture)
							pSceneTechnique->SetTexture((*samplerToUse)[iTex].index, pTexture);
						if (pTextureState)
							pSceneTechnique->SetTextureState((*samplerToUse)[iTex].index, pTextureState);
					}	
				}
			}
		}

		return pNewMaterial;
	}

	MashMaterial* MaterialLoader::CreateMaterial(mash::MashVideo *pRenderer, sMaterial *pMaterial, 
		const sEffectMacro *compileArgs, unsigned int argCount, bool reloadMaterial)
	{
		MashMaterialManager *pSkinManager = pRenderer->GetMaterialManager();

		MashMaterial *pNewMaterial = pSkinManager->FindMaterial(pMaterial->sMaterialName);
		if (pNewMaterial)
        {
            if (reloadMaterial)
                pNewMaterial->_OnReload();
            else
                return pNewMaterial;
        }

		const unsigned int iTechniqueCount = pMaterial->techniques.Size();
		if (iTechniqueCount == 0)
		{
			MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_WARNING, 
				"MaterialLoader::CreateMaterial", 
				"Material '%s' wasn't created because it doesn't contain any techniques.",
				pMaterial->sMaterialName);

			return 0;
		}

		//check vertex declarations
		if (!pMaterial->vertexDeclaration.Empty())
		{
			//set all techniques to use the common vertex declaration
			for(unsigned int iTechnique = 0; iTechnique < iTechniqueCount; ++iTechnique)
				pMaterial->techniques[iTechnique].vertexDeclaration = pMaterial->vertexDeclaration;
		}
		else
		{
			MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_ERROR, 
				"MaterialLoader::CreateMaterial", 
				"Failed to create vertex declaration for material '%s'.",
				pMaterial->sMaterialName);
		}

		//check common blend states
		if (pMaterial->blendState)
		{
			//copy blend state to all techniques
			for(unsigned int iTechnique = 0; iTechnique < iTechniqueCount; ++iTechnique)
				pMaterial->techniques[iTechnique].blendState = *pMaterial->blendState;
		}

		//check common rasterizer
		if (pMaterial->rasteriserState)
		{
			//copy rasterizer to all techniques
			for(unsigned int iTechnique = 0; iTechnique < iTechniqueCount; ++iTechnique)
				pMaterial->techniques[iTechnique].rasteriserState = *pMaterial->rasteriserState;
		}

		//check common sampler states
		if (!pMaterial->samplers.Empty())
		{
			//copy sampler to all techniques that are not empty
			for(unsigned int iTechnique = 0; iTechnique < iTechniqueCount; ++iTechnique)
			{
				if (pMaterial->techniques[iTechnique].samplers.Empty())
					pMaterial->techniques[iTechnique].samplers = pMaterial->samplers;
			}
		}

		//generate the vertex declaration
		MashArray<sMashVertexElement> vertexDeclElements;
		if (GetVertex(pRenderer, pMaterial->vertexDeclaration, vertexDeclElements) == aMASH_FAILED)
		{
			MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_ERROR, 
				"MaterialLoader::CreateMaterial",
				"Failed to vertex declaration for material '%s'.",
				pMaterial->sMaterialName);
			return 0;
		}

        if (!pNewMaterial)
            pNewMaterial = pSkinManager->AddMaterial(pMaterial->sMaterialName, &vertexDeclElements[0], vertexDeclElements.Size());
        
		if (!pNewMaterial)
		{
			MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_ERROR, 
				"MaterialLoader::CreateMaterial", 
				"Failed to create material '%s'.",
				pMaterial->sMaterialName);
			return 0;
		}

		if (!pMaterial->lodDistances.Empty())
			pNewMaterial->SetLodStartDistances(&pMaterial->lodDistances[0], pMaterial->lodDistances.Size());

		for(unsigned int iTechnique = 0; iTechnique < iTechniqueCount; ++iTechnique)
		{
			sTechnique *pTechnique = &pMaterial->techniques[iTechnique];

			if (pTechnique->vertexDeclaration.Empty())
			{
				MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_ERROR, 
					"MaterialLoader::CreateMaterial", 
					"Technique '%s' in material '%s' has no vertex declaration.",
					pTechnique->sName, pMaterial->sMaterialName);

				return 0;
			}

			/*
				Convert all auto profiles to a profile that will
				work on the current system
			*/
			bool isTechniqueValid = true;
			for(unsigned int i = 0; i < aTECH_PROG_COUNT; ++i)
			{
				if (helpers::IsValidString(pTechnique->programs[i].profileString))
				{
					if (strcmp(pTechnique->programs[i].profileString, "auto") == 0)
					{
						switch(i)
						{
						case aTECH_PROG_VERTEX:
							pTechnique->programs[aTECH_PROG_VERTEX].profileString =  helpers::GetShaderProfileString(pSkinManager->GetLatestVertexProfile());
							break;
						case aTECH_PROG_PIXEL:
							pTechnique->programs[aTECH_PROG_PIXEL].profileString = helpers::GetShaderProfileString(pSkinManager->GetLatestFragmentProfile());
							break;
						case aTECH_PROG_GEOMETRY:
							pTechnique->programs[aTECH_PROG_GEOMETRY].profileString = helpers::GetShaderProfileString(pSkinManager->GetLatestGeometryProfile());
							break;
						case aTECH_PROG_SHADOW_VERTEX:
							pTechnique->programs[aTECH_PROG_SHADOW_VERTEX].profileString = helpers::GetShaderProfileString(pSkinManager->GetLatestVertexProfile());
							break;
						};
					}
					
					pTechnique->programs[i].profileEnum = helpers::GetShaderProfileFromString(pTechnique->programs[i].profileString);

					if (!pSkinManager->IsProfileSupported(pTechnique->programs[i].profileEnum))
					{
						isTechniqueValid = false;
						break;
					}
				}
				else
				{
					pTechnique->programs[i].profileEnum = aSHADER_PROFILE_UNKNOWN;
				}
			}

			if (!helpers::IsValidString(pTechnique->programs[aTECH_PROG_VERTEX].fileName))
			{
				MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_ERROR, 
					"MaterialLoader::CreateMaterial", 
					"Failed to create technique '%s' in material '%s'. A vertex program is mising.",
					pTechnique->sName, pMaterial->sMaterialName);

				return 0;
			}

			if (!helpers::IsValidString(pTechnique->programs[aTECH_PROG_PIXEL].fileName))
			{
				MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_ERROR, 
					"MaterialLoader::CreateMaterial", 
					"Failed to create technique '%s' in material '%s'. A pixel program is mising.",
					pTechnique->sName, pMaterial->sMaterialName);

				return 0;
			}

			/*
				If you want to start saving compiled material data, then you will need to remove
				this check as its only needed for runtime generation.
			*/
			if (!isTechniqueValid)
			{
				MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_INFORMATION, 
					"MaterialLoader::CreateMaterial", 
					"Skipping technique creation '%s' in material '%s'. Technique is not valid on the current system.",
					pTechnique->sName, pMaterial->sMaterialName);

				continue;
			}
			
			bool isVertexProgramEffectType = helpers::IsFileANativeEffectProgram(pTechnique->programs[aTECH_PROG_VERTEX].fileName);
			bool isPixelProgramEffectType = helpers::IsFileANativeEffectProgram(pTechnique->programs[aTECH_PROG_PIXEL].fileName);

			if (isVertexProgramEffectType ^ isPixelProgramEffectType)
			{
				MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_ERROR, 
					"MaterialLoader::CreateMaterial", 
					"Failed to create technique '%s' in material '%s'. Both vertex and pixel shaders must have the same file extention if .eff files are used.",
					pTechnique->sName, pMaterial->sMaterialName);

				return 0;
			}

			//create blend states
			int blendState = pRenderer->AddBlendState(pTechnique->blendState);
			//create rasterizer
			int rasterizer = pRenderer->AddRasteriserState(pTechnique->rasteriserState);

			MashTechniqueInstance *pNewSceneTechnique = pNewMaterial->AddTechniqueToGroup(pTechnique->sGroupName, pTechnique->sName);	
			if (!pNewSceneTechnique)
			{
				MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_ERROR, 
					"MaterialLoader::CreateMaterial", 
					"Failed to create technique '%s' in material '%s'",
					pTechnique->sName, pMaterial->sMaterialName);

				return 0;
			}

			MashTechnique *sharedTechnique = pNewSceneTechnique->GetTechnique();

			if (pTechnique->sShadingEffect)
				sharedTechnique->SetCustomLightShadingEffect(MashStringc(pTechnique->sShadingEffect));

			for(unsigned int i = 0; i < aTECH_PROG_COUNT; ++i)
			{
				//leave loading the shadow shaders till after this step
				if ((i != aTECH_PROG_SHADOW_VERTEX) && helpers::IsValidString(pTechnique->programs[i].fileName))
				{
					MashEffectProgram *newEffectProgram = sharedTechnique->GetEffect()->AddProgram(pTechnique->programs[i].fileName,
								pTechnique->programs[i].entry,
								pTechnique->programs[i].profileEnum);

					if (!newEffectProgram)
					{
						MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, "Failed to add shader to technique.", "MaterialLoader::CreateMaterial");
						return 0;
					}

					//add macros from the material
					unsigned int defineCount = pTechnique->programs[i].macros.Size();
					for(unsigned int mac = 0; mac < defineCount; ++mac)
					{
						newEffectProgram->AddCompileArgument(pTechnique->programs[i].macros[mac]);
					}

					//add macros from runtime
					if (argCount > 0)
					{
						for(unsigned int mac = 0; mac < argCount; ++mac)
						{
							newEffectProgram->AddCompileArgument(compileArgs[mac]);
						}
					}
				}
			}

			//generate shadow program if needed
			if (helpers::IsValidString(pTechnique->programs[aTECH_PROG_SHADOW_VERTEX].fileName))
			{
				//generate the shaders if needed
				if (!helpers::IsFileANativeEffectProgram(pTechnique->programs[aTECH_PROG_SHADOW_VERTEX].fileName))
				{
					MashStringc generatedPixelFileName;
					for(unsigned int shadowEffectIndex = 0; shadowEffectIndex < aLIGHT_TYPE_COUNT; ++shadowEffectIndex)
					{
						if (!pTechnique->disabledShadowCasters[shadowEffectIndex])
						{
							MashEffect *newEffect = 0;
							//create the shadow effects
							generatedPixelFileName.Clear();
							switch(shadowEffectIndex)
							{
							case aLIGHT_DIRECTIONAL:
								newEffect = sharedTechnique->InitialiseShadowEffect(aLIGHT_DIRECTIONAL);
								generatedPixelFileName = "GeneratedPixelDirShadowCaster.eff";
								break;
							case aLIGHT_SPOT:
								newEffect = sharedTechnique->InitialiseShadowEffect(aLIGHT_SPOT);
								generatedPixelFileName = "GeneratedPixelSpotShadowCaster.eff";
								break;
							case aLIGHT_POINT:
								newEffect = sharedTechnique->InitialiseShadowEffect(aLIGHT_POINT);
								generatedPixelFileName = "GeneratedPixelPointShadowCaster.eff";
								break;
							};

							if (!newEffect)
							{
								//error
								continue;
							}

							MashEffectProgram *vertexProgram = newEffect->AddProgram(pTechnique->programs[aTECH_PROG_SHADOW_VERTEX].fileName,
								pTechnique->programs[aTECH_PROG_SHADOW_VERTEX].entry,
								pTechnique->programs[aTECH_PROG_SHADOW_VERTEX].profileEnum);

							if (!vertexProgram)
							{
								MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, "Failed to add vertex shadow caster to technique.", "MaterialLoader::CreateMaterial");
								//return 0;
							}
							else
							{
								//add defines to vertex shader
								unsigned int defineCount = pTechnique->programs[aTECH_PROG_SHADOW_VERTEX].macros.Size();
								for(unsigned int mac = 0; mac < defineCount; ++mac)
								{
									vertexProgram->AddCompileArgument(pTechnique->programs[aTECH_PROG_SHADOW_VERTEX].macros[mac]);
								}
							}

							/*
								The values for the shadow pixel shader are fudged. They will be updated
								when the effect is compiled...this program is a special case. 

								The target profile for the pixel program is based on the vertex program.
							*/
							MashEffectProgram *pixelProgram = newEffect->AddProgram(generatedPixelFileName.GetCString(),
								"main",
								helpers::GetPixelProfileFromVertexProfile(pTechnique->programs[aTECH_PROG_SHADOW_VERTEX].profileEnum));

							if (!pixelProgram)
							{
								MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, "Failed to add pixel shadow caster to technique.", "MaterialLoader::CreateMaterial");
								//return 0;
							}
						}
					}
				}
				else
				{
					MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_ERROR, 
						"MaterialLoader::CreateMaterial", 
						"Failed to create shadow effect '%s' in material '%s'. Shadow effects must be created from .eff files.",
						pTechnique->sName, pMaterial->sMaterialName);

					//reports an error but no need to quit
				}
			}

			const unsigned int iLodLevelCount = pTechnique->supportedLodLevels.Size();
			//always have at least lod level 0
			if (iLodLevelCount == 0)
				sharedTechnique->AddLodLevelSupport(0);
			else
			{
				for(unsigned int iLod = 0; iLod < iLodLevelCount; ++iLod)
					sharedTechnique->AddLodLevelSupport(pTechnique->supportedLodLevels[iLod]);
			}

			sharedTechnique->SetLightingType(pTechnique->lightingType);
			sharedTechnique->SetBlendStateIndex(blendState);
			sharedTechnique->SetRasteriserStateIndex(rasterizer);
			
			for(unsigned int iTex = 0; iTex < pTechnique->samplers.Size(); ++iTex)
			{
				mash::MashTexture *pTexture = 0;
				if (pTechnique->samplers[iTex].sTextureFile)
					pTexture = pRenderer->GetTexture(pTechnique->samplers[iTex].sTextureFile);

				sSamplerState samplerState;
				samplerState.type = pTechnique->samplers[iTex].type;
				samplerState.uMode = pTechnique->samplers[iTex].addressU;
				samplerState.vMode = pTechnique->samplers[iTex].addressV;
				samplerState.filter = MaterialSamplerFilterToEngineState(pTechnique->samplers[iTex].minMagFilter,
					pTechnique->samplers[iTex].mipFilter);

				MashTextureState *pTextureState = pRenderer->AddSamplerState(samplerState);

				if (pTexture)
					pNewSceneTechnique->SetTexture(pTechnique->samplers[iTex].index, pTexture);
				if (pTextureState)
					pNewSceneTechnique->SetTextureState(pTechnique->samplers[iTex].index, pTextureState);
			}			
		}

		return pNewMaterial;
	}
}
