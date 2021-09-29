//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashTriColliderKDTree.h"
#include "MashTriangle.h"
#include "MashRay.h"
#include "MashMatrix4.h"
#include "MashTriangleBuffer.h"
#include "MashGenericArray.h"
#include "MashGeometryHelper.h"
#include "MashTransformState.h"

namespace mash
{
	CMashTriColliderKDTree::CMashTriColliderKDTree():m_root(0)
	{

	}

	CMashTriColliderKDTree::~CMashTriColliderKDTree()
	{
		if (m_root)
		{
			MASH_DELETE_T(KDNode, m_root);
		}

		for(uint32 i = 0; i < m_triangleBuffers.Size(); ++i)
		{
			if (m_triangleBuffers[i])
				m_triangleBuffers[i]->Drop();
		}
	}

	void CMashTriColliderKDTree::_Serialize(KDNode *node, MashGenericArray &out)const
	{
		out.AppendUnsignedInt(node->axis);
		out.AppendFloat(node->splitPosition);
		out.AppendUnsignedInt(node->triangleCount);

		if (node->triangleCount > 0)
			out.Append(node->triangleIndexList, node->triangleCount * sizeof(uint32));

		if (node->child[0])
			out.AppendUnsignedInt(1);
		else
			out.AppendUnsignedInt(0);

		if (node->child[1])
			out.AppendUnsignedInt(1);
		else
			out.AppendUnsignedInt(0);

		if (node->child[0])
			_Serialize(node->child[0], out);

		if (node->child[1])
			_Serialize(node->child[1], out);
	}

	void CMashTriColliderKDTree::Serialize(MashGenericArray &out)const
	{
		if (m_root)
			out.AppendUnsignedInt(1);
		else
			out.AppendUnsignedInt(0);

		if (m_root)
			_Serialize(m_root, out);

		out.AppendUnsignedInt(m_triangleDataPool.Size());
		
		if (!m_triangleDataPool.Empty())
			out.Append(&m_triangleDataPool[0], m_triangleDataPool.Size() * sizeof(sTriangleData));
	}

	void CMashTriColliderKDTree::_Deserialize(KDNode *node, const uint8 *dataArray, uint32 &nextByte)
	{
		memcpy(&node->axis, &dataArray[nextByte], sizeof(uint32));
		nextByte += sizeof(uint32);

		memcpy(&node->splitPosition, &dataArray[nextByte], sizeof(f32));
		nextByte += sizeof(f32);

		memcpy(&node->triangleCount, &dataArray[nextByte], sizeof(uint32));
		nextByte += sizeof(uint32);

		if (node->triangleCount > 0)
		{
			node->triangleIndexList = MASH_NEW_ARRAY_T_COMMON(uint32, node->triangleCount);
			memcpy(node->triangleIndexList, &dataArray[nextByte], sizeof(uint32) * node->triangleCount);
			nextByte += sizeof(uint32) * node->triangleCount;
		}
		else
			node->triangleIndexList = 0;

		uint32 childIndex = 0;
		memcpy(&childIndex, &dataArray[nextByte], sizeof(uint32));
		nextByte += sizeof(uint32);

		if (childIndex == 1)
			node->child[0] = MASH_NEW_T_COMMON(KDNode);

		memcpy(&childIndex, &dataArray[nextByte], sizeof(uint32));
		nextByte += sizeof(uint32);

		if (childIndex == 1)
			node->child[1] = MASH_NEW_T_COMMON(KDNode);

		if (node->child[0])
			_Deserialize(node->child[0], dataArray, nextByte);
		if (node->child[1])
			_Deserialize(node->child[1], dataArray, nextByte);
	}

	void CMashTriColliderKDTree::Deserialize(const uint8 *dataArray, uint32 &bytesRead)
	{
		uint32 nextByte = 0;

		uint32 containsRoot = 0;
		memcpy(&containsRoot, dataArray, sizeof(uint32));
		nextByte += sizeof(uint32);

		if (containsRoot == 1)
		{
			m_root = MASH_NEW_T_COMMON(KDNode);
			_Deserialize(m_root, dataArray, nextByte);
		}

		uint32 triPoolSize = 0;
		memcpy(&triPoolSize, &dataArray[nextByte], sizeof(uint32));
		nextByte += sizeof(uint32);

		if (triPoolSize > 0)
		{
			m_triangleDataPool.Resize(triPoolSize);
			memcpy(&m_triangleDataPool[0], &dataArray[nextByte], sizeof(sTriangleData) * triPoolSize);
			nextByte += sizeof(sTriangleData) * triPoolSize;
		}

		bytesRead += nextByte;
	}

	void CMashTriColliderKDTree::SplitNode(KDNode *parent, 
		const MashArray<uint32> &triangleList, 
		MashAABB &parentBounds, 
		//uint32 maxDepth, 
		uint32 desiredTriangledPerNode, 
		uint32 &currentDepth,
		uint32 axisTriCount[3])
	{
		parent->axis = currentDepth % 3;
		//just halve the aabb on the current axis to determine the splitting plane
		parent->splitPosition = (parentBounds.max[parent->axis] + parentBounds.min[parent->axis]) * 0.5f;
		parent->child[0] = 0;
		parent->child[1] = 0;

		//quit condition
		const uint32 triangleCount = triangleList.Size();

		/*
			If all axis have the same tri count then weve reached a point where
			tris wont subdivide any further.
		*/
		bool forceLeafNode = axisTriCount[0] == axisTriCount[1] && axisTriCount[1] == axisTriCount[2];
		if (forceLeafNode || (triangleCount < desiredTriangledPerNode))
		{
			//this is a leaf node
			parent->triangleCount = triangleCount;
			//use alloc for speedier allocations
			parent->triangleIndexList = MASH_ALLOC_T_COMMON(uint32, parent->triangleCount);

			for(uint32 i = 0; i < triangleCount; ++i)
				parent->triangleIndexList[i] = triangleList[i];

			return;
		}

		MashArray<uint32> leftTris;
		MashArray<uint32> rightTris;

		f32 p0, p1, p2;
		sTriangleData *curTri = 0;
		int boundarySide = 0;
		for(uint32 i = 0; i < triangleCount; ++i)
		{
			boundarySide = 0;
			curTri = &m_triangleDataPool[triangleList[i]];
			p0 = m_triangleBuffers[curTri->triangleBuffer]->GetPoint(curTri->triangleIndex, 0)[parent->axis];
			p1 = m_triangleBuffers[curTri->triangleBuffer]->GetPoint(curTri->triangleIndex, 1)[parent->axis];
			p2 = m_triangleBuffers[curTri->triangleBuffer]->GetPoint(curTri->triangleIndex, 2)[parent->axis];

			/*
				TODO : Should this be checking edge intersections too?
			*/

			//a triangle can live in both sides if it overlaps the boundary.
			if ((p0 < parent->splitPosition) || (p1 < parent->splitPosition) || (p2 < parent->splitPosition))
				leftTris.PushBack(triangleList[i]);
			
			if ((p0 >= parent->splitPosition) || (p1 >= parent->splitPosition) || (p2 >= parent->splitPosition))
				rightTris.PushBack(triangleList[i]);
		}

		if (!leftTris.Empty())
		{
			f32 oldMax = parentBounds.max[parent->axis];
			parentBounds.max.v[parent->axis] = parent->splitPosition;
			uint32 oldAxisCount = axisTriCount[parent->axis];
			axisTriCount[parent->axis] = leftTris.Size();

			++currentDepth;
			parent->child[0] = MASH_NEW_T_COMMON(KDNode);
			SplitNode(parent->child[0], leftTris, parentBounds/*, maxDepth*/, desiredTriangledPerNode, currentDepth, axisTriCount);
			--currentDepth;

			parentBounds.max.v[parent->axis] = oldMax;
			axisTriCount[parent->axis] = oldAxisCount;
		}

		if (!rightTris.Empty())
		{
			f32 oldMin = parentBounds.min[parent->axis];
			parentBounds.min.v[parent->axis] = parent->splitPosition;
			uint32 oldAxisCount = axisTriCount[parent->axis];
			axisTriCount[parent->axis] = rightTris.Size();

			++currentDepth;
			parent->child[1] = MASH_NEW_T_COMMON(KDNode);
			SplitNode(parent->child[1], rightTris, parentBounds/*, maxDepth*/, desiredTriangledPerNode, currentDepth, axisTriCount);
			--currentDepth;

			parentBounds.min.v[parent->axis] = oldMin;
			axisTriCount[parent->axis] = oldAxisCount;
		}
	}

	void CMashTriColliderKDTree::CreateTree(uint32 maxDepth, uint32 desiredTriangledPerNode)
	{
		MashAABB totalAABB(MashVector3(mash::math::MaxFloat(), mash::math::MaxFloat(), mash::math::MaxFloat()),
			MashVector3(mash::math::MinFloat(), mash::math::MinFloat(), mash::math::MinFloat()));

		uint32 triangleCount = 0;
		for(uint32 i = 0; i < m_triangleBuffers.Size(); ++i)
		{
			const MashTriangleBuffer *triangleBuffer = m_triangleBuffers[i];

			/*
				Create an aabb that contains all the triangles. This will be used to determine the
				splitting plane later.
			*/
			MashArray<mash::MashVector3>::ConstIterator pointIter = triangleBuffer->GetVertexList().Begin();
			MashArray<mash::MashVector3>::ConstIterator pointIterEnd = triangleBuffer->GetVertexList().End();
			for(; pointIter != pointIterEnd; ++pointIter)
			{
				totalAABB.Add(*pointIter);
			}

			triangleCount += triangleBuffer->GetTriangleCount();
		}

		//nothing to store
		if (triangleCount == 0)
			return;

		MashArray<uint32> triangleIndexList(triangleCount);

		/*
			Now we build the pool of triangles for this tree. A node will reference this data so we
			don't waste memory when triangle data needs to be duplicated due to boundary cases.
		*/
		m_triangleDataPool.Resize(triangleCount);
		uint32 currentTriangle = 0;
		for(uint32 i = 0; i < m_triangleBuffers.Size(); ++i)
		{
			uint32 curBufferTriCount = m_triangleBuffers[i]->GetTriangleCount();
			for(uint32 tri = 0; tri < curBufferTriCount; ++tri)
			{
				m_triangleDataPool[currentTriangle].triangleBuffer = i;
				m_triangleDataPool[currentTriangle].triangleIndex = tri;

				//generate a list of triangle indices. This is what the leaf nodes will hold
				triangleIndexList[currentTriangle] = currentTriangle;

				++currentTriangle;
			}
		}

		m_root = MASH_NEW_T_COMMON(KDNode);

		uint32 currentDepth = 0;
		uint32 axisTriCount[] = {0, mash::math::MaxUInt32() / 2, mash::math::MaxUInt32()};
		SplitNode(m_root, triangleIndexList, totalAABB/*, maxDepth*/, desiredTriangledPerNode, currentDepth, axisTriCount);
	}

	bool CMashTriColliderKDTree::GetIntersectingTriangles(KDNode *node, const mash::MashRay &ray, MashArray<sTriPickResult> &out)const
	{
		if (!node)
			return (!out.Empty());

		if (node->triangleCount > 0)
		{
			//check triangles
			for(uint32 i = 0; i < node->triangleCount; ++i)
			{
				const sTriangleData *currentTriangleData = &m_triangleDataPool[node->triangleIndexList[i]];
				m_triangleBuffers[currentTriangleData->triangleBuffer]->GetPoint(currentTriangleData->triangleIndex, 0);
				sTriPickResult result;
				if (mash::collision::Ray_Triangle(m_triangleBuffers[currentTriangleData->triangleBuffer]->GetPoint(currentTriangleData->triangleIndex, 0), 
					m_triangleBuffers[currentTriangleData->triangleBuffer]->GetPoint(currentTriangleData->triangleIndex, 1), 
					m_triangleBuffers[currentTriangleData->triangleBuffer]->GetPoint(currentTriangleData->triangleIndex, 2),//tri,
					ray,
					result.u,
					result.v,
					result.w,
					result.distance))
				{
					result.bufferIndex = currentTriangleData->triangleBuffer;
					result.triangleIndex = currentTriangleData->triangleIndex;
					result.collision = true;

					out.PushBack(result);
				}
			}
		}

		uint32 axis = node->axis;
		uint32 firstNodeToCheck = ray.origin[axis] > node->splitPosition;

		if (ray.dir[axis] == 0.0f)
		{
			GetIntersectingTriangles(node->child[firstNodeToCheck], ray, out);
		}
		else
		{
			float t = (node->splitPosition - ray.origin[axis]) / ray.dir[axis];

			if (0.0f <= t)
			{
				GetIntersectingTriangles(node->child[firstNodeToCheck], ray, out);
				GetIntersectingTriangles(node->child[firstNodeToCheck ^ 1], ray, out);
			}
			else
			{
				GetIntersectingTriangles(node->child[firstNodeToCheck], ray, out);
			}
		}

		return (!out.Empty());
	}

	void CMashTriColliderKDTree::_GetIntersectingTriangles(KDNode *node, 
			const sCollectIntersectionData &bodyData,
			MashVector3 &volNearPt,
			bool &collisionFound,
			MashArray<sIntersectingTriangleResult> &out)const
	{
		if (!node)
			return;

		if (node->triangleCount > 0)
		{
			//check triangles
			sIntersectingTriangleResult result;
			for(uint32 i = 0; i < node->triangleCount; ++i)
			{
				
				if (m_intersectionCheckCache[node->triangleIndexList[i]] == 0)
				{//++tricolcounter;
					const sTriangleData *currentTriangleData = &m_triangleDataPool[node->triangleIndexList[i]];
					
					if (mash::collision::AABB_Triangle(bodyData.aabb, 
						m_triangleBuffers[currentTriangleData->triangleBuffer]->GetPoint(currentTriangleData->triangleIndex, 0), 
						m_triangleBuffers[currentTriangleData->triangleBuffer]->GetPoint(currentTriangleData->triangleIndex, 1), 
						m_triangleBuffers[currentTriangleData->triangleBuffer]->GetPoint(currentTriangleData->triangleIndex, 2)))
					{
						collisionFound = true;

						result.bufferIndex = currentTriangleData->triangleBuffer;
						result.triangleIndex = currentTriangleData->triangleIndex;
						out.PushBack(result);
					}

					m_intersectionCheckCache[node->triangleIndexList[i]] = 1;
				}
			}
		}

		// Figure out which child to recurse into first (0 = near, 1 = far)
		uint32 firstNodeToCheck = bodyData.aabbCenter.v[node->axis] > node->splitPosition;

		// Always recurse into the subtree the sphere center is in
		_GetIntersectingTriangles(node->child[firstNodeToCheck], bodyData, volNearPt, collisionFound, out);

		// Update (by clamping) nearest point on volume when traversing far side.
		// Keep old value on the local stack so it can be restored later
		f32 oldValue = volNearPt[node->axis];
		volNearPt.v[node->axis] = node->splitPosition;

		// If sphere overlaps the volume of the far node, recurse that subtree too
		//if (volNearPt.GetDistanceToSQ(bodyData.sphereCenter) < (bodyData.sphereRadius * bodyData.sphereRadius))
		//	_GetIntersectingTriangles(node->child[firstNodeToCheck ^ 1], bodyData, volNearPt, collisionFound, out);
		if (bodyData.aabb.Intersects(volNearPt))
			_GetIntersectingTriangles(node->child[firstNodeToCheck ^ 1], bodyData, volNearPt, collisionFound, out);
		

		// Restore component of nearest pt on volume when returning
		volNearPt.v[node->axis] = oldValue;
	}

	bool CMashTriColliderKDTree::GetIntersectingTriangles(const MashAABB &bounds, 
			const MashTransformState &transform, 
			MashArray<sIntersectingTriangleResult> &out)const
	{
		//tricolcounter = 0;
		sCollectIntersectionData bodyData;

		/*
			m_intersectionCheckCache is only used here. So we resize it here only so
			it doesn't reserve memory when its not needed.

			fill is specialized for char types using memset.
		*/
		if (m_intersectionCheckCache.size() < m_triangleDataPool.Size())
			m_intersectionCheckCache.resize(m_triangleDataPool.Size(), false);
		else
			std::fill(m_intersectionCheckCache.begin(), m_intersectionCheckCache.end(), 0);

		bodyData.aabb = bounds;
		bodyData.aabb.TransformInverse(transform);

		//convert aabb to sphere
		bodyData.aabbCenter = bodyData.aabb.GetCenter();

		MashVector3 volNearPt = bodyData.aabbCenter;
		bool collisionFound = false;
		_GetIntersectingTriangles(m_root, bodyData, volNearPt, collisionFound, out);
		return collisionFound;
	}

	bool CMashTriColliderKDTree::GetClosestTriangle(KDNode *node, const mash::MashRay &ray, bool quitOnFirstCollision, sTriPickResult &out)const
	{
		if (!node)
			return out.collision;

		if (node->triangleCount > 0)
		{
			//check triangles
			for(uint32 i = 0; i < node->triangleCount; ++i)
			{
				const sTriangleData *currentTriangleData = &m_triangleDataPool[node->triangleIndexList[i]];
				sTriPickResult result;
				if (mash::collision::Ray_Triangle(m_triangleBuffers[currentTriangleData->triangleBuffer]->GetPoint(currentTriangleData->triangleIndex, 0), 
					m_triangleBuffers[currentTriangleData->triangleBuffer]->GetPoint(currentTriangleData->triangleIndex, 1), 
					m_triangleBuffers[currentTriangleData->triangleBuffer]->GetPoint(currentTriangleData->triangleIndex, 2),//tri,
					ray,
					result.u,
					result.v,
					result.w,
					result.distance))
				{
					if (result.distance < out.distance)
					{
						out.bufferIndex = currentTriangleData->triangleBuffer;
						out.triangleIndex = currentTriangleData->triangleIndex;
						out.distance = result.distance;

						out.u = result.u;
						out.v = result.v;
						out.w = result.w;
						out.collision = true;

						if (quitOnFirstCollision)
							return true;
					}
				}
			}
		}

		/*
			Nodes are traversed in near to far order, so once a node has a triangle that
			intersects with the ray we can exit. Note we don't exit right away because the node
			may contain many triangles at different distances (the triangles at this node will be
			the closest out of all other nodes).
		*/
		if (out.collision)
			return true;

		uint32 axis = node->axis;
		uint32 firstNodeToCheck = ray.origin[axis] > node->splitPosition;

		/*
			If the ray is parallel to the plane then we only need to check the closest node because
			the ray doesn't pass into the neighbour.
		*/
		if (ray.dir[axis] == 0.0f)
		{
			if (GetClosestTriangle(node->child[firstNodeToCheck], ray, quitOnFirstCollision, out))
				return true;
		}
		else
		{
			//Gets the length along the ray (on the current axis) that an intersection with the plane occurs.
			float t = (node->splitPosition - ray.origin[axis]) / ray.dir[axis];

			/*
				A positive length means the ray intersects with the plane, therefore we need to check
				both children.
				First we check the closest child as this will contain the closest triangles.
			*/
			if (0.0f <= t)
			{
				if (GetClosestTriangle(node->child[firstNodeToCheck], ray, quitOnFirstCollision, out))
					return true;

				if (GetClosestTriangle(node->child[firstNodeToCheck ^ 1], ray, quitOnFirstCollision, out))
					return true;
			}
			else
			{
				/*
					The ray doesn't intersect the far child so we only need to check the closest child.
				*/
				if (GetClosestTriangle(node->child[firstNodeToCheck], ray, quitOnFirstCollision, out))
					return true;
			}
		}

		return out.collision;
	}

	void CMashTriColliderKDTree::SetTriangleBuffers(MashTriangleBuffer **buffer, uint32 bufferCount)
	{
		for(uint32 i = 0; i < bufferCount; ++i)
		{
			if (buffer[i])
				buffer[i]->Grab();
		}

		for(uint32 i = 0; i < m_triangleBuffers.Size(); ++i)
		{
			if (m_triangleBuffers[i])
				m_triangleBuffers[i]->Drop();
		}

		m_triangleBuffers.Clear();

		for(uint32 i = 0; i < bufferCount; ++i)
		{
			if (buffer[i])
				m_triangleBuffers.PushBack(buffer[i]);
		}

		//m_intersectionCheckCache.resize(m_triangleDataPool.Size(), 0);
	}

	void CMashTriColliderKDTree::GenerateSpacialData()
	{
		/*
			Use a value thats not too small to stop the tree from growing too large.
			Meshes with very large triangles will still fill the tree unnecessarily due
			to them not fitting into a node neatly.

			The following calculations use a rough approximation of the best case
			scenario of triangle dispersion
		*/
		uint32 desiredTrisPerNode = 20; 
		uint32 maxDepth = 5;//NOTE : Max depth is currently not being used

		const uint32 triCount = m_triangleDataPool.Size();
		if (triCount > 500)
		{
			desiredTrisPerNode = 20;
			maxDepth = 10;
		}
		if (triCount > 2000)
		{
			desiredTrisPerNode = (triCount / 100) + 1;
			maxDepth = 10;
		}

		CreateTree(maxDepth, desiredTrisPerNode);
	}

	bool CMashTriColliderKDTree::CheckCollision(const mash::MashRay &ray, const MashTransformState &transform)const
	{
		mash::MashRay transformedRay(ray);
		transformedRay.TransformInverse(transform);

		sTriPickResult out;
		out.distance = mash::math::MaxFloat();
		out.collision = false;

		return GetClosestTriangle(m_root, transformedRay, true, out);
	}

	bool CMashTriColliderKDTree::GetClosestTriangle(const mash::MashRay &ray,
		const MashTransformState &transform,
		sTriPickResult &out)const
	{
		mash::MashRay transformedRay(ray);
		transformedRay.TransformInverse(transform);

		out.distance = mash::math::MaxFloat();
		out.collision = false;

		GetClosestTriangle(m_root, transformedRay, false, out);

		/*
			This is needed to scale the distance back into world space.
			At the moment it may be scaled by the local scale, which would
			be wrong.
		*/
		if (out.collision)
		{
			const MashTriangleBuffer *triangleBuffer = m_triangleBuffers[out.bufferIndex];

			mash::MashVector3 intersectionPoint((triangleBuffer->GetPoint(out.triangleIndex, 0) * 
					out.u) +
					(triangleBuffer->GetPoint(out.triangleIndex, 1) * 
					out.v) +
					(triangleBuffer->GetPoint(out.triangleIndex, 2) * 
					out.w));

			//transform into world space
			intersectionPoint = transform.Transform(intersectionPoint);
			//get actual distance
			out.distance = (ray.origin - intersectionPoint).Length();
		}

		return out.collision;
	}

	bool CMashTriColliderKDTree::GetIntersectingTriangles(const mash::MashRay &ray,
		const MashTransformState &transform,
		MashArray<sTriPickResult> &out)const
	{
		mash::MashRay transformedRay(ray);
		transformedRay.TransformInverse(transform);

		GetIntersectingTriangles(m_root, transformedRay, out);

		/*
			This is needed to scale the distance back into world space.
			At the moment it may be scaled by the local scale, which would
			be wrong.
		*/
		const uint32 trisFound = out.Size();
		for(uint32 i = 0; i < trisFound; ++i)
		{
			const MashTriangleBuffer *triangleBuffer = m_triangleBuffers[out[i].bufferIndex];

			mash::MashVector3 intersectionPoint((triangleBuffer->GetPoint(out[i].triangleIndex, 0) * 
					out[i].u) +
					(triangleBuffer->GetPoint(out[i].triangleIndex, 1) * 
					out[i].v) +
					(triangleBuffer->GetPoint(out[i].triangleIndex, 2) * 
					out[i].w));

			//transform into world space
			intersectionPoint = transform.Transform(intersectionPoint);
			//get actual distance
			out[i].distance = (ray.origin - intersectionPoint).Length();
		}

		return trisFound;
	}
}