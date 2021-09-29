//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_MATERIAL_MANAGER_INTERMEDIATE_H_
#define _MASH_MATERIAL_MANAGER_INTERMEDIATE_H_

#include "MashMaterialManager.h"

namespace mash
{
	class MashVideo;	

    /*!
        Should not be accessed directly, instead use MashMaterial.
    */
	class MashMaterialManagerIntermediate : public MashMaterialManager
	{
	protected:
		MashMaterialManagerIntermediate(mash::MashVideo *renderer);
		virtual ~MashMaterialManagerIntermediate();
		MashVideo *m_renderer;
	private:
		MashList<MashMaterial*> m_materials;
		MashMaterialBuilder *m_materialBuilder;
		
		uint32 m_materialNameCounter;

		MashStringc m_intermediateEffectOutputDirectory;
		MashStringc m_compiledEffectOutputDirectory;
		MashArray<MashAutoEffectParameter*> m_autoShaderParameters;

		eMASH_STATUS GenerateUniqueMaterialName(const MashStringc &orig, MashStringc &out);
	public:
		MashMaterial* GetStandardMaterial(eSTANDARD_MATERIAL materialType, bool *wasLoaded);
        MashMaterialBuilder* GetMaterialBuilder()const;

		eMASH_STATUS _Initialise(const sMashDeviceSettings &creationParameters);

		void RegisterAutoParameterHandler(MashAutoEffectParameter *autoParamHandler, bool overWrite = false);
		bool IsAutoParameter(const int8 *simpleName, uint32 &autoParamHandlerIndex)const;
		MashAutoEffectParameter* GetAutoParameterByName(const int8 *simpleName)const;

		const MashList<MashMaterial*>& GetMaterialList()const;

		eMASH_STATUS SaveAllSkinsToFile(const int8 *fileName);

		MashMaterial* GetMaterial(const int8 *materialName, 
			const int8 *filePath, 
			const sEffectMacro *compileArgs, 
			uint32 argCount, bool *wasLoaded, bool reload = false);

		eMASH_STATUS LoadMaterialFile(const int8 *filePath, 
			const sEffectMacro *compileArgs = 0, 
			uint32 argCount = 0, 
			MashArray<mash::MashMaterial*> *materialsOut = 0,
            bool reloadMaterial = false);

		eMASH_STATUS BuildRunTimeEffect(MashEffect *effect, 
			const sEffectCompileArgs &compileArgs);

		MashMaterial* AddMaterial(const int8 *name, const sMashVertexElement *vertexElements, uint32 elementCount);

		MashMaterial* FindMaterial(const int8 *name)const;

		void RemoveAllMaterials();
		void RemoveMaterial(MashMaterial *material);

		const MashStringc& GetCompiledEffectOutputDirectory()const;
		const MashStringc& GetIntermediateEffectOutputDirectory()const;

		void SetCompiledEffectOutputDirectory(const MashStringc &dir);
		void SetIntermediateEffectOutputDirectory(const MashStringc &dir);

		MashTechnique* _CreateTechnique();
		MashTechniqueInstance* _CreateTechniqueInstance(MashTechnique *refTechnique);
		MashMaterial* _CreateMaterial(const int8 *sName, MashMaterial *reference);

		eMASH_STATUS _RebuildDeferredLightingShaders();
		eMASH_STATUS _RebuildCommonSceneShaders();
		eMASH_STATUS _CompileAllMaterials(MashSceneManager *sceneManager, uint32 compileFlags = 0);
		void _SetProgramAutoParameter(MashEffect *effect, MashEffectParamHandle *parameter, uint32 parameterType, uint32 index = 0);

		void _BeginBatchMaterialCompile();
		void _EndBatchMaterialCompile();
	};
    
    inline MashMaterialBuilder* MashMaterialManagerIntermediate::GetMaterialBuilder()const
    {
        return m_materialBuilder;
    }

	inline const MashStringc& MashMaterialManagerIntermediate::GetCompiledEffectOutputDirectory()const
	{
		return m_compiledEffectOutputDirectory;
	}

	inline const MashStringc& MashMaterialManagerIntermediate::GetIntermediateEffectOutputDirectory()const
	{
		return m_intermediateEffectOutputDirectory;
	}

	inline const MashList<MashMaterial*>& MashMaterialManagerIntermediate::GetMaterialList()const
	{
		return m_materials;
	}
}

#endif