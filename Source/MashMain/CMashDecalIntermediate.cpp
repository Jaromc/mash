//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashDecalIntermediate.h"
#include "MashSceneManager.h"
#include "MashVideo.h"
#include "MashGeometryHelper.h"
#include "MashPlane.h"
#include "MashCamera.h"
#include "MashHelper.h"
#include "MashSkin.h"
#include "MashMaterial.h"
#include "MashTriangleCollider.h"
#include "MashTriangleBuffer.h"
#include "MashTriangle.h"
#include "MashVertex.h"
#include <queue>
namespace mash
{
	CMashDecalIntermediate::CMashDecalIntermediate(MashSceneNode *parent,
			MashSceneManager *pSceneManager,
			mash::MashVideo *pRenderer,
			const MashStringc &sName,
			MashMaterial *pMaterial,
			MashSkin *skin):MashDecal(parent, pSceneManager, sName), m_iVertexCount(0),
			m_ePrimitiveType(aPRIMITIVE_TRIANGLE_LIST), m_iPrimitiveCount(0), m_pRenderer(pRenderer),
			m_pMaterial(pMaterial), m_bIsVisible(true),
			m_iReservedSizeInBytes(0), m_skin(skin), m_pVertices(0), m_meshBuffer(0), m_mergeTolerance(math::DegsToRads(45))
	{
		if (m_skin)
			m_skin->Grab();	

		if (m_pMaterial)
			m_pMaterial->Grab();
        
        m_aabb.min = mash::MashVector3(mash::math::MinFloat(), mash::math::MinFloat(), mash::math::MinFloat());
		m_aabb.max = mash::MashVector3(mash::math::MaxFloat(), mash::math::MaxFloat(), mash::math::MaxFloat());
	}

	CMashDecalIntermediate::~CMashDecalIntermediate()
	{
		if (m_pVertices)
		{
			MASH_FREE(m_pVertices);
			m_pVertices = 0;
			m_iVertexCount = 0;
		}

		if (m_meshBuffer)
		{
			m_meshBuffer->Drop();
			m_meshBuffer = 0;
		}

		if (m_pMaterial)
		{
			m_pMaterial->Drop();
			m_pMaterial = 0;
		}

		if (m_skin)
		{
			m_skin->Drop();
			m_skin = 0;
		}
	}

	void CMashDecalIntermediate::GetVertexData(sVertexData &out)
	{
		const MashVertex *currentVertex = m_pMaterial->GetVertexDeclaration();
		const mash::sMashVertexElement *vertexElements = currentVertex->GetVertexElements();
		for(uint32 i = 0; i < currentVertex->GetVertexElementCount(); ++i)
		{
			switch(vertexElements[i].usage)
			{
			case aDECLUSAGE_POSITION:
				{
					out.positionElementLocation = vertexElements[i].stride;
					out.positionElementSize = math::Min<uint32>(sizeof(mash::MashVector3), mash::helpers::GetVertexDeclTypeSize(vertexElements[i].type));
					break;
				}
			case aDECLUSAGE_NORMAL:
				{
					out.normalElementLocation = vertexElements[i].stride;
					out.normalElementSize = math::Min<uint32>(sizeof(mash::MashVector3), mash::helpers::GetVertexDeclTypeSize(vertexElements[i].type));
					break;
				}
			case aDECLUSAGE_TEXCOORD:
				{
					out.textureElementLocation = vertexElements[i].stride;
					out.texcoordElementSize = math::Min<uint32>(sizeof(mash::MashVector2), mash::helpers::GetVertexDeclTypeSize(vertexElements[i].type));
					break;
				}
			case aDECLUSAGE_BLENDWEIGHT:
				{
					out.blendWeightElementLocation = vertexElements[i].stride;
					out.blendWeightElementSize = math::Min<uint32>(sizeof(mash::MashVector4), mash::helpers::GetVertexDeclTypeSize(vertexElements[i].type));
					break;
				}
			case aDECLUSAGE_BLENDINDICES:
				{
					out.blendIndexElementLocation = vertexElements[i].stride;
					out.blendIndexElementSize = math::Min<uint32>(sizeof(mash::MashVector4), mash::helpers::GetVertexDeclTypeSize(vertexElements[i].type));
					break;
				}
			}
		}
	}

	void CMashDecalIntermediate::InstanceDecalMembers(CMashDecalIntermediate *from)
	{
		InstanceMembers(from);

		m_iVertexCount = from->m_iVertexCount;
		m_ePrimitiveType = from->m_ePrimitiveType;
		m_iPrimitiveCount = from->m_iPrimitiveCount;
		m_aabb = from->m_aabb;
		m_iReservedSizeInBytes = from->m_iReservedSizeInBytes;

		ResizeVertexBuffer(m_iReservedSizeInBytes);
		memcpy(m_pVertices, from->m_pVertices, m_iReservedSizeInBytes);
	}

	eMASH_STATUS CMashDecalIntermediate::_AppendVertices(const MashTriangleCollider *pTriangleCollection,
				const sTriPickResult &collisionResult,
				const mash::MashVector2 &vTextureDim,
				f32 fRotation,
				const mash::MashMatrix4 *pTransformation,
				const sVertexData &vertexData,
				uint32 &verticesAdded)
	{
		verticesAdded = 0;

		{
			const MashTriangleBuffer *triangleBuffer = pTriangleCollection->GetTriangleBuffer(collisionResult.bufferIndex);

			const uint32 iTriangleCount = triangleBuffer->GetTriangleCount();

			if (collisionResult.triangleIndex >= iTriangleCount)
				return aMASH_FAILED;

			const sTriangleRecord *pCollisionTriangle = &triangleBuffer->GetTriangleList()[collisionResult.triangleIndex];

			const mash::MashVector3 vIntersectionPoint((triangleBuffer->GetPoint(collisionResult.triangleIndex, 0) * 
				collisionResult.u) +
				(triangleBuffer->GetPoint(collisionResult.triangleIndex, 1) * 
				collisionResult.v) +
				(triangleBuffer->GetPoint(collisionResult.triangleIndex, 2) * 
				collisionResult.w));

			//surface normal
			MashVector3 triNormal = triangleBuffer->GetNormal(collisionResult.triangleIndex, 0);
			triNormal += triangleBuffer->GetNormal(collisionResult.triangleIndex, 1);
			triNormal += triangleBuffer->GetNormal(collisionResult.triangleIndex, 2);
			triNormal /= 3;

				mash::MashQuaternion qRotation;
				mash::MashVector3 vBiTangent(0, 1, 0);
				qRotation.SetRotationAxis(triNormal, math::RadsToDegs(fRotation));

				mash::MashVector3 vTangent(triNormal.Cross(vBiTangent));

				if (vTangent.IsZero())
					vTangent = mash::MashVector3(1.0f, 0.0f, 0.0f);
				else
					vTangent.Normalize();

				vBiTangent = triNormal.Cross(vTangent);
				vBiTangent.Normalize();

				vTangent = qRotation.TransformVector(vTangent);
				vTangent.Normalize();
				vBiTangent = qRotation.TransformVector(vBiTangent);
				vBiTangent.Normalize();

			f32 longestHalfDim = vTextureDim.x * 0.5f;
			if ((vTextureDim.y * 0.5f) > longestHalfDim)
				longestHalfDim = (vTextureDim.y * 0.5f);
			
			std::queue<uint32> trianglesToCheck;

			//vector<bool> uses only 1 bit per element.
			MashArray<bool> triangleFlags(iTriangleCount, false);
			MashArray<uint32> intersectingTriangles;
			intersectingTriangles.Reserve(10);			

			{
				triangleFlags[collisionResult.triangleIndex] = 1;
				intersectingTriangles.PushBack(collisionResult.triangleIndex);
				const sTriangleRecord *triangleRecord = &triangleBuffer->GetTriangleList()[collisionResult.triangleIndex];
				const uint32 iTriA = triangleRecord->adjacencyEdgeList[0];
				const uint32 iTriB = triangleRecord->adjacencyEdgeList[1];
				const uint32 iTriC = triangleRecord->adjacencyEdgeList[2];

				if ((iTriA != 0xffffffff) && !triangleFlags[iTriA])
				{
					trianglesToCheck.push(iTriA);
					triangleFlags[iTriA] = true;
				}
				if ((iTriB != 0xffffffff) && !triangleFlags[iTriB])
				{
					trianglesToCheck.push(iTriB);
					triangleFlags[iTriB] = true;
				}
				if ((iTriC != 0xffffffff) && !triangleFlags[iTriC])
				{
					trianglesToCheck.push(iTriC);
					triangleFlags[iTriC] = true;
				}
			}

			mash::MashVector3 trianglePoints[3];
			while(!trianglesToCheck.empty())
			{
				const uint32 iCurrentTriangle = trianglesToCheck.front();
				trianglesToCheck.pop();

				trianglePoints[0] = triangleBuffer->GetPoint(iCurrentTriangle, 0);
				trianglePoints[1] = triangleBuffer->GetPoint(iCurrentTriangle, 1);
				trianglePoints[2] = triangleBuffer->GetPoint(iCurrentTriangle, 2);

				/*
					Just does a simple distance check. If the distance from the intersection point to the current
					triangle is less then the half width of the texture, then we need to consider it to create
					the final mesh.

					In a perfect world I'd be checking the exact distance along the surface, but this will do.
				*/
				mash::MashVector3 c;
				mash::MashTriangle newTri(trianglePoints[0], trianglePoints[1], trianglePoints[2]);
				newTri.ClosestPoint(vIntersectionPoint, c);

				bool addTri = false;
				if (vIntersectionPoint.GetDistanceTo(c) <= longestHalfDim)
					addTri = true;

				if (addTri)
				{
					intersectingTriangles.PushBack(iCurrentTriangle);

					const sTriangleRecord *triangleRecord = &triangleBuffer->GetTriangleList()[iCurrentTriangle];
					const uint32 iTriA = triangleRecord->adjacencyEdgeList[0];
					const uint32 iTriB = triangleRecord->adjacencyEdgeList[1];
					const uint32 iTriC = triangleRecord->adjacencyEdgeList[2];

					if ((iTriA != 0xffffffff) && !triangleFlags[iTriA])
					{
						trianglesToCheck.push(iTriA);
						triangleFlags[iTriA] = true;
					}
					if ((iTriB != 0xffffffff) && !triangleFlags[iTriB])
					{
						trianglesToCheck.push(iTriB);
						triangleFlags[iTriB] = true;
					}
					if ((iTriC != 0xffffffff) && !triangleFlags[iTriC])
					{
						trianglesToCheck.push(iTriC);
						triangleFlags[iTriC] = true;
					}
				}
			}

			// no decals to render
			if (intersectingTriangles.Empty())
				return aMASH_OK;

			OnAddNewDecal();

			if (m_skin)
			{
				CreateSkinnedDecalVertices(triangleBuffer,
					intersectingTriangles,
					vIntersectionPoint,
					triNormal,
					vTextureDim,
					vTangent,
					vBiTangent,
					pTransformation,
					vertexData,
					verticesAdded);
			}
			else
			{

				/*
					The planes that make up the bounding cube are rotated to match the hit
					triangles normal.
					End caps are added to the cube so that it doesn't extend forever resulting
					in triangles on a curved surface, in the distance, being added.
				*/
				const f32 fHalfWidth = vTextureDim.x * 0.5f;
				const f32 fHalfHeight = vTextureDim.y * 0.5f;
				mash::MashPlane left(-vTangent, vIntersectionPoint - (vTangent * fHalfWidth));
				mash::MashPlane right(vTangent, vIntersectionPoint + (vTangent * fHalfWidth));
				mash::MashPlane bottom(-vBiTangent, vIntersectionPoint - (vBiTangent * fHalfHeight));
				mash::MashPlane top(vBiTangent, vIntersectionPoint + (vBiTangent * fHalfHeight));
				/*
					End caps.
					This minimizes decal stretch. It may still occur with rectangular
					decals, but only in certain cases.
				*/
				mash::MashPlane back(-triNormal, vIntersectionPoint - (triNormal * math::Max<f32>(fHalfWidth, fHalfHeight)));
				/*
					This stops triangles in from of the hit point from being included.
					We only add a small buffer in front as an epsilon value.
				*/
				mash::MashPlane front(triNormal, vIntersectionPoint + (triNormal * math::Max<f32>(fHalfWidth, fHalfHeight)));

				MashArray<mash::MashPlane> clippingPlanes(6);
				clippingPlanes[0] = back;
				clippingPlanes[1] = front;
				clippingPlanes[2] = left;
				clippingPlanes[3] = right;
				clippingPlanes[4] = top;
				clippingPlanes[5] = bottom;
				
				CreateNonSkinnedDecalVertices(triangleBuffer,
					intersectingTriangles,
					clippingPlanes,
					vIntersectionPoint,
					triNormal,
					vTextureDim,
					vTangent,
					vBiTangent,
					pTransformation,
					vertexData,
					verticesAdded);
			}
	}

		return aMASH_OK;
	}

	bool CMashDecalIntermediate::Clip(const MashArray<mash::MashPlane> &clippingPlanes,
			MashArray<sClipPoint> &pointsToClip,
			MashArray<sClipPoint> &clippedPoints,
			MashArray<sClipPoint> &triangleListOut)
	{
		/*
			Any point on the backside of a plane is kept.

			If both points are on the backside then the line starting point is written to the buffer.
			In the case of an intersection, the line start and intersection point are written.
		*/
		sClipPoint newClippedPoint;
		f32 t = 0.0f;
		uint32 pointToClipSize = 0;
		triangleListOut.Clear();

		uint32 clippingPlaneCount = clippingPlanes.Size();
		for(uint32 plane = 0; plane < clippingPlaneCount; ++plane)
		{
			clippedPoints.Clear();
			pointToClipSize = pointsToClip.Size();
			uint32 pointBIndex = 1;
			for(uint32 point = 0; point < pointToClipSize; ++point, ++pointBIndex)
			{
				/*
					TODO : Store the last classify. We are redoing work here.
				*/
				eCLASSIFY classifyPointA = clippingPlanes[plane].Classify(pointsToClip[point].position);

				if (pointBIndex == pointToClipSize)
					pointBIndex = 0;
				
				eCLASSIFY classifyPointB = clippingPlanes[plane].Classify(pointsToClip[pointBIndex].position);

				if (classifyPointA == aCLASS_FRONT)
				{
					if (classifyPointB == aCLASS_BEHIND)
					{
						clippingPlanes[plane].Intersect(pointsToClip[point].position, pointsToClip[pointBIndex].position, t);

						newClippedPoint.position = pointsToClip[point].position.Lerp(pointsToClip[pointBIndex].position, t);
						newClippedPoint.normal = pointsToClip[point].normal.Lerp(pointsToClip[pointBIndex].normal, t).Normalize();
						clippedPoints.PushBack(newClippedPoint);
					}
				}
				else if (classifyPointA == aCLASS_BEHIND)
				{
					clippedPoints.PushBack(pointsToClip[point]);

					if (classifyPointB == aCLASS_FRONT)
					{
						clippingPlanes[plane].Intersect(pointsToClip[point].position, pointsToClip[pointBIndex].position, t);

						newClippedPoint.position = pointsToClip[point].position.Lerp(pointsToClip[pointBIndex].position, t);
						newClippedPoint.normal = pointsToClip[point].normal.Lerp(pointsToClip[pointBIndex].normal, t).Normalize();
						clippedPoints.PushBack(newClippedPoint);
					}
				}
				else if (classifyPointA == aCLASS_STRADDLE)
				{
					clippedPoints.PushBack(pointsToClip[point]);
				}
			}

			//polygon is out of bounds
			if (clippedPoints.Empty())
				break;

			pointsToClip = clippedPoints;
		}

		uint32 finalPointCount = clippedPoints.Size();
		
		if (finalPointCount < 3)//shouldnt happen
			return false;

		/*
			Unfortunatly creating triangle strips from a polygon is rather hard.
			And triangle fans are not supported anymore. So we have to settle
			for a simple triangle list.

			We don't optimize the mesh so that it retains the original meshes form....
			It could be done however, if needed.
		*/
		uint32 pointA = 0;
		uint32 pointB = 1;
		uint32 pointC = 2;

		do
		{
			triangleListOut.PushBack(clippedPoints[pointA]);
			triangleListOut.PushBack(clippedPoints[pointB]);
			triangleListOut.PushBack(clippedPoints[pointC]);

			pointB = pointC;
			++pointC;

		}while(pointC < finalPointCount);

		return true;
	}

	void CMashDecalIntermediate::CreateNonSkinnedDecalVertices(const MashTriangleBuffer *triangleBuffer,
			const MashArray<uint32> &decalTriangles,
			const MashArray<mash::MashPlane> &clippingPlanes,
				const mash::MashVector3 &vHitPoint,
				const mash::MashVector3 &vCollisionNormal,
				const mash::MashVector2 &vTextureDim,
				const mash::MashVector3 &vTangent,
				const mash::MashVector3 &vBiTangent,
				const mash::MashMatrix4 *pTransformation,
				const sVertexData &vertexData,
				uint32 &verticesAdded)
	{
		const uint32 iDecalTriangleCount = decalTriangles.Size();

		uint32 vertexSize = m_pMaterial->GetVertexDeclaration()->GetStreamSizeInBytes(0);
		
		mash::MashVector3 position;
		mash::MashVector2 texcoord;
		mash::MashVector3 normal;

		MashArray<sClipPoint> trianglePoints;
		MashArray<sClipPoint> workingClipBuffer;
		MashArray<sClipPoint> clippedTrianglePoints;

		for(uint32 i = 0; i < iDecalTriangleCount; ++i)
		{
			uint32 currentTriangleIndex = decalTriangles[i];

			/*
				This stops decals stretching and looking bad across corners.
			*/
			MashVector3 currentTriNormal = triangleBuffer->GetNormal(currentTriangleIndex, 0);
			currentTriNormal += triangleBuffer->GetNormal(currentTriangleIndex, 1);
			currentTriNormal += triangleBuffer->GetNormal(currentTriangleIndex, 2);
			currentTriNormal /= 3;

			if (math::Safeacos(vCollisionNormal.Dot(currentTriNormal)) > m_mergeTolerance)
				continue;

			trianglePoints.Clear();
			clippedTrianglePoints.Clear();
			workingClipBuffer.Clear();
			trianglePoints.PushBack(sClipPoint(triangleBuffer->GetPoint(currentTriangleIndex, 0), triangleBuffer->GetNormal(currentTriangleIndex, 0)));
			trianglePoints.PushBack(sClipPoint(triangleBuffer->GetPoint(currentTriangleIndex, 1), triangleBuffer->GetNormal(currentTriangleIndex, 1)));
			trianglePoints.PushBack(sClipPoint(triangleBuffer->GetPoint(currentTriangleIndex, 2), triangleBuffer->GetNormal(currentTriangleIndex, 2)));

			if (Clip(clippingPlanes, trianglePoints, workingClipBuffer, clippedTrianglePoints))
			{
				uint32 clippedTriangleCount = clippedTrianglePoints.Size() / 3;

				//resize the buffer if necessary
				const uint32 iNewSizeInBytes = (vertexSize * clippedTriangleCount * 3) + (vertexSize * m_iVertexCount);
				if (iNewSizeInBytes >= m_iReservedSizeInBytes)
					ResizeVertexBuffer(iNewSizeInBytes * 2);

				uint8 *charVertices = (uint8*)m_pVertices;

				for(uint32 triangle = 0; triangle < clippedTriangleCount; ++triangle)
				{
					for(uint32 iTriPoint = 0; iTriPoint < 3; ++iTriPoint)
					{
						if (pTransformation)
							position = pTransformation->TransformVector(clippedTrianglePoints[iTriPoint + (triangle * 3)].position);
						else
							position = clippedTrianglePoints[iTriPoint + (triangle * 3)].position;

						memcpy(&charVertices[(m_iVertexCount * vertexSize) + vertexData.positionElementLocation], position.v, vertexData.positionElementSize);

						if (vertexData.normalElementLocation != mash::math::MaxUInt8())
						{
							if (pTransformation)
								memcpy(&charVertices[(m_iVertexCount * vertexSize) + vertexData.normalElementLocation], pTransformation->TransformRotation(clippedTrianglePoints[iTriPoint + (triangle * 3)].normal).v, vertexData.normalElementSize);
							else
								memcpy(&charVertices[(m_iVertexCount * vertexSize) + vertexData.normalElementLocation], clippedTrianglePoints[iTriPoint + (triangle * 3)].normal.v, vertexData.normalElementSize);
						}

						if (vertexData.textureElementLocation != mash::math::MaxUInt8())
						{
							/*
								Seems to work fine without calculating decal corner.
								0.5 is added to center the decal at the hit point.
							*/
							texcoord.x = (vTangent.Dot(vHitPoint - clippedTrianglePoints[iTriPoint + (triangle * 3)].position) / ((vTextureDim.x))) + 0.5f;
							texcoord.y = (vBiTangent.Dot(vHitPoint - clippedTrianglePoints[iTriPoint + (triangle * 3)].position) / ((vTextureDim.y))) + 0.5f;
							memcpy(&charVertices[(m_iVertexCount * vertexSize) + vertexData.textureElementLocation], texcoord.v, vertexData.texcoordElementSize);
						}

						//expand the bounding box
						m_aabb.Add(position);

						++m_iVertexCount;
						++verticesAdded;
					}

					++m_iPrimitiveCount;
				}
			}
		}

		m_ePrimitiveType = aPRIMITIVE_TRIANGLE_LIST;
	}

	const mash::MashAABB& CMashDecalIntermediate::GetTotalWorldBoundingBox()const
	{
		return MashSceneNode::GetTotalBoundingBox();
	}

	const mash::MashAABB& CMashDecalIntermediate::GetWorldBoundingBox()const
	{
		return MashSceneNode::GetWorldBoundingBox();
	}

	void CMashDecalIntermediate::Draw()
	{
		/*
			TODO : Test any performance issues for determining 
			material LODs for decals.
		*/
		if (!m_pMaterial || !m_meshBuffer)
			return ;

		if (m_pMaterial->GetHasMultipleLodLevels())
		{
			const MashCamera *pActiveCamera = m_pRenderer->GetRenderInfo()->GetCamera();
			int32 iDistanceFromCamera = (int32)collision::GetDistanceToAABB(pActiveCamera->GetWorldTransformState().translation, GetWorldBoundingBox());
			m_pMaterial->UpdateActiveTechnique(iDistanceFromCamera);
		}

		//Set node positions so that either the custom renderer or normal render path can access them
		if (m_skin)
			m_skin->OnRender();

		m_pRenderer->GetRenderInfo()->SetWorldTransform(GetRenderTransformation());

		MashCustomRenderPath *pCustomRenderer = m_pMaterial->GetCustomRenderPath();
		if (pCustomRenderer)
		{
			m_sceneManager->_AddCustomRenderPathToFlushList(pCustomRenderer);
			pCustomRenderer->AddObject(this);
		}
		else
		{
			if (m_pMaterial->OnSet())
			{
				m_pRenderer->DrawVertexList(m_meshBuffer, 
					m_iVertexCount,
					m_iPrimitiveCount,
					m_ePrimitiveType);
			}
		}

		return;
	}

	void CMashDecalIntermediate::SetMaximumDecalLimit(uint32 limit)
	{
		
	}

	void CMashDecalIntermediate::ResizeVertexBuffer(uint32 iNewSizeInBytes)
	{
		if (iNewSizeInBytes == m_iReservedSizeInBytes)
			return;

		uint32 iVertexSize = m_pMaterial->GetVertexDeclaration()->GetStreamSizeInBytes(0);

		iNewSizeInBytes = math::Max<uint32>(iNewSizeInBytes, iVertexSize);

		//make it a multiple of the vertex size to simplify things.
		iNewSizeInBytes -= (iNewSizeInBytes % iVertexSize);

		void *pNewVertices = MASH_ALLOC_COMMON(iNewSizeInBytes);

		//copy over old vertices
		if (m_pVertices)
		{
			/*
				Only copy over old data only if the new buffer is large enough
			*/
			uint32 iCurrentSizeInBytes = iVertexSize * m_iVertexCount;
			if (iNewSizeInBytes >= iCurrentSizeInBytes)
				memcpy(pNewVertices, m_pVertices, iCurrentSizeInBytes);

			MASH_FREE(m_pVertices);
			m_pVertices = 0;
		}

		m_pVertices = pNewVertices;

		m_iReservedSizeInBytes = iNewSizeInBytes;
	}

	void CMashDecalIntermediate::CreateSkinnedDecalVertices(const MashTriangleBuffer *triangleBuffer,
				const MashArray<uint32> &decalTriangles,
				const mash::MashVector3 &vHitPoint,
				const mash::MashVector3 &vCollisionNormal,
				const mash::MashVector2 &vTextureDim,
				const mash::MashVector3 &vTangent,
				const mash::MashVector3 &vBiTangent,
				const mash::MashMatrix4 *pTransformation,
				const sVertexData &vertexData,
				uint32 &verticesAdded)
	{
		const uint32 iDecalTriangleCount = decalTriangles.Size();

		uint32 vertexSize = m_pMaterial->GetVertexDeclaration()->GetStreamSizeInBytes(0);

		//resize the buffer if necessary
		const uint32 iNewSizeInBytes = (vertexSize * iDecalTriangleCount * 3) + (vertexSize * m_iVertexCount);
		if (iNewSizeInBytes >= m_iReservedSizeInBytes)
			ResizeVertexBuffer(iNewSizeInBytes * 2);

		uint8 *charVertices = (uint8*)m_pVertices;
		
		mash::MashVector3 position;
		mash::MashVector2 texcoord;

		/*
			Skinned decals aren't clipped like non-skinned because we need to keep the bone influences
			valid. This may create some visual artifacts if the applied textures edges are not transparent.

			In the furture think about interpolating skinning values?
		*/

		const MashArray<sTriangleSkinnngRecord> &triangleSkinningRecords = triangleBuffer->GetTriangleSkinningList();
		for(uint32 i = 0; i < iDecalTriangleCount; ++i)
		{
			uint32 currentTriangleIndex = decalTriangles[i];
			const sTriangleSkinnngRecord *currentTriangleSkinning = &triangleSkinningRecords[currentTriangleIndex];

			MashVector3 currentTriNormal = triangleBuffer->GetNormal(currentTriangleIndex, 0);
			currentTriNormal += triangleBuffer->GetNormal(currentTriangleIndex, 1);
			currentTriNormal += triangleBuffer->GetNormal(currentTriangleIndex, 2);
			currentTriNormal /= 3;
			/*
				This stops decals stretching and looking bad across corners.
			*/
			if (math::Safeacos(vCollisionNormal.Dot(currentTriNormal)) > m_mergeTolerance)
				continue;

			for(uint32 iTriPoint = 0; iTriPoint < 3; ++iTriPoint)
			{
				position = triangleBuffer->GetPoint(currentTriangleIndex, iTriPoint);
				memcpy(&charVertices[(m_iVertexCount * vertexSize) + vertexData.positionElementLocation], position.v, vertexData.positionElementSize);

				if (vertexData.blendWeightElementLocation != mash::math::MaxUInt8())
					memcpy(&charVertices[(m_iVertexCount * vertexSize) + vertexData.blendWeightElementLocation], currentTriangleSkinning->boneWeights[iTriPoint].v, vertexData.blendWeightElementSize);

				if (vertexData.blendIndexElementLocation != mash::math::MaxUInt8())
					memcpy(&charVertices[(m_iVertexCount * vertexSize) + vertexData.blendIndexElementLocation], currentTriangleSkinning->boneIndices[iTriPoint].v, vertexData.blendIndexElementSize);

				if (vertexData.normalElementLocation != mash::math::MaxUInt8())
				{
					memcpy(&charVertices[(m_iVertexCount * vertexSize) + vertexData.normalElementLocation], triangleBuffer->GetNormal(currentTriangleIndex, iTriPoint).v, vertexData.normalElementSize);
				}

				if (vertexData.textureElementLocation != mash::math::MaxUInt8())
				{
					texcoord.x = (vTangent.Dot(vHitPoint - position) / vTextureDim.x) + 0.5f;
					texcoord.y = (vBiTangent.Dot(vHitPoint - position) / vTextureDim.y) + 0.5f;
					memcpy(&charVertices[(m_iVertexCount * vertexSize) + vertexData.textureElementLocation], texcoord.v, vertexData.texcoordElementSize);
				}

				//expand the bounding box
				m_aabb.Add(position);

				++m_iVertexCount;
				++verticesAdded;
			}
		}

		m_ePrimitiveType = aPRIMITIVE_TRIANGLE_LIST;
		m_iPrimitiveCount += iDecalTriangleCount;
	}

	bool CMashDecalIntermediate::AddRenderablesToRenderQueue(eRENDER_STAGE stage, MashCullTechnique::CullRenderableFunctPtr functPtr)
	{
		if (!functPtr(this))
        {
			m_sceneManager->AddRenderableToRenderQueue(this, aHLPASS_DECAL, stage);
            return true;
        }
        
        return false;
	}
}
