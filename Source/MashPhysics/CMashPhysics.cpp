//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashPhysics.h"
#include "MashMesh.h"
#include "MashModel.h"
#include "MashEntity.h"
#include "MashVector3.h"
#include "BulletCollision/CollisionShapes/btCollisionShape.h"
#include "BulletCollision/CollisionShapes/btShapeHull.h"
#include "BulletCollision/CollisionShapes/btConvexHullShape.h"
#include "BulletCollision/CollisionShapes/btStaticPlaneShape.h"
#include "MashLog.h"

namespace mash
{
	MashPhysics* CreateMashPhysics()
	{
		CMashPhysics *physicsSystem = MASH_NEW_COMMON CMashPhysics();

		return physicsSystem;
	}

	CMashPhysics::CMashPhysics():MashPhysics(), m_renderer(0), m_sceneManager(0),
		m_debugRenderer(0), m_fixedStepRate(1.0f / 60.0f)
	{
		
	}

	CMashPhysics::~CMashPhysics()
	{
		if (m_debugRenderer)
		{
			m_debugRenderer->Drop();
			m_debugRenderer = 0;
		}

		RemoveAllRigidBodies();

		MASH_DELETE_T(btDynamicsWorld, m_dynamicsWorld);
		MASH_DELETE_T(btDefaultCollisionConfiguration, m_collisionConfiguration);
		MASH_DELETE_T(btCollisionDispatcher, m_dispatcher);
		MASH_DELETE_T(btBroadphaseInterface, m_overlappingPairCache);
		MASH_DELETE_T(btConstraintSolver, m_constraintSolver);
	}

	void CMashPhysics::RemoveAllRigidBodies()
	{
		for(uint32 i = 0; i < m_worldObjects.Size(); ++i)
		{
			m_dynamicsWorld->removeRigidBody(((CMashPhysicsRigidBody*)m_worldObjects[i])->GetBtRigidBody());
			m_worldObjects[i]->Drop();
		}
		
		m_worldObjects.Clear();
	}

	void CMashPhysics::RemoveRigidBody(MashPhysicsNode *rigidBody)
	{
		if (!rigidBody)
			return;

		for(uint32 i = 0; i < m_worldObjects.Size(); ++i)
		{
			if (m_worldObjects[i] == rigidBody)
			{
				m_worldObjects.Erase(m_worldObjects.Begin() + i);
				m_dynamicsWorld->removeRigidBody(((CMashPhysicsRigidBody*)rigidBody)->GetBtRigidBody());
				rigidBody->Drop();
				break;
			}
		}
	}

	void CMashPhysics::_Initialise(mash::MashVideo *renderer, MashSceneManager *sceneManager, const mash::sMashDeviceSettings &settings)
	{
		m_renderer = renderer;
		m_sceneManager = sceneManager;

		m_collisionConfiguration = MASH_NEW_T_COMMON(btDefaultCollisionConfiguration)();
		m_dispatcher = MASH_NEW_T_COMMON(btCollisionDispatcher)(m_collisionConfiguration);
		m_overlappingPairCache = MASH_NEW_T_COMMON(btDbvtBroadphase)();
		m_constraintSolver = MASH_NEW_T_COMMON(btSequentialImpulseConstraintSolver)();
		m_dynamicsWorld = MASH_NEW_T_COMMON(btDiscreteDynamicsWorld)(m_dispatcher,m_overlappingPairCache,m_constraintSolver,m_collisionConfiguration);
		m_dynamicsWorld->setGravity(btVector3(0, -9.8f, 0));

		m_fixedStepRate = settings.fixedTimeStep;
	}

	MashPhysicsNode* CMashPhysics::GetPhysicsNodeByName(const MashStringc &name)const
	{
		uint32 objectCount = m_worldObjects.Size();
		for(uint32 i = 0; i < objectCount; ++i)
		{
            if (m_worldObjects[i]->GetSceneNode()->GetNodeName() == name)
				return m_worldObjects[i];
		}

		return 0;
	}

	MashPhysicsNode* CMashPhysics::GetPhysicsNodeById(uint32 id)const
	{
		uint32 objectCount = m_worldObjects.Size();
		for(uint32 i = 0; i < objectCount; ++i)
		{
			if (m_worldObjects[i]->GetSceneNode()->GetNodeID() == id)
				return m_worldObjects[i];
		}

		return 0;
	}

	void CMashPhysics::_Simulate(f32 dt)
	{
		// play with numbers to reduce missed collision problems
		m_dynamicsWorld->stepSimulation(dt, 5, /*1.0f / 120.0f*/m_fixedStepRate * 0.25f);
	}

	void CMashPhysics::SetGravity(const mash::MashVector3 &gravity)
	{
		m_dynamicsWorld->setGravity(btVector3(gravity.x, gravity.y, gravity.z));
	}

	MashPhysicsCollisionShape* CMashPhysics::_CreateCollisionShape(btCollisionShape *shape, bool isConcave)
	{
		return MASH_NEW_COMMON CMashPhysicsCollisionShape(shape, isConcave);
	}

	MashPhysicsRigidBody* CMashPhysics::AddRigidBody(mash::MashSceneNode *node, const sRigisBodyConstruction &rbConstructor, MashPhysicsCollisionShape *collisionObject)
	{
		//copy it over so we can change a few things if needed
		sRigisBodyConstruction rigidBodyConstructor = rbConstructor;

		btTransform graphicsOffset;
		graphicsOffset.setIdentity();

		btVector3 localInertia(0,0,0);

		btCollisionShape *btShape = 0;
		if (collisionObject)
		{
			btShape = ((CMashPhysicsCollisionShape*)collisionObject)->GetBulletCollsionShape();

			btVector3 min, max;
			btShape->getAabb(graphicsOffset, min, max);
			graphicsOffset.getOrigin().setY((max.y() - min.y()) / 2.0f); 

			//concave objects cannot move 
			if (collisionObject->IsConcave())
			{
				rigidBodyConstructor.mass = 0.0f;
			}
			else if (rigidBodyConstructor.mass > 0.0f)
			{
				btShape->calculateLocalInertia(rigidBodyConstructor.mass, localInertia);
			}
		}

		CMashPhysicsMotionState *newMotionState = MASH_NEW_T_COMMON(CMashPhysicsMotionState)(node, graphicsOffset);

		btRigidBody::btRigidBodyConstructionInfo bodyInfo(rigidBodyConstructor.mass, newMotionState, btShape, localInertia);
		btRigidBody *newBody = MASH_NEW_T_COMMON(btRigidBody)(bodyInfo);
		newBody->setFriction(rigidBodyConstructor.friction);
		newBody->setRestitution(rigidBodyConstructor.restitution);
		newBody->setDamping(rigidBodyConstructor.linearDamping, rigidBodyConstructor.angularDamping);

		m_dynamicsWorld->addRigidBody(newBody);

		CMashPhysicsRigidBody *newWorldObject = MASH_NEW_COMMON CMashPhysicsRigidBody(node, newMotionState, newBody, collisionObject);
		m_worldObjects.PushBack(newWorldObject);

		MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_INFORMATION,
					"CMashPhysics::AddRigidBody",
					"New physics node created from scene node '%s'.",
					node->GetNodeName().GetCString());

		return newWorldObject;
	}

	MashPhysicsCollisionShape* CMashPhysics::CreateCollisionShape(const sCollisionObjectConstruction &collisionObj)
	{
		btCollisionShape *newCollisionShape = 0;

		switch(collisionObj.type)
		{
		case aPHYSICS_SHAPE_PLANE:
			{
				newCollisionShape = MASH_NEW_T_COMMON(btStaticPlaneShape)(btVector3(collisionObj.plane.normal.x, collisionObj.plane.normal.y, collisionObj.plane.normal.z), collisionObj.plane.d);
				break;
			}
		case aPHYSICS_SHAPE_CUBE:
			{
				btVector3 halfExt(collisionObj.cubeHalfExt.x, collisionObj.cubeHalfExt.y, collisionObj.cubeHalfExt.z);
				newCollisionShape = MASH_NEW_T_COMMON(btBoxShape)(halfExt);
				break;
			}
		case aPHYSICS_SHAPE_CAPSULE:
			{
				newCollisionShape = MASH_NEW_T_COMMON(btCapsuleShape)(collisionObj.capsule.radius, collisionObj.capsule.height);
				break;
			} 
		case aPHYSICS_SHAPE_CYLINDER:
			{
				btVector3 halfExt(collisionObj.cylinderHalfExt.x, collisionObj.cylinderHalfExt.y, collisionObj.cylinderHalfExt.z);
				newCollisionShape = MASH_NEW_T_COMMON(btCylinderShape)(halfExt);
				break;
			}
		case aPHYSICS_SHAPE_CONE:
			{
				newCollisionShape = MASH_NEW_T_COMMON(btConeShape)(collisionObj.cone.radius, collisionObj.cone.height);
				break;
			}
		case aPHYSICS_SHAPE_SPHERE:
			{
				newCollisionShape = MASH_NEW_T_COMMON(btSphereShape)(collisionObj.sphereRadius);
				break;
			}
		default:
			return 0;
		};

		MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_INFORMATION,
					"New dynamic physics collision shape created from bsaic primitive type.",
					"CMashPhysics::CreateCollisionShape");

		return _CreateCollisionShape(newCollisionShape, false);
	}

	MashPhysicsCollisionShape* CMashPhysics::CreateCollisionShape(mash::MashModel *model, uint32 lod, bool generateTriangleBufferIfNull, bool optimize)
	{
		MashPhysicsCollisionShape *shape = 0;
		uint32 meshCount = model->GetMeshCount(lod);
		if (meshCount > 0)
		{
			MashTriangleBuffer **triangleBuffers = MASH_ALLOC_T_COMMON(MashTriangleBuffer*, meshCount);
			for(uint32 i = 0; i < meshCount; ++i)
			{
				mash::MashMesh *mesh = model->GetMesh(i, lod);
				if (mesh)
				{
					triangleBuffers[i] = mesh->GetTriangleBuffer();
					if (!triangleBuffers[i] && generateTriangleBufferIfNull)
					{
						triangleBuffers[i] = m_sceneManager->CreateTriangleBuffer(mesh);
						if (triangleBuffers[i])
						{
							mesh->SetTriangleBuffer(triangleBuffers[i]);
							triangleBuffers[i]->Drop();
						}
					}
				}
			}

			//creation logging info is done in this function
			shape = CreateCollisionShape(triangleBuffers, meshCount, optimize);

			MASH_FREE(triangleBuffers);
		}

		return shape;
	}

	MashPhysicsCollisionShape* CMashPhysics::CreateCollisionShape(MashTriangleBuffer **triangleBuffers, uint32 bufferCount, bool optimize)
	{
		btConvexHullShape *collisionObj = MASH_NEW_T_COMMON(btConvexHullShape)();

		for(uint32 i = 0; i < bufferCount; ++i)
		{
			if (triangleBuffers[i] && (triangleBuffers[i]->GetTriangleCount() > 0))
			{
				MashArray<mash::MashVector3>::ConstIterator iter = triangleBuffers[i]->GetVertexList().Begin();
				MashArray<mash::MashVector3>::ConstIterator iterEnd = triangleBuffers[i]->GetVertexList().End();
				for(; iter != iterEnd; ++iter)
				{
					collisionObj->addPoint(btVector3(iter->x, iter->y, iter->z));
				}
			}
		}

		if (optimize)
		{
			//create an optimized convext hull
			btShapeHull* hull = MASH_NEW_T_COMMON(btShapeHull)(collisionObj);
			if (hull->buildHull(collisionObj->getMargin()) != false)
			{
				btConvexHullShape* optimizedConvexHull = MASH_NEW_T_COMMON(btConvexHullShape)();

				for(uint32 i = 0; i < hull->numVertices(); ++i)
					optimizedConvexHull->addPoint(hull->getVertexPointer()[i]);

				MASH_DELETE_T(btConvexHullShape, collisionObj);

				collisionObj = optimizedConvexHull;
			}

			MASH_DELETE_T(btShapeHull, hull);
		}

		MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_INFORMATION,
					"New dynamic physics collision shape created from triangle mesh data.",
					"CMashPhysics::CreateCollisionShape");

		return _CreateCollisionShape(collisionObj, false);
	}

	MashPhysicsCollisionShape* CMashPhysics::CreateStaticTriangleCollisionShape(mash::MashModel *model, uint32 lod, bool generateTriangleBufferIfNull)
	{
		MashPhysicsCollisionShape *shape = 0;
		uint32 meshCount = model->GetMeshCount(lod);
		if (meshCount > 0)
		{
			MashTriangleBuffer **triangleBuffers = MASH_ALLOC_T_COMMON(MashTriangleBuffer*, meshCount);
			for(uint32 i = 0; i < meshCount; ++i)
			{
				mash::MashMesh *mesh = model->GetMesh(i, lod);
				if (mesh)
				{
					triangleBuffers[i] = mesh->GetTriangleBuffer();
					if (!triangleBuffers[i] && generateTriangleBufferIfNull)
					{
						triangleBuffers[i] = m_sceneManager->CreateTriangleBuffer(mesh);
						if (triangleBuffers[i])
						{
							mesh->SetTriangleBuffer(triangleBuffers[i]);
							triangleBuffers[i]->Drop();
						}
					}
				}
			}

			//creation logging info is done in this function
			shape = CreateStaticTriangleCollisionShape(triangleBuffers, meshCount);

			MASH_FREE(triangleBuffers);
		}

		return shape;
	}

	MashPhysicsCollisionShape* CMashPhysics::CreateStaticTriangleCollisionShape(MashTriangleBuffer **triangleBuffers, uint32 bufferCount)
	{
		 btTriangleIndexVertexArray *triangleMesh = MASH_NEW_T_COMMON(btTriangleIndexVertexArray)();

		 /*
			Bullet can share triangle information that is generated in the engine,
			meaning big savings in memory.

			We keep track of the triangle buffers used so we can Grab() and Drop() when needed.
		 */
		MashArray<MashTriangleBuffer*> usedTriangleBuffers;
		for(uint32 i = 0; i < bufferCount; ++i)
		{
			MashTriangleBuffer *buffer = triangleBuffers[i];
			if (buffer && (buffer->GetTriangleCount() > 0))
			{
				btIndexedMesh bulletMesh;
				bulletMesh.m_numTriangles = buffer->GetTriangleCount();
				bulletMesh.m_triangleIndexBase = (const uint8 *)&buffer->GetIndexList()[0];
				bulletMesh.m_triangleIndexStride = sizeof(uint32) * 3;
				bulletMesh.m_numVertices = buffer->GetVertexList().Size();
				bulletMesh.m_vertexBase = (const uint8 *)&buffer->GetVertexList()[0];
				bulletMesh.m_vertexStride = sizeof(mash::MashVector3);

				triangleMesh->addIndexedMesh(bulletMesh);
				usedTriangleBuffers.PushBack(buffer);
			}			
		}

		btBvhTriangleMeshShape *triMeshShape = MASH_NEW_T_COMMON(btBvhTriangleMeshShape)(triangleMesh,true);
		CMashPhysicsCollisionShape *newCollisionShape = (CMashPhysicsCollisionShape*)_CreateCollisionShape(triMeshShape, true);
		newCollisionShape->SetTriangleBuffers(usedTriangleBuffers);
		newCollisionShape->SetTriangleArray(triangleMesh);

		MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_INFORMATION,
					"New static physics collision shape created from triangle mesh data.",
					"CMashPhysics::CreateCollisionShape");

		return newCollisionShape;
	}

	void CMashPhysics::DrawDebug(ePHYSICS_DEBUG_RENDER type)
	{
		if (!m_dynamicsWorld)
			return;

		if (!m_debugRenderer)
		{
			m_debugRenderer = MASH_NEW_COMMON CMashPhysicsDebugDraw(m_sceneManager);
			m_dynamicsWorld->setDebugDrawer(m_debugRenderer);
		}
		
		if (type == aPDR_WIREFRAME)
			m_debugRenderer->setDebugMode(btIDebugDraw::DBG_DrawWireframe);
		else
			m_debugRenderer->setDebugMode(btIDebugDraw::DBG_DrawAabb);

		m_dynamicsWorld->debugDrawWorld();
	}
}