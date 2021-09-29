//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_TRI_COLLIDER_KDTREE_H_
#define _C_MASH_TRI_COLLIDER_KDTREE_H_

#include "MashTriangleCollider.h"
#include "MashAABB.h"
#include <vector>

namespace mash
{
	class CMashTriColliderKDTree : public MashTriangleCollider
	{
	private:
		class sTriangleData
		{
		public:
			uint32 triangleBuffer;
			uint32 triangleIndex;

			sTriangleData():triangleBuffer(0), triangleIndex(0){}
			~sTriangleData()
			{
			}
		};

		struct sCollectIntersectionData
		{
			MashAABB aabb;
			MashVector3 aabbCenter;
		};

		class KDNode
		{
		public:
			uint32 axis;
			f32 splitPosition;
			KDNode *child[2];

			uint32 *triangleIndexList;
			uint32 triangleCount;

			KDNode():axis(0), triangleIndexList(0), splitPosition(0.0f), triangleCount(0)
			{
				child[0] = 0;
				child[1] = 0;
			}
			~KDNode()
			{
				if (child[0])
				{
					MASH_DELETE_T(KDNode, child[0]);
					child[0] = 0;
				}

				if (child[1])
				{
					MASH_DELETE_T(KDNode, child[1]);
					child[1] = 0;
				}

				if (triangleIndexList)
				{
					MASH_FREE(triangleIndexList);
					triangleIndexList = 0;
				}
			}
		};

		MashArray<MashTriangleBuffer*> m_triangleBuffers;
		KDNode *m_root;

		//TODO : Dynamic bitset
		mutable std::vector<int8> m_intersectionCheckCache;

		//this list must never change memory position after init.
		MashArray<sTriangleData> m_triangleDataPool;

		//parentBounds used to determine splitting plane
		void SplitNode(KDNode *parent, const MashArray<uint32> &triangleList, MashAABB &parentBounds, /*uint32 maxDepth, */uint32 desiredTriangledPerNode, uint32 &currentDepth, uint32 axisTriCount[3]);
		void CreateTree(uint32 maxDepth, uint32 desiredTriangledPerNode);
		bool GetClosestTriangle(KDNode *node, const mash::MashRay &ray, bool quitOnFirstCollision, sTriPickResult &out)const;
		bool GetIntersectingTriangles(KDNode *node, const mash::MashRay &ray, MashArray<sTriPickResult> &out)const;

		void _GetIntersectingTriangles(KDNode *node, 
			const sCollectIntersectionData &bodyData,
			MashVector3 &volNearPt,
			bool &collisionFound,
			MashArray<sIntersectingTriangleResult> &out)const;

		bool GetIntersectingTriangles(const MashAABB &bounds, 
			const MashTransformState &transform, 
			MashArray<sIntersectingTriangleResult> &out)const;

		void _Serialize(KDNode *node, MashGenericArray &out)const;
		void _Deserialize(KDNode *node, const uint8 *dataArray, uint32 &nextBytes);
	public:
		CMashTriColliderKDTree();
		~CMashTriColliderKDTree();

		void SetTriangleBuffers(MashTriangleBuffer **buffer, uint32 bufferCount);

		bool CheckCollision(const mash::MashRay &ray, 
			const MashTransformState &transform)const;

		bool GetClosestTriangle(const mash::MashRay &ray,
			const MashTransformState &transform,
			sTriPickResult &out)const;

		bool GetIntersectingTriangles(const mash::MashRay &ray,
			const MashTransformState &transform,
			MashArray<sTriPickResult> &out)const;

		const MashArray<MashTriangleBuffer*>& GetTriangleBufferCollection()const;
		const MashTriangleBuffer* GetTriangleBuffer(uint32 index)const;
		uint32 GetTriangleBufferCount()const;
		eTRIANGLE_COLLIDER_TYPE GetColliderType()const;

		void Serialize(MashGenericArray &out)const;
		void Deserialize(const uint8 *dataArray, uint32 &bytesRead);

		//must only be called once when loaded
		void GenerateSpacialData();
	};

	inline eTRIANGLE_COLLIDER_TYPE CMashTriColliderKDTree::GetColliderType()const
	{
		return aTRIANGLE_COLLIDER_KD_TREE;
	}

	inline uint32 CMashTriColliderKDTree::GetTriangleBufferCount()const
	{
		return m_triangleBuffers.Size();
	}

	inline const MashTriangleBuffer* CMashTriColliderKDTree::GetTriangleBuffer(uint32 index)const
	{
		return m_triangleBuffers[index];
	}

	inline const MashArray<MashTriangleBuffer*>& CMashTriColliderKDTree::GetTriangleBufferCollection()const
	{
		return m_triangleBuffers;
	}
}

#endif