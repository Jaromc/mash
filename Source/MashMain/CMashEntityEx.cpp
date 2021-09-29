//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashEntityEx.h"
#include "MashVideo.h"
#include "MashSceneManager.h"
#include "MashGeometryHelper.h"
#include "MashCamera.h"

#include "MashModel.h"
#include "MashSkin.h"

#include "MashLog.h"
#include <assert.h>
namespace mash
{
	CMashEntityEx::CMashEntityEx(MashSceneNode *pParent,
			MashSceneManager *pSceneManager,
			mash::MashVideo *pMashRenderer,
			const MashStringc &sUserName,
			bool bUseOctree ):MashEntity(pParent, pSceneManager, sUserName),m_pMashRenderer(pMashRenderer),
			m_bTransformChanged(true), m_skin(0), m_currentLod(0), m_model(0)
	{
	}

	CMashEntityEx::CMashEntityEx(MashSceneNode *pParent,
			MashSceneManager *pSceneManager,
			mash::MashVideo *pMashRenderer,
			MashModel *pModel,
			const MashStringc &sUserName,
			bool bUseOctree ):MashEntity(pParent, pSceneManager, sUserName), m_model(0),m_pMashRenderer(pMashRenderer),
			m_bTransformChanged(true), m_skin(0), m_currentLod(0)
	{
		if (pModel)
			SetModel(pModel);
	}

	CMashEntityEx::~CMashEntityEx()
	{
		uint32 lodCount = m_subEntityLodList.Size();
		for(uint32 i = 0; i < lodCount; ++i)
		{
			uint32 subEntityCount = m_subEntityLodList[i].Size();
			for(uint32 j = 0; j < subEntityCount; ++j)
			{
				m_subEntityLodList[i][j]->Drop();
			}
		}

		m_subEntityLodList.Clear();

		if (m_skin)
		{
			m_skin->Drop();
			m_skin = 0;
		}

		if (m_model)
		{
			m_model->Drop();
			m_model = 0;
		}
	}

	MashSceneNode* CMashEntityEx::_CreateInstance(MashSceneNode *parent, const MashStringc &name)
	{
		CMashEntityEx *pNewEntity = (CMashEntityEx*)m_sceneManager->AddEntity(parent, m_model, name);

		if (!pNewEntity)
			return 0;

		pNewEntity->InstanceMembers(this);

		uint32 lodCount = m_subEntityLodList.Size();
		for(uint32 i = 0; i < lodCount; ++i)
		{
			uint32 subEntityCount = m_subEntityLodList[i].Size();
			for(uint32 j = 0; j < subEntityCount; ++j)
			{
				pNewEntity->m_subEntityLodList[i][j]->SetMaterial(m_subEntityLodList[i][j]->GetMaterial());
			}
		}

		pNewEntity->m_lodDistances = m_lodDistances;
        
        if (m_skin)
        {
            MashSkin *newSkin = m_sceneManager->_CreateSkinInstance(m_skin);
            pNewEntity->SetSkin(newSkin);
            newSkin->Drop();
        }

		return pNewEntity;
	}

	void CMashEntityEx::SetSkin(MashSkin *skin)
	{
		if (skin)
			skin->Grab();

		if (m_skin)
			m_skin->Drop();

		m_skin = skin;
	}

	void CMashEntityEx::SetModel(mash::MashModel *model)
	{
		if (!model)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Invalid model pointer.", 
					"CMashEntityEx::SetModel");

			return;
		}

		uint32 lodCount = m_subEntityLodList.Size();
		for(uint32 i = 0; i < lodCount; ++i)
		{
			uint32 subEntityCount = m_subEntityLodList[i].Size();
			for(uint32 j = 0; j < subEntityCount; ++j)
			{
				m_subEntityLodList[i][j]->Drop();
			}
		}

		m_subEntityLodList.Clear();

		lodCount = model->GetLodCount();
		m_subEntityLodList.Resize(lodCount);
		for(uint32 i = 0; i < lodCount; ++i)
		{
			uint32 subEntityCount = model->GetMeshCount(i);
			m_subEntityLodList[i].Resize(subEntityCount);
			for(uint32 j = 0; j < subEntityCount; ++j)
			{
				m_subEntityLodList[i][j] = MASH_NEW_COMMON CMashSubEntity(m_pMashRenderer, m_sceneManager, model->GetMesh(j, i), this);
			}
		}

		if (m_lodDistances.Size() != lodCount)
		{
			//reset the lod array because the old and current sizes do not match
			m_lodDistances.Resize(lodCount, 0);
		}

		m_currentLod = 0;

		if (model)
			model->Grab();

		if (m_model)
			m_model->Drop();

		m_model = model;

		/*
			Note we do not do anything regarding the material here. It is assumed the
			material will always be set after a call to this.
		*/
	}

	const mash::MashAABB& CMashEntityEx::GetLocalBoundingBox()const
	{
		return m_model->GetBoundingBox();
	}
    
    void CMashEntityEx::OnNodeTransformChange()
    {
        m_bTransformChanged = true;
    }

	void CMashEntityEx::OnPassCullImpl(f32 interpolateAmount)
	{
		mash::MashVector3 cameraPosition = m_sceneManager->GetActiveCamera()->GetWorldTransformState().translation;

		uint32 distanceFromCamera = (int32)collision::GetDistanceToAABB(cameraPosition, GetWorldBoundingBox());

		//update mesh lod
		uint32 lodCount = m_subEntityLodList.Size();
		if (lodCount > 1)
		{
			m_currentLod = 0;
			for(uint32 i = 0; i < lodCount; ++i)
			{
				/*
					The check for 0 is so that the first (best) lod is always used
					if the distances have not been initialised.
				*/
				if ((distanceFromCamera > m_lodDistances[i]) && (m_lodDistances[i] != 0))
				{
					m_currentLod = i;
					break;
				}
			}
		}

		uint32 subEntityCount = m_subEntityLodList[m_currentLod].Size();
		for(uint32 i = 0; i < subEntityCount; ++i)
		{
			m_subEntityLodList[m_currentLod][i]->LateUpdate(m_bTransformChanged, distanceFromCamera);
		}

		m_bTransformChanged = false;
	}

	void CMashEntityEx::SetMaterialToAllSubEntities(MashMaterial *material)
	{
		uint32 lodCount = m_subEntityLodList.Size();
		for(uint32 i = 0; i < lodCount; ++i)
		{
			uint32 subEntityCount = m_subEntityLodList[i].Size();
			for(uint32 j = 0; j < subEntityCount; ++j)
			{
				m_subEntityLodList[i][j]->SetMaterial(material);
			}
		}
	}

	void CMashEntityEx::SetSubEntityMaterial(MashMaterial *material, uint32 meshIndex, uint32 lod)
	{
		if (lod >= m_subEntityLodList.Size())
			return;

		if (meshIndex >= m_subEntityLodList[lod].Size())
			return;

		m_subEntityLodList[lod][meshIndex]->SetMaterial(material);
	}

	MashSubEntity* CMashEntityEx::GetSubEntity(uint32 meshIndex, uint32 lod)
	{
		if (lod >= m_subEntityLodList.Size())
			return 0;

		if (meshIndex >= m_subEntityLodList[lod].Size())
			return 0;

		return m_subEntityLodList[lod][meshIndex];
	}

	uint32 CMashEntityEx::GetSubEntityCount(uint32 lod)const
	{
		if (lod >= m_subEntityLodList.Size())
			return 0;

		return m_subEntityLodList[lod].Size();
	}

	eMASH_STATUS CMashEntityEx::SetLodDistance(uint32 lod, uint32 distance)
	{
		if (lod == 0)
			return aMASH_OK;

		if (m_lodDistances.Empty())
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_WARNING, 
					"The model given does not have any lods so a lod distance could not be set.", 
					"CMashEntityEx::SetLodDistance");

			return aMASH_OK;
		}

		if (lod >= m_lodDistances.Size())
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Lod index is greater than the number of lods within the given model.", 
					"CMashEntityEx::SetLodDistance");

			return aMASH_FAILED;
		}

		if (m_lodDistances[lod - 1] >= distance)
		{
            MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
                             "Lod distances must be added in ascending order.", 
                             "CMashEntityEx::SetLodDistance");
            
			return aMASH_FAILED;
		}

		m_lodDistances[lod] = distance;

		return aMASH_OK;
	}

	const mash::MashTriangleCollider* CMashEntityEx::GetTriangleCollider()const
	{
		return m_model->GetTriangleCollider();
	}

	bool CMashEntityEx::AddRenderablesToRenderQueue(eRENDER_STAGE stage, MashCullTechnique::CullRenderableFunctPtr functPtr)
	{
		uint32 subEntityCount = m_subEntityLodList[m_currentLod].Size();
		MashSubEntity *subEntity = 0;
        bool rendered = false;
		for(uint32 i = 0; i < subEntityCount; ++i)
		{
			subEntity = m_subEntityLodList[m_currentLod][i];
			if (subEntity->GetIsActive() && !functPtr(subEntity))
			{
				m_sceneManager->AddRenderableToRenderQueue(subEntity, aHLPASS_SCENE, stage);
                rendered = true;
			}
		}

        return rendered;
	}
}