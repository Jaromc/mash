//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashDynamicDecal.h"
#include "MashSceneManager.h"
#include "MashVideo.h"
#include "MashGeometryHelper.h"
#include "MashPlane.h"
#include "MashHelper.h"
#include "MashSkin.h"
#include "MashMaterial.h"
#include "MashMaterialManager.h"
#include "MashVertex.h"
#include "MashVertexBuffer.h"
#include "MashLog.h"
#include <queue>
namespace mash
{
	CMashDynamicDecal::CMashDynamicDecal(MashSceneNode *parent,
			MashSceneManager *pSceneManager,
			mash::MashVideo *pRenderer,
			const MashStringc &sName,
			MashMaterial *pMaterial,
			MashSkin *skin,
			uint32 decalLimit):CMashDecalIntermediate(parent, pSceneManager, pRenderer, sName, pMaterial, skin), 
			m_decalLimitSet(false), m_decalLimitReached(false), 
			m_newestDecalIndex(0), m_decalLimit(0)
	{
		if (decalLimit > 0)
		{
			m_decalLimitSet = true;
			m_decalLimit = decalLimit;
			m_decalVertexCountList.Resize(m_decalLimit, 0);
		}

		GetVertexData(m_vertexData);
	}

	CMashDynamicDecal::~CMashDynamicDecal()
	{
	}

	mash::MashSceneNode* CMashDynamicDecal::_CreateInstance(MashSceneNode *parent, const MashStringc &name)
	{
		CMashDynamicDecal *pNewDecal = (CMashDynamicDecal*)m_sceneManager->AddDecalCustom(parent, name, m_pMaterial, m_decalLimit, m_skin);

		if (!pNewDecal)
			return 0;

		pNewDecal->InstanceDecalMembers(this);

		pNewDecal->m_decalLimitSet = m_decalLimitSet;
		pNewDecal->m_decalLimitReached = m_decalLimitReached;
		pNewDecal->m_newestDecalIndex = m_newestDecalIndex;
		pNewDecal->m_decalLimit = m_decalLimit;
		pNewDecal->m_decalVertexCountList = m_decalVertexCountList;
		pNewDecal->m_vertexData = m_vertexData;

		return pNewDecal;
	}

	void CMashDynamicDecal::OnAddNewDecal()
	{
		++m_decalCount;

		/*
			If a decal limit is set, this removes the last decal from the list to free
			up space at the end of the list.

			It was decided to simply copy the memory down in the vertex array instead of
			storing each decals vertices in a list/array. This was to help reduce memory
			fragmentation and possibly reduce the overall amount of memory used.
			In most cases this will probably prove to be faster anyway rather than looping
			through a list to copy all the vertices into the render buffer whenever a
			change occurs.
		*/
		if (m_decalLimitSet)
		{
			if (m_newestDecalIndex == m_decalLimit)
				m_decalLimitReached = true;

			/*
				Once we reach the decal limit we need to move some memory around
				to fit each new decal.
			*/
			if (m_decalLimitReached)
			{
				uint32 oldestDecalIndex = m_newestDecalIndex;
				if (oldestDecalIndex >= m_decalLimit)
					oldestDecalIndex = 0;

				uint32 decalSizeInBytes = m_decalVertexCountList[oldestDecalIndex] * m_pMaterial->GetVertexDeclaration()->GetStreamSizeInBytes(0);
				//move the array down by the number of vertices in the oldest decal
				memcpy(m_pVertices, &((uint8*)m_pVertices)[decalSizeInBytes], m_iReservedSizeInBytes - decalSizeInBytes);
				m_iVertexCount -= m_decalVertexCountList[oldestDecalIndex];

				//loop the current decal number around the array if needed.
				if (m_newestDecalIndex >= m_decalLimit)
					m_newestDecalIndex = 0;

				m_decalCount = m_decalLimit;
			}
		}
	}

	eMASH_STATUS CMashDynamicDecal::AppendVertices(const MashTriangleCollider *pTriangleCollection,
				const sTriPickResult &collisionResult,
				const mash::MashVector2 &vTextureDim,
				f32 fRotation,
				const mash::MashMatrix4 *pTransformation)
	{
		uint32 verticesAdded = 0;

		if (_AppendVertices(pTriangleCollection, collisionResult, vTextureDim,
			fRotation, pTransformation, m_vertexData, verticesAdded) == aMASH_FAILED)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Failed to append vertices to decal.", 
					"CMashDynamicDecal::AppendVertices");

			return aMASH_FAILED;
		}

		if (verticesAdded == 0)
			return aMASH_OK;
		
		if (!m_meshBuffer)
		{
			sVertexStreamInit streamData;
			streamData.data = 0;
			streamData.dataSizeInBytes = m_iReservedSizeInBytes;
			streamData.usage = aUSAGE_DYNAMIC;

			m_meshBuffer = m_pRenderer->CreateMeshBuffer(&streamData, 1, m_pMaterial->GetVertexDeclaration());
			if (!m_meshBuffer)
			{
				MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Failed to create mesh buffer for decals.", 
					"CMashDynamicDecal::AppendVertices");

				return aMASH_FAILED;
			}
		}
		else
		{
			if (m_meshBuffer->GetVertexBuffer(0)->GetBufferSize() < m_iReservedSizeInBytes)
			{
				if (m_meshBuffer->ResizeVertexBuffers(0, m_iReservedSizeInBytes) == aMASH_FAILED)
				{
					MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
						"Failed to resize vertex buffer for decals.", 
						"CMashDynamicDecal::AppendVertices");

					return aMASH_FAILED;
				}
			}
		}

		uint8 *vertexPtr = 0;
		if (m_meshBuffer->GetVertexBuffer()->Lock(mash::aLOCK_WRITE_DISCARD, (void**)(&vertexPtr)) == aMASH_FAILED)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Failed to lock decal vertex buffer.", 
					"CMashDynamicDecal::AppendVertices");

			return aMASH_FAILED;
		}

		memcpy(vertexPtr, m_pVertices, m_iReservedSizeInBytes);

		if (m_meshBuffer->GetVertexBuffer()->Unlock() == aMASH_FAILED)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Failed to unlock decal vertex buffer.", 
					"CMashDynamicDecal::AppendVertices");

			return aMASH_FAILED;
		}

		if (m_decalLimitSet)
			m_decalVertexCountList[m_newestDecalIndex++] = verticesAdded;

		return aMASH_OK;
	}
}
