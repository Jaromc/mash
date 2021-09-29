//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashSubEntity.h"
#include "MashSceneManager.h"
#include "MashVideo.h"
#include "MashLog.h"
#include "MashCamera.h"
#include "MashMeshBuilder.h"
#include "MashMaterial.h"
#include "MashSkin.h"
#include "MashVertex.h"
#include "MashMesh.h"
#include "MashEntity.h"
namespace mash
{
	CMashSubEntity::CMashSubEntity(mash::MashVideo *pRenderer, 
		mash::MashSceneManager *pSceneManager,
		mash::MashMesh *mesh,
		MashEntity *pOwner):m_bActive(true),
		m_pOwner(pOwner), m_pMaterial(0),
		m_pRenderer(pRenderer), m_pSceneManager(pSceneManager),
		m_iDistanceFromCamera(0),
		m_mesh(mesh)
	{
		if (m_mesh)
			m_mesh->Grab();
	}

	CMashSubEntity::~CMashSubEntity()
	{
		if (m_mesh)
		{
			m_mesh->Drop();
			m_mesh = 0;
		}

		if (m_pMaterial)
		{
			m_pMaterial->Drop();
			m_pMaterial = 0;
		}
	}

	const mash::MashAABB& CMashSubEntity::GetTotalWorldBoundingBox()const
	{
		return m_pOwner->GetTotalBoundingBox();
	}
    
    const mash::MashAABB& CMashSubEntity::GetWorldBoundingBox()const
	{
		return m_pOwner->GetWorldBoundingBox();
	}

	eMASH_STATUS CMashSubEntity::LateUpdate(bool parentTransformChanged, uint32 iDistanceFromCamera)
	{
		m_iDistanceFromCamera = iDistanceFromCamera;
		return aMASH_OK;
	}

	eMASH_STATUS CMashSubEntity::SetMaterial(MashMaterial *pMaterial)
	{
		if (!pMaterial)
			return aMASH_FAILED;

		const MashVertex *pCurrentVertexDecl = m_mesh->GetVertexDeclaration();

		if (!pCurrentVertexDecl)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
					"Failed to set sub entity material. No vertex decl set to mesh",
					"CMashSubEntity::SetMaterial");

			return aMASH_FAILED;
		}

		MashVertex* pToVertex = pMaterial->GetVertexDeclaration();
		if (!pCurrentVertexDecl || !pCurrentVertexDecl->IsEqual(pToVertex))
		{
			uint32 flags = MashMeshBuilder::aMESH_UPDATE_CHANGE_VERTEX_FORMAT;
			if (m_pSceneManager->GetMeshBuilder()->UpdateMesh(m_mesh, flags, pToVertex) == aMASH_FAILED)
			{
				MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
					"Failed to set sub entity material",
					"CMashSubEntity::SetMaterial");

				return aMASH_FAILED;
			}
		}

		if (pMaterial)
			pMaterial->Grab();

		if (m_pMaterial)
			m_pMaterial->Drop();

		m_pMaterial = pMaterial;

		return aMASH_OK;
	}

    MashMeshBuffer* CMashSubEntity::GetMeshBuffer()const
	{
		return m_mesh->GetMeshBuffer();
	}

	void CMashSubEntity::Draw()
	{
		if (m_pMaterial && m_mesh)
		{			
			/*
				All lodding is done here 
			*/
			if (m_pMaterial->GetHasMultipleLodLevels())
			{
				m_pMaterial->UpdateActiveTechnique(m_iDistanceFromCamera);
			}

			/*
				TODO : Maybe move some of the calcs done here into a cull pass function
			*/
			MashSkin *skin = m_pOwner->GetSkin();
			if (skin)
				skin->OnRender();

			m_pRenderer->GetRenderInfo()->SetWorldTransform(m_pOwner->GetRenderTransformation());

			MashCustomRenderPath *pCustomRenderer = m_pMaterial->GetCustomRenderPath();
			
			if (pCustomRenderer)
			{
				m_pSceneManager->_AddCustomRenderPathToFlushList(pCustomRenderer);
				pCustomRenderer->AddObject(this);
			}
			else
			{
				if (m_pMaterial->OnSet() == aMASH_OK)
				{
					m_pRenderer->DrawMesh(m_mesh, 0);
				}
			}
		}

		return;
	}
}