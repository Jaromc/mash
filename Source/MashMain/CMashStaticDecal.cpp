//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashStaticDecal.h"
#include "MashSceneManager.h"
#include "MashVertex.h"
#include "MashVideo.h"
#include "MashMaterial.h"
#include "MashGeometryHelper.h"
#include "MashPlane.h"
#include "MashHelper.h"
#include "MashSkin.h"
#include "MashLog.h"
#include <queue>
namespace mash
{
	CMashStaticDecal::CMashStaticDecal(MashSceneNode *parent,
			MashSceneManager *pSceneManager,
			mash::MashVideo *pRenderer,
			const MashStringc &sName,
			MashMaterial *pMaterial,
			MashSkin *skin):CMashDecalIntermediate(parent, pSceneManager, pRenderer, sName, pMaterial, skin)
	{
	}

	CMashStaticDecal::~CMashStaticDecal()
	{
	}

	mash::MashSceneNode* CMashStaticDecal::_CreateInstance(MashSceneNode *parent, const MashStringc &name)
	{
		CMashStaticDecal *pNewDecal = (CMashStaticDecal*)m_sceneManager->AddDecalCustom(parent, name, m_pMaterial, -1, m_skin);

		if (!pNewDecal)
			return 0;

		pNewDecal->InstanceDecalMembers(this);

		return pNewDecal;
	}

	uint32 CMashStaticDecal::GetDecalCount()const
	{
		if (m_meshBuffer)
			return 1;

		return 0;
	}

	eMASH_STATUS CMashStaticDecal::AppendVertices(const MashTriangleCollider *pTriangleCollection,
				const sTriPickResult &collisionResult,
				const mash::MashVector2 &vTextureDim,
				f32 fRotation,
				const mash::MashMatrix4 *pTransformation)
	{
		//once the mesh buffer is created no more data can be appended.
		if (m_meshBuffer)
			return aMASH_OK;

		uint32 verticesAdded = 0;

		sVertexData vertexData;
		GetVertexData(vertexData);

		if (_AppendVertices(pTriangleCollection, collisionResult, vTextureDim,
			fRotation, pTransformation, vertexData, verticesAdded) == aMASH_FAILED)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Failed to append vertices to decal.", 
					"CMashStaticDecal::AppendVertices");

			return aMASH_FAILED;
		}

		if (verticesAdded == 0)
			return aMASH_OK;

		return aMASH_OK;
	}

	void CMashStaticDecal::Draw()
	{
		if (!m_meshBuffer)
		{
			sVertexStreamInit streamData;
			streamData.data = m_pVertices;
			streamData.dataSizeInBytes = m_iVertexCount * m_pMaterial->GetVertexDeclaration()->GetStreamSizeInBytes(0);
			streamData.usage = aUSAGE_STATIC;

			m_meshBuffer = m_pRenderer->CreateMeshBuffer(&streamData, 1, m_pMaterial->GetVertexDeclaration());
			if (!m_meshBuffer)
			{
                MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
                                 "Failed to create mesh buffer.",
                                 "CMashStaticDecal::Draw");
			}

			MASH_FREE(m_pVertices);
			m_pVertices = 0;
			m_iReservedSizeInBytes = 0;
		}

		return CMashDecalIntermediate::Draw();
	}
}
