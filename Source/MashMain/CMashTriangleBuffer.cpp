//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashTriangleBuffer.h"
#include "CMashIndexingHashTable.h"
#include "MashHelper.h"
#include "MashLog.h"
namespace mash
{
	const mash::MashVector3& CMashTriangleBuffer::GetPoint(uint32 triangle, uint32 point)const
	{
		return m_vertexList[m_indexList[(triangle * 3) + point]];
	}

	uint32 CMashTriangleBuffer::GetIndex(uint32 triangle, uint32 point)const
	{
		return m_indexList[(triangle * 3) + point];
	}

	uint32 CMashTriangleBuffer::GetNormalIndex(uint32 triangle, uint32 point)const
	{
		return m_normalIndexList[(triangle * 3) + point];
	}

	const mash::MashVector3& CMashTriangleBuffer::GetNormal(uint32 triangle, uint32 point)const
	{
		return m_normalList[GetNormalIndex(triangle, point)];
	}

	void CMashTriangleBuffer::Set(uint32 uniquePointCount, const mash::MashVector3 *uniquePoints,
			uint32 indexCount, uint32 *indexList,
			uint32 normalCount, const mash::MashVector3 *normalList,
			uint32 normalIndexCount, uint32 *normalIndexList,
			const sTriangleRecord *triangleRecordList, const sTriangleSkinnngRecord *triangleSkinningRecordList)
	{
		uint32 triangleCount = indexCount / 3;

		m_vertexList.Clear();
		m_vertexList.Assign(uniquePoints, uniquePointCount);

		m_indexList.Clear();
		m_indexList.Assign(indexList, indexCount);

		m_triangles.Clear();
		m_triangles.Assign(triangleRecordList, triangleCount);

		m_normalList.Clear();
		m_normalList.Assign(normalList, normalCount);

		m_normalIndexList.Clear();
		m_normalIndexList.Assign(normalIndexList, normalIndexCount);

		m_triangleSkinningData.Clear();
		if (triangleSkinningRecordList)
			m_triangleSkinningData.Assign(triangleSkinningRecordList, triangleCount);
	}

	uint32 TriangleBufferHashingFunction(const mash::MashVector3 &item)
	{
		//take the floating point into account for numbers less than 1
		return ((uint32)(item.x * 98625731) ^ (uint32)(item.y * 10313717) ^ (uint32)(item.z * 77606537));
	}

	eMASH_STATUS CMashTriangleBuffer::Set(const void *pVertexBuffer,
			uint32 iNumVertices,
			const MashVertex *pVertexType,
			const void *pIndexBuffer,
			uint32 iNumIndices,
			eFORMAT eIndexFormat)
	{
		if (!pVertexBuffer || !iNumVertices)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
					"Input mesh does not contain vertices",
					"CMashTriangleBuffer::Set");

			return aMASH_FAILED;
		}

		if (!pIndexBuffer || !iNumIndices)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
					"Input mesh does not contain indices",
					"CMashTriangleBuffer::Set");

			return aMASH_FAILED;
		}

		if (!pVertexType)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
					"Invalid vertex type",
					"CMashTriangleBuffer::Set");

			return aMASH_FAILED;
		}

		if ((iNumIndices % 3) != 0)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
					"Input mesh does not appear to be made up of triangles",
					"CMashTriangleBuffer::Set");

			return aMASH_FAILED;
		}

		MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_INFORMATION,
					"Creating triangle buffer started",
					"CMashTriangleBuffer::Set");

		MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_WARNING,
					"Creating triangle buffer. Performance will be affected. This should be done offline where possible",
					"CMashTriangleBuffer::Set");

		const uint8 *pVertices = (uint8*)pVertexBuffer;
		const uint8 *pIndices = (uint8*)pIndexBuffer;

		int32 iIndexSizeInBytes = 0;
		if (eIndexFormat == aFORMAT_R16_UINT)
			iIndexSizeInBytes = sizeof(int16);
		else
			iIndexSizeInBytes = sizeof(int32);

		//int32 indexA, indexB, indexC;
		int32 tempIndices[3];

		const int32 iVertexStride = pVertexType->GetStreamSizeInBytes(0);
		//uint32 iPositionStride = 0;
		const sMashVertexElement *positionElement = 0;
		const sMashVertexElement *normalElement = 0;
		const sMashVertexElement *boneIndexElement = 0;
		const sMashVertexElement *boneWeightElement = 0;

		int32 iTriangleCount = iNumIndices / 3;
		m_triangles.Resize(iTriangleCount);
		m_indexList.Resize(iNumIndices);

		positionElement = pVertexType->GetElement(0, aDECLUSAGE_POSITION);
		if (!positionElement)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
					"Couldn't find position element in vertex data",
					"CMashTriangleBuffer::Set");

			return aMASH_FAILED;
		}

        normalElement = pVertexType->GetElement(0, aDECLUSAGE_NORMAL);

		boneIndexElement = pVertexType->GetElement(0, aDECLUSAGE_BLENDINDICES);

		if (boneIndexElement)
		{
			boneWeightElement = pVertexType->GetElement(0, aDECLUSAGE_BLENDWEIGHT);
			m_triangleSkinningData.Resize(iTriangleCount);
		}

		uint32 boneWeightWidth = 0;
		uint32 boneIndexWidth = 0;
		uint32 normalWidth = 0;
		uint32 positionWidth = 0;
		
		if (boneWeightElement)
			boneWeightWidth = math::Clamp<uint32>(0, sizeof(mash::MashVector4), mash::helpers::GetVertexDeclTypeSize(boneWeightElement->type));

		if (boneIndexElement)
			boneIndexWidth = math::Clamp<uint32>(0, sizeof(mash::MashVector4), mash::helpers::GetVertexDeclTypeSize(boneIndexElement->type));

		if (normalElement)
			normalWidth = math::Clamp<uint32>(0, sizeof(mash::MashVector3), mash::helpers::GetVertexDeclTypeSize(normalElement->type));

		if (positionElement)
			positionWidth = math::Clamp<uint32>(0, sizeof(mash::MashVector3), mash::helpers::GetVertexDeclTypeSize(positionElement->type));

		eMASH_STATUS status = aMASH_OK;
		uint32 currentTableIndex = 0;
		//re-used for normals too
		CMashIndexingHashTable<mash::MashVector3> indexingHashTable(iNumVertices, TriangleBufferHashingFunction);
		
		uint32 iIndexCount = 0;
		mash::MashVector3 a, b, c;
		for(uint32 i = 0; i < iTriangleCount; ++i, iIndexCount += 3)
		{
			tempIndices[0] = 0;
			tempIndices[1] = 0;
			tempIndices[2] = 0;

			memcpy(&tempIndices[0], &pIndices[iIndexCount * iIndexSizeInBytes], iIndexSizeInBytes);
			memcpy(&tempIndices[1], &pIndices[(iIndexCount + 1) * iIndexSizeInBytes], iIndexSizeInBytes);
			memcpy(&tempIndices[2], &pIndices[(iIndexCount + 2) * iIndexSizeInBytes], iIndexSizeInBytes);

			MashVector3 pa(0.0f, 0.0f, 0.0f);
			MashVector3 pb(0.0f, 0.0f, 0.0f);
			MashVector3 pc(0.0f, 0.0f, 0.0f);

			memcpy(pa.v, &pVertices[(tempIndices[0] * iVertexStride) + positionElement->stride], positionWidth);
			memcpy(pb.v, &pVertices[(tempIndices[1] * iVertexStride) + positionElement->stride], positionWidth);
			memcpy(pc.v, &pVertices[(tempIndices[2] * iVertexStride) + positionElement->stride], positionWidth);

			uint32 createdIndexValue = 0;

			uint32 foundIndexValue = indexingHashTable.Add(pa, m_vertexList.Pointer(), createdIndexValue);
			if (foundIndexValue == mash::math::MaxUInt32())
			{
				m_vertexList.PushBack(pa);
			}
			m_indexList[iIndexCount] = createdIndexValue;

			createdIndexValue = 0;
			foundIndexValue = indexingHashTable.Add(pb, m_vertexList.Pointer(), createdIndexValue);
			if (foundIndexValue == mash::math::MaxUInt32())
			{
				m_vertexList.PushBack(pb);
			}
			m_indexList[iIndexCount+1] = createdIndexValue;

			createdIndexValue = 0;
			foundIndexValue = indexingHashTable.Add(pc, m_vertexList.Pointer(), createdIndexValue);
			if (foundIndexValue == mash::math::MaxUInt32())
			{
				m_vertexList.PushBack(pc);
			}
			m_indexList[iIndexCount+2] = createdIndexValue;

			if (boneWeightElement)
			{
				/*
					Zero out all elements in case we copy less than float4. Shouldnt really ever happen though.
				*/
				m_triangleSkinningData[i].boneWeights[0].Zero();
				m_triangleSkinningData[i].boneWeights[1].Zero();
				m_triangleSkinningData[i].boneWeights[2].Zero();

				memcpy(m_triangleSkinningData[i].boneWeights[0].v, &pVertices[(tempIndices[0] * iVertexStride) + boneWeightElement->stride], boneWeightWidth);
				memcpy(m_triangleSkinningData[i].boneWeights[1].v, &pVertices[(tempIndices[1] * iVertexStride) + boneWeightElement->stride], boneWeightWidth);
				memcpy(m_triangleSkinningData[i].boneWeights[2].v, &pVertices[(tempIndices[2] * iVertexStride) + boneWeightElement->stride], boneWeightWidth);

				m_triangleSkinningData[i].boneIndices[0].Zero();
				m_triangleSkinningData[i].boneIndices[1].Zero();
				m_triangleSkinningData[i].boneIndices[2].Zero();

				memcpy(m_triangleSkinningData[i].boneIndices[0].v, &pVertices[(tempIndices[0] * iVertexStride) + boneIndexElement->stride], boneIndexWidth);
				memcpy(m_triangleSkinningData[i].boneIndices[1].v, &pVertices[(tempIndices[1] * iVertexStride) + boneIndexElement->stride], boneIndexWidth);
				memcpy(m_triangleSkinningData[i].boneIndices[2].v, &pVertices[(tempIndices[2] * iVertexStride) + boneIndexElement->stride], boneIndexWidth);
			}
		}

		m_normalIndexList.Resize(m_indexList.Size());
		indexingHashTable.Clear();

		//create normals
		if (normalElement && (normalElement->stride > 0))
		{
			int32 indice = 0;
			int32 oldIndice = 0;

			//grab normals from the mesh
			for(uint32 tri = 0; tri < iTriangleCount; ++tri)
			{
				for(uint32 p = 0; p < 3; ++p, ++indice)
				{
					oldIndice = 0;
					memcpy(&oldIndice, &pIndices[indice * iIndexSizeInBytes], iIndexSizeInBytes);

					MashVector3 normal(0.0f, 0.0f, 0.0f);
					memcpy(normal.v, &pVertices[(oldIndice * iVertexStride) + normalElement->stride], normalWidth);

					uint32 createdIndexValue = 0;
					uint32 foundIndexValue = indexingHashTable.Add(normal, m_normalList.Pointer(), createdIndexValue);
					if (foundIndexValue == mash::math::MaxUInt32())
						m_normalList.PushBack(normal);

					m_normalIndexList[indice] = createdIndexValue;
				}
			}
		}
		else //generate flat normals
		{
			for(uint32 tri = 0; tri < iTriangleCount; ++tri)
			{
				MashVector3 normal = math::ComputeFaceNormal(GetPoint(tri, 0), GetPoint(tri, 1), GetPoint(tri, 2));

				uint32 createdIndexValue = 0;
				uint32 foundIndexValue = indexingHashTable.Add(normal, m_normalList.Pointer(), createdIndexValue);
				if (foundIndexValue == mash::math::MaxUInt32())
					m_normalList.PushBack(normal);

				//all three points share the same value
				m_normalIndexList[tri * 3] = createdIndexValue;
				m_normalIndexList[(tri * 3) + 1] = createdIndexValue;
				m_normalIndexList[(tri * 3) + 2] = createdIndexValue;
			}
		}

		GenerateAdjacencyList();

		MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_INFORMATION,
					"Creating triangle buffer succeeded",
					"CMashTriangleBuffer::Set");

		return status;
	}

	void CMashTriangleBuffer::GenerateAdjacencyList()
	{
		const uint32 triangleCount = m_indexList.Size() / 3;

		sTriangleRecord *tr1 = 0;
		sTriangleRecord *tr2 = 0;

		uint32 edgeList1[6];
		uint32 edgeList2[6];
		for(uint32 t1 = 0; t1 < triangleCount; ++t1)
		{
			tr1 = &m_triangles[t1];

			uint32 t1p0 = m_indexList[(t1 * 3)];
			uint32 t1p1 = m_indexList[(t1 * 3) + 1];
			uint32 t1p2 = m_indexList[(t1 * 3) + 2];

			edgeList1[0] = t1p0;
			edgeList1[1] = t1p1;
			edgeList1[2] = t1p1;
			edgeList1[3] = t1p2;
			edgeList1[4] = t1p2;
			edgeList1[5] = t1p0;
			//uint32 edgesFound = 0;
			for(uint32 e1 = 0; e1 < 3; ++e1)
			{
				//only search if this edge has not been found
				if (tr1->adjacencyEdgeList[e1] == mash::math::MaxUInt32())
				{
					/*
						we start the search from t1 because any previous triangles
						have had all their edges discovered.
					*/
					for(uint32 t2 = t1; t2 < triangleCount; ++t2)
					{
						//if we are not testing the same triangle
						if (t1 != t2)
						{
							tr2 = &m_triangles[t2];

							uint32 t2p0 = m_indexList[(t2 * 3)];
							uint32 t2p1 = m_indexList[(t2 * 3) + 1];
							uint32 t2p2 = m_indexList[(t2 * 3) + 2];

							/*
								Edges of an adjacent triangle will be in backwards order relative
								to the current triangle
							*/
							edgeList2[0] = t2p1;
							edgeList2[1] = t2p0;
							edgeList2[2] = t2p2;
							edgeList2[3] = t2p1;
							edgeList2[4] = t2p0;
							edgeList2[5] = t2p2;

							for(uint32 e2 = 0; e2 < 3; ++e2)
							{
								if ((edgeList1[e1 * 2] == edgeList2[e2 * 2]) && 
									(edgeList1[(e1 * 2) + 1] == edgeList2[(e2 * 2) + 1]))
								{
									tr1->adjacencyEdgeList[e1] = t2;
									tr2->adjacencyEdgeList[e2] = t1;
									//++edgesFound;
									t2 = triangleCount;
									break;
								}
							}
						}
					}
				}
			}
		}
	}


}