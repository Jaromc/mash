//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_TRI_COLLECTION_CACHED_H_
#define _C_MASH_TRI_COLLECTION_CACHED_H_

#include "MashDataTypes.h"
#include "MashTriangleCollider.h"
#include "MashMathHelper.h"
#include "MashTypes.h"
#include "MashVertexBuffer.h"
#include "MashIndexBuffer.h"

namespace mash
{
	/*
		There is also an octree implimentation
	*/
	class CMashTriCollectionCached : public MashTriangleCollider
	{
	private:
		MashArray<MashTriangleBuffer*> m_triangleBuffers;
	public:
		CMashTriCollectionCached();
		virtual ~CMashTriCollectionCached();
		
		void SetTriangleBuffers(MashTriangleBuffer **buffer, uint32 bufferCount);

		bool GetIntersectingTriangles(const MashAABB &bounds, 
			const MashTransformState &transform, 
			MashArray<sIntersectingTriangleResult> &out)const;

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

		//no data to (de)serialize.
		void Serialize(MashGenericArray &out)const{}
		void Deserialize(const uint8 *dataArray, uint32 &bytesRead){}
	};

	inline eTRIANGLE_COLLIDER_TYPE CMashTriCollectionCached::GetColliderType()const
	{
		return aTRIANGLE_COLLIDER_STANDARD;
	}

	inline uint32 CMashTriCollectionCached::GetTriangleBufferCount()const
	{
		return m_triangleBuffers.Size();
	}

	inline const MashTriangleBuffer* CMashTriCollectionCached::GetTriangleBuffer(uint32 index)const
	{
		return m_triangleBuffers[index];
	}

	inline const MashArray<MashTriangleBuffer*>& CMashTriCollectionCached::GetTriangleBufferCollection()const
	{
		return m_triangleBuffers;
	}
}

#endif