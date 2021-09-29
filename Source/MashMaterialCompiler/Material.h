//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MATERIAL_H_
#define _MATERIAL_H_

#include "MashTypes.h"
#include "MashArray.h"
#include "MashString.h"
#include "MashTechniqueInstance.h"
#include "MashMemoryPoolLexer.h"

namespace mash
{

	/*
		Note, all strings belong to a memory pool and not these objects.
	*/

	inline void ResetRasterizer(sRasteriserStates &state)
	{
		state = sRasteriserStates();
	}

	inline void ResetBlendState(sBlendStates &state)
	{
		state = sBlendStates();
	}

	enum eTECHNIQUE_PROGRAM_TYPE
	{
		aTECH_PROG_VERTEX,
		aTECH_PROG_GEOMETRY,
		aTECH_PROG_PIXEL,

		aTECH_PROG_SHADOW_VERTEX,

		aTECH_PROG_COUNT
	};

	enum eTECHNIQUE_SHADOW_PROGRAM_TYPE
	{
		aTECH_PROG_SHADOW_DIR_VERTEX,
		aTECH_PROG_SHADOW_DIR_PIXEL,
		aTECH_PROG_SHADOW_SPOT_VERTEX,
		aTECH_PROG_SHADOW_SPOT_PIXEL,
		aTECH_PROG_SHADOW_POINT_VERTEX,
		aTECH_PROG_SHADOW_POINT_PIXEL,

		aTECH_PROG_SHADOW_COUNT
	};

	enum eTEXTURE_FILTERS
	{
		TEX_FILTER_LINEAR,
		TEX_FILTER_POINT,
		TEX_FILTER_MIPNONE,

		TEX_FILTER_COUNT
	};

	enum eVERTEX_OUTPUT
	{
		aVERTEX_OUTPUT_VPOS,
		aVERTEX_OUTPUT_VNORM,
		aVERTEX_OUTPUT_SPECULAR,
		aVERTEX_OUTPUT_COLOR,
		aVERTEX_OUTPUT_TEXCOORD,
		aVERTEX_OUTPUT_CUSTOM,
		aVERTEX_OUTPUT_HPOS,
		aVERTEX_OUTPUT_LIGHT_DIFFUSE,
		aVERTEX_OUTPUT_LIGHT_SPECULAR,

		aVERTEX_OUTPUT_COUNT
	};

	enum ePIXEL_OUTPUT
	{
		aPIXEL_OUTPUT_DIFFUSE,
		aPIXEL_OUTPUT_SPECULAR,
		aPIXEL_OUTPUT_VNORM,

		aPIXEL_OUTPUT_COUNT
	};

	struct sVertexElement
	{
		mash::eVERTEX_DECLUSAGE declUsage;
		mash::eVERTEX_DECLTYPE declType;
		unsigned int stream;
		unsigned int stepRate;

		bool operator==(const sVertexElement &other)
		{
			return ((declUsage == other.declUsage) && 
				(declType == other.declType) &&
				(stream == other.stream) &&
				(stepRate == other.stepRate));
		}
	};

	struct sProgramDetails
	{

		//file to load and/or save name. May be NULL.
		const char *fileName;
		//used during loading
		const char *profileString;
		//profile this program is targeting
		eSHADER_PROFILE profileEnum;
		//program entry
		const char *entry;

		//used only during HLSL2GLSL conversions
		MashArray<sEffectMacro> macros;

		bool operator==(const sProgramDetails &other)const
		{
			return (fileName == other.fileName &&
				profileString == other.profileString && 
				entry == other.entry);
		}

		sProgramDetails():fileName(0), profileString(0), entry(0){}
	};

	struct sSampler
	{
		char *sSamplerName;
		char *sTextureFile;
		unsigned int index;
		mash::eSAMPLER type;
		eTEXTURE_FILTERS minMagFilter;
		eTEXTURE_FILTERS mipFilter;

		mash::eTEXTURE_ADDRESS addressU;
		mash::eTEXTURE_ADDRESS addressV;

		void Reset()
		{
			index = 0;
			sSamplerName = 0;
			sTextureFile = 0;
			type = mash::aSAMPLER2D;
			minMagFilter = TEX_FILTER_LINEAR;
			mipFilter = TEX_FILTER_LINEAR;
			addressU = mash::aTEXTURE_ADDRESS_CLAMP;
			addressV = mash::aTEXTURE_ADDRESS_CLAMP;
		}

		void Delete()
		{
			Reset();
		}

		sSampler():sSamplerName(0), sTextureFile(0){Reset();}
		~sSampler(){}
	};

	struct sTechnique
	{
		char *sName;
		char *sGroupName;
		char *sShadingEffect;
		sRasteriserStates rasteriserState;
		sBlendStates blendState;
		MashArray<sVertexElement> vertexDeclaration;
		MashArray<sSampler> samplers;
		MashArray<unsigned int> supportedLodLevels;
		mash::sProgramDetails programs[aTECH_PROG_COUNT];
		mash::sProgramDetails shadowVertexProgram;
		mash::eLIGHTING_TYPE lightingType;
		bool disabledShadowCasters[aLIGHT_TYPE_COUNT];

		void Reset()
		{
			for(unsigned int i = 0; i < aTECH_PROG_COUNT; ++i)
			{
				programs[i].entry = "";
				programs[i].fileName = "";
				programs[i].profileString = "";
				programs[i].profileEnum = aSHADER_PROFILE_UNKNOWN;

				if (!programs[i].macros.Empty())
					programs[i].macros.Clear();
			}

			for(unsigned int i = 0; i < aLIGHT_TYPE_COUNT; ++i)
				disabledShadowCasters[i] = false;

			if (!samplers.Empty())
				samplers.Clear();

			if (!vertexDeclaration.Empty())
				vertexDeclaration.Clear();

			sName = 0;
			sGroupName = 0;
			sShadingEffect = 0;
			ResetRasterizer(rasteriserState);
			ResetBlendState(blendState);

			if (!supportedLodLevels.Empty())
				supportedLodLevels.Clear();

			lightingType = mash::aLIGHT_TYPE_NONE;
		}

		void Delete()
		{
			for(unsigned int i = 0; i < samplers.Size(); ++i)
			{
				samplers[i].Delete();
			}

			Reset();
		}

		sTechnique():sName(0), sGroupName(0) {Reset();}
		~sTechnique(){}
	};

	struct sMaterial
	{
		sRasteriserStates *rasteriserState;
		sBlendStates *blendState;
		MashArray<sVertexElement> vertexDeclaration;
		MashArray<sSampler> samplers;
		MashArray<sTechnique> techniques;
		MashArray<unsigned int> lodDistances;
		char *sMaterialName;
		char *sShadowMaterialName;
		char *sRefName;
		MashArray<MashStringc> userString;

		//TODO : Remove all references to this and from the lexer
		bool meshLodEnabled;

		void CreateRef(char *thisMaterialName, char *refName)
		{
			sMaterialName = thisMaterialName;
			sRefName = refName;
		}

		void Reset()
		{
			for(unsigned int i = 0; i < techniques.Size(); ++i)
			{
				techniques[i].Reset();
			}

			lodDistances.Clear();
			vertexDeclaration.Clear();
			samplers.Clear();
			techniques.Clear();
			sMaterialName = 0;
			rasteriserState = 0;
			blendState = 0;
			sShadowMaterialName = 0;
			sRefName = 0;
			meshLodEnabled = false;
		}

		void Delete()
		{
			for(unsigned int i = 0; i < techniques.Size(); ++i)
			{
				techniques[i].Delete();
			}

			techniques.Clear();

			if (rasteriserState)
			{
				delete rasteriserState;
				rasteriserState = 0;
			}

			if (blendState)
			{
				delete blendState;
				blendState = 0;
			}

			Reset();
		}

		sMaterial(){Reset();}
		~sMaterial(){}
	};

	struct sShaderCompilerData
	{
		mash::sBlendStates g_blendState;
		mash::sRasteriserStates g_currentRasteriser;
		mash::sMaterial g_currentMaterial;
		mash::sTechnique g_currentTechnique;
		mash::sSampler g_currentSampler;
		mash::MashArray<mash::sEffectMacro> g_tempDefineList;
		mash::MashArray<mash::sVertexElement> g_vertexDeclaration;
		mash::MashArray<mash::sMaterial> g_materials;
		MashMemoryPoolLexer *g_materialLexerMemoryPool;

		sShaderCompilerData()
		{
			g_materialLexerMemoryPool = MASH_NEW_T_COMMON(MashMemoryPoolLexer);
		}
		~sShaderCompilerData()
		{
			MASH_DELETE_T(MashMemoryPoolLexer, g_materialLexerMemoryPool);

			g_currentMaterial.Delete();
			g_currentTechnique.Delete();
			g_currentSampler.Delete();

			const unsigned int iMaterialCount = g_materials.Size();
			for(unsigned int i = 0; i < iMaterialCount; ++i)
			{
				g_materials[i].Delete();
			}
		}
	};

}

#endif