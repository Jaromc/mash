//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _CMASH_ELLIPSOID_COLLIDER_CONTROLLER_H_
#define _CMASH_ELLIPSOID_COLLIDER_CONTROLLER_H_

#include "MashEllipsoidColliderController.h"
#include "MashEventTypes.h"
#include "MashVector3.h"
#include "MashAABB.h"
#include "MashArray.h"
namespace mash
{
    struct sIntersectingTriangleResult;
    
	class CMashEllipsoidColliderController : public MashEllipsoidColliderController
	{
		struct CollisionPacket
		{
			//R3 space
			MashVector3 R3Velocity;
			MashVector3 R3Position;
			MashVector3 eRadius;

			MashAABB aabb;

			//eSpace
			MashVector3 velocity;
			MashVector3 normalizedVelocity;
			MashVector3 basePoint;

			//hit information
			bool foundCollision;
			f32 nearestDistance;
			MashVector3 intersectionPoint;
		};
    private:
		MashSceneNode *m_collisionScene;

		//held here so that memory allocations dont happen each node, each update
		MashArray<sIntersectingTriangleResult> m_collidingTriangleBuffer;
		
		bool CheckPointInTriangle(const MashVector3 &vPoint,
													const MashVector3 &pa,
													const MashVector3 &pb,
													const MashVector3 &pc);
		bool GetLowestRoot(f32 a, f32 b, f32 c, f32 maxR, f32 &root);
		void CheckTriangle(CollisionPacket &colPacket,
											const MashVector3 &p1,
											const MashVector3 &p2,
											const MashVector3 &p3);

		MashVector3 CollideWithWorld(MashSceneNode *thisSceneNode, CollisionPacket &collisionPackage, const MashVector3 &pos, const MashVector3 &vel);
		void CollideWithWorldTriangles(MashSceneNode *thisSceneNode, CollisionPacket &collisionPacket, MashSceneNode *node);

		MashVector3 m_lastPosition;
		uint32 collisionRecursionDepth;
		MashVector3 m_radius;
		MashVector3 m_gravity;
    public:
        CMashEllipsoidColliderController(MashSceneNode *sceneNode, 
			MashSceneNode *collisionScene,
			const MashVector3 &radius,
			const MashVector3 &gravity);
        
        ~CMashEllipsoidColliderController();
        
        void OnNodeUpdate(MashSceneNode *sceneNode, f32 dt);
        
		void SetCollisionScene(MashSceneNode *collisionScene);
		MashSceneNode* GetCollisionScene()const;
	};

	inline MashSceneNode* CMashEllipsoidColliderController::GetCollisionScene()const
	{
		return m_collisionScene;
	}
}

#endif