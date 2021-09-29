//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_SHADER_COMPILER_H_
#define _C_MASH_SHADER_COMPILER_H_

#include "MashReferenceCounter.h"
#include "MashEnum.h"
#include "MashTypes.h"
#include "MashString.h"
#include "MashArray.h"
#include "MashGenericScriptReader.h"
#include "Material.h"
#include <stack>
#include <set>
#include "MashList.h"
#include <map>
#include "MashMemoryPool.h"
#include "MashString.h"
#include "MashFunctor.h"
namespace mash
{
	class MashSceneManager;
	class MashVideo;

	class MashFileManager;

	typedef MashFunctor<MashArray<sEffectMacro> > MashEffectIncludeFunctor;

	class CMashShaderCompiler : public MashReferenceCounter
	{
	public:
		typedef MashMemoryPool<sMashMemoryPoolError, sMashAllocAllocatorFunctor, sMashFreeDeallocatorFunctor> MemPoolType;
		typedef MashBaseString<int8, MemPoolType> MashShaderString;
		//this pool is used only by MashShaderString.
		MemPoolType m_stringMemoryPool;

		struct sEffectScriptData
		{
			struct sAutoDeclaration
			{
				MashShaderString type;
				MashShaderString name;
				uint32 arraySize;

				sAutoDeclaration(const MemPoolType &stringPool):type(stringPool), name(stringPool), arraySize(0){}
			};

			struct sVertexInput
			{
				MashShaderString type;
				MashShaderString name;
				eVERTEX_DECLUSAGE semantic;

				sVertexInput(const MemPoolType &stringPool):type(stringPool), name(stringPool){}
			};

			struct sUserFunctionOutput
			{
				MashShaderString type;
				MashShaderString name;
				bool passToPixel;
				bool isUserData;

				union
				{
					eVERTEX_OUTPUT vertexSemantic;
					ePIXEL_OUTPUT pixelSemantic;
				};

				sUserFunctionOutput(const MemPoolType &stringPool):type(stringPool), name(stringPool){}

				sUserFunctionOutput(const MemPoolType &stringPool, const int8 *_name, const int8 *_type, bool _passToPixel, eVERTEX_OUTPUT _semantic, bool _isUserData = false):
				type(_type, stringPool), name(_name, stringPool), passToPixel(_passToPixel), vertexSemantic(_semantic), isUserData(_isUserData){}

				sUserFunctionOutput(const MemPoolType &stringPool, const int8 *_name, const int8 *_type,bool _passToPixel, ePIXEL_OUTPUT _semantic, bool _isUserData = false):
				type(_type, stringPool), name(_name, stringPool), passToPixel(_passToPixel), pixelSemantic(_semantic), isUserData(_isUserData){}
			};

			MashArray<sEffectMacro> macros;
			MashArray<sVertexInput> vertexInput;
			MashArray<sUserFunctionOutput> userOutput;
			MashArray<sAutoDeclaration> autos;
			MashArray<MashShaderString> includes;
			MashShaderString header;
			MashShaderString source;
			MashShaderString finalProgram;
			MashShaderString entry;
			ePROGRAM_TYPE programType;
			eSHADER_PROFILE target;
			MashShaderString fileName;
            int uniqueEffectNumber;
            
            int userSourceLineStart;
            int userSourceScriptLineStart;

			sEffectScriptData(const MemPoolType &stringPool):header(stringPool), 
				source(stringPool), 
				finalProgram(stringPool),
				entry(stringPool),
				fileName(stringPool){}

			sEffectScriptData():header(), 
				source(), 
				finalProgram(),
				entry(),
				fileName(){}
		};

        
        enum eEFFECT_DATA_TYPE
        {
            aEFFECT_DATA_FLOAT,
            aEFFECT_DATA_FLOAT2,
            aEFFECT_DATA_FLOAT3,
            aEFFECT_DATA_FLOAT4,
            aEFFECT_DATA_FLOAT4X4
        };

		enum eMATERIAL_TYPE
		{
			aMATERIAL_TYPE_NORMAL,
			aMATERIAL_TYPE_DIR_SHADOW_CASTER,
			aMATERIAL_TYPE_SPOT_SHADOW_CASTER,
			aMATERIAL_TYPE_POINT_SHADOW_CASTER
		};

		struct sProgramData
		{
			MashShaderString fileName;
			MashShaderString entry;
			eSHADER_PROFILE profile;

			sProgramData(const MemPoolType &stringPool):fileName(stringPool), entry(stringPool){}
		};
	private:

		struct sFileLayout
		{
			uint32 vertexDeclStart;
			uint32 vertexDeclEnd;

			uint32 userOutputStart;
			uint32 userOutputEnd;

			uint32 autoStart;
			uint32 autoEnd;

			uint32 includeStart;
			uint32 includeEnd;

			uint32 sourceStart;
			uint32 sourceEnd;

			uint32 headerStart;
			uint32 headerEnd;

			sFileLayout():vertexDeclStart(0),vertexDeclEnd(0),
			userOutputStart(0), userOutputEnd(0), 
			autoStart(0), autoEnd(0), includeStart(0), includeEnd(0),
			sourceStart(0), sourceEnd(0), headerStart(0), headerEnd(0){}
		};

		//utility function
		MashList<CMashShaderCompiler::sEffectScriptData>::Iterator FindIncludeStringInList(MashList<sEffectScriptData> &listToSearch, const int8 *s)const;
		eMASH_STATUS CollectIncludesFromEffectScript(MashFileManager *fileManager, sEffectScriptData &currentInclude, MashList<sEffectScriptData> &closedBranch);

		eMASH_STATUS RecompileForwardRenderedLights(MashSceneManager *sceneManager, MashFileManager *fileManager);

		mash::eVERTEX_DECLUSAGE GetVertexInputSemanticAsHLSLType(mash::eVERTEX_DECLUSAGE usage);
		eVERTEX_OUTPUT GetVertexOutputSemanticAsInterpolator(eVERTEX_OUTPUT semantic);
		eMASH_STATUS LinkEffectProgram(MashFileManager *fileManager, sEffectScriptData &scriptData, const MashStringc &overrideLightingShading);
		void AddEffectAutoUnique(eEFFECT_DATA_TYPE dataType, eEFFECT_SEMANTICS autoParam, MashArray<sEffectScriptData::sAutoDeclaration> &autos);
		eVERTEX_DECLUSAGE ConvertInputSemanticStringToEnum(const int8 *s);
		eVERTEX_OUTPUT ConvertVertexOutputSemanticStringToEnum(const int8 *s);
		ePIXEL_OUTPUT ConvertPixelOutputSemanticStringToEnum(const int8 *s);
		const int8* InputSemanticTypeToHLSLString(eVERTEX_DECLUSAGE semantic);
		const int8* VertexOutputInterpolatorToHLSLString(eVERTEX_OUTPUT semantic);
		eMASH_STATUS ReadIncludes(const int8 *fileData, uint32 start, uint32 end, MashArray<MashShaderString> &out);
		eMASH_STATUS ReadPixelOutput(const int8 *fileData, uint32 start, uint32 end, MashArray<sEffectScriptData::sUserFunctionOutput> &out);
		eMASH_STATUS ReadVertexInput(const int8 *fileData, uint32 start, uint32 end, MashArray<sEffectScriptData::sVertexInput> &out);
		eMASH_STATUS ReadVertexOutput(const int8 *fileData, uint32 start, uint32 end, MashArray<sEffectScriptData::sUserFunctionOutput> &out);
		eMASH_STATUS AnalyzeFile(MashFileManager *fileManager, const int8 *fileData, int32 fileDataLength, sEffectScriptData &formatedFileData);

		eMASH_STATUS ReadAutos(const int8 *fileData, uint32 start, uint32 end, MashArray<sEffectScriptData::sAutoDeclaration> &out);

		eMASH_STATUS _BuildVertxPixelShadowCasters(MashFileManager *fileManager,
			eLIGHTTYPE shadowEffectType,
			sEffectScriptData &vertexScriptData,
			sEffectScriptData &pixelScriptData);
		
		eMASH_STATUS _BuildVertxPixelShaders(sEffectScriptData &vertexScriptData,
			sEffectScriptData &pixelScriptData, eLIGHTING_TYPE lightingType);

		bool IsSemantic(const MashShaderString &stringToCheck, const int8 *semantic, int8 *digitBufferOut, uint32 digitBufferLen);
		eMASH_STATUS ConvertEffectToDX9HLSL(const int8 *source, uint32 sourceLength, MashShaderString &out);

		eMASH_STATUS SaveGeneratedData(MashFileManager *fileManager,
			const MashStringc &saveDir,
			const MashStringc &fileName,
			eSHADER_PROFILE target,
			const MashStringc &source,
			const MashStringc &prefixFilename);

		eMASH_STATUS ConvertProgramsIntoNativeFormat(MashFileManager *fileManager, eSHADER_API_TYPE effectAPI,
			sEffectScriptData &vertexScriptData, sEffectScriptData &pixelScriptData, MashStringc generatedEffectNames[2]);

		MashVideo *m_renderer;
		bool m_isBatchCompileEnabled;
		bool m_isMemoryPoolInitialised;

		struct sEffectIncludeCallback
		{
			MashStringc includeString;
			MashEffectIncludeFunctor functor;

			sEffectIncludeCallback(){}
			sEffectIncludeCallback(const MashStringc &str, MashEffectIncludeFunctor funct):includeString(str), functor(funct){}
		};

		MashStringc m_effectIncludes[aEFF_INC_COUNT];
		MashArray<sEffectIncludeCallback> m_effectIncludeCallbacks;//dont think a map is really needed for this

		void InitialiseMemoryPool();
		void DestroyMemoryPool();
	public:
		CMashShaderCompiler(MashVideo *renderer);
		~CMashShaderCompiler();

		void SetAlternateInclude(eSHADER_EFFECT_INCLUDES type, const int8 *includeString);
		void SetIncludeCallback(const int8 *includeString, MashEffectIncludeFunctor includeFunctor);
		void BeginBatchCompile();
		void EndBatchCompile();

		/*
				overrideLightingShading, This holds the file name of an effect that will override
				the default shading. Useful for different objects such
				as wood, metal, etc...
			*/
		eMASH_STATUS BuildRunTimeEffect(MashFileManager *fileManager, 
			MashEffect *effect, 
			const sEffectCompileArgs &compileArgs);

		eMASH_STATUS RecompileDeferredLightingShaders(MashSceneManager *sceneManager, MashFileManager *fileManager);

		eMASH_STATUS RecompileCommonRunTimeFunctions(MashSceneManager *sceneManager, MashFileManager *fileManager);
	};
}

#endif