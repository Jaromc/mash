//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#include "MashString.h"
#include <sstream>
#include "MashCompileSettings.h"
#include "MashEffectProgram.h"
#include "MashEffect.h"
#include "MashMaterialManager.h"
#include "CMashShaderCompiler.h"
#include "MashFileStream.h"
#include "MashString.h"
#include "MashStringHelper.h"
#include "MashDevice.h"
#include "MashTimer.h"
bool g_parsingHLSLFiles;
static mash::CMashShaderCompiler::sEffectScriptData *g_mashCurrentParsingEffect = 0;
static const mash::uint32 g_MemPoolTypeSize = 10000;

_MASH_EXPORT std::string MashGetHLSLParserLineToUserLineString(int line)
{
    std::stringstream lineString;
    
    if (g_parsingHLSLFiles && g_mashCurrentParsingEffect)
    {
        int convertedLine = 0;
        {
            convertedLine = line + g_mashCurrentParsingEffect->userSourceScriptLineStart;
            if (g_mashCurrentParsingEffect->userSourceLineStart > 0)
                convertedLine = convertedLine - g_mashCurrentParsingEffect->userSourceLineStart;
        }
        
        lineString << "Effect line: '" << convertedLine << "' Intermediate line: '" << line << "': ";
    }
    else
    {
        lineString << "' Intermediate line: '" << line << "': ";
    }
    return lineString.str();
}


#include "MashSceneManager.h"
#include "MashFileManager.h"
#include "MashVideo.h"
#include "MashLight.h"
#include "MashHelper.h"
#include "HLSL2GLSL.h"
#include "Material.h"

#include "MashLog.h"

namespace mash
{  
	enum eSHADER_DEFINE
	{
		aSHADER_DEFINE_VERTEX_LIGHT,
		aSHADER_DEFINE_PIXEL_LIGHT,
		aSHADER_DEFINE_DEFERRED,

		aSHADER_DEFINE_SPOT_SHADOWS,
		aSHADER_DEFINE_DIRECTIONAL_SHADOWS,
		aSHADER_DEFINE_POINT_SHADOWS,
		aSHADER_DEFINE_FOG,

		aSHADER_DEFINE_DIRECTX,
		aSHADER_DEFINE_OPENGL
	};

	static const int8 *const g_pShaderDefineTypes[] = {
		"_DEFINE_VERTEX_LIGHTING",
		"_DEFINE_PIXEL_LIGHTING",
		"_DEFINE_DEFERRED_LIGHTING",

		"_DEFINE_SPOT_SHADOWS",
		"_DEFINE_DIRECTIONAL_SHADOWS",
		"_DEFINE_POINT_SHADOWS",
		"_DEFINE_FOG",

		"_DEFINE_DIRECTX",
		"_DEFINE_OPENGL",
		0
	};

	/*
		Names with a postfix _g are runtime generated
	*/
	static const int8 *const g_effectIncludes[] = {
		"MashDirectionalLighting.eff",
		"MashSpotLighting.eff",
		"MashPointLighting.eff",
		"MashLightShading.eff",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"MashLightStructures.eff",
		"MashForwardRenderedLighting_g.eff",
		"MashDirectionalDeferredLighting_g.eff",
		"MashSpotDeferredLighting_g.eff",
		"MashPointDeferredLighting_g.eff",
		0
	};

	static const int8 *const g_effectDataTypes[] = {
		"float",
		"float2",
		"float3",
		"float4",
		"float4x4",
		0
	};

	CMashShaderCompiler::CMashShaderCompiler(MashVideo *renderer):m_renderer(renderer), m_isBatchCompileEnabled(false),
		m_isMemoryPoolInitialised(false)
	{
		//set up includes, these can be overriden by the user
		for(uint32 i = 0; i < aEFF_INC_COUNT; ++i)
			m_effectIncludes[i] = g_effectIncludes[i];
	}

	CMashShaderCompiler::~CMashShaderCompiler()
	{
	}

	eVERTEX_DECLUSAGE CMashShaderCompiler::ConvertInputSemanticStringToEnum(const int8 *s)
	{
		if (scriptreader::CompareStrings(s, "position"))
			return aDECLUSAGE_POSITION;
		if (scriptreader::CompareStrings(s, "texcoord"))
			return aDECLUSAGE_TEXCOORD;
		if (scriptreader::CompareStrings(s, "custom"))
			return aDECLUSAGE_CUSTOM;
		if (scriptreader::CompareStrings(s, "colour"))
			return aDECLUSAGE_COLOUR;
		if (scriptreader::CompareStrings(s, "normal"))
			return aDECLUSAGE_NORMAL;
		if (scriptreader::CompareStrings(s, "blendweight"))
			return aDECLUSAGE_BLENDWEIGHT;
		if (scriptreader::CompareStrings(s, "blendindex"))
			return aDECLUSAGE_BLENDINDICES;
		if (scriptreader::CompareStrings(s, "tangent"))
			return aDECLUSAGE_TANGENT;

		return aVERTEX_DECLUSAGE_COUNT;
	}

	eVERTEX_OUTPUT CMashShaderCompiler::ConvertVertexOutputSemanticStringToEnum(const int8 *s)
	{
		if (scriptreader::CompareStrings(s, "viewposition"))
			return aVERTEX_OUTPUT_VPOS;
		if (scriptreader::CompareStrings(s, "viewnormal"))
			return aVERTEX_OUTPUT_VNORM;
		if (scriptreader::CompareStrings(s, "specular"))
			return aVERTEX_OUTPUT_SPECULAR;
		if (scriptreader::CompareStrings(s, "colour"))
			return aVERTEX_OUTPUT_COLOR;
		if (scriptreader::CompareStrings(s, "texcoord"))
			return aVERTEX_OUTPUT_TEXCOORD;
		if (scriptreader::CompareStrings(s, "custom"))
			return aVERTEX_OUTPUT_CUSTOM;
		if (scriptreader::CompareStrings(s, "hposition"))
			return aVERTEX_OUTPUT_HPOS;

		return aVERTEX_OUTPUT_COUNT;
	}

	ePIXEL_OUTPUT CMashShaderCompiler::ConvertPixelOutputSemanticStringToEnum(const int8 *s)
	{
		if (scriptreader::CompareStrings(s, "diffuse"))
			return aPIXEL_OUTPUT_DIFFUSE;
		if (scriptreader::CompareStrings(s, "specular"))
			return aPIXEL_OUTPUT_SPECULAR;
		if (scriptreader::CompareStrings(s, "viewnormal"))
			return aPIXEL_OUTPUT_VNORM;

		return aPIXEL_OUTPUT_COUNT;
	}

	const int8* CMashShaderCompiler::InputSemanticTypeToHLSLString(eVERTEX_DECLUSAGE semantic)
	{
		switch(semantic)
		{
		case aDECLUSAGE_POSITION:
			return "POSITION";
		case aDECLUSAGE_BLENDWEIGHT:
			return "BLENDWEIGHT";
		case aDECLUSAGE_BLENDINDICES:
			return "BLENDINDICES";
		case aDECLUSAGE_NORMAL:
			return "NORMAL";
		case aDECLUSAGE_TEXCOORD:
			return "TEXCOORD";
		case aDECLUSAGE_CUSTOM:
			return "CUSTOM";
		case aDECLUSAGE_TANGENT:
			return "TANGENT";
		case aDECLUSAGE_COLOUR:
			return "COLOR";
		};

		return "";
	}

	const int8* CMashShaderCompiler::VertexOutputInterpolatorToHLSLString(eVERTEX_OUTPUT semantic)
	{
		switch(semantic)
		{
		case aVERTEX_OUTPUT_COLOR:
			return "COLOR";
		case aVERTEX_OUTPUT_TEXCOORD:
			return "TEXCOORD";
		};

		return "";
	}

	eVERTEX_OUTPUT CMashShaderCompiler::GetVertexOutputSemanticAsInterpolator(eVERTEX_OUTPUT semantic)
	{
		switch(semantic)
		{
		case aVERTEX_OUTPUT_VPOS:
		case aVERTEX_OUTPUT_VNORM:
		case aVERTEX_OUTPUT_SPECULAR:
		case aVERTEX_OUTPUT_TEXCOORD:
		case aVERTEX_OUTPUT_CUSTOM:
		case aVERTEX_OUTPUT_HPOS:
			return aVERTEX_OUTPUT_TEXCOORD;
		case aVERTEX_OUTPUT_LIGHT_DIFFUSE:
		case aVERTEX_OUTPUT_LIGHT_SPECULAR:
		case aVERTEX_OUTPUT_COLOR:
			return aVERTEX_OUTPUT_COLOR;
		};

		return aVERTEX_OUTPUT_TEXCOORD;
	}

	mash::eVERTEX_DECLUSAGE CMashShaderCompiler::GetVertexInputSemanticAsHLSLType(mash::eVERTEX_DECLUSAGE usage)
	{
		if (usage == aDECLUSAGE_CUSTOM)
			return aDECLUSAGE_TEXCOORD;

		return usage;
	}

	void CMashShaderCompiler::AddEffectAutoUnique(eEFFECT_DATA_TYPE dataType, eEFFECT_SEMANTICS autoParam, MashArray<sEffectScriptData::sAutoDeclaration> &autos)
	{
		const uint32 size = autos.Size();
		for(uint32 i = 0; i < size; ++i)
		{
			if (scriptreader::CompareStrings(autos[i].name.GetCString(), g_effectAutoNames[autoParam]))
				return;
		}

		sEffectScriptData::sAutoDeclaration newDecl(m_stringMemoryPool);
		newDecl.name = g_effectAutoNames[autoParam];
		newDecl.type = g_effectDataTypes[dataType];
		autos.PushBack(newDecl);
	}

	eMASH_STATUS CMashShaderCompiler::ReadIncludes(const int8 *fileData, uint32 start, uint32 end, MashArray<MashShaderString> &out)
	{
		MashShaderString filename(m_stringMemoryPool);
		MashShaderString ext(m_stringMemoryPool);
		int8 p;
		while(start < end)
		{
			filename.Clear();
			ext.Clear();
			scriptreader::ReadNextString(fileData, start, end, filename);
			scriptreader::ReadNextChar(fileData, start, end, p);
			if (p != '.')
			{
				if (!filename.Empty())
				{
					MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_ERROR, 
						"CMashShaderCompiler::ReadIncludes",
						"Error reading includes in effect file. '%s' doesn't appear to have an extention.", filename.GetCString());
				}
				else
				{
					MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
						"Error reading includes in effect file. An include appears to be missing an extention", 
						"CMashShaderCompiler::ReadIncludes");
				}

				return aMASH_FAILED;
			}

			scriptreader::ReadNextString(fileData, start, end, ext);

			if (!filename.Empty() && !ext.Empty())
			{
				out.PushBack(filename + "." + ext);
			}
		}

		return aMASH_OK;
	}

	eMASH_STATUS CMashShaderCompiler::ReadVertexInput(const int8 *fileData, uint32 start, uint32 end, MashArray<sEffectScriptData::sVertexInput> &out)
	{
		eMASH_STATUS status = aMASH_OK;
		MashShaderString semantic(m_stringMemoryPool);
		while(start < end)
		{
			sEffectScriptData::sVertexInput decl(m_stringMemoryPool);
			semantic.Clear();
			uint32 typeStrLen = scriptreader::ReadNextString(fileData, start, end, decl.type);

			if (typeStrLen > 0)
			{
				if (scriptreader::ReadNextString(fileData, start, end, decl.name) == 0)
				{
					//error
					status = aMASH_FAILED;
				}

				if (scriptreader::ReadNextString(fileData, start, end, semantic) == 0)
				{
					//error
					status = aMASH_FAILED;
				}

				decl.semantic = ConvertInputSemanticStringToEnum(semantic.GetCString());

				if (decl.semantic == aVERTEX_DECLUSAGE_COUNT)
				{
					//error
					status = aMASH_FAILED;
				}

				if (status == aMASH_FAILED)
				{
					MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
						"Error reading vertex declaration in effect file.", 
						"CMashShaderCompiler::ReadVertexInput");

					break;
				}
				
				out.PushBack(decl);
			}			
		}

		return status;
	}

	eMASH_STATUS CMashShaderCompiler::ReadVertexOutput(const int8 *fileData, uint32 start, uint32 end, MashArray<sEffectScriptData::sUserFunctionOutput> &out)
	{
		eMASH_STATUS status = aMASH_OK;
		MashShaderString semantic(m_stringMemoryPool);
		MashShaderString pass(m_stringMemoryPool);
		MashShaderString lineString(m_stringMemoryPool);

		while(start < end)
		{
			lineString.Clear();
			uint32 lineLength = scriptreader::ReadLine(fileData, start, end, lineString);
			uint32 lineStart = 0;

			sEffectScriptData::sUserFunctionOutput decl(m_stringMemoryPool);
			semantic.Clear();
			scriptreader::ReadNextString(lineString.GetCString(), lineStart, lineLength, decl.type);

			//nothing to read
			if (decl.type.Empty())
				continue;

			if (scriptreader::ReadNextString(lineString.GetCString(), lineStart, lineLength, decl.name) == 0)
			{
				//error
				status = aMASH_FAILED;
			}

			if (scriptreader::ReadNextString(lineString.GetCString(), lineStart, lineLength, semantic) == 0)
			{
				//error
				status = aMASH_FAILED;
			}

			decl.vertexSemantic = ConvertVertexOutputSemanticStringToEnum(semantic.GetCString());

			if (decl.vertexSemantic == aVERTEX_OUTPUT_COUNT)
			{
				//error
				status = aMASH_FAILED;
			}

			//read optional pass string
			scriptreader::ReadNextString(lineString.GetCString(), lineStart, lineLength, pass);
			if (scriptreader::CompareStrings(pass.GetCString(), "pass"))
				decl.passToPixel = true;
			else
				decl.passToPixel = false;

			if (status == aMASH_FAILED)
			{
				MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Error reading outputs from a user vertex function in effect file.", 
					"CMashShaderCompiler::ReadVertexOutput");

				break;
			}

			decl.isUserData = true;
			out.PushBack(decl);
		}

		return status;
	}

	eMASH_STATUS CMashShaderCompiler::ReadPixelOutput(const int8 *fileData, uint32 start, uint32 end, MashArray<sEffectScriptData::sUserFunctionOutput> &out)
	{
		eMASH_STATUS status = aMASH_OK;
		MashShaderString semantic(m_stringMemoryPool);
		while(start < end)
		{
			sEffectScriptData::sUserFunctionOutput decl(m_stringMemoryPool);
			semantic.Clear();
			uint32 typeStrLen = scriptreader::ReadNextString(fileData, start, end, decl.type);

			if (typeStrLen > 0)
			{
				if (scriptreader::ReadNextString(fileData, start, end, decl.name) == 0)
				{
					//error
					status = aMASH_FAILED;
				}

				if (scriptreader::ReadNextString(fileData, start, end, semantic) == 0)
				{
					//error
					status = aMASH_FAILED;
				}

				decl.pixelSemantic = ConvertPixelOutputSemanticStringToEnum(semantic.GetCString());

				if (decl.pixelSemantic == aPIXEL_OUTPUT_COUNT)
				{
					//error
					status = aMASH_FAILED;
				}

				if (status == aMASH_FAILED)
				{
					MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
						"Error reading outputs from a user pixel function in effect file.", 
						"CMashShaderCompiler::ReadPixelOutput");

					break;
				}

				decl.isUserData = true;
				out.PushBack(decl);
			}
		}

		return status;
	}

	eMASH_STATUS CMashShaderCompiler::ReadAutos(const int8 *fileData, uint32 start, uint32 end, MashArray<sEffectScriptData::sAutoDeclaration> &out)
	{
		eMASH_STATUS status = aMASH_OK;
		sEffectScriptData::sAutoDeclaration newDecl(m_stringMemoryPool);
		MashShaderString lineString(m_stringMemoryPool);
		MashShaderString arraySizeString(m_stringMemoryPool);
		while(start < end)
		{
			lineString.Clear();
			if (scriptreader::ReadLine(fileData, start, end, lineString) > 0)
			{
				const uint32 lineLength = lineString.Size();
				newDecl.type.Clear();
				newDecl.name.Clear();
				newDecl.arraySize = 0;
				uint32 lineLoc = 0;
				scriptreader::ReadNextString(lineString.GetCString(), lineLoc, lineLength, newDecl.type);
				scriptreader::ReadNextString(lineString.GetCString(), lineLoc, lineLength, newDecl.name);

				/*
					The type and auto name have been read. If there is anything left over
					then it may be an array. So we grab the next string. If its a number
					then we have an array decl.
				*/
				if (lineLoc < lineLength)
				{
					if (scriptreader::ReadNextString(lineString.GetCString(), lineLoc, lineLength, arraySizeString) > 0)
					{
						//make sure the next element is a number, if anything
						for(uint32 i = 0; i < arraySizeString.Size(); ++i)
						{
							if (!isspace(arraySizeString[i]) && !isdigit(arraySizeString[i]))
							{
								MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_ERROR, 
									"CMashShaderCompiler::ReadAutos", 
									"Invalid value after auto '%s'.", newDecl.name.GetCString());

								return aMASH_FAILED;
							}
						}
						newDecl.arraySize = atoi(arraySizeString.GetCString());
					}
				}

				if (!newDecl.type.Empty() && !newDecl.name.Empty())
				{
					//dont worry about checking for duplicates. This will be done when the effect is compiled
					out.PushBack(newDecl);
				}
				else if (!newDecl.type.Empty() ^ !newDecl.name.Empty())
				{
					MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
							"Error reading autos from an effect file.", 
							"CMashShaderCompiler::ReadAutos");

					status = aMASH_FAILED;
					break;
				}
			}
		}

		return status;
	}

	void CMashShaderCompiler::SetAlternateInclude(eSHADER_EFFECT_INCLUDES type, const int8 *includeString)
	{
		if (!includeString)
			m_effectIncludes[type] = "";
		else
			m_effectIncludes[type] = includeString;
	}

	void CMashShaderCompiler::SetIncludeCallback(const int8 *includeString, MashEffectIncludeFunctor includeFunctor)
	{
		if (includeString && includeFunctor.IsValid())
		{
			MashStringc mashIncString = includeString;

			const uint32 c = m_effectIncludeCallbacks.Size();
			for(uint32 i = 0; i < c; ++i)
			{
				if (m_effectIncludeCallbacks[i].includeString == mashIncString)
				{
					m_effectIncludeCallbacks[i].functor = includeFunctor;
					return;
				}
			}

			//not found so add it
			m_effectIncludeCallbacks.PushBack(sEffectIncludeCallback(mashIncString, includeFunctor));
		}
	}

	void CMashShaderCompiler::BeginBatchCompile()
	{
		if (!m_isBatchCompileEnabled)
		{
			//init memory pool
			InitialiseMemoryPool();

			//now start HLSL2GLSL
			Hlsl2Glsl_Initialize();

			m_isBatchCompileEnabled = true;
		}
	}

	void CMashShaderCompiler::EndBatchCompile()
	{
		Hlsl2Glsl_Finalize();

		m_isBatchCompileEnabled = false;

		//be sure to free the pool as it will be taking up a fair chunk of memory.
		DestroyMemoryPool();
	}

	MashList<CMashShaderCompiler::sEffectScriptData>::Iterator CMashShaderCompiler::FindIncludeStringInList(MashList<sEffectScriptData> &listToSearch, const int8 *s)const
	{
		MashList<sEffectScriptData>::Iterator iterEnd = listToSearch.End();
		for(MashList<sEffectScriptData>::Iterator iter = listToSearch.Begin(); iter != iterEnd; ++iter)
		{
			if (scriptreader::CompareStrings(s, iter->fileName.GetCString()))
				return iter;
		}

		return iterEnd;
	}

	eMASH_STATUS CMashShaderCompiler::CollectIncludesFromEffectScript(MashFileManager *fileManager, sEffectScriptData &currentInclude, MashList<sEffectScriptData> &closedBranch)
	{
		for(MashArray<MashShaderString>::Iterator iter = currentInclude.includes.Begin(); iter != currentInclude.includes.End(); ++iter)
		{
			MashFileStream *pFileStream = fileManager->CreateFileStream();
			if (pFileStream->LoadFile(iter->GetCString(), aFILE_IO_TEXT) == aMASH_FAILED)
			{
				int8 buffer[256];
				sprintf(buffer, "Failed to read effect include file '%s'.", iter->GetCString());
				MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, buffer, "CMashShaderCompiler::CollectIncludesFromEffectScript");
				pFileStream->Destroy();
				return aMASH_FAILED;
			}

			sEffectScriptData newInclude(m_stringMemoryPool);
			eMASH_STATUS status = AnalyzeFile(fileManager, (const int8*)pFileStream->GetData(), pFileStream->GetDataSizeInBytes(), newInclude);

			pFileStream->Destroy();

			if (status == aMASH_FAILED)
				return aMASH_FAILED;

			newInclude.fileName = *iter;

			if (CollectIncludesFromEffectScript(fileManager, newInclude, closedBranch) == aMASH_FAILED)
				return aMASH_FAILED;
		}

		if (FindIncludeStringInList(closedBranch, currentInclude.fileName.GetCString()) == closedBranch.End())
			closedBranch.PushBack(currentInclude);

		return aMASH_OK;
	}

	void CMashShaderCompiler::InitialiseMemoryPool()
	{
		if (!m_isMemoryPoolInitialised)
		{
			m_isMemoryPoolInitialised = true;
			m_stringMemoryPool = MemPoolType(g_MemPoolTypeSize);
		}
	}

	void CMashShaderCompiler::DestroyMemoryPool()
	{
		if (!m_isBatchCompileEnabled)
		{
			if (m_isMemoryPoolInitialised)
			{
				m_stringMemoryPool.Destroy();
				m_isMemoryPoolInitialised = false;
			}
		}
		else
		{
			m_stringMemoryPool.Clear();
		}
	}

	eMASH_STATUS CMashShaderCompiler::BuildRunTimeEffect(MashFileManager *fileManager, MashEffect *effect, const sEffectCompileArgs &compileArgs)
	{
		InitialiseMemoryPool();

		//indent so that the memory pool is destroyed last
		{
		MashEffectProgram *vertexProgram = effect->GetProgramByType(aPROGRAM_VERTEX);
		MashEffectProgram *pixelProgram = effect->GetProgramByType(aPROGRAM_PIXEL);

		if (!vertexProgram)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
				"Generated effects must contain a vertex program.", 
				"CMashShaderCompiler::BuildRunTimeEffect");

			return aMASH_FAILED;
		}
		
		if (!pixelProgram)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
				"Generated effects must contain a pixel program.", 
				"CMashShaderCompiler::BuildRunTimeEffect");

			return aMASH_FAILED;
		}
        
        static int uniqueEffectNumber = 0;
        int effectUniqueNumbers[2] = {++uniqueEffectNumber, ++uniqueEffectNumber};

		MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_INFORMATION, 
			"CMashMaterialBuilder::BuildRunTimeEffect", 
			"Effect build started on vertex program '%s %s %d' and pixel program '%s %s %d'.",
			vertexProgram->GetFileName().GetCString(), mash::helpers::GetShaderProfileString(vertexProgram->GetProfile()), 
			effectUniqueNumbers[0], pixelProgram->GetFileName().GetCString(), mash::helpers::GetShaderProfileString(pixelProgram->GetProfile()), effectUniqueNumbers[1]);

		MashShaderString vertexScriptSource(m_stringMemoryPool);
		MashShaderString pixelScriptSource(m_stringMemoryPool);

		MashFileStream *pFileStream = fileManager->CreateFileStream();
		if (pFileStream->LoadFile(vertexProgram->GetFileName().GetCString(), aFILE_IO_TEXT) == aMASH_FAILED)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, "Failed to read shader file.", "CMashShaderCompiler::BuildRunTimeEffect");
			pFileStream->Destroy();
			return aMASH_FAILED;
		}

		vertexScriptSource = (const int8*)pFileStream->GetData();
		pFileStream->Destroy();

		sEffectScriptData vertexScriptData(m_stringMemoryPool);
		vertexScriptData.fileName = vertexProgram->GetFileName().GetCString();
		vertexScriptData.target = vertexProgram->GetProfile();
		vertexScriptData.programType = vertexProgram->GetProgramType();
		vertexScriptData.entry = vertexProgram->GetEntry().GetCString();
        vertexScriptData.uniqueEffectNumber = effectUniqueNumbers[0];

		//shadow pixel shaders are generated. There is nothing to load here.
		if (!compileArgs.isShadowEffect)
		{
			pFileStream = fileManager->CreateFileStream();
			if (pFileStream->LoadFile(pixelProgram->GetFileName().GetCString(), aFILE_IO_TEXT) == aMASH_FAILED)
			{
				MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, "Failed to read shader file.", "CMashShaderCompiler::BuildRunTimeEffect");
				pFileStream->Destroy();
				return aMASH_FAILED;
			}

			pixelScriptSource = (const int8*)pFileStream->GetData();
			pFileStream->Destroy();
		}

		sEffectScriptData pixelScriptData(m_stringMemoryPool);

		pixelScriptData.fileName = pixelProgram->GetFileName().GetCString();
		pixelScriptData.target = pixelProgram->GetProfile();
		pixelScriptData.programType = pixelProgram->GetProgramType();
		pixelScriptData.entry = pixelProgram->GetEntry().GetCString();
        pixelScriptData.uniqueEffectNumber = effectUniqueNumbers[1];

		AnalyzeFile(fileManager, vertexScriptSource.GetCString(), vertexScriptSource.Size(), vertexScriptData);

		if (vertexScriptData.source.Empty())
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
				"Effects must contain vertex source.",  
				"CMashShaderCompiler::BuildRunTimeEffect");

			return aMASH_FAILED;
		}


//////////////////////pixel data/////////////////////////
		if (!pixelScriptSource.Empty())
		{
			//sFileData formatedPixelFileData;
			AnalyzeFile(fileManager, pixelScriptSource.GetCString(), pixelScriptSource.Size(), pixelScriptData);
		}

		/*
			If there is no vertex input block declared then it's
			assumed the code within the 'source' block is in
			native format.
		*/
		if (!vertexScriptData.vertexInput.Empty())
		{
			if (!compileArgs.isShadowEffect)
			{
				if (_BuildVertxPixelShaders(vertexScriptData, pixelScriptData, compileArgs.lightingType) == aMASH_FAILED)
				{
					return aMASH_FAILED;
				}
			}
			else
			{
				if (_BuildVertxPixelShadowCasters(fileManager, compileArgs.shadowEffectType, vertexScriptData, pixelScriptData) == aMASH_FAILED)
				{
					return aMASH_FAILED;
				}
			}
		}

		if (LinkEffectProgram(fileManager, vertexScriptData, compileArgs.overrideLightShadingFile) == aMASH_FAILED)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
				"Failed to add include files from effect.",  
				"CMashShaderCompiler::BuildRunTimeEffect");

			return aMASH_FAILED;
		}

		if (LinkEffectProgram(fileManager, pixelScriptData, compileArgs.overrideLightShadingFile) == aMASH_FAILED)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
				"Failed to add include files from effect.",  
				"CMashShaderCompiler::BuildRunTimeEffect");

			return aMASH_FAILED;
		}

		/*
			Add API define to the programs
		*/
		eSHADER_API_TYPE shaderAPIType = mash::helpers::GetAPIFromShaderProfile(vertexProgram->GetProfile());
		if (shaderAPIType == aSHADERAPITYPE_OPENGL)
		{

			/*
				This could be added in the macro list but its defined
				this way instead.
			*/
			MashShaderString def(m_stringMemoryPool);
			def += "#define ";
			def += g_pShaderDefineTypes[aSHADER_DEFINE_OPENGL];
            def += "\n";

			vertexScriptData.finalProgram.Insert(0, def);
			pixelScriptData.finalProgram.Insert(0, def);
            
            ++vertexScriptData.userSourceLineStart;
            ++pixelScriptData.userSourceLineStart;
		}
		else
		{
			MashShaderString def(m_stringMemoryPool);
			def += "#define ";
			def += g_pShaderDefineTypes[aSHADER_DEFINE_DIRECTX];
			def += "\n";

			vertexScriptData.finalProgram.Insert(0, def);
			pixelScriptData.finalProgram.Insert(0, def);
            
            ++vertexScriptData.userSourceLineStart;
            ++pixelScriptData.userSourceLineStart;
		}

		if (compileArgs.macros && compileArgs.macroCount)
		{
			vertexScriptData.macros.Assign(compileArgs.macros, compileArgs.macroCount);
			pixelScriptData.macros.Assign(compileArgs.macros, compileArgs.macroCount);
		}

		if (!vertexProgram->GetCompileArguments().Empty())
		{
			MashArray<sEffectMacro>::ConstIterator macroIter = vertexProgram->GetCompileArguments().Begin();
			MashArray<sEffectMacro>::ConstIterator macroIterEnd = vertexProgram->GetCompileArguments().End();
			for(; macroIter != macroIterEnd; ++macroIter)
				vertexScriptData.macros.PushBack(*macroIter);
		}

		if (!pixelProgram->GetCompileArguments().Empty())
		{
			MashArray<sEffectMacro>::ConstIterator macroIter = pixelProgram->GetCompileArguments().Begin();
			MashArray<sEffectMacro>::ConstIterator macroIterEnd = pixelProgram->GetCompileArguments().End();
			for(; macroIter != macroIterEnd; ++macroIter)
				pixelScriptData.macros.PushBack(*macroIter);
		}

            MashStringc generatedEffectNames[2];
		if (ConvertProgramsIntoNativeFormat(fileManager, shaderAPIType, vertexScriptData, pixelScriptData, generatedEffectNames) == aMASH_FAILED)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
				"Failed to convert effect into native format.",  
				"CMashShaderCompiler::BuildRunTimeEffect");

			return aMASH_FAILED;
		}

		vertexProgram->SetHighLevelSource(vertexScriptData.finalProgram.GetCString(), vertexScriptData.entry.GetCString(), generatedEffectNames[0]);
		pixelProgram->SetHighLevelSource(pixelScriptData.finalProgram.GetCString(), pixelScriptData.entry.GetCString(), generatedEffectNames[1]);

		MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_INFORMATION, 
			"Effect build complete.", 
			"CMashMaterialBuilder::BuildRunTimeEffect");
		}

		//destroy the mem pool if batch compile in not enabled.
		DestroyMemoryPool();
        
		return aMASH_OK;
	}

	eMASH_STATUS CMashShaderCompiler::LinkEffectProgram(MashFileManager *fileManager, sEffectScriptData &scriptData, const MashStringc &overrideLightingShading)
	{
		MashList<sEffectScriptData> orderedIncludeList;
		CollectIncludesFromEffectScript(fileManager, scriptData, orderedIncludeList);

		MashShaderString hlslData(m_stringMemoryPool);
		MashShaderString headerHlsl(m_stringMemoryPool);

		/*
			This part adds engine generated code.

			If lighting is included, then include either the user defined
			or default light shading code.

			This is added per effect compiled, and not with the generated lighting code because
			each material may have different shading properties (metal, plastic, wood, etc...)
		*/
		for(MashList<sEffectScriptData>::Iterator includeIter = orderedIncludeList.Begin(); includeIter != orderedIncludeList.End(); ++includeIter)
		{
			if (scriptreader::CompareStrings(includeIter->fileName.GetCString(), m_effectIncludes[aEFF_INC_DIRECTIONAL_LIGHTING].GetCString()) ||
				scriptreader::CompareStrings(includeIter->fileName.GetCString(), m_effectIncludes[aEFF_INC_SPOT_LIGHTING].GetCString()) ||
				scriptreader::CompareStrings(includeIter->fileName.GetCString(), m_effectIncludes[aEFF_INC_POINT_LIGHTING].GetCString()))
			{
				MashShaderString inc(m_stringMemoryPool);

				if (!overrideLightingShading.Empty())
					inc = overrideLightingShading.GetCString();
				else
					inc = m_effectIncludes[aEFF_INC_LIGHT_SHADING].GetCString();

				MashFileStream *pFileStream = fileManager->CreateFileStream();
				if (pFileStream->LoadFile(inc.GetCString(), aFILE_IO_TEXT) == aMASH_FAILED)
				{
					int8 buffer[256];
					sprintf(buffer, "Failed to read light shade include file '%s'.", inc.GetCString());
					MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, buffer, "CMashShaderCompiler::CompileEffectScript");
					pFileStream->Destroy();
					return aMASH_FAILED;
				}

				sEffectScriptData newInclude(m_stringMemoryPool);
				eMASH_STATUS status = AnalyzeFile(fileManager, (const int8*)pFileStream->GetData(), pFileStream->GetDataSizeInBytes(), newInclude);
				pFileStream->Destroy();

				if (status == aMASH_FAILED)
					return aMASH_FAILED;

				newInclude.fileName = inc;

				orderedIncludeList.Insert(includeIter, newInclude);
				break;
			}
		}

		std::set<MashShaderString> includedAutos;
		for(MashList<sEffectScriptData>::Iterator includeIter = orderedIncludeList.Begin(); includeIter != orderedIncludeList.End(); ++includeIter)
		{
			//add unique autos
			for(MashArray<sEffectScriptData::sAutoDeclaration>::Iterator autoIter = includeIter->autos.Begin(); autoIter != includeIter->autos.End(); ++autoIter)
			{
				if (includedAutos.find(autoIter->name) == includedAutos.end())
				{
					includedAutos.insert(autoIter->name);

					hlslData += autoIter->type;
					hlslData += " ";
					hlslData += autoIter->name;
					if (autoIter->arraySize > 0)
					{
						int8 buffer[256];
						mash::helpers::PrintToBuffer(buffer, 256, "[%d]", autoIter->arraySize);
						hlslData += buffer;
					}
					hlslData += ";\n";
				}
			}

			//look for macros if callback is set
			if (!m_effectIncludeCallbacks.Empty())
			{
				const uint32 callbackSize = m_effectIncludeCallbacks.Size();
				for(uint32 currCallback = 0; currCallback < callbackSize; ++currCallback)
				{
					if (strcmp(m_effectIncludeCallbacks[currCallback].includeString.GetCString(), includeIter->fileName.GetCString()) == 0)
					{
						m_effectIncludeCallbacks[currCallback].functor.Call(scriptData.macros);
						break;
					}
				}
			}

			hlslData += includeIter->source;
			headerHlsl += includeIter->header;
		}

		scriptData.finalProgram = headerHlsl + hlslData;

        int searchLength = scriptData.finalProgram.Size();
        int finalLineCount = 0;
        int userLineCount = 0;
        for(int i = 0; i < searchLength; ++i)
        {
            if (scriptData.finalProgram[i] == 10)
                ++finalLineCount;
        }
        searchLength = scriptData.source.Size();
        for(int i = 0; i < searchLength; ++i)
        {
            if (scriptData.source[i] == 10)
                ++userLineCount;
        }
        
        scriptData.userSourceLineStart = finalLineCount - (userLineCount - scriptData.userSourceLineStart);

		return aMASH_OK;
	}

	eMASH_STATUS CMashShaderCompiler::AnalyzeFile(MashFileManager *fileManager, const int8 *fileData, int32 fileDataLength, sEffectScriptData &formatedFileData)
	{
		sFileLayout fileLayout;

		uint32 location = 0;
		const uint32 end = fileDataLength;//strlen(fileData);

		if (end == 0)
			return aMASH_FAILED;

		bool readVertexOutputData = false;

		MashShaderString buffer(m_stringMemoryPool);

		/*
			This search shouldn't be too time consuming as a script will have most of the code contained
			within these headers. Therefore large chunks of code will be quickly skipped over.
		*/
		while(location < end)
		{
			buffer.Clear();
			if (scriptreader::ReadNextString(fileData, location, end, buffer) == 0)
			{
				//no data read.
			}
			else if (scriptreader::CompareStrings(buffer.GetCString(), "vertexinput"))
			{
				if (scriptreader::ReadNextBlockLimits(fileData, location, end, fileLayout.vertexDeclStart, fileLayout.vertexDeclEnd) == aMASH_FAILED)
					return aMASH_FAILED;
			}
			else if (scriptreader::CompareStrings(buffer.GetCString(), "vertexoutput"))
			{
				readVertexOutputData = true;
				if (scriptreader::ReadNextBlockLimits(fileData, location, end, fileLayout.userOutputStart, fileLayout.userOutputEnd) == aMASH_FAILED)
					return aMASH_FAILED;
			}
			else if (scriptreader::CompareStrings(buffer.GetCString(), "pixeloutput"))
			{
				readVertexOutputData = false;
				if (scriptreader::ReadNextBlockLimits(fileData, location, end, fileLayout.userOutputStart, fileLayout.userOutputEnd) == aMASH_FAILED)
					return aMASH_FAILED;
			}
			else if (scriptreader::CompareStrings(buffer.GetCString(), "autos"))
			{
				if (scriptreader::ReadNextBlockLimits(fileData, location, end, fileLayout.autoStart, fileLayout.autoEnd) == aMASH_FAILED)
					return aMASH_FAILED;
			}
			else if (scriptreader::CompareStrings(buffer.GetCString(), "include"))
			{
				if (scriptreader::ReadNextBlockLimits(fileData, location, end, fileLayout.includeStart, fileLayout.includeEnd) == aMASH_FAILED)
					return aMASH_FAILED;
			}
			else if (scriptreader::CompareStrings(buffer.GetCString(), "source"))
			{
				if (scriptreader::ReadNextBlockLimits(fileData, location, end, fileLayout.sourceStart, fileLayout.sourceEnd) == aMASH_FAILED)
					return aMASH_FAILED;
                
#ifdef MASH_LOG_ENABLED
				//this is only needed for debugging
                formatedFileData.userSourceLineStart = 0;
                formatedFileData.userSourceScriptLineStart = 0;
                for (unsigned int i = 0; i < fileLayout.sourceStart; ++i)
                {
                    if (fileData[i] == 10)
                        ++formatedFileData.userSourceScriptLineStart;
                }
#endif
			}
			else if (scriptreader::CompareStrings(buffer.GetCString(), "header"))
			{
				if (scriptreader::ReadNextBlockLimits(fileData, location, end, fileLayout.headerStart, fileLayout.headerEnd) == aMASH_FAILED)
					return aMASH_FAILED;
			}
			else
			{
				int8 msgBuffer[256];
				sprintf(msgBuffer, "Undefined element '%s' in effect file.", buffer.GetCString());
				MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, msgBuffer, "CMashShaderCompiler::AnalyzeFile");
				return aMASH_FAILED;
			}
		}

		if ((fileLayout.includeEnd - fileLayout.includeStart) > 0)
		{
			if (ReadIncludes(fileData, fileLayout.includeStart, fileLayout.includeEnd, formatedFileData.includes) == aMASH_FAILED)
				return aMASH_FAILED;
		}

		if (readVertexOutputData)
		{
			if ((fileLayout.vertexDeclEnd - fileLayout.vertexDeclStart) > 0)
			{
				if (ReadVertexInput(fileData, fileLayout.vertexDeclStart, fileLayout.vertexDeclEnd, formatedFileData.vertexInput) == aMASH_FAILED)
					return aMASH_FAILED;
			}

			if ((fileLayout.userOutputEnd - fileLayout.userOutputStart) > 0)
			{
				if (ReadVertexOutput(fileData, fileLayout.userOutputStart, fileLayout.userOutputEnd, formatedFileData.userOutput) == aMASH_FAILED)
					return aMASH_FAILED;
			}
		}
		else
		{
			if ((fileLayout.userOutputEnd - fileLayout.userOutputStart) > 0)
			{
				if (ReadPixelOutput(fileData, fileLayout.userOutputStart, fileLayout.userOutputEnd, formatedFileData.userOutput) == aMASH_FAILED)
					return aMASH_FAILED;
			}
		}

		if ((fileLayout.autoEnd - fileLayout.autoStart) > 0)
		{
			if (ReadAutos(fileData, fileLayout.autoStart, fileLayout.autoEnd, formatedFileData.autos) == aMASH_FAILED)
				return aMASH_FAILED;
		}

		if ((fileLayout.headerEnd - fileLayout.headerStart) > 0)
		{
			formatedFileData.header.Append(&fileData[fileLayout.headerStart], fileLayout.headerEnd - fileLayout.headerStart);
		}

		if ((fileLayout.sourceEnd - fileLayout.sourceStart) > 0)
		{
			formatedFileData.source.Append(&fileData[fileLayout.sourceStart], fileLayout.sourceEnd - fileLayout.sourceStart);
		}

		return aMASH_OK;
	}

	eMASH_STATUS CMashShaderCompiler::_BuildVertxPixelShadowCasters(MashFileManager *fileManager,
		eLIGHTTYPE shadowEffectType,
		sEffectScriptData &vertexScriptData,
		sEffectScriptData &pixelScriptData)
	{
		//add custom shadow includes
		MashShaderString veretxShadowCasterFileName(m_stringMemoryPool);
		MashShaderString pixelShadowCasterFileName(m_stringMemoryPool);

		switch(shadowEffectType)
		{
		case aLIGHT_DIRECTIONAL:
			veretxShadowCasterFileName = m_effectIncludes[aEFF_INC_DIRECTIONAL_SHADOW_CASTER_VERTEX].GetCString();
			pixelShadowCasterFileName = m_effectIncludes[aEFF_INC_DIRECTIONAL_SHADOW_CASTER_PIXEL].GetCString();
			break;
		case aLIGHT_SPOT:
			veretxShadowCasterFileName = m_effectIncludes[aEFF_INC_SPOT_SHADOW_CASTER_VERTEX].GetCString();
			pixelShadowCasterFileName = m_effectIncludes[aEFF_INC_SPOT_SHADOW_CASTER_PIXEL].GetCString();
			break;
		case aLIGHT_POINT:
			veretxShadowCasterFileName = m_effectIncludes[aEFF_INC_POINT_SHADOW_CASTER_VERTEX].GetCString();
			pixelShadowCasterFileName = m_effectIncludes[aEFF_INC_POINT_SHADOW_CASTER_PIXEL].GetCString();
			break;
		};

		/*
			The shadow caster fragment files are opened and added to the
			current data.
		*/
		MashFileStream *pFileStream = fileManager->CreateFileStream();
		if (pFileStream->LoadFile(veretxShadowCasterFileName.GetCString(), aFILE_IO_TEXT) == aMASH_FAILED)
		{
			int8 buffer[256];
			sprintf(buffer, "Failed to read vertex shadow caster include file '%s'.", veretxShadowCasterFileName.GetCString());
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, buffer, "CMashShaderCompiler::_BuildVertxPixelShadowCasters");
			pFileStream->Destroy();
			return aMASH_FAILED;
		}

		sEffectScriptData vertexShadowCasterData(m_stringMemoryPool);
		eMASH_STATUS status = AnalyzeFile(fileManager, (const int8*)pFileStream->GetData(), pFileStream->GetDataSizeInBytes(), vertexShadowCasterData);
		pFileStream->Destroy();

		if (status == aMASH_FAILED)
		{
			int8 buffer[256];
			sprintf(buffer, "Failed to read vertex shadow caster include file '%s'.", veretxShadowCasterFileName.GetCString());
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, buffer, "CMashShaderCompiler::_BuildVertxPixelShadowCasters");
			pFileStream->Destroy();
			return aMASH_FAILED;
		}

		pFileStream = fileManager->CreateFileStream();
		if (pFileStream->LoadFile(pixelShadowCasterFileName.GetCString(), aFILE_IO_TEXT) == aMASH_FAILED)
		{
			int8 buffer[256];
			sprintf(buffer, "Failed to vertex shadow caster include file '%s'.", pixelShadowCasterFileName.GetCString());
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, buffer, "CMashShaderCompiler::_BuildVertxPixelShadowCasters");
			pFileStream->Destroy();
			return aMASH_FAILED;
		}

		sEffectScriptData pixelShadowCasterData(m_stringMemoryPool);
		status = AnalyzeFile(fileManager, (const int8*)pFileStream->GetData(), pFileStream->GetDataSizeInBytes(), pixelShadowCasterData);
		pFileStream->Destroy();

		if (status == aMASH_FAILED)
		{
			int8 buffer[256];
			sprintf(buffer, "Failed to vertex shadow caster include file '%s'.", pixelShadowCasterFileName.GetCString());
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, buffer, "CMashShaderCompiler::_BuildVertxPixelShadowCasters");
			pFileStream->Destroy();
			return aMASH_FAILED;
		}

		vertexScriptData.autos.Insert(vertexScriptData.autos.Begin(), vertexShadowCasterData.autos.Begin(), vertexShadowCasterData.autos.End());

		vertexScriptData.includes.Insert(vertexScriptData.includes.Begin(), vertexShadowCasterData.includes.Begin(), vertexShadowCasterData.includes.End());
		vertexScriptData.header.Insert(0, vertexShadowCasterData.header);

		pixelScriptData.userOutput.Insert(pixelScriptData.userOutput.Begin(), pixelShadowCasterData.userOutput.Begin(), pixelShadowCasterData.userOutput.End());
		pixelScriptData.autos.Insert(pixelScriptData.autos.Begin(), pixelShadowCasterData.autos.Begin(), pixelShadowCasterData.autos.End());
        
		pixelScriptData.includes.Insert(pixelScriptData.includes.Begin(), pixelShadowCasterData.includes.Begin(), pixelShadowCasterData.includes.End());
		pixelScriptData.header.Insert(0, pixelShadowCasterData.header);

		//Build vertex input struct string
		MashShaderString vertexInputString(m_stringMemoryPool);
		vertexInputString += "struct VIN {\n";
		
		uint32 semanticCounter[aVERTEX_DECLUSAGE_COUNT];
		memset(semanticCounter, 0, sizeof(uint32) * aVERTEX_DECLUSAGE_COUNT);

		const uint32 vertexInputCount = vertexScriptData.vertexInput.Size();
		int8 intBuffer[256];
		for(uint32 i = 0; i < vertexInputCount; ++i)
		{
			vertexInputString += vertexScriptData.vertexInput[i].type;
			vertexInputString += " ";
			vertexInputString += vertexScriptData.vertexInput[i].name;
			vertexInputString += ":";
			vertexInputString += InputSemanticTypeToHLSLString(vertexScriptData.vertexInput[i].semantic);
			vertexInputString += mash::helpers::NumberToString(intBuffer, 256, semanticCounter[vertexScriptData.vertexInput[i].semantic]++);
			vertexInputString += ";\n";
		}

		vertexInputString += "};\n";

		//Build user function output struct string
		MashShaderString userFunctionOutputString(m_stringMemoryPool);
		userFunctionOutputString += "struct VOUT {\n";

		/*
			This map provides fast lookup for specific variables into the original array.
			One is also created for the pixel shader later.
		*/
		uint32 vertexOutputSemanticMap[aVERTEX_OUTPUT_COUNT];
		for(uint32 i = 0; i < aVERTEX_OUTPUT_COUNT; ++i)
			vertexOutputSemanticMap[i] = aVERTEX_OUTPUT_COUNT;

		uint32 userFunctionOutputCount = vertexScriptData.userOutput.Size();
		for(uint32 i = 0; i < userFunctionOutputCount; ++i)
		{
			userFunctionOutputString += vertexScriptData.userOutput[i].type;
			userFunctionOutputString += " ";
			userFunctionOutputString += vertexScriptData.userOutput[i].name;
			userFunctionOutputString += ";\n";

			vertexOutputSemanticMap[vertexScriptData.userOutput[i].vertexSemantic] = i;
		}

		userFunctionOutputString += "};\n";

		//create shadow output struct
		userFunctionOutputString += "struct SVOUT {\n";
		uint32 shadowVertexOutputSemanticMap[aVERTEX_OUTPUT_COUNT];
		for(uint32 i = 0; i < aVERTEX_OUTPUT_COUNT; ++i)
			shadowVertexOutputSemanticMap[i] = aVERTEX_OUTPUT_COUNT;

		uint32 shadowFunctionOutputCount = vertexShadowCasterData.userOutput.Size();
		for(uint32 i = 0; i < shadowFunctionOutputCount; ++i)
		{
			userFunctionOutputString += vertexShadowCasterData.userOutput[i].type;
			userFunctionOutputString += " ";
			userFunctionOutputString += vertexShadowCasterData.userOutput[i].name;
			userFunctionOutputString += ";\n";

			shadowVertexOutputSemanticMap[vertexShadowCasterData.userOutput[i].vertexSemantic] = i;
		}
		userFunctionOutputString += "};\n";

		if (vertexOutputSemanticMap[aVERTEX_OUTPUT_VPOS] == aVERTEX_OUTPUT_COUNT)
		{
			int8 buffer[256];
			sprintf(buffer, "Failed to create shadow casters for effect '%s'. The vertex function must output a variable with the 'viewposition' semantic.", vertexScriptData.fileName.GetCString());
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, buffer, "CMashShaderCompiler::_BuildVertxPixelShadowCasters");
			return aMASH_FAILED;
		}

		if (shadowVertexOutputSemanticMap[aVERTEX_OUTPUT_HPOS] == aVERTEX_OUTPUT_COUNT)
		{
			int8 buffer[256];
			sprintf(buffer, "Failed to create shadow casters for effect '%s'. The user vertex function was fine, however the shadow caster function must output a variable with the 'hposition' semantic.", vertexScriptData.fileName.GetCString());
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, buffer, "CMashShaderCompiler::_BuildVertxPixelShadowCasters");
			return aMASH_FAILED;
		}

		//build vertex function
		MashShaderString vertexFunction(m_stringMemoryPool);
		vertexFunction += "_PIN _vsmain(VIN input){\n\
			_PIN output;\n\
			VOUT userOutput = ";

		vertexFunction += vertexScriptData.entry + "(input);\n\
		SVOUT shadowOutput = MashShadowCasterVertex(userOutput." + vertexScriptData.userOutput[vertexOutputSemanticMap[aVERTEX_OUTPUT_VPOS]].name + ".xyz);\n";

		//copy user data into output struct
		for(uint32 i = 0; i < shadowFunctionOutputCount; ++i)
		{
			/*
				only particular sematics are used for interpolation from vertex to pixel shaders
			*/
			if (vertexShadowCasterData.userOutput[i].passToPixel)
			{
				vertexFunction += "output.";
				vertexFunction += vertexShadowCasterData.userOutput[i].name;
				vertexFunction += "=";
				vertexFunction += "shadowOutput.";
				vertexFunction += vertexShadowCasterData.userOutput[i].name;
				vertexFunction += ";\n";
			}
		}

		//finally add in the hpos semantic (this must be last)
		vertexFunction += "output._posH";
		vertexFunction += "=";
		vertexFunction += "shadowOutput.";
		vertexFunction += vertexShadowCasterData.userOutput[shadowVertexOutputSemanticMap[aVERTEX_OUTPUT_HPOS]].name;
		vertexFunction += ";\n";

		vertexFunction += "return output;\n";
		vertexFunction += "}\n";

		uint32 pixelSemanticCounter[aVERTEX_OUTPUT_COUNT];
		memset(pixelSemanticCounter, 0, sizeof(uint32) * aVERTEX_OUTPUT_COUNT);

		//Build vout/pin struct
		MashShaderString pixelInputString(m_stringMemoryPool);

		//add in shadow interpolated data into the struct
		for(uint32 i = 0; i < shadowFunctionOutputCount; ++i)
		{
			if (vertexShadowCasterData.userOutput[i].passToPixel)
			{
				pixelInputString += vertexShadowCasterData.userOutput[i].type;
				pixelInputString += " ";
				pixelInputString += vertexShadowCasterData.userOutput[i].name;
				pixelInputString += ":";

				eVERTEX_OUTPUT interp = GetVertexOutputSemanticAsInterpolator(vertexShadowCasterData.userOutput[i].vertexSemantic);
				pixelInputString += VertexOutputInterpolatorToHLSLString(interp);
				pixelInputString += mash::helpers::NumberToString(intBuffer, 256, pixelSemanticCounter[interp]++);
				pixelInputString += ";\n";
			}
		}

		//MashStringc vertexOutputString = "struct _PIN {\n";
		MashShaderString vertexOutputString(m_stringMemoryPool);
		vertexOutputString += "struct _PIN {\n";
		vertexOutputString += pixelInputString;
		/*
			Make sure we add the mandatory position constant last! Otherwise
			DX10 will throw an error because the user variables will be in
			different registers from VS to PS
		*/
		vertexOutputString += "float4 _posH : SV_POSITION;\n};\n";
		pixelInputString = "struct PIN {\n" + pixelInputString;
		pixelInputString += "};\n";

		//built shader string
		MashShaderString userFunctionString(m_stringMemoryPool);
		userFunctionString += vertexScriptData.source;
		vertexScriptData.source = vertexInputString;
		vertexScriptData.source += userFunctionOutputString;
		vertexScriptData.source += vertexOutputString;        
		vertexScriptData.source += vertexShadowCasterData.source;
        
#ifdef MASH_LOG_ENABLED
		//this is only needed for debugging
        for(unsigned int i = 0; i < vertexScriptData.source.Size(); ++i)
        {
            if (vertexScriptData.source[i] == 10)
                ++vertexScriptData.userSourceLineStart;
        }
#endif
        
		vertexScriptData.source += userFunctionString;
		vertexScriptData.source += vertexFunction;
		vertexScriptData.entry = "_vsmain";

		MashShaderString pixelOutString(m_stringMemoryPool);
		pixelOutString += "struct _PIXELOUT{\n\
			float4 color:SV_TARGET0;\n};\n";

		uint32 pixelOutputSemanticMap[aPIXEL_OUTPUT_COUNT];
		for(uint32 i = 0; i < aPIXEL_OUTPUT_COUNT; ++i)
			pixelOutputSemanticMap[i] = aPIXEL_OUTPUT_COUNT;

		MashShaderString pixelFunction(m_stringMemoryPool);
		pixelFunction += "_PIXELOUT _psmain(PIN input){\n\
			_PIXELOUT output;\n\
			output.color = MashShadowCasterPixel(input);\n\
			return output;\n}\n";

		userFunctionString = pixelScriptData.source;
		pixelScriptData.source = pixelInputString;
		pixelScriptData.source += pixelOutString;
		pixelScriptData.source += pixelShadowCasterData.source;

#ifdef MASH_LOG_ENABLED
		//this is only needed for debugging
        for(unsigned int i = 0; i < pixelScriptData.source.Size(); ++i)
        {
            if (pixelScriptData.source[i] == 10)
                ++pixelScriptData.userSourceLineStart;
        }
#endif

		pixelScriptData.source += userFunctionString;
		pixelScriptData.source += pixelFunction;
		pixelScriptData.entry = "_psmain";

		return aMASH_OK;
	}

	eMASH_STATUS CMashShaderCompiler::_BuildVertxPixelShaders(sEffectScriptData &vertexScriptData,
		sEffectScriptData &pixelScriptData,
		eLIGHTING_TYPE lightingType)
	{
		//Build vertex input struct string
		MashShaderString vertexInputString(m_stringMemoryPool);
		vertexInputString = "struct VIN {\n";
		
		uint32 semanticCounter[aVERTEX_DECLUSAGE_COUNT];
		memset(semanticCounter, 0, sizeof(uint32) * aVERTEX_DECLUSAGE_COUNT);

		const uint32 vertexInputCount = vertexScriptData.vertexInput.Size();
		int8 intBuffer[256];
		for(uint32 i = 0; i < vertexInputCount; ++i)
		{
			vertexInputString += vertexScriptData.vertexInput[i].type;
			vertexInputString += " ";
			vertexInputString += vertexScriptData.vertexInput[i].name;
			vertexInputString += ":";
			vertexInputString += InputSemanticTypeToHLSLString(vertexScriptData.vertexInput[i].semantic);
			vertexInputString += mash::helpers::NumberToString(intBuffer, 256, semanticCounter[vertexScriptData.vertexInput[i].semantic]++);
			vertexInputString += ";\n";
		}

		vertexInputString += "};\n";

		//Build user function output struct string
		MashShaderString userFunctionOutputString(m_stringMemoryPool);
		userFunctionOutputString = "struct VOUT {\n";

		/*
			This map provides fast lookup for specific variables into the original array.
			One is also created for the pixel shader later.
		*/
		uint32 vertexOutputSemanticMap[aVERTEX_OUTPUT_COUNT];
		for(uint32 i = 0; i < aVERTEX_OUTPUT_COUNT; ++i)
			vertexOutputSemanticMap[i] = aVERTEX_OUTPUT_COUNT;

		uint32 userFunctionOutputCount = vertexScriptData.userOutput.Size();
		for(uint32 i = 0; i < userFunctionOutputCount; ++i)
		{
			userFunctionOutputString += vertexScriptData.userOutput[i].type;
			userFunctionOutputString += " ";
			userFunctionOutputString += vertexScriptData.userOutput[i].name;
			userFunctionOutputString += ";\n";

			vertexOutputSemanticMap[vertexScriptData.userOutput[i].vertexSemantic] = i;
		}

		userFunctionOutputString += "};\n";

		//build vertex function
		MashShaderString vertexFunction(m_stringMemoryPool);
			vertexFunction = "_PIN _vsmain(VIN input){\n\
			_PIN output;\n\
			VOUT userOutput = ";

		vertexFunction += vertexScriptData.entry + "(input);\n";

		/*
			The user can provide diffuse light data. This overrides the built in lighting.
		*/
		if (vertexOutputSemanticMap[aVERTEX_OUTPUT_LIGHT_DIFFUSE] == aVERTEX_OUTPUT_COUNT)
		{
			if (lightingType == aLIGHT_TYPE_VERTEX)
			{
				//add forward rendered lighting code
				vertexScriptData.includes.PushBack(MashShaderString(m_effectIncludes[aEFF_INC_FORWARD_RENDERED_LIGHTING_G].GetCString(), m_stringMemoryPool));

				MashShaderString viewSpaceNormalName(m_stringMemoryPool);
				if (vertexOutputSemanticMap[aVERTEX_OUTPUT_VNORM] == aVERTEX_OUTPUT_COUNT)
				{
					vertexFunction += "float3 viewSpaceNorm = float4(0.0f, 0.0f, 0.0f);\n";
					viewSpaceNormalName = "viewSpaceNorm";
				}
				else
				{
					
					viewSpaceNormalName = "userOutput." + vertexScriptData.userOutput[vertexOutputSemanticMap[aVERTEX_OUTPUT_VNORM]].name;

					//normalize
					vertexFunction += viewSpaceNormalName + " = normalize(" + viewSpaceNormalName + ");\n";
				}

				MashShaderString specularName(m_stringMemoryPool);
				if (vertexOutputSemanticMap[aVERTEX_OUTPUT_SPECULAR] == aVERTEX_OUTPUT_COUNT)
				{
					vertexFunction += "float4 specular = float4(1.0f, 1.0f, 1.0f, 1.0f);\n";
					specularName = "specular";
				}
				else
				{
					specularName = "userOutput." + vertexScriptData.userOutput[vertexOutputSemanticMap[aVERTEX_OUTPUT_SPECULAR]].name;
				}

				vertexFunction += "sLightOutput lightingOutput = MashForwardRenderedLighting(";
				vertexFunction += viewSpaceNormalName;
				vertexFunction += ", userOutput.";
				vertexFunction += vertexScriptData.userOutput[vertexOutputSemanticMap[aVERTEX_OUTPUT_VPOS]].name + ".xyz ,";
				vertexFunction += specularName;
				vertexFunction += ");\n";

				/*
					Vertex lighting values are now added to the interpolator list.
				*/

				//diffuse lighting was not present so we add it in
				vertexScriptData.userOutput.PushBack(sEffectScriptData::sUserFunctionOutput(m_stringMemoryPool, "_vertexLightDiffuse", "float3", true, aVERTEX_OUTPUT_LIGHT_DIFFUSE));
				vertexOutputSemanticMap[aVERTEX_OUTPUT_LIGHT_DIFFUSE] = vertexScriptData.userOutput.Size() - 1;

				//add spec lighting value
				if (vertexOutputSemanticMap[aVERTEX_OUTPUT_LIGHT_SPECULAR] == aVERTEX_OUTPUT_COUNT)
				{
					vertexScriptData.userOutput.PushBack(sEffectScriptData::sUserFunctionOutput(m_stringMemoryPool, "_vertexLightSpecular", "float3", true, aVERTEX_OUTPUT_LIGHT_SPECULAR));
					vertexOutputSemanticMap[aVERTEX_OUTPUT_LIGHT_SPECULAR] = vertexScriptData.userOutput.Size() - 1;
				}
				else
				{
					//force the interpolation flag on since it was provided
					vertexScriptData.userOutput[vertexOutputSemanticMap[aVERTEX_OUTPUT_LIGHT_SPECULAR]].passToPixel = true;
				}

				vertexFunction += "output.";
				vertexFunction += vertexScriptData.userOutput[vertexOutputSemanticMap[aVERTEX_OUTPUT_LIGHT_DIFFUSE]].name;
				vertexFunction += " = lightingOutput.diffuse;\noutput.";
				vertexFunction += vertexScriptData.userOutput[vertexOutputSemanticMap[aVERTEX_OUTPUT_LIGHT_SPECULAR]].name;
				vertexFunction += " = lightingOutput.specular;\n";

			}
			else if (lightingType == aLIGHT_TYPE_PIXEL || lightingType == aLIGHT_TYPE_DEFERRED)
			{
				//view space position must be passed to pixel shadow for pixel or deferred lighting
				vertexScriptData.userOutput[vertexOutputSemanticMap[aVERTEX_OUTPUT_VPOS]].passToPixel = true;

				//same goes for the normal if present
				if (vertexOutputSemanticMap[aVERTEX_OUTPUT_VNORM] != aVERTEX_OUTPUT_COUNT)
					vertexScriptData.userOutput[vertexOutputSemanticMap[aVERTEX_OUTPUT_VNORM]].passToPixel = true;
			}
		}

		/*
			copy user data into output struct.
		*/
		userFunctionOutputCount = vertexScriptData.userOutput.Size();
		for(uint32 i = 0; i < userFunctionOutputCount; ++i)
		{
			if (vertexScriptData.userOutput[i].passToPixel && vertexScriptData.userOutput[i].isUserData)
			{
				vertexFunction += "output.";
				vertexFunction += vertexScriptData.userOutput[i].name;
				vertexFunction += "=";
				vertexFunction += "userOutput.";
				vertexFunction += vertexScriptData.userOutput[i].name;
				vertexFunction += ";\n";
			}
		}
		
		if (vertexOutputSemanticMap[aVERTEX_OUTPUT_HPOS] != aVERTEX_OUTPUT_COUNT)
		{
			vertexFunction += "output._posH = userOutput.";
			vertexFunction += vertexScriptData.userOutput[vertexOutputSemanticMap[aVERTEX_OUTPUT_HPOS]].name;
			vertexFunction += ";\n";
		}
		else
		{
			AddEffectAutoUnique(aEFFECT_DATA_FLOAT4X4, aEFFECT_PROJECTION, vertexScriptData.autos);
			vertexFunction += "output._posH = mul(autoProjection, float4(userOutput.";
			vertexFunction += vertexScriptData.userOutput[vertexOutputSemanticMap[aVERTEX_OUTPUT_VPOS]].name;
			vertexFunction += ".xyz, 1.0f));\n";
		}

		vertexFunction += "return output;\n";

		vertexFunction += "}\n";

		uint32 pixelSemanticCounter[aVERTEX_OUTPUT_COUNT];
		memset(pixelSemanticCounter, 0, sizeof(uint32) * aVERTEX_OUTPUT_COUNT);

		//Build vout/pin struct
		MashShaderString pixelInputString(m_stringMemoryPool);

		//add in user interpolated data into the struct
		userFunctionOutputCount = vertexScriptData.userOutput.Size();
		for(uint32 i = 0; i < userFunctionOutputCount; ++i)
		{
			if (vertexScriptData.userOutput[i].passToPixel)
			{
				pixelInputString += vertexScriptData.userOutput[i].type;
				pixelInputString += " ";
				pixelInputString += vertexScriptData.userOutput[i].name;
				pixelInputString += ":";

				eVERTEX_OUTPUT interp = GetVertexOutputSemanticAsInterpolator(vertexScriptData.userOutput[i].vertexSemantic);
				pixelInputString += VertexOutputInterpolatorToHLSLString(interp);
				pixelInputString += mash::helpers::NumberToString(intBuffer, 256, pixelSemanticCounter[interp]++);
				pixelInputString += ";\n";
			}
		}

		MashShaderString vertexOutputString(m_stringMemoryPool);
		vertexOutputString += "struct _PIN {\n";
		vertexOutputString += pixelInputString;
		/*
			Make sure we add the mandatory position constant last! Otherwise
			DX10 will throw an error because the user variables will be in
			different registers from VS to PS
		*/
		vertexOutputString += "float4 _posH : SV_POSITION;\n";
		vertexOutputString += "};\n";
        
        //if there is no data to pass then add dummy data
        if (pixelInputString.Empty())
        {
            pixelInputString = "float4 fill;\n";
        }

		pixelInputString = "struct PIN {\n" + pixelInputString;
		pixelInputString += "};\n";

		//built shader string
		MashShaderString userFunctionString(m_stringMemoryPool);
		userFunctionString += vertexScriptData.source;
		vertexScriptData.source = vertexInputString;
		vertexScriptData.source += userFunctionOutputString;
		vertexScriptData.source += vertexOutputString;
        
#ifdef MASH_LOG_ENABLED
		//this is only needed for debugging
        for(unsigned int i = 0; i < vertexScriptData.source.Size(); ++i)
        {
            if (vertexScriptData.source[i] == 10)
                ++vertexScriptData.userSourceLineStart;
        }
#endif
        
		vertexScriptData.source += userFunctionString;
		vertexScriptData.source += vertexFunction;
		vertexScriptData.entry = "_vsmain";

		//now create the pixel shader

		MashShaderString pixelOutString(m_stringMemoryPool);
		pixelOutString += "struct _PIXELOUT{\n";

		if (lightingType == aLIGHT_TYPE_DEFERRED)
		{
			pixelOutString += "float4 colour:SV_TARGET0;\n\
				float4 normal:SV_TARGET1;\n\
				float4 specular:SV_TARGET2;\n\
				float4 depth:SV_TARGET3;\n";
		}
		else
		{
			pixelOutString += "float4 colour:SV_TARGET0;\n";
		}

		pixelOutString += "};\n";

		uint32 pixelOutputSemanticMap[aPIXEL_OUTPUT_COUNT];
		for(uint32 i = 0; i < aPIXEL_OUTPUT_COUNT; ++i)
			pixelOutputSemanticMap[i] = aPIXEL_OUTPUT_COUNT;

		userFunctionOutputString.Clear();
		if (!pixelScriptData.userOutput.Empty())
		{
			//Build pixel user function output struct string
			userFunctionOutputString = "struct POUT {\n";
			userFunctionOutputCount = pixelScriptData.userOutput.Size();
			for(uint32 i = 0; i < userFunctionOutputCount; ++i)
			{
				userFunctionOutputString += pixelScriptData.userOutput[i].type;
				userFunctionOutputString += " ";
				userFunctionOutputString += pixelScriptData.userOutput[i].name;
				userFunctionOutputString += ";\n";

				pixelOutputSemanticMap[pixelScriptData.userOutput[i].pixelSemantic] = i;
			}

			userFunctionOutputString += "};\n";
		}

		MashShaderString pixelFunction(m_stringMemoryPool);
		pixelFunction += "_PIXELOUT _psmain(PIN input){\n\
			_PIXELOUT output;\n";

		/*
			If the pixel shader doesn't return anything, or it hasn't been provided,
			then we just supply some default variables
		*/
		if (!pixelScriptData.userOutput.Empty())
		{
			//call the user function
			pixelFunction += "POUT userOutput = ";
			pixelFunction +=  pixelScriptData.entry + "(input);\n";
		}

		MashShaderString diffuseName(m_stringMemoryPool);
		if (pixelOutputSemanticMap[aPIXEL_OUTPUT_DIFFUSE] == aPIXEL_OUTPUT_COUNT)
		{
			pixelFunction += "float4 _diffuse = float4(1.0, 1.0, 1.0, 1.0);\n";
			diffuseName = "_diffuse";
		}
		else
		{
			diffuseName = "userOutput." + pixelScriptData.userOutput[pixelOutputSemanticMap[aPIXEL_OUTPUT_DIFFUSE]].name;
		}

		if (lightingType == aLIGHT_TYPE_VERTEX || lightingType == aLIGHT_TYPE_PIXEL || lightingType == aLIGHT_TYPE_DEFERRED)
		{
			if (lightingType == aLIGHT_TYPE_VERTEX)
			{
				pixelFunction += "output.colour = float4((input.";
				pixelFunction += vertexScriptData.userOutput[vertexOutputSemanticMap[aVERTEX_OUTPUT_LIGHT_DIFFUSE]].name;
				pixelFunction += "*" + diffuseName;
				pixelFunction += ".xyz) + input.";
				pixelFunction += vertexScriptData.userOutput[vertexOutputSemanticMap[aVERTEX_OUTPUT_LIGHT_SPECULAR]].name;
				pixelFunction += ", " + diffuseName;
				pixelFunction += ".w);\n";
			}
			else
			{
				MashShaderString viewNormName(m_stringMemoryPool);
				MashShaderString specularName(m_stringMemoryPool);

				if (pixelOutputSemanticMap[aPIXEL_OUTPUT_SPECULAR] == aPIXEL_OUTPUT_COUNT)
				{
					if (vertexOutputSemanticMap[aVERTEX_OUTPUT_SPECULAR] != aVERTEX_OUTPUT_COUNT)
					{
						/*
							No specular data was given from the pixel function, but it was given from the vertex
							function. So we fill in the pixel data here from the vertex data.
						*/
						specularName = "input." + vertexScriptData.userOutput[vertexOutputSemanticMap[aVERTEX_OUTPUT_SPECULAR]].name;
					}
					else
					{
						/*
							No specular data given so just create a dummy.
						*/
						pixelFunction += "float4 _specular = float4(1.0, 1.0, 1.0, 1.0);\n";
						specularName = "_specular";
					}
				}
				else
				{
					specularName = "userOutput." + pixelScriptData.userOutput[vertexOutputSemanticMap[aPIXEL_OUTPUT_SPECULAR]].name;
				}

				if (pixelOutputSemanticMap[aPIXEL_OUTPUT_VNORM] == aPIXEL_OUTPUT_COUNT)
				{
					/*
						user didnt output vnorm from the pixel function, but it was given from the vertex function.
						So we fill the pixel normal from the vertex normal
					*/
					if (vertexOutputSemanticMap[aVERTEX_OUTPUT_VNORM] != aVERTEX_OUTPUT_COUNT)
					{
						//first normalize it
						pixelFunction += "input.";
						pixelFunction += vertexScriptData.userOutput[vertexOutputSemanticMap[aVERTEX_OUTPUT_VNORM]].name;
						pixelFunction += " = normalize(input.";
						pixelFunction += vertexScriptData.userOutput[vertexOutputSemanticMap[aVERTEX_OUTPUT_VNORM]].name;
						pixelFunction += ");\n";

						viewNormName = "input." + vertexScriptData.userOutput[vertexOutputSemanticMap[aVERTEX_OUTPUT_VNORM]].name;
					}
					else
					{
						/*
							No normal given from either vertex or pixel functions. So we create a dummy.

							TODO : Just calculate ambient lighting instead
						*/
						pixelFunction += "float3 _normal = float3(0.0, 0.0, 0.0);\n";
						viewNormName = "_normal";
					}
				}
				else
				{
					//normalize it
					pixelFunction += "userOutput.";
					pixelFunction += pixelScriptData.userOutput[pixelOutputSemanticMap[aPIXEL_OUTPUT_VNORM]].name;
					pixelFunction += " = normalize(userOutput.";
					pixelFunction += pixelScriptData.userOutput[pixelOutputSemanticMap[aPIXEL_OUTPUT_VNORM]].name;
					pixelFunction += ");\n";

					viewNormName = "userOutput." + pixelScriptData.userOutput[pixelOutputSemanticMap[aPIXEL_OUTPUT_VNORM]].name;
				}

				if (lightingType == aLIGHT_TYPE_PIXEL)
				{
					//add forward rendered lighting code
					pixelScriptData.includes.PushBack(MashShaderString(m_effectIncludes[aEFF_INC_FORWARD_RENDERED_LIGHTING_G].GetCString(), m_stringMemoryPool));

					pixelFunction += "sLightOutput lightingOutput = MashForwardRenderedLighting(";
					pixelFunction += viewNormName;
					pixelFunction += ", input.";
					pixelFunction += vertexScriptData.userOutput[vertexOutputSemanticMap[aVERTEX_OUTPUT_VPOS]].name;
					pixelFunction += ".xyz, ";
					pixelFunction += specularName + ");\n";

					pixelFunction += "output.colour = float4(lightingOutput.specular + (lightingOutput.diffuse * ";
					pixelFunction += diffuseName;
					pixelFunction += ".xyz), ";
					pixelFunction += diffuseName;
					pixelFunction += ".w);\n";
				}
				else if (lightingType == aLIGHT_TYPE_DEFERRED)
				{
					AddEffectAutoUnique(aEFFECT_DATA_FLOAT2, aEFFECT_CAMERA_NEAR_FAR, pixelScriptData.autos);

					pixelFunction += "output.colour = ";
					pixelFunction += diffuseName;
					pixelFunction += ";\noutput.normal = 0.5 * (float4(";
					pixelFunction += viewNormName;
					pixelFunction += ", 1.0) + 1.0);\n";
					pixelFunction += "output.specular = ";
					pixelFunction += specularName;
					pixelFunction += ";\n";
					pixelFunction += "output.depth = (input.";
					pixelFunction += vertexScriptData.userOutput[vertexOutputSemanticMap[aVERTEX_OUTPUT_VPOS]].name;
					pixelFunction += ".z - autoCameraNearFar.x) / (autoCameraNearFar.y - autoCameraNearFar.x);\n";
				}
			}
		}
		else
		{
			pixelFunction += "output.colour = ";
			pixelFunction += diffuseName;
			pixelFunction += ";\n";
		}

		pixelFunction += "return output;\n}\n";

		userFunctionString = pixelScriptData.source;
		pixelScriptData.source = pixelInputString;
		pixelScriptData.source += pixelOutString;
		pixelScriptData.source += userFunctionOutputString;
        
#ifdef MASH_LOG_ENABLED
		//this is only needed for debugging
        for(unsigned int i = 0; i < pixelScriptData.source.Size(); ++i)
        {
            if (pixelScriptData.source[i] == 10)
                ++pixelScriptData.userSourceLineStart;
        }
#endif
		pixelScriptData.source += userFunctionString;
		pixelScriptData.source += pixelFunction;
		pixelScriptData.entry = "_psmain";
		return aMASH_OK;
	}

	bool CMashShaderCompiler::IsSemantic(const MashShaderString &stringToCheck, const int8 *semantic, int8 *digitBufferOut, uint32 digitBufferLen)
	{
		memset(digitBufferOut, 0, digitBufferLen);

		const uint32 chkLen = stringToCheck.Size();//strlen(stringToCheck);
		const uint32 semLen = strlen(semantic);

		if (semLen > chkLen)
			return false;
	
		uint32 location = 0;
		while(location < semLen)
		{
			int8 a = toupper(stringToCheck[location]);
			int8 b = toupper(semantic[location]);
			++location;

			if (a != b)
				return false;
		}

		if (isdigit(stringToCheck[location]))
		{
			uint32 digitCounter = 0;
			while ((location < chkLen) && (digitCounter < digitBufferLen))
			{
				int8 n = stringToCheck[location];
				++location;

				if (isdigit(n))
					digitBufferOut[digitCounter++] = n;
				else
				{
					digitBufferOut[digitCounter] = 0;
					break;
				}
			}
		}

		return true;
	}

	eMASH_STATUS CMashShaderCompiler::ConvertEffectToDX9HLSL(const int8 *source, uint32 sourceLength, MashShaderString &out)
	{
		/*
			Converting to/from shader profile 3 to 4 is fairly simple, as too
			is this code. We just need to convert a few semantics.
		*/
		const uint32 end = sourceLength;
		const int8 *data = source;
		uint32 location = 0;
		int8 outChar;
		MashShaderString outString(m_stringMemoryPool);
		MashShaderString convertedShader(m_stringMemoryPool);
		convertedShader.Reserve(end);
		const uint32 semanticNumberBufferLen = 10;
		int8 semanticNumber[semanticNumberBufferLen];
		while(location < end)
		{
			uint32 readCount = scriptreader::ReadNextChar(data, location, end, outChar);

			if (readCount)
			{
				convertedShader += outChar;
				if (outChar == ':')
				{
					scriptreader::ReadNextString(data, location, end, outString);

					if (!outString.Empty())
					{
						if (IsSemantic(outString, "SV_POSITION", semanticNumber, semanticNumberBufferLen))
						{
							outString = "POSITION";
							outString += semanticNumber;
						}
						else if (IsSemantic(outString, "SV_TARGET", semanticNumber, semanticNumberBufferLen))
						{
							outString = "COLOR";
							outString += semanticNumber;
						}

						convertedShader += outString;
					}
				}
			}
		}

		out = convertedShader;

		return aMASH_OK;
	}

	eMASH_STATUS CMashShaderCompiler::ConvertProgramsIntoNativeFormat(MashFileManager *fileManager, eSHADER_API_TYPE effectAPI,
			sEffectScriptData &vertexScriptData, sEffectScriptData &pixelScriptData, MashStringc generatedEffectNames[2])
	{
		const uint32 effectScriptCount = 2;
		sEffectScriptData *effectScriptArray[] = {&vertexScriptData, &pixelScriptData};
		MashShaderString intermediateEffects[effectScriptCount] = {MashShaderString(m_stringMemoryPool), MashShaderString(m_stringMemoryPool)};

		MashShaderString macroString(m_stringMemoryPool);
		/*
			Add in macros.
			For ogl, all macros are added here as they are not supported in the
			actual compiler.
			For dx, only macros added during generation are added here. Other user
			macros will be added by the dx compiler.
		*/
		for(uint32 i = 0; i < effectScriptCount; ++i)
		{
			if (!effectScriptArray[i]->macros.Empty())
			{
				macroString.Clear();
				for(uint32 macro = 0; macro < effectScriptArray[i]->macros.Size(); ++macro)
				{
					macroString += "#define ";
					macroString += effectScriptArray[i]->macros[macro].name.GetCString();
					if (!effectScriptArray[i]->macros[macro].definition.Empty())
					{
						macroString += " ";
						macroString += effectScriptArray[i]->macros[macro].definition.GetCString();
					}
					macroString += "\n";
                    
                    ++effectScriptArray[i]->userSourceLineStart;
				}
				
                intermediateEffects[i] = macroString + effectScriptArray[i]->finalProgram;
			}
            else
            {
                intermediateEffects[i] = effectScriptArray[i]->finalProgram;
            }
		}

        /*
            The HLSL2GLSL parser is always used no matter what the target
            so we can handle all errors here.
        */
		bool error = false;
		{
			ShHandle parsers[EShLangCount];
			uint32 shaderCount = 0;
			int32 debugOptions = 0;

			if (!m_isBatchCompileEnabled)
			{
				//now start HLSL2GLSL
				Hlsl2Glsl_Initialize();
			}

			MashShaderString macroString(m_stringMemoryPool);
			for(uint32 i = 0; i < effectScriptCount; ++i)
			{
				if(!error)
				{
					EShLanguage language = EShLangVertex;
					switch(effectScriptArray[i]->programType)
					{
					case aPROGRAM_VERTEX:
						language = EShLangVertex;
						break;
					case aPROGRAM_PIXEL:
						language = EShLangFragment;
						break;
					default:
						{
							MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, "Unknown program type.", "CMashShaderCompiler::ConvertEffectPrograms");
							error = true;
						}
					}

					parsers[shaderCount] = Hlsl2Glsl_ConstructParser(language, debugOptions);
					if (!parsers[shaderCount])
					{
						MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, "Failed to construct parser.", "CMashShaderCompiler::ConvertEffectPrograms");
						error = true;
					}
					else
					{
                        
                        g_mashCurrentParsingEffect = effectScriptArray[i];
                        g_parsingHLSLFiles = true;
                        const int8 *shaderSourceArray[] = {intermediateEffects[i].GetCString()};
						++shaderCount;
						if (!Hlsl2Glsl_Parse(parsers[shaderCount-1], shaderSourceArray, 1, debugOptions))
						{
							MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_ERROR, 
                                               "CMashShaderCompiler::ConvertEffectPrograms",
                                               "Failed to parse shader '%s'.", effectScriptArray[i]->fileName.GetCString());
                            
							MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, Hlsl2Glsl_GetInfoLog(parsers[shaderCount-1]), "CMashShaderCompiler::ConvertEffectPrograms");
							error = true;
						}
					}
				}
			}

            //convert effect into openGL
			ShHandle translator = 0;
			if (!error && (effectAPI == aSHADERAPITYPE_OPENGL))
			{
				translator = Hlsl2Glsl_ConstructTranslator(debugOptions);
				Hlsl2Glsl_UseUserVaryings(translator, true);

                g_mashCurrentParsingEffect = 0;
                g_parsingHLSLFiles = false;
				if (!Hlsl2Glsl_Translate(translator, parsers, shaderCount, vertexScriptData.entry.GetCString(), pixelScriptData.entry.GetCString()))
				{
					MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, "Failed to translate shaders.", "CMashShaderCompiler::ConvertEffectPrograms");
					MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, Hlsl2Glsl_GetInfoLog(translator), "CMashShaderCompiler::ConvertEffectPrograms");
					error = true;
				}
				else
				{
					const int8 *apiHeader = m_renderer->GetMaterialManager()->_GetAPIShaderHeader();
					//add in #version
					if (apiHeader)
					{
						vertexScriptData.finalProgram = apiHeader;
                        vertexScriptData.finalProgram += "\n#extension GL_ARB_uniform_buffer_object : enable\n";
						vertexScriptData.finalProgram += Hlsl2Glsl_GetShader(translator, EShLangVertex);

						pixelScriptData.finalProgram = apiHeader;
                        pixelScriptData.finalProgram += "\n#extension GL_ARB_uniform_buffer_object : enable\n";
						pixelScriptData.finalProgram += Hlsl2Glsl_GetShader(translator, EShLangFragment);
					}
				}
			}

			for(uint32 i = 0; i < shaderCount; ++i)
				Hlsl2Glsl_Destruct(parsers[i]);

			if (translator)
				Hlsl2Glsl_Destruct(translator);

			if (!m_isBatchCompileEnabled)
				Hlsl2Glsl_Finalize();
		}
		
        if (effectAPI == aSHADERAPITYPE_D3D9)
		{
			/*
				Convert the shader to DX9 compatable
			*/
			for(uint32 i = 0; i < effectScriptCount; ++i)
			{
				if (effectScriptArray[i])
				{
					ConvertEffectToDX9HLSL(intermediateEffects[i].GetCString(), 
					effectScriptArray[i]->finalProgram.Size(), 
					effectScriptArray[i]->finalProgram);
				}
			}
		}
		else if (effectAPI == aSHADERAPITYPE_D3D10)
		{
			//nothing todo
			//D3D handles the preprocessor step
            for(uint32 i = 0; i < effectScriptCount; ++i)
			{
				if (effectScriptArray[i])
				{
					effectScriptArray[i]->finalProgram = intermediateEffects[i];
				}
			}
		}
#ifdef MASH_LOG_ENABLED
		//save intermediate shader, regardless of errors
		if (!m_renderer->GetMaterialManager()->GetIntermediateEffectOutputDirectory().Empty() || 
            !m_renderer->GetMaterialManager()->GetCompiledEffectOutputDirectory().Empty())
		{
			const int8 *oglPrefix = "_ogl_intermediate_";
			const int8 *dxPrefix = "_dx_intermediate_";
            char numberBuffer[100];
            MashFileStream *pFileStream = fileManager->CreateFileStream();
			for(uint32 i = 0; i < effectScriptCount; ++i)
			{
				/*
					Save intermediate shaders to file. This is the effect plus any includes plus generated runtime includes
				*/
                GetFileName(effectScriptArray[i]->fileName.GetCString(), generatedEffectNames[i]);
                generatedEffectNames[i] += (effectAPI == aSHADERAPITYPE_OPENGL)?oglPrefix:dxPrefix;
                generatedEffectNames[i] += mash::helpers::NumberToString(numberBuffer, sizeof(numberBuffer), effectScriptArray[i]->uniqueEffectNumber);
                generatedEffectNames[i] += ".eff";
                
                MashStringc concatenatedPath;
                ConcatenatePaths(m_renderer->GetMaterialManager()->GetIntermediateEffectOutputDirectory().GetCString(), 
                                             generatedEffectNames[i].GetCString(), concatenatedPath);
                
                pFileStream->AppendStringToStream(intermediateEffects[i].GetCString());
                if (pFileStream->SaveFile(concatenatedPath.GetCString(), aFILE_IO_TEXT) == aMASH_FAILED)
                {
                    MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_ERROR, 
                                        "CMashShaderCompiler::ConvertEffectPrograms",
                                        "Failed to save intermediate shader '%s' to file.", 
                                        effectScriptArray[i]->fileName.GetCString());
                }
                
                pFileStream->ClearStream();
                
                generatedEffectNames[i].Clear();
                
                if (!m_renderer->GetMaterialManager()->GetCompiledEffectOutputDirectory().Empty())
                {
                    GetFileName(effectScriptArray[i]->fileName.GetCString(), generatedEffectNames[i]);
                    generatedEffectNames[i] += "_";
                    generatedEffectNames[i] += mash::helpers::GetShaderProfileString(effectScriptArray[i]->target);
                    generatedEffectNames[i] += "_";
                    generatedEffectNames[i] += mash::helpers::NumberToString(numberBuffer, sizeof(numberBuffer), effectScriptArray[i]->uniqueEffectNumber);
                    generatedEffectNames[i] += ".";
                    generatedEffectNames[i] += mash::helpers::GetShaderFileExtention(mash::helpers::GetAPIFromShaderProfile(effectScriptArray[i]->target));
                    
                    concatenatedPath.Clear();
                    ConcatenatePaths(m_renderer->GetMaterialManager()->GetCompiledEffectOutputDirectory().GetCString(), 
                                                 generatedEffectNames[i].GetCString(), concatenatedPath);
                    
                    pFileStream->AppendStringToStream(effectScriptArray[i]->finalProgram.GetCString());
                    if (pFileStream->SaveFile(concatenatedPath.GetCString(), aFILE_IO_TEXT) == aMASH_FAILED)
                    {
                        MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_ERROR, 
                                            "CMashShaderCompiler::ConvertEffectPrograms",
                                            "Failed to save converted shader '%s' to file.", 
                                            effectScriptArray[i]->fileName.GetCString());
                    }
                }
                
                pFileStream->ClearStream();
			}
            
            if (pFileStream)
                pFileStream->Destroy(); 
		}
#endif
		if (error)
			return aMASH_FAILED;

		return aMASH_OK;
	}

	eMASH_STATUS CMashShaderCompiler::RecompileDeferredLightingShaders(MashSceneManager *sceneManager, MashFileManager *fileManager)
	{
		///////////////directional/////////////////////////
		InitialiseMemoryPool();
		{//indent so the memory pool is destroyed last
		bool postShadowIncError = false;

		MashStringc lightCode;
		lightCode.Reserve(500);
		lightCode = "autos {\nsLight autoLight\n";

		if (sceneManager->GetDeferredDirShadowsEnabled())
			lightCode += "bool autoShadowsEnabled\n";

		lightCode += "}\ninclude {\n";
		lightCode += m_effectIncludes[aEFF_INC_DIRECTIONAL_LIGHTING] + "\n";

		if (sceneManager->GetDeferredDirShadowsEnabled())
		{
			if (m_effectIncludes[aEFF_INC_DIRECTIONAL_SHADOW_RECEIVER].Empty())
				postShadowIncError = true;

			lightCode += m_effectIncludes[aEFF_INC_DIRECTIONAL_SHADOW_RECEIVER] + "\n";
		}

		lightCode += "}\nsource {\nsLightOutput MashDeferredDirectionalLighting(float3 normalvs, float3 positionvs, float4 specular) {\n\
					 sLightOutput output = (sLightOutput)0;\n";
		
		if (sceneManager->GetDeferredDirShadowsEnabled())
		{
			lightCode += "float shadowFactor = 1.0f;\n \
						 if (autoShadowsEnabled) {\n\
						 shadowFactor = MashDirectionalShadows(positionvs);\n}\n";
		}
		else
			lightCode += "float shadowFactor = 1.0f;\n";

		lightCode += "output = MashDirectionalLighting(autoLight, normalvs, positionvs, specular, shadowFactor);\n";
		
		lightCode += "return output;\n}\n}\n";

		const int8 *saveFileName = m_effectIncludes[aEFF_INC_DIRECTIONAL_DEFERRED_LIGHTING_G].GetCString();
		//remove any previous entries
		fileManager->AddStringToVirtualFileSystem(saveFileName, 0);
		fileManager->AddStringToVirtualFileSystem(saveFileName, lightCode.GetCString());

		lightCode.Clear();

		/////////////////////////////spot///////////////////////////////////
		lightCode = "autos {\nsLight autoLight\n";

		if (sceneManager->GetDeferredSpotShadowsEnabled())
			lightCode += "bool autoShadowsEnabled\n";

		lightCode += "}\ninclude {\n";
		//lightCode += m_effectIncludes[aEFF_INC_LIGHT_SHADING] + "\n";//added at compile time
		lightCode += m_effectIncludes[aEFF_INC_SPOT_LIGHTING] + "\n";

		if (sceneManager->GetDeferredSpotShadowsEnabled())
		{
			if (m_effectIncludes[aEFF_INC_SPOT_SHADOW_RECEIVER].Empty())
				postShadowIncError = true;

			lightCode += m_effectIncludes[aEFF_INC_SPOT_SHADOW_RECEIVER] + "\n";
		}

		lightCode += "}\nsource {\nsLightOutput MashDeferredSpotLighting(float3 normalvs, float3 positionvs, float4 specular) {\n\
					 sLightOutput output = (sLightOutput)0;\n";
		
		if (sceneManager->GetDeferredSpotShadowsEnabled())
		{
			lightCode += "float shadowFactor = 1.0f;\n \
						 if (autoShadowsEnabled) {\n\
						 shadowFactor = MashSpotShadows(positionvs);\n}\n";
		}
		else
			lightCode += "float shadowFactor = 1.0f;\n";

		lightCode += "output = MashSpotLighting(autoLight, normalvs, positionvs, specular, shadowFactor);\n";
		
		lightCode += "return output;\n}\n}\n";

		saveFileName = m_effectIncludes[aEFF_INC_SPOT_DEFERRED_LIGHTING_G].GetCString();
		//remove any previous entries
		fileManager->AddStringToVirtualFileSystem(saveFileName, 0);
		fileManager->AddStringToVirtualFileSystem(saveFileName, lightCode.GetCString());

		lightCode.Clear();

		/////////////////////////////point///////////////////////////////////
		lightCode = "autos {\nsLight autoLight\n";

		if (sceneManager->GetDeferredPointShadowsEnabled())
			lightCode += "bool autoShadowsEnabled\n";

		lightCode += "}\ninclude {\n";
		lightCode += m_effectIncludes[aEFF_INC_POINT_LIGHTING] + "\n";

		if (sceneManager->GetDeferredPointShadowsEnabled())
		{
			if (m_effectIncludes[aEFF_INC_POINT_SHADOW_RECEIVER].Empty())
				postShadowIncError = true;

			lightCode += m_effectIncludes[aEFF_INC_POINT_SHADOW_RECEIVER] + "\n";
		}

		lightCode += "}\nsource {\nsLightOutput MashDeferredPointLighting(float3 normalvs, float3 positionvs, float4 specular) {\n\
					 sLightOutput output = (sLightOutput)0;\n";
		
		if (sceneManager->GetDeferredPointShadowsEnabled())
		{
			lightCode += "float shadowFactor = 1.0f;\n \
						 if (autoShadowsEnabled) {\n\
						 shadowFactor = MashPointShadows(positionvs);\n}\n";
		}
		else
			lightCode += "float shadowFactor = 1.0f;\n";

		lightCode += "output = MashPointLighting(autoLight, normalvs, positionvs, specular, shadowFactor);\n";
		
		lightCode += "return output;\n}\n}\n";

		if (postShadowIncError)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
				"Shadows are enabled but no shadow receiver include file is present. Make sure you have a valid shadow caster set in the scene manager for all active shadow light types.",
				"CMashShaderCompiler::RecompileDeferredLightingShaders");
		}

		saveFileName = m_effectIncludes[aEFF_INC_POINT_DEFERRED_LIGHTING_G].GetCString();
		//remove any previous entries
		fileManager->AddStringToVirtualFileSystem(saveFileName, 0);
		fileManager->AddStringToVirtualFileSystem(saveFileName, lightCode.GetCString());
		}

		//destroy the mem pool if batch compile is not enabled
		DestroyMemoryPool();
		return aMASH_OK;
	}

	eMASH_STATUS CMashShaderCompiler::RecompileForwardRenderedLights(MashSceneManager *sceneManager, MashFileManager *fileManager)
	{
		InitialiseMemoryPool();
		{//indent so the memory pool is destroyed last
		MashArray<MashStringc> lightOutputNames;
		uint32 iLightCount = 0;
		int8 buffer[100];
		
		const uint32 iForwardRenderedLightCount = sceneManager->GetForwardRenderedLightCount();
		const MashLight *firstLight = sceneManager->GetFirstForwardRenderedLight();

		MashStringc lightCode;
		lightCode.Reserve(300);

		lightCode += "source {\n";

		//define minimum function
		lightCode += "sLightOutput MashForwardRenderedLighting(float3 viewSpaceNormal, float3 viewSpacePosition, float4 specularIntensity) {\n\
			sLightOutput output = (sLightOutput)0;\n";

		bool includeDirLights = false;
		bool includeSpotLights = false;
		bool includePointLights = false;
		bool useShadows = false;
		int32 shadowIncludeType = -1;
		if (iForwardRenderedLightCount > 0)
		{
			//add shadow code if needed
			if (firstLight && firstLight->IsShadowsEnabled())
			{
				switch(firstLight->GetLightType())
				{
				case mash::aLIGHT_DIRECTIONAL:
					{
						shadowIncludeType = mash::aLIGHT_DIRECTIONAL;
						lightCode += "float shadowFactor = MashDirectionalShadows(viewSpacePosition);\n";
						useShadows = true;
						break;
					}
				case mash::aLIGHT_POINT:
					{
						shadowIncludeType = mash::aLIGHT_POINT;
						lightCode += "float shadowFactor = MashPointShadows(viewSpacePosition);\n";
						useShadows = true;
						break;
					}
				case mash::aLIGHT_SPOT:
					{
						shadowIncludeType = mash::aLIGHT_SPOT;
						lightCode += "float shadowFactor = MashSpotShadows(viewSpacePosition);\n";
						useShadows = true;
						break;
					}
				};
			}

			const MashArray<MashLight*> &forwardLightList = sceneManager->GetForwardRenderedLightList();
			MashArray<MashLight*>::ConstIterator lightIter = forwardLightList.Begin();
			MashArray<MashLight*>::ConstIterator lightIterEnd = forwardLightList.End();

			for(; lightIter != lightIterEnd; ++lightIter)
			{
				MashStringc sLightOutputName = "lightOutput";
				sLightOutputName += mash::helpers::NumberToString(buffer, 100, iLightCount);
				lightOutputNames.PushBack(sLightOutputName);
				lightCode += "sLightOutput " + sLightOutputName;
				MashStringc sLightIndex = "autoLight[";
				sLightIndex += buffer;
				sLightIndex += "]";
				//Add light function + code based on light type
				switch((*lightIter)->GetLightType())
				{
				case mash::aLIGHT_DIRECTIONAL:
					{
						includeDirLights = true;
						lightCode += "=MashDirectionalLighting(";
						lightCode += sLightIndex;
					}
					break;
				case mash::aLIGHT_SPOT:
					{
						includeSpotLights = true;
						lightCode += "=MashSpotLighting(";
						lightCode += sLightIndex;
					}
					break;
				case mash::aLIGHT_POINT:
					{
						includePointLights = true;
						lightCode += "=MashPointLighting(";
						lightCode += sLightIndex;
					}
					break;
				};

				/*
					shadows are used for the first light only.
				*/
				if (useShadows)
				{
					lightCode += ",viewSpaceNormal, viewSpacePosition, specularIntensity, shadowFactor);\n";
					useShadows = false;
				}
				else
				{
					lightCode += ",viewSpaceNormal, viewSpacePosition, specularIntensity, 1.0f);\n";
				}

				++iLightCount;
			}

			for(uint32 i = 0; i < iLightCount; ++i)
			{
				if (i == 0)
					lightCode += "output.diffuse = ";
				else
					lightCode += "output.diffuse += ";

				lightCode += lightOutputNames[i];
				lightCode += ".diffuse;\n";

				if (i == 0)
					lightCode += "output.specular = ";
				else
					lightCode += "output.specular += ";

				lightCode += lightOutputNames[i];
				lightCode += ".specular;\n";
			}

			lightCode += "output.diffuse = saturate(output.diffuse);\n\
				output.specular = saturate(output.specular);\n";
		}
		else
		{
			lightCode += "output.diffuse = float3(1.0f, 1.0f, 1.0f);\n\
				output.specular = float3(0.0f, 0.0f, 0.0f);\n";
		}

		lightCode += "return output;\n";

		lightCode += "}\n";

		lightCode += "}\n";

		//include lighting shader
		/*
			Needs to be included even if no lights are in the scene so that
			all structs and functions are defined
		*/
		lightCode += "include {\n";

		if (includeDirLights)
		{
			lightCode += m_effectIncludes[aEFF_INC_DIRECTIONAL_LIGHTING];
			lightCode += "\n";
		}
		if (includeSpotLights)
		{
			lightCode += m_effectIncludes[aEFF_INC_SPOT_LIGHTING];
			lightCode += "\n";
		}
		if (includePointLights)
		{
			lightCode += m_effectIncludes[aEFF_INC_POINT_LIGHTING];
			lightCode += "\n";
		}

		if (!includeDirLights && !includeSpotLights && !includePointLights)
		{
			lightCode += m_effectIncludes[aEFF_INC_LIGHT_STRUCTURES];
			lightCode += "\n";
		}

		bool postShadowIncError = false;
		switch(shadowIncludeType)
		{
		case mash::aLIGHT_DIRECTIONAL:
			{
				if (m_effectIncludes[aEFF_INC_DIRECTIONAL_SHADOW_RECEIVER].Empty())
					postShadowIncError = true;
				else
					lightCode += m_effectIncludes[aEFF_INC_DIRECTIONAL_SHADOW_RECEIVER];
			}
			break;
		case mash::aLIGHT_POINT:
			{
				if (m_effectIncludes[aEFF_INC_POINT_SHADOW_RECEIVER].Empty())
					postShadowIncError = true;
				else
					lightCode += m_effectIncludes[aEFF_INC_POINT_SHADOW_RECEIVER];
			}
			break;
		case mash::aLIGHT_SPOT:
			{
				if (m_effectIncludes[aEFF_INC_SPOT_SHADOW_RECEIVER].Empty())
					postShadowIncError = true;
				else
					lightCode += m_effectIncludes[aEFF_INC_SPOT_SHADOW_RECEIVER];
			}
			break;
		default:
			break;
		};

		if (postShadowIncError)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
				"Shadows are enabled but no shadow receiver include file is present. Make sure you have a valid shadow caster set in the scene manager for all active shadow light types.",
				"CMashShaderCompiler::RecompileForwardRenderedLights");
		}

		lightCode += "\n}\n";

		lightCode += "autos {\n";

		//add in the light array struct
		if (firstLight)
		{
			int8 buffer[100];
			memset(buffer, 0, 100);
			mash::helpers::NumberToString(buffer, 100, iForwardRenderedLightCount);

			lightCode += "sLight autoLight ";
			lightCode += buffer;
			lightCode += "\n";
		}

		lightCode += "}\n";

		const int8 *saveFileName = m_effectIncludes[aEFF_INC_FORWARD_RENDERED_LIGHTING_G].GetCString();

		//remove any previous entries
		fileManager->AddStringToVirtualFileSystem(saveFileName, 0);
		fileManager->AddStringToVirtualFileSystem(saveFileName, lightCode.GetCString());

		}

		//destroy the mem pool if batch compile is not enabled
		DestroyMemoryPool();

		return aMASH_OK;
	}

	eMASH_STATUS CMashShaderCompiler::RecompileCommonRunTimeFunctions(MashSceneManager *sceneManager, MashFileManager *fileManager)
	{
		RecompileForwardRenderedLights(sceneManager, fileManager);
		return aMASH_OK;
	}
}
