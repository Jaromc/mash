//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "MashScenePick.h"
#include "MashGeometryHelper.h"
#include "MashTriangleCollider.h"
#include "MashRay.h"
#include "MashSceneNode.h"

namespace mash
{
	bool MashScenePick::GetNodesByBounds(mash::MashSceneNode *pScene,
			const mash::MashRay &ray,
			uint32 iTypesToTest,
			MashArray<mash::MashSceneNode*> &out)
	{
		if (!pScene)
			return false;
        
		/*
			If this is a child node and of correct type
			then we can add it to the list
		*/
		if ((iTypesToTest & pScene->GetNodeType()) && mash::collision::Ray_AABB(pScene->GetWorldBoundingBox(), ray))
		{
			//this is a valid leaf node
			out.PushBack(pScene);
		}

            if (mash::collision::Ray_AABB(pScene->GetTotalBoundingBox(), ray))
            {
                MashList<mash::MashSceneNode*>::ConstIterator iter = pScene->GetChildren().Begin();
                MashList<mash::MashSceneNode*>::ConstIterator end = pScene->GetChildren().End();
                for(; iter != end; ++iter)
                {
                    GetNodesByBounds((*iter), ray, iTypesToTest, out);
                }
            }

		return out.Size();
	}

	bool MashScenePick::GetTrianglesFromScene(mash::MashSceneNode *pScene,
			const mash::MashRay &ray,
			uint32 iTypesToTest,
			bool bBackFaceCull,
			MashArray<sTriPickResult> &out)
	{
		if (!pScene)
			return false;

		const MashTriangleCollider *collider = pScene->GetTriangleCollider();
		//if this node has triangles
		if (collider)
		{
			//and of correct type
			if ((iTypesToTest & pScene->GetNodeType()) && mash::collision::Ray_AABB(pScene->GetWorldBoundingBox(), ray))
			{
				int32 iPreCount = out.Size();

				collider->GetIntersectingTriangles(ray, 
					pScene->GetWorldTransformState(), out);

				int32 iPostCount = out.Size();
				int32 iDiff = iPostCount - iPreCount;
				for(int32 j = 0; j < iDiff; ++j)
				{
					//out[iPreCount + j].triangleCollectionIndex = i;
					out[iPreCount + j].node = pScene;

				}
			}
		}

        //If there is no intersection here than no need to check children
		if (mash::collision::Ray_AABB(pScene->GetTotalBoundingBox(), ray))
        {
			MashList<mash::MashSceneNode*>::ConstIterator iter = pScene->GetChildren().Begin();
			MashList<mash::MashSceneNode*>::ConstIterator end = pScene->GetChildren().End();
			for(; iter != end; ++iter)
			{
				GetTrianglesFromScene((*iter), ray, iTypesToTest, bBackFaceCull, out);
			}
        }

		return out.Size();
	}

	bool MashScenePick::GetClosestNodeByBounds(mash::MashSceneNode *pScene,
			const mash::MashRay &ray,
			uint32 iTypesToTest,
			f32 &fMaximumRayLength,
			mash::MashSceneNode **out)
	{
		if (!pScene)
			return *out;

		f32 temp = 0.0f;

		/*
			If this node is of correct type
			then we can add it to the list
		*/
		if ((iTypesToTest & pScene->GetNodeType()) && mash::collision::Ray_AABB(pScene->GetWorldBoundingBox(), ray, temp))
		{
			//handles if we are inside a boundingbox
			if (temp < fMaximumRayLength)
			{
				*out = pScene;
				if (temp > 0.0f)
					fMaximumRayLength = temp;
				else if (!out)
					fMaximumRayLength = mash::math::MaxFloat();
			}
		}

        //If there is no intersection here than no need to check children
		if (mash::collision::Ray_AABB(pScene->GetTotalBoundingBox(), ray))
        {
            MashList<mash::MashSceneNode*>::ConstIterator iter = pScene->GetChildren().Begin();
            MashList<mash::MashSceneNode*>::ConstIterator end = pScene->GetChildren().End();
            for(; iter != end; ++iter)
            {
                GetClosestNodeByBounds((*iter), ray, iTypesToTest, fMaximumRayLength, out);
            }
        }

		return *out;
	}

	bool MashScenePick::GetClosestTriFromScene(mash::MashSceneNode *pScene,
		const mash::MashRay &ray,
		uint32 iTypesToTest,
		bool bBackFaceCull,
		sTriPickResult &out)
	{
		if (!pScene)
			return false;

		f32 BBDist = 0.0f;

		const MashTriangleCollider *collider = pScene->GetTriangleCollider();
		//if this node has triangles
		if (collider)
		{
            //and of correct type
            if ((iTypesToTest & pScene->GetNodeType()) && mash::collision::Ray_AABB(pScene->GetWorldBoundingBox(), ray, BBDist))
            {
				//If this node is close enough
				if (BBDist < out.distance)
				{
					//handles if we are inside a boundingbox
					bool doTest = false;
					if (BBDist > 0.0f)
						doTest = true;
					else if (!out.collision)
						doTest = true;

					if (doTest)
					{
						//check the triangles
						sTriPickResult tempResult;
						if (collider->GetClosestTriangle(ray, 
														 pScene->GetWorldTransformState(), tempResult))
						{
							if (tempResult.distance < out.distance)
							{
								out = tempResult;
								//out.triangleCollectionIndex = i;
								out.node = pScene;
							}
						}
					}
				}
            }
		}

        //If there is no intersection here than no need to check children
        if (mash::collision::Ray_AABB(pScene->GetTotalBoundingBox(), ray))
        {
            MashList<mash::MashSceneNode*>::ConstIterator iter = pScene->GetChildren().Begin();
            MashList<mash::MashSceneNode*>::ConstIterator end = pScene->GetChildren().End();
            for(; iter != end; ++iter)
            {
                GetClosestTriFromScene((*iter), ray, iTypesToTest, bBackFaceCull, out);
            }
        }

		return out.collision;
	}
}