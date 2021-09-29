//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_PHYSICS_RIGID_BODY_H_
#define _C_MASH_PHYSICS_RIGID_BODY_H_

#include "btBulletDynamicsCommon.h"
#include "MashPhysicsRigidBody.h"
#include "MashPhysicsCollisionShape.h"
#include "CMashPhysicsMotionState.h"
namespace mash
{
	class CMashPhysicsRigidBody : public MashPhysicsRigidBody
	{
	public:
		btRigidBody *m_rigidBody;
	private:
		CMashPhysicsMotionState *m_motionState;
		mash::MashSceneNode *m_node;
		
		MashPhysicsCollisionShape *m_collisionShape;
	public:
		CMashPhysicsRigidBody(mash::MashSceneNode *node, 
			CMashPhysicsMotionState *motionState,
			btRigidBody *rigidBody,
			MashPhysicsCollisionShape *collisionShape);

		~CMashPhysicsRigidBody();

		void SetLinearVelocity(const mash::MashVector3 &velocity);
		void SetAngularVelocity(const mash::MashVector3 &velocity);

		void ApplyImpulse(const mash::MashVector3 &impulse, 
			const mash::MashVector3 &relPos = mash::MashVector3(0.0f, 0.0f, 0.0f));

		void ApplyForce(const mash::MashVector3 &force, 
			const mash::MashVector3 &relPos = mash::MashVector3(0.0f, 0.0f, 0.0f));

		void ApplyTorque(const mash::MashVector3 &torque);
		void ApplyTorqueImpulse(const mash::MashVector3 &torque);

		void SetSleepState(bool enable);

		MashSceneNode* GetSceneNode()const;

		btRigidBody* GetBtRigidBody()const;

		eMASH_PHYSICS_OBJECT_TYPE GetPhysicsObjectType()const;

	};

	inline btRigidBody* CMashPhysicsRigidBody::GetBtRigidBody()const
	{
		return m_rigidBody;
	}

	inline eMASH_PHYSICS_OBJECT_TYPE CMashPhysicsRigidBody::GetPhysicsObjectType()const
	{
		return aPHYSICS_OBJ_RIGIDBODY;
	}

	inline MashSceneNode* CMashPhysicsRigidBody::GetSceneNode()const
	{
		return m_node;
	}
}

#endif