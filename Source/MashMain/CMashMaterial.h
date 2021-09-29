//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_MATERIAL_H_
#define _C_MASH_MATERIAL_H_

#include "MashMaterial.h"
namespace mash
{
	
	class MashVideo;

	class CMashMaterial : public MashMaterial
	{
	private:
		mash::MashVideo *m_pRenderer;
		std::map<MashStringc, MashArray<MashTechniqueInstance*> > m_techniqueGroups;
		MashStringc m_sMaterialName;

		MashCustomRenderPath *m_batch;
		MashVertex *m_vertexDeclaration;
		MashArray<uint32> m_lodDistances;
		bool m_autoLod;
		bool m_compiled;
		uint32 m_lastValidLod;
		
		MashTechniqueInstance *m_pActiveTechnique;
		MashStringc m_sActiveGroup;
		
		eMASH_STATUS _SetActiveTechniqueByLod(uint32 iLodLevel);
        void ClearMaterialData();
	public:
		CMashMaterial(mash::MashVideo *pRenderer, const int8 *sName, MashMaterial *reference);
		~CMashMaterial();

		MashMaterial* CreateInstance(const int8 *sName, bool copyTextures = true);
		MashMaterial* CreateIndependentCopy(const int8 *sName, bool copyTextures = true);

		const MashStringc& GetMaterialName()const;

		//not used here
		void OnDependencyCompiled(MashVideo *renderer, MashMaterialDependentResourceBase *dependency){}

		eMASH_STATUS OnSet();

		void SetTexture(uint32 index, MashTexture *texture);
		void SetTextureState(uint32 index, MashTextureState *state);

		bool GetAutoLodEnabled()const;
		void SetAutoLod(bool bValue);
		eMASH_STATUS SetActiveGroup(const int8 *sGroup);

		const MashStringc& GetActiveGroup()const;
		MashTechniqueInstance* GetActiveTechnique();

		MashTechniqueInstance* AddTechniqueToGroup(const int8 *groupName, const int8 *techniqueName, MashTechnique *refTechnique = 0);
		MashTechniqueInstance* GetTechniqueByName(const int8 *sName);

		eMASH_STATUS CompileTechniques(MashFileManager *pFileManager, MashSceneManager *sceneManager, uint32 compileFlags, 
			const sEffectMacro *args = 0, uint32 argCount = 0);

		MashTechniqueInstance* GetBestTechniqueForLod(uint32 iLod);
		MashTechniqueInstance* GetBestTechniqueForLodByGroup(const int8 *sGroup, uint32 iLod);

		eMASH_STATUS SetLodStartDistances(const uint32 *iDistances, uint32 iLodCount);
		uint32 GetLodFromDistance(uint32 iDistance)const;

		bool GetHasMultipleLodLevels()const;

		eMASH_STATUS UpdateActiveTechnique(uint32 iViewSpaceDepth = 0);

		void SetCustomRenderPath(MashCustomRenderPath *pBatch);
		MashCustomRenderPath* GetCustomRenderPath()const;

		MashVertex* GetVertexDeclaration()const;

		MashTechniqueInstance* GetFirstTechnique()const;

		bool IsValid()const;
		bool IsCompiled()const;

        void _OnReload();
		void _SetVertexDeclaration(MashVertex *pVertexDeclaration);

		const std::map<MashStringc, MashArray<MashTechniqueInstance*> >& GetTechniqueList()const;
		const MashArray<uint32>& GetLodList()const;
	};

	inline bool CMashMaterial::IsCompiled()const
	{
		return m_compiled;
	}

	inline bool CMashMaterial::GetAutoLodEnabled()const
	{
		return m_autoLod;
	}

	inline const MashArray<uint32>& CMashMaterial::GetLodList()const
	{
		return m_lodDistances;
	}

	inline const std::map<MashStringc, MashArray<MashTechniqueInstance*> >& CMashMaterial::GetTechniqueList()const
	{
		return m_techniqueGroups;
	}

	inline MashVertex* CMashMaterial::GetVertexDeclaration()const
	{
		return m_vertexDeclaration;
	}

	inline MashCustomRenderPath* CMashMaterial::GetCustomRenderPath()const
	{
		return m_batch;
	}

	inline bool CMashMaterial::GetHasMultipleLodLevels()const
	{
		return (!m_lodDistances.Empty());
	}

	inline const MashStringc& CMashMaterial::GetActiveGroup()const
	{
		return m_sActiveGroup;
	}

	inline void CMashMaterial::SetAutoLod(bool bValue)
	{
		m_autoLod = bValue;
	}

	inline const MashStringc& CMashMaterial::GetMaterialName()const
	{
		return m_sMaterialName;
	}
}

#endif