//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashMaterial.h"
#include "MashTechniqueInstance.h"
#include "MashTexture.h"
#include "MashGeometryBatch.h"
#include "MashVertex.h"
#include "MashSceneManager.h"
#include "MashMaterialManager.h"
#include "MashTechnique.h"
#include "MashTechniqueInstance.h"
#include "MashVideo.h"
#include "MashHelper.h"
#include "MashLog.h"
namespace mash
{
	CMashMaterial::CMashMaterial(mash::MashVideo *pRenderer, const int8 *sName, MashMaterial *reference):MashMaterial(), m_sMaterialName(sName),
		m_pActiveTechnique(0), m_sActiveGroup(""), m_pRenderer(pRenderer), m_autoLod(true), m_batch(0), m_vertexDeclaration(0), m_compiled(false),
		m_lastValidLod(mash::math::MaxUInt32())
	{
	}

	CMashMaterial::~CMashMaterial()
	{
		ClearMaterialData();
	}
    
    void CMashMaterial::ClearMaterialData()
    {
        std::map<MashStringc, MashArray<MashTechniqueInstance*> >::iterator groupStart = m_techniqueGroups.begin();
		std::map<MashStringc, MashArray<MashTechniqueInstance*> >::iterator groupEnd = m_techniqueGroups.end();
		for(; groupStart != groupEnd; ++groupStart)
		{
			const uint32 iTechCount = groupStart->second.Size();
			for(uint32 iTech = 0; iTechCount != iTech; ++iTech)
			{
				groupStart->second[iTech]->Drop();
			}
		}
        
		m_techniqueGroups.clear();
        
		if (m_batch)
		{
			m_batch->Drop();
			m_batch = 0;
		}
        
		if (m_vertexDeclaration)
        {
			m_vertexDeclaration->Drop();
            m_vertexDeclaration = 0;
        }
        
        m_lodDistances.Clear();
        m_autoLod = false;
        m_compiled = false;
        m_sActiveGroup = "";
        m_pActiveTechnique = 0;
        
        //just for safety
        m_pRenderer->GetRenderInfo()->SetTechnique(0);
    }
    
    void CMashMaterial::_OnReload()
    {
        ClearMaterialData();
    }

	MashMaterial* CMashMaterial::CreateIndependentCopy(const int8 *sName, bool copyTextures)
	{
		CMashMaterial *pNewMaterial = (CMashMaterial*)m_pRenderer->GetMaterialManager()->_CreateMaterial(sName, this);
		if (!pNewMaterial)
			return 0;

		pNewMaterial->m_sActiveGroup = m_sActiveGroup;
		pNewMaterial->_SetVertexDeclaration(m_vertexDeclaration);
		pNewMaterial->SetCustomRenderPath(m_batch);
		//pNewMaterial->m_userStrings = m_userStrings;
		pNewMaterial->m_autoLod = m_autoLod;

		if (m_lodDistances.Size() > 0)
			pNewMaterial->SetLodStartDistances(&m_lodDistances[0], m_lodDistances.Size());

		std::map<MashStringc, MashArray<MashTechniqueInstance*> >::iterator groupStart = m_techniqueGroups.begin();
		std::map<MashStringc, MashArray<MashTechniqueInstance*> >::iterator groupEnd = m_techniqueGroups.end();
		for(; groupStart != groupEnd; ++groupStart)
		{
			const uint32 iTechCount = groupStart->second.Size();
			for(uint32 iTech = 0; iTechCount != iTech; ++iTech)
			{
				MashTechniqueInstance *pNewTechnique = (MashTechniqueInstance*)groupStart->second[iTech]->CreateIndependentCopy(groupStart->second[iTech]->GetTechniqueName().GetCString(), copyTextures);
				pNewMaterial->m_techniqueGroups[groupStart->first].PushBack(pNewTechnique);
			}
		}

		return pNewMaterial;
	}

	MashMaterial* CMashMaterial::CreateInstance(const int8 *sName, bool copyTextures)
	{
		CMashMaterial *pNewMaterial = (CMashMaterial*)m_pRenderer->GetMaterialManager()->_CreateMaterial(sName, this);
		if (!pNewMaterial)
			return 0;

		pNewMaterial->m_sActiveGroup = m_sActiveGroup;
		pNewMaterial->_SetVertexDeclaration(m_vertexDeclaration);
		pNewMaterial->SetCustomRenderPath(m_batch);
		pNewMaterial->m_autoLod = m_autoLod;
		pNewMaterial->m_compiled = m_compiled;

		if (m_lodDistances.Size() > 0)
			pNewMaterial->SetLodStartDistances(&m_lodDistances[0], m_lodDistances.Size());

		std::map<MashStringc, MashArray<MashTechniqueInstance*> >::iterator groupStart = m_techniqueGroups.begin();
		std::map<MashStringc, MashArray<MashTechniqueInstance*> >::iterator groupEnd = m_techniqueGroups.end();
		for(; groupStart != groupEnd; ++groupStart)
		{
			const uint32 iTechCount = groupStart->second.Size();
			for(uint32 iTech = 0; iTechCount != iTech; ++iTech)
			{
				MashTechniqueInstance *pNewTechnique = (MashTechniqueInstance*)groupStart->second[iTech]->CreateInstance(groupStart->second[iTech]->GetTechniqueName().GetCString(), copyTextures);
				pNewMaterial->m_techniqueGroups[groupStart->first].PushBack(pNewTechnique);
			}
		}

		return pNewMaterial;
	}

	eMASH_STATUS CMashMaterial::OnSet()
	{
		MashTechniqueInstance *activeTechnique = GetActiveTechnique();
		if (activeTechnique)
		{
			return activeTechnique->_OnSet(/*stage*/);
		}

		return aMASH_FAILED;
	}

	eMASH_STATUS CMashMaterial::SetActiveGroup(const int8 *sGroup)
	{
		MashStringc newGroup(sGroup);
		if (m_sActiveGroup != newGroup)
		{
			std::map<MashStringc, MashArray<MashTechniqueInstance*> >::iterator iter = m_techniqueGroups.find(newGroup);
			if (iter == m_techniqueGroups.end())
			{
				MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_WARNING, 
					"CMashMaterial::SetActiveGroup",
					"Technique group '%s' was not found within material '%s'.", newGroup.GetCString(), m_sMaterialName.GetCString());

				return aMASH_FAILED;
			}

			if (m_pActiveTechnique)
			{
				m_pActiveTechnique->_OnUnload();
				m_pActiveTechnique = 0;
			}

			m_sActiveGroup = newGroup;
			UpdateActiveTechnique();
		}

        return aMASH_OK;
	}

	MashTechniqueInstance* CMashMaterial::GetFirstTechnique()const
	{
		if (m_techniqueGroups.empty())
			return 0;

		return m_techniqueGroups.begin()->second.Front();
	}

	void CMashMaterial::SetTexture(uint32 index, MashTexture *texture)
	{
		std::map<MashStringc, MashArray<MashTechniqueInstance*> >::iterator groupStart = m_techniqueGroups.begin();
		std::map<MashStringc, MashArray<MashTechniqueInstance*> >::iterator groupEnd = m_techniqueGroups.end();
		for(; groupStart != groupEnd; ++groupStart)
		{
			const uint32 iTechCount = groupStart->second.Size();
			for(uint32 iTech = 0; iTechCount != iTech; ++iTech)
			{
				groupStart->second[iTech]->SetTexture(index, texture);
			}
		}
	}

	void CMashMaterial::SetTextureState(uint32 index, MashTextureState *state)
	{
		std::map<MashStringc, MashArray<MashTechniqueInstance*> >::iterator groupStart = m_techniqueGroups.begin();
		std::map<MashStringc, MashArray<MashTechniqueInstance*> >::iterator groupEnd = m_techniqueGroups.end();
		for(; groupStart != groupEnd; ++groupStart)
		{
			const uint32 iTechCount = groupStart->second.Size();
			for(uint32 iTech = 0; iTechCount != iTech; ++iTech)
			{
				groupStart->second[iTech]->SetTextureState(index, state);
			}
		}
	}

	eMASH_STATUS CMashMaterial::CompileTechniques(MashFileManager *pFileManager, MashSceneManager *sceneManager, uint32 compileFlags, const sEffectMacro *args, uint32 argCount)
	{
		/*
			Note, shouldnt need to worry about the same technique being compiled multiple times
			here because in theory, the same techniques shouldnt be preset in a material.
		*/
		std::map<MashStringc, MashArray<MashTechniqueInstance*> >::iterator groupStart = m_techniqueGroups.begin();
		std::map<MashStringc, MashArray<MashTechniqueInstance*> >::iterator groupEnd = m_techniqueGroups.end();
		for(; groupStart != groupEnd; ++groupStart)
		{
			const uint32 iTechCount = groupStart->second.Size();
			for(uint32 iTech = 0; iTechCount != iTech; ++iTech)
			{
				MashTechniqueInstance *tech = groupStart->second[iTech];
				tech->GetTechnique()->CompileTechnique(pFileManager, sceneManager, compileFlags, args, argCount);
			}
		}		

		/*
			Build anything relying on this.
		*/
		MashTechniqueInstance *act = GetActiveTechnique();
		if (act)
			m_pRenderer->_OnDependencyCompiled(this);
		else
		{
			int8 buffer[256];
			mash::helpers::PrintToBuffer(buffer, sizeof(buffer), "Material '%s' does not contain any supported techniques.", GetMaterialName().GetCString());
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, buffer, "CMashMaterial::CompileTechniques");
		}

		m_compiled = true;

		return aMASH_OK;
	}

	MashTechniqueInstance* CMashMaterial::AddTechniqueToGroup(const int8 *groupName, const int8 *techniqueName, MashTechnique *refTechnique)
	{
		MashStringc sNewGroupName = "";
		MashStringc sNewTechniqueName = "";

		if (groupName)
			sNewGroupName = groupName;

		if (techniqueName)
			sNewTechniqueName = techniqueName;

		std::map<MashStringc, MashArray<MashTechniqueInstance*> >::iterator groupStart = m_techniqueGroups.begin();
		std::map<MashStringc, MashArray<MashTechniqueInstance*> >::iterator groupEnd = m_techniqueGroups.end();
		for(; groupStart != groupEnd; ++groupStart)
		{
			const uint32 iTechCount = groupStart->second.Size();
			for(uint32 iTech = 0; iTechCount != iTech; ++iTech)
			{
				if (strcmp(groupStart->second[iTech]->GetTechniqueName().GetCString(), sNewTechniqueName.GetCString()) == 0)
				{
					MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
						"Technique names must be unique within a material.", 
						"CMashMaterial::AddTechniqueToGroup");

					return 0;
				}
			}
		}

		MashTechniqueInstance *pNewTechniqueInstance = m_pRenderer->GetMaterialManager()->_CreateTechniqueInstance(refTechnique);
		pNewTechniqueInstance->_SetTechniqueName(sNewTechniqueName.GetCString());
		
		//fetch the ref technique
		refTechnique = pNewTechniqueInstance->GetTechnique();

		refTechnique->_SetVertexDeclaration(m_vertexDeclaration);

		m_techniqueGroups[sNewGroupName.GetCString()].PushBack(pNewTechniqueInstance);

		return pNewTechniqueInstance;
	}

	void CMashMaterial::_SetVertexDeclaration(MashVertex *pVertexDeclaration)
	{
		if (!pVertexDeclaration)
			return;

		std::map<MashStringc, MashArray<MashTechniqueInstance*> >::iterator groupStart = m_techniqueGroups.begin();
		std::map<MashStringc, MashArray<MashTechniqueInstance*> >::iterator groupEnd = m_techniqueGroups.end();
		for(; groupStart != groupEnd; ++groupStart)
		{
			const uint32 iTechCount = groupStart->second.Size();
			for(uint32 iTech = 0; iTechCount != iTech; ++iTech)
			{
				groupStart->second[iTech]->GetTechnique()->_SetVertexDeclaration(pVertexDeclaration);
			}
		}

		pVertexDeclaration->Grab();

		if (m_vertexDeclaration)
			m_vertexDeclaration->Drop();

		m_vertexDeclaration = pVertexDeclaration;
	}

	MashTechniqueInstance* CMashMaterial::GetTechniqueByName(const int8 *sName)
	{
		std::map<MashStringc, MashArray<MashTechniqueInstance*> >::iterator groupStart = m_techniqueGroups.begin();
		std::map<MashStringc, MashArray<MashTechniqueInstance*> >::iterator groupEnd = m_techniqueGroups.end();
		for(; groupStart != groupEnd; ++groupStart)
		{
			const uint32 iTechCount = groupStart->second.Size();
			for(uint32 iTech = 0; iTechCount != iTech; ++iTech)
			{
				if (strcmp(groupStart->second[iTech]->GetTechniqueName().GetCString(), sName) == 0)
					return groupStart->second[iTech];
			}
		}

		return 0;
	}

	bool CMashMaterial::IsValid()const
	{
		std::map<MashStringc, MashArray<MashTechniqueInstance*> >::const_iterator groupStart = m_techniqueGroups.begin();
		std::map<MashStringc, MashArray<MashTechniqueInstance*> >::const_iterator groupEnd = m_techniqueGroups.end();
		for(; groupStart != groupEnd; ++groupStart)
		{
			const uint32 iTechCount = groupStart->second.Size();
			for(uint32 iTech = 0; iTechCount != iTech; ++iTech)
			{
				MashTechnique *pSceneTechnique = groupStart->second[iTech]->GetTechnique();

				/*
					The material is valid if at least one technqiue is valid
				*/
				if (pSceneTechnique->IsValid())
					return true;
			}
		}

		return false;
	}

	MashTechniqueInstance* CMashMaterial::GetBestTechniqueForLod(uint32 iLod)
	{
		std::map<MashStringc, MashArray<MashTechniqueInstance*> >::iterator groupStart = m_techniqueGroups.begin();
		std::map<MashStringc, MashArray<MashTechniqueInstance*> >::iterator groupEnd = m_techniqueGroups.end();
		for(; groupStart != groupEnd; ++groupStart)
		{
			const uint32 iTechCount = groupStart->second.Size();
			for(uint32 iTech = 0; iTechCount != iTech; ++iTech)
			{
				MashTechnique *pSceneTechnique = groupStart->second[iTech]->GetTechnique();

				if (pSceneTechnique->IsValid() && pSceneTechnique->IsLodLevelSupported(iLod))
					return groupStart->second[iTech];
			}
		}

		return 0;
	}

	MashTechniqueInstance* CMashMaterial::GetBestTechniqueForLodByGroup(const int8 *sGroup, uint32 iLod)
	{
		std::map<MashStringc, MashArray<MashTechniqueInstance*> >::iterator iter = m_techniqueGroups.find(sGroup);

		if (iter != m_techniqueGroups.end())
		{
			const uint32 iTechCount = iter->second.Size();
			for(uint32 iTech = 0; iTechCount != iTech; ++iTech)
			{
				MashTechnique *pSceneTechnique = iter->second[iTech]->GetTechnique();

				//check lod is supported
				if (pSceneTechnique->IsLodLevelSupported(iLod))
				{
					/*
						If the technique has been compiled then make sure its valid.
						If it hasn't been compiled yet then return it anyway.
					*/
					if (!pSceneTechnique->IsCompiled())
						return iter->second[iTech];
					else if (pSceneTechnique->IsValid())
						return iter->second[iTech];
				}
			}
		}

		return 0;
	}

	eMASH_STATUS CMashMaterial::SetLodStartDistances(const uint32 *iDistances, uint32 iLodCount)
	{
		if (iDistances == 0 || iLodCount == 0)
			return aMASH_FAILED;

		//first validate the distances		
		for(uint32 i = 0; i < iLodCount; ++i)
		{
			uint32 iNext = i+1;
			if (iNext < iLodCount)
			{
				if (iDistances[i] >= iDistances[iNext])
				{
					MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
						"Distances must be greater than the previous Lod level.", 
						"CMashMaterial::SetLodStartDistances");

					return aMASH_FAILED;
				}
			}
		}
		
		m_lodDistances.Clear();
		if (iDistances[0] != 0)
			m_lodDistances.PushBack(0);//lod level 0 as default

		for(uint32 i = 0; i < iLodCount; ++i)
			m_lodDistances.PushBack(iDistances[i]);
		

		return aMASH_OK;
	}

	uint32 CMashMaterial::GetLodFromDistance(uint32 iDistance)const
	{
		uint32 iLod = 0;

		/*
			TODO : Impliment a better search algorithm here
		*/
		const uint32 iLodCount = m_lodDistances.Size();
		for(uint32 i = 0; i < iLodCount; ++i)
		{
			if (m_lodDistances[i] < iDistance)
				iLod = i+1;
		}

		return iLod;
	}

	void CMashMaterial::SetCustomRenderPath(MashCustomRenderPath *pBatch)
	{
        if (pBatch)
            pBatch->Grab();
        
        if (m_batch)
            m_batch->Drop();
            
        m_batch = pBatch;
	}

	eMASH_STATUS CMashMaterial::_SetActiveTechniqueByLod(uint32 iLod)
	{
		MashTechniqueInstance *newTechnique = 0;

		if (m_sActiveGroup != "")
		{
			newTechnique = GetBestTechniqueForLodByGroup(m_sActiveGroup.GetCString(), iLod);

			if (!newTechnique)
			{
				MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_WARNING, 
						"CMashMaterial::_SetActiveTechniqueByLod",
						"Failed to find LOD level %i for technique group %s, in material %s. This may affect performance.", 
						iLod, m_sActiveGroup.GetCString(), m_sMaterialName.GetCString());

				newTechnique = GetBestTechniqueForLod(iLod);
			}
		}
		else
		{
			newTechnique = GetBestTechniqueForLod(iLod);
		}

		if (!newTechnique)
		{
			MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_ERROR, 
						"CMashMaterial::_SetActiveTechniqueByLod",
						"Failed to find a technique that supports LOD level %i, in material %s.", 
						iLod, m_sMaterialName.GetCString());

			return aMASH_FAILED;
		}

		if (m_pActiveTechnique && (newTechnique != m_pActiveTechnique))
			m_pActiveTechnique->_OnUnload();
		
		m_pActiveTechnique = newTechnique;
		m_lastValidLod = iLod;

		return aMASH_OK;
	}

	MashTechniqueInstance* CMashMaterial::GetActiveTechnique()
	{
		if (!m_pActiveTechnique)
			UpdateActiveTechnique();

		return m_pActiveTechnique;
	}

	eMASH_STATUS CMashMaterial::UpdateActiveTechnique(uint32 iViewSpaceDepth)
	{
		uint32 iLod = GetLodFromDistance(iViewSpaceDepth);

		if (!m_pActiveTechnique || (m_lastValidLod != iLod))
		{
			//no active technique set, so find the best one for this lod
			if (!m_pActiveTechnique)
			{
				return _SetActiveTechniqueByLod(iLod);
			}
			else if (m_autoLod)
			{
				//technique needs to change due to lod. So find a new technique.
				if (!m_pActiveTechnique->GetTechnique()->IsLodLevelSupported(iLod))
				{
					return _SetActiveTechniqueByLod(iLod);
				}
			}
		}

		return aMASH_OK;
	}
}