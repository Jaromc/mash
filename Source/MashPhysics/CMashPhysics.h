//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_PHYSICS_H_
#define _C_MASH_PHYSICS_H_

#include "MashPhysics.h"
#include "MashVideo.h"
#include "btBulletDynamicsCommon.h"
#include "CMashPhysicsRigidBody.h"
#include "CMashPhysicsCollisionShape.h"
#include "CMashPhysicsDebugDraw.h"
namespace mash
{
	class CMashPhysics : public MashPhysics
	{
	private:
		btDynamicsWorld *m_dynamicsWorld;
		btDefaultCollisionConfiguration *m_collisionConfiguration;
		btBroadphaseInterface *m_overlappingPairCache;
		btCollisionDispatcher *m_dispatcher;
		btConstraintSolver *m_constraintSolver;

		MashArray<MashPhysicsNode*> m_worldObjects;

		mash::MashVideo *m_renderer;
		MashSceneManager *m_sceneManager;
		f32 m_fixedStepRate;

		CMashPhysicsDebugDraw *m_debugRenderer;

		MashPhysicsCollisionShape* _CreateCollisionShape(btCollisionShape *shape, bool isConcave);
	public:
		CMashPhysics();
		~CMashPhysics();

		void _Initialise(mash::MashVideo *renderer, MashSceneManager *sceneManager, const mash::sMashDeviceSettings &settings);

		MashPhysicsRigidBody* AddRigidBody(mash::MashSceneNode *node, const sRigisBodyConstruction &rigidBodyConstructor, MashPhysicsCollisionShape *collisionObject);

		MashPhysicsCollisionShape* CreateCollisionShape(const sCollisionObjectConstruction &collisionObj);
		MashPhysicsCollisionShape* CreateCollisionShape(MashTriangleBuffer **triangleBuffers, uint32 bufferCount, bool optimize = true);
		MashPhysicsCollisionShape* CreateCollisionShape(mash::MashModel *model, uint32 lod = 0, bool generateTriangleBufferIfNull = true, bool optimize = true);

		MashPhysicsCollisionShape* CreateStaticTriangleCollisionShape(MashTriangleBuffer **triangleBuffers, uint32 bufferCount);
		MashPhysicsCollisionShape* CreateStaticTriangleCollisionShape(mash::MashModel *model, uint32 lod = 0, bool generateTriangleBufferIfNull = true);
        
		MashPhysicsNode* GetPhysicsNodeByName(const MashStringc &name)const;
		MashPhysicsNode* GetPhysicsNodeById(uint32 id)const;

		void SetWorldBounds(const mash::MashAABB &bounds){}
		void SetGravity(const mash::MashVector3 &gravity);

		void _Simulate(f32 dt);

		void DrawDebug(ePHYSICS_DEBUG_RENDER type = aPDR_AABB);

		void RemoveRigidBody(MashPhysicsNode *rigidBody);
		void RemoveAllRigidBodies();
	};
}

#endif