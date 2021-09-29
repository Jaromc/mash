//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashStaticMesh.h"
#include "MashVideo.h"
#include "MashVertex.h"
#include "MashMeshBuffer.h"
#include "MashSceneManager.h"
#include "MashMeshBuilder.h"
#include "MashTriangleBuffer.h"
#include "MashLog.h"

namespace mash
{
	CMashStaticMesh::CMashStaticMesh(MashVideo *pRenderer, MashSceneManager *pSceneManager):MashStaticMesh(),
		m_pSceneManager(pSceneManager), m_pRenderer(pRenderer), m_iVertexCount(0), m_iIndexCount(0),
		m_eIndexFormat(aFORMAT_R32_UINT), m_iPrimitiveCount(0), m_ePrimitiveType(aPRIMITIVE_TRIANGLE_LIST),
		m_boundingBox(),
		m_boneVertexIndices(0), m_boneVertexWeights(0), m_meshBuffer(0), m_triangleBuffer(0),
		m_saveInitialiseDataFlags(aSAVE_MESH_TRIANGLE_BUFFFER)
	{

	}

	CMashStaticMesh::~CMashStaticMesh()
	{
		m_saveInitialiseDataFlags = 0;
		DeleteInitialiseData();

		if (m_meshBuffer)
		{
			m_meshBuffer->Drop();
			m_meshBuffer = 0;
		}
	}

	MashMesh* CMashStaticMesh::Clone()const
	{
		CMashStaticMesh *pNewMesh = (CMashStaticMesh*)m_pSceneManager->CreateStaticMesh();

		if (!pNewMesh)
			return 0;

		pNewMesh->m_iVertexCount = m_iVertexCount;
		pNewMesh->m_iIndexCount = m_iIndexCount;
		pNewMesh->m_ePrimitiveType = m_ePrimitiveType;
		pNewMesh->m_iPrimitiveCount = m_iPrimitiveCount;
		pNewMesh->m_boundingBox = m_boundingBox;
		pNewMesh->m_eIndexFormat = m_eIndexFormat;
		
		if (m_meshBuffer)
			pNewMesh->m_meshBuffer = m_meshBuffer->Clone();
		
		return pNewMesh;
	}

	void CMashStaticMesh::DeleteInitialiseData()
	{
		if (!(m_saveInitialiseDataFlags & aSAVE_MESH_RAW_GEOMETRY))
		{
			m_rawVertices.DeleteData();
			m_rawIndices.DeleteData();
		}

		if (!(m_saveInitialiseDataFlags & aSAVE_MESH_BONES))
		{
			if (m_boneVertexWeights)
			{
				MASH_FREE(m_boneVertexWeights);
				m_boneVertexWeights = 0;
			}

			if (m_boneVertexIndices)
			{
				MASH_FREE(m_boneVertexIndices);
				m_boneVertexIndices = 0;
			}
		}

		if (!(m_saveInitialiseDataFlags & aSAVE_MESH_TRIANGLE_BUFFFER))
		{
			if (m_triangleBuffer)
			{
				m_triangleBuffer->Drop();
				m_triangleBuffer = 0;
			}
		}
	}

	MashVertex* CMashStaticMesh::GetVertexDeclaration()const
	{
		if (m_meshBuffer)
			return m_meshBuffer->GetVertexDeclaration();

		return 0;
	}

	void CMashStaticMesh::_SetVertexDeclaration(MashVertex *vertex)
	{
		if (vertex && m_meshBuffer)
			m_meshBuffer->_SetVertexDeclaration(vertex);
	}

	void CMashStaticMesh::SetTriangleBuffer(MashTriangleBuffer *buffer)
	{
		if (m_triangleBuffer)
			m_triangleBuffer->Drop();

		m_triangleBuffer = buffer;
	
		if (m_triangleBuffer)
			m_triangleBuffer->Grab();
	}

	void CMashStaticMesh::ClearBuffers()
	{
		if (m_meshBuffer)
		{
			m_meshBuffer->Drop();
			m_meshBuffer = 0;
		}

		m_rawVertices.Clear();
		m_rawIndices.Clear();

		m_iVertexCount = 0;
		m_iIndexCount = 0;
	}

	void CMashStaticMesh::SetBoneWeights(const mash::MashVector4 *weights, uint32 count)
	{
		if (m_boneVertexWeights)
		{
			MASH_FREE(m_boneVertexWeights);
			m_boneVertexWeights = 0;
		}

		if (count > 0)
		{
			m_boneVertexWeights = MASH_ALLOC_T_COMMON(mash::MashVector4, count);
			memcpy(m_boneVertexWeights, weights, sizeof(mash::MashVector4) * count);
		}
	}

	void CMashStaticMesh::SetBoneIndices(const mash::MashVector4 *indices, uint32 count)
	{
		if (m_boneVertexIndices)
		{
			MASH_FREE(m_boneVertexIndices);
			m_boneVertexIndices = 0;
		}

		if (count > 0)
		{
			m_boneVertexIndices = MASH_ALLOC_T_COMMON(mash::MashVector4, count);
			memcpy(m_boneVertexIndices, indices, sizeof(mash::MashVector4) * count);
		}
	}

	eMASH_STATUS CMashStaticMesh::SetGeometry(const void *pVertexBuffer,
				uint32 vertexCount,
				const MashVertex *pVertexType,
				const void *pIndexBuffer,
				uint32 indexCount,
				eFORMAT eIndexFormat,
				ePRIMITIVE_TYPE ePrimitiveType,
				uint32 primitiveCount,
				bool calculateBoundingBox)
	{
		if (!pVertexBuffer)
		{
			 MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
					"Invalid vertex pointer",
					"CMashStaticMesh::SetGeometry");

			return aMASH_FAILED;
		}

		if (vertexCount <= 0)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
					"Invalid vertex count",
					"CMashStaticMesh::SetGeometry");

			return aMASH_FAILED;
		}

		if (!pVertexType)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
					"Invalid vertex type pointer",
					"CMashStaticMesh::SetGeometry");

			return aMASH_FAILED;
		}

		if (!pIndexBuffer)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
					"Invalid index buffer pointer",
					"CMashStaticMesh::SetGeometry");

			return aMASH_FAILED;
		}

		if (indexCount <= 0)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
					"Invalid index count",
					"CMashStaticMesh::SetGeometry");

			return aMASH_FAILED;
		}

		if (primitiveCount <= 0)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
					"Invalid primitive count",
					"CMashStaticMesh::SetGeometry");

			return aMASH_FAILED;
		}

		ClearBuffers();

		/*
			Buffer needs to be recreated cause its static. Resize is not possible.
		*/
		sVertexStreamInit streamData;
		streamData.data = pVertexBuffer;
		streamData.dataSizeInBytes = vertexCount * pVertexType->GetStreamSizeInBytes(0);
		streamData.usage = aUSAGE_STATIC;

		m_meshBuffer = m_pRenderer->CreateMeshBuffer(&streamData, 1, pVertexType,
			pIndexBuffer, indexCount, eIndexFormat, aUSAGE_STATIC);

		if (!m_meshBuffer)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
					"Failed to create mesh buffer",
					"CMashStaticMesh::SetGeometry");

			return aMASH_FAILED;
		}

		uint32 iIndexElementSize = 4;
		if (eIndexFormat == aFORMAT_R16_UINT)
			iIndexElementSize = 2;

		m_iVertexCount = vertexCount;
		m_iIndexCount = indexCount;
		m_ePrimitiveType = ePrimitiveType;
		m_iPrimitiveCount = primitiveCount;
		m_eIndexFormat = eIndexFormat;

		m_rawVertices.Clear();
		m_rawVertices.Append(pVertexBuffer, vertexCount * pVertexType->GetStreamSizeInBytes(0));

		m_rawIndices.Clear();
		m_rawIndices.Append(pIndexBuffer, indexCount * iIndexElementSize);

		if (calculateBoundingBox)
		{
			if (m_pSceneManager->GetMeshBuilder()->CalculateBoundingBox(pVertexType, m_rawVertices.Pointer(), m_iVertexCount, m_boundingBox) == aMASH_FAILED)
			{
				MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
					"Failed to create bounding box",
					"CMashStaticMesh::SetGeometry");

				return aMASH_FAILED;
			}
		}

		return aMASH_OK;
	}
}