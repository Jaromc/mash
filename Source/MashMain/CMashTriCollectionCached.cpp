//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashTriCollectionCached.h"
#include "MashGeometryHelper.h"
#include "MashTransformState.h"
#include "MashTriangle.h"
#include "MashRay.h"
#include "MashMatrix4.h"
#include "MashAABB.h"
#include "MashTriangleBuffer.h"
#include "MashGenericArray.h"

namespace mash
{
	CMashTriCollectionCached::CMashTriCollectionCached()
	{
		
	}

	CMashTriCollectionCached::~CMashTriCollectionCached()
	{
		for(uint32 i = 0; i < m_triangleBuffers.Size(); ++i)
		{
			if (m_triangleBuffers[i])
				m_triangleBuffers[i]->Drop();
		}
	}

	void CMashTriCollectionCached::SetTriangleBuffers(MashTriangleBuffer **buffer, uint32 bufferCount)
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
	}

	bool CMashTriCollectionCached::GetIntersectingTriangles(const MashAABB &bounds, 
			const MashTransformState &transform, 
			MashArray<sIntersectingTriangleResult> &out)const
	{
		MashAABB transformedAABB = bounds;
		transformedAABB.TransformInverse(transform);

		sIntersectingTriangleResult result;

		bool collisionFound = false;
		uint32 bufferCount = m_triangleBuffers.Size();
		for(uint32 i = 0; i < bufferCount; ++i)
		{
			MashTriangleBuffer *triangleBuffer = m_triangleBuffers[i];
			const uint32 triangleCount = triangleBuffer->GetTriangleCount();
			for(uint32 tri = 0; tri < triangleCount; ++tri)
			{
				if (mash::collision::AABB_Triangle(transformedAABB, triangleBuffer->GetPoint(tri, 0), triangleBuffer->GetPoint(tri, 1), triangleBuffer->GetPoint(tri, 2)))
				{
					collisionFound = true;

					result.bufferIndex = i;
					result.triangleIndex = tri;
					out.PushBack(result);
				}
			}
		}

		return collisionFound;
	}

	bool CMashTriCollectionCached::CheckCollision(const mash::MashRay &ray, 
		const MashTransformState &transform)const
	{
		mash::MashRay transformedRay(ray);
		transformedRay.TransformInverse(transform);

		uint32 bufferCount = m_triangleBuffers.Size();
		for(uint32 i = 0; i < bufferCount; ++i)
		{
			MashTriangleBuffer *triangleBuffer = m_triangleBuffers[i];

			MashArray<uint32>::ConstIterator indexIter = triangleBuffer->GetIndexList().Begin();
			MashArray<uint32>::ConstIterator indexIterEnd = triangleBuffer->GetIndexList().End();
			const MashArray<mash::MashVector3>& vertices = triangleBuffer->GetVertexList();
			for(; indexIter != indexIterEnd; indexIter+=3)
			{
				if (mash::collision::Ray_Triangle(vertices[*indexIter], vertices[*(indexIter+1)], vertices[*(indexIter+2)],
					transformedRay))
				{
					return true;
				}
			}
		}

		return false;
	}

	bool CMashTriCollectionCached::GetClosestTriangle(const mash::MashRay &ray,
			const MashTransformState &transform,
			sTriPickResult &out)const
	{
		mash::MashRay transformedRay(ray);
		transformedRay.TransformInverse(transform);

		out.distance = mash::math::MaxFloat();
		out.collision = false;
		
		uint32 bufferCount = m_triangleBuffers.Size();
		for(uint32 i = 0; i < bufferCount; ++i)
		{
			MashTriangleBuffer *triangleBuffer = m_triangleBuffers[i];

			MashArray<uint32>::ConstIterator indexIter = triangleBuffer->GetIndexList().Begin();
			MashArray<uint32>::ConstIterator indexIterEnd = triangleBuffer->GetIndexList().End();
			const MashArray<mash::MashVector3>& vertices = triangleBuffer->GetVertexList();
			uint32 triangleCount = 0;
			sTriPickResult result;
			for(; indexIter != indexIterEnd; indexIter+=3)
			{
				if (mash::collision::Ray_Triangle(vertices[*indexIter], vertices[*(indexIter+1)], vertices[*(indexIter+2)],//tri,
					transformedRay,
					result.u,
					result.v,
					result.w,
					result.distance))
				{
					if (result.distance < out.distance)
					{
						out.bufferIndex = i;
						out.triangleIndex = triangleCount;
						out.distance = result.distance;

						out.u = result.u;
						out.v = result.v;
						out.w = result.w;
						out.collision = true;
					}
				}

				++triangleCount;
			}
		}

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

	bool CMashTriCollectionCached::GetIntersectingTriangles(const mash::MashRay &ray,
		const MashTransformState &transform,
		MashArray<sTriPickResult> &out)const
	{
		mash::MashRay transformedRay(ray);
		transformedRay.TransformInverse(transform);

		sTriPickResult result;

		uint32 bufferCount = m_triangleBuffers.Size();
		for(uint32 i = 0; i < bufferCount; ++i)
		{
			MashTriangleBuffer *triangleBuffer = m_triangleBuffers[i];

			MashArray<uint32>::ConstIterator indexIter = triangleBuffer->GetIndexList().Begin();
			MashArray<uint32>::ConstIterator indexIterEnd = triangleBuffer->GetIndexList().End();
			const MashArray<mash::MashVector3>& vertices = triangleBuffer->GetVertexList();
			uint32 triangleCount = 0;
			for(; indexIter != indexIterEnd; indexIter+=3)
			{
				if (mash::collision::Ray_Triangle(vertices[*indexIter], vertices[*(indexIter+1)], vertices[*(indexIter+2)],
					transformedRay,
					result.u,
					result.v,
					result.w,
					result.distance))
				{
					result.triangleIndex = triangleCount;
					result.collision = true;
					out.PushBack(result);
				}

				++triangleCount;
			}
		}

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