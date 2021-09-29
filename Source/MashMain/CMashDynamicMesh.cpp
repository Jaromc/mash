//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashDynamicMesh.h"
#include "MashVideo.h"
#include "MashVertex.h"
#include "MashMeshBuffer.h"
#include "MashSceneManager.h"
#include "MashMeshBuilder.h"
#include "MashTriangleBuffer.h"
#include "MashIndexBuffer.h"
#include "MashVertexBuffer.h"
#include "MashLog.h"

namespace mash
{
	CMashDynamicMesh::CMashDynamicMesh(MashVideo *pRenderer, MashSceneManager *pSceneManager):MashDynamicMesh(),
		m_pSceneManager(pSceneManager), m_pRenderer(pRenderer),
		m_eIndexFormat(aFORMAT_R32_UINT), m_iPrimitiveCount(0), m_ePrimitiveType(aPRIMITIVE_TRIANGLE_LIST),
		/*m_pSkin(0), */m_boundingBox(), 
		m_bIsLocked(false), m_iTotalIndexBufferSizeInBytes(0), m_iTotalVertexBufferSizeInBytes(0), 
		/*m_pVertexDeclaration(0), */m_rawVertices(), m_rawIndices(), m_bIsInitialised(false), m_iIndexFormatSize(4),
		m_boneVertexIndices(0), m_boneVertexWeights(0), m_meshBuffer(0), m_iVertexCount(0), m_iIndexCount(0), m_triangleBuffer(0),
		m_saveInitialiseDataFlags(aSAVE_MESH_DATA_ALL)
	{

	}

	CMashDynamicMesh::~CMashDynamicMesh()
	{
		m_saveInitialiseDataFlags = 0;
		DeleteInitialiseData();

		if (m_meshBuffer)
		{
			m_meshBuffer->Drop();
			m_meshBuffer = 0;
		}
	}

	MashMesh* CMashDynamicMesh::Clone()const
	{
		CMashDynamicMesh *pNewMesh = (CMashDynamicMesh*)m_pSceneManager->CreateDynamicMesh();

		if (!pNewMesh)
			return 0;

		pNewMesh->m_ePrimitiveType = m_ePrimitiveType;
		pNewMesh->m_iPrimitiveCount = m_iPrimitiveCount;
		pNewMesh->m_boundingBox = m_boundingBox;
		pNewMesh->m_eIndexFormat = m_eIndexFormat;
		pNewMesh->m_rawVertices = m_rawVertices;
		pNewMesh->m_rawIndices = m_rawIndices;
		pNewMesh->m_iIndexFormatSize = m_iIndexFormatSize;
		pNewMesh->m_bIsInitialised = m_bIsInitialised;

		if (m_meshBuffer)
			pNewMesh->m_meshBuffer = m_meshBuffer->Clone();

		return pNewMesh;
	}

	void CMashDynamicMesh::DeleteInitialiseData()
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

	void CMashDynamicMesh::SetTriangleBuffer(MashTriangleBuffer *buffer)
	{
		if (m_triangleBuffer)
			m_triangleBuffer->Drop();

		m_triangleBuffer = buffer;
	
		if (m_triangleBuffer)
			m_triangleBuffer->Grab();
	}

	void CMashDynamicMesh::SetBoneWeights(const mash::MashVector4 *weights, uint32 count)
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

	void CMashDynamicMesh::SetBoneIndices(const mash::MashVector4 *indices, uint32 count)
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

	eMASH_STATUS CMashDynamicMesh::LockBuffersWrite(eBUFFER_LOCK lockType, void **pVertexBufferOut, void **pIndexBufferOut)
	{
		MashVertexBuffer *vertexBuffer = m_meshBuffer->GetVertexBuffer();
		MashIndexBuffer *indexBuffer = m_meshBuffer->GetIndexBuffer();

		if (vertexBuffer)
		{
			if (vertexBuffer->Lock(lockType, pVertexBufferOut) == aMASH_FAILED)
				return aMASH_FAILED;
		}

		if (indexBuffer)
		{
			if (indexBuffer->Lock(lockType, pIndexBufferOut) == aMASH_FAILED)
				return aMASH_FAILED;
		}

		m_bIsLocked = true;

		return aMASH_OK;
	}

	eMASH_STATUS CMashDynamicMesh::UnLockBuffers()
	{
		/*
			Raw vertex and index buffers are used for buffer reading,
			the API buffers will not lock in that case, so
			we need to test for that here.
		*/
		if (m_bIsLocked)
		{
			MashVertexBuffer *vertexBuffer = m_meshBuffer->GetVertexBuffer();
			MashIndexBuffer *indexBuffer = m_meshBuffer->GetIndexBuffer();

			if (vertexBuffer)
			{
				if (vertexBuffer->Unlock() == aMASH_FAILED)
					return aMASH_FAILED;
			}

			if (indexBuffer)
			{
				if (indexBuffer->Unlock() == aMASH_FAILED)
					return aMASH_FAILED;
			}

			m_bIsLocked = false;
		}

		return aMASH_OK;
	}

	void CMashDynamicMesh::_SetVertexDeclaration(MashVertex *vertex)
	{
		if (vertex && m_meshBuffer)
			m_meshBuffer->_SetVertexDeclaration(vertex);
	}

	MashVertex* CMashDynamicMesh::GetVertexDeclaration()const
	{
		if (m_meshBuffer)
			return m_meshBuffer->GetVertexDeclaration();

		return 0;
	}

	void CMashDynamicMesh::ClearGeometry()
	{
		m_rawVertices.Clear();
		m_rawIndices.Clear();

		m_bIsInitialised = false;
	}

	eMASH_STATUS CMashDynamicMesh::SetGeometry(const void *pVertexBuffer,
				uint32 vertexCount,
				const MashVertex *pVertexType,
				const void *pIndexBuffer,
				uint32 indexCount,
				eFORMAT eIndexFormat,
				ePRIMITIVE_TYPE ePrimitiveType,
				uint32 primitiveCount,
				bool calculateBoundingBox)
	{
		ClearGeometry();

		return AppendGeometry(pVertexBuffer,
			vertexCount,
			pVertexType,
			pIndexBuffer,
			eIndexFormat,
			indexCount,
			ePrimitiveType,
			primitiveCount,
			calculateBoundingBox);
	}

	eMASH_STATUS CMashDynamicMesh::AppendGeometry(const void *pVertexBuffer,
				uint32 vertexCount,
				const MashVertex *pVertexType,
				const void *pIndexBuffer,
				eFORMAT eIndexFormat,
				uint32 indexCount,
				ePRIMITIVE_TYPE ePrimitiveType,
				uint32 primitiveCount,
				bool calculateBoundingBox)
	{
		if (!pVertexBuffer)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
				"Invalid vertex pointer.", 
				"CMashDynamicMesh::AppendGeometry");

			return aMASH_FAILED;
		}

		if (vertexCount <= 0)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
				"Invalid vertex count.", 
				"CMashDynamicMesh::AppendGeometry");

			return aMASH_FAILED;
		}

		if (!pVertexType)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
				"Invalid vertex type pointer.", 
				"CMashDynamicMesh::AppendGeometry");

			return aMASH_FAILED;
		}

		if (primitiveCount <= 0)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
				"Invalid primitive count.", 
				"CMashDynamicMesh::AppendGeometry");

			return aMASH_FAILED;
		}

		if (m_bIsInitialised && (pVertexType != m_meshBuffer->GetVertexDeclaration()))
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
				"Vertex types must be the same for each append.", 
				"CMashDynamicMesh::AppendGeometry");

			return aMASH_FAILED;
		}

		if (m_bIsInitialised && (ePrimitiveType != m_ePrimitiveType))
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
				"Primitive types must be the same for each append.", 
				"CMashDynamicMesh::AppendGeometry");

			return aMASH_FAILED;
		}

		uint16 iNewIndexFormat = 2;
		switch(eIndexFormat)
		{
		case aFORMAT_R16_UINT:
			iNewIndexFormat = 2;
			break;
		case aFORMAT_R32_UINT:
			iNewIndexFormat = 4;
			break;
		default:
			{
				MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Invalid index format.", 
					"CMashDynamicMesh::AppendGeometry");

				return aMASH_FAILED;	
			}
		};

		if (m_bIsInitialised && (indexCount > 0) && (m_iIndexFormatSize != iNewIndexFormat))
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Index format must be the same for each append.", 
					"CMashDynamicMesh::AppendGeometry");

			return aMASH_FAILED;
		}

		m_iIndexFormatSize = iNewIndexFormat;

		if (!m_bIsInitialised)
		{
			m_rawVertices.Clear();
			m_rawIndices.Clear();
			m_iVertexCount = 0;
			m_iIndexCount = 0;
			m_iPrimitiveCount = 0;
		}

		//append new geometry to the API buffers
		const uint32 iRequiredVertexBufferSize = vertexCount * pVertexType->GetStreamSizeInBytes(0);
		const uint32 iRequiredIndexBufferSize = indexCount * m_iIndexFormatSize;
		if (!m_meshBuffer)
		{
			m_iTotalVertexBufferSizeInBytes = math::Max<uint32>(m_iTotalVertexBufferSizeInBytes, iRequiredVertexBufferSize);
			m_iTotalIndexBufferSizeInBytes = math::Max<uint32>(m_iTotalIndexBufferSizeInBytes, iRequiredIndexBufferSize);

			sVertexStreamInit streamData;
			streamData.data = m_rawVertices.Pointer();
			streamData.dataSizeInBytes = vertexCount * pVertexType->GetStreamSizeInBytes(0);
			streamData.usage = aUSAGE_DYNAMIC;

			m_meshBuffer = m_pRenderer->CreateMeshBuffer(&streamData, 1, pVertexType, 
				m_rawIndices.Pointer(), indexCount, eIndexFormat, aUSAGE_DYNAMIC);

			if (!m_meshBuffer)
			{
				MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Failed to create mesh buffer.", 
					"CMashDynamicMesh::AppendGeometry");

				return aMASH_FAILED;
			}

			if (m_iTotalVertexBufferSizeInBytes > iRequiredVertexBufferSize)
			{
				if (m_meshBuffer->ResizeVertexBuffers(0, m_iTotalVertexBufferSizeInBytes, aUSAGE_DYNAMIC, true) == aMASH_FAILED)
				{
					MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
						"Failed to resize decal vertex buffer.", 
						"CMashDynamicMesh::AppendGeometry");

					return aMASH_FAILED;
				}
			}

			if (m_iTotalIndexBufferSizeInBytes > iRequiredIndexBufferSize)
			{
				if (m_meshBuffer->ResizeIndexBuffer(m_iTotalIndexBufferSizeInBytes, true) == aMASH_FAILED)
				{
					MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
						"Failed to resize decal index buffer.", 
						"CMashDynamicMesh::AppendGeometry");

					return aMASH_FAILED;
				}
			}
		}
		else
		{
			//expand API vertex buffer if required
			bool resizeVertexBuffer = iRequiredVertexBufferSize > m_iTotalVertexBufferSizeInBytes;
			//expand API index buffer if required
			bool resizeIndexBuffer = iRequiredIndexBufferSize > m_iTotalIndexBufferSizeInBytes;

			if (resizeVertexBuffer)
				m_iTotalVertexBufferSizeInBytes = iRequiredVertexBufferSize * 2;

			if (resizeIndexBuffer)
				m_iTotalIndexBufferSizeInBytes = iRequiredIndexBufferSize * 2;

			if (resizeVertexBuffer)
			{
				if (m_meshBuffer->ResizeVertexBuffers(0, m_iTotalVertexBufferSizeInBytes, aUSAGE_DYNAMIC, true) == aMASH_FAILED)
				{
					MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
						"Failed to resize decal vertex buffer.", 
						"CMashDynamicMesh::AppendGeometry");

					return aMASH_FAILED;
				}
			}

			if (resizeIndexBuffer)
			{
				if (m_meshBuffer->ResizeIndexBuffer(m_iTotalIndexBufferSizeInBytes, true) == aMASH_FAILED)
				{
					MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
						"Failed to resize decal index buffer.", 
						"CMashDynamicMesh::AppendGeometry");

					return aMASH_FAILED;
				}
			}
		}

		/*
			If the buffer has already been initialised then we do aLOCK_WRITE_NOOVERWRITE
			as we are only appending new geometry.
			Otherwise we flush the buffers with aLOCK_WRITE_DISCARD, ready for new data.
		*/
		void *pLockedVertexData = 0;
		void *pLockedIndexData = 0;
		eBUFFER_LOCK eLockFlag = aLOCK_WRITE_NOOVERWRITE;
		if (!m_bIsInitialised)
			eLockFlag = aLOCK_WRITE_DISCARD;

		LockBuffersWrite(eLockFlag, &pLockedVertexData, &pLockedIndexData);

		memcpy(pLockedVertexData, pVertexBuffer, vertexCount * pVertexType->GetStreamSizeInBytes(0));

		if (pLockedIndexData && (indexCount > 0))
			memcpy(pLockedIndexData, pIndexBuffer, (uint32)indexCount * m_iIndexFormatSize);

		UnLockBuffers();

		//append the geometry to the raw buffers
		m_rawVertices.Append(pVertexBuffer, vertexCount * pVertexType->GetStreamSizeInBytes(0));

		if (pLockedIndexData && (indexCount > 0))
			m_rawIndices.Append(pIndexBuffer, indexCount * m_iIndexFormatSize);

		m_iVertexCount += vertexCount;
		m_iIndexCount += indexCount;
		m_ePrimitiveType = ePrimitiveType;
		m_iPrimitiveCount += primitiveCount;
		m_eIndexFormat = eIndexFormat;

		if (calculateBoundingBox)
		{
			if (m_pSceneManager->GetMeshBuilder()->CalculateBoundingBox(pVertexType, m_rawVertices.Pointer(), m_iVertexCount, m_boundingBox) == aMASH_FAILED)
			{
				MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Failed to create bounding volume.", 
					"CMashDynamicMesh::AppendGeometry");

				return aMASH_FAILED;
			}
		}

		m_bIsInitialised = true;
        
        return aMASH_OK;
	}

	eMASH_STATUS CMashDynamicMesh::SetReservedBufferSize(uint32 iVertexBufferSizeInBytes, uint32 iIndexBufferSizeInBytes)
	{
		if (m_bIsLocked)
			return aMASH_FAILED;

		if (iVertexBufferSizeInBytes == m_iTotalVertexBufferSizeInBytes)
			return aMASH_OK;

		if (m_meshBuffer)
		{
			m_iTotalVertexBufferSizeInBytes = math::Max<uint32>(iVertexBufferSizeInBytes, m_meshBuffer->GetVertexDeclaration()->GetStreamSizeInBytes(0));
			m_iTotalIndexBufferSizeInBytes = math::Max<uint32>(iIndexBufferSizeInBytes, m_iIndexFormatSize);

			uint32 iReservedElementCount = m_iTotalVertexBufferSizeInBytes / math::Max<uint32>(1, m_meshBuffer->GetVertexDeclaration()->GetStreamSizeInBytes(0));
			m_rawVertices.Reserve(iReservedElementCount);

			iReservedElementCount = m_iTotalIndexBufferSizeInBytes / math::Max<uint32>(1, m_iIndexFormatSize);
			m_rawIndices.Reserve(iReservedElementCount);

			if (m_meshBuffer->ResizeVertexBuffers(0, m_iTotalVertexBufferSizeInBytes, aUSAGE_DYNAMIC, true) == aMASH_FAILED)
				return aMASH_FAILED;

			if (m_meshBuffer->ResizeIndexBuffer(m_iTotalIndexBufferSizeInBytes, true) == aMASH_FAILED)
				return aMASH_FAILED;
		}
		else
		{
			m_iTotalVertexBufferSizeInBytes = iVertexBufferSizeInBytes;
			m_iTotalIndexBufferSizeInBytes = iIndexBufferSizeInBytes;
		}

		return aMASH_OK;	
	}
}