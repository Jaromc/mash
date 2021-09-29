//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashPhysicsRigidBody.h"
#include "MashSceneNode.h"

namespace mash
{
	CMashPhysicsRigidBody::CMashPhysicsRigidBody(mash::MashSceneNode *node, CMashPhysicsMotionState *motionState,
		btRigidBody *rigidBody, MashPhysicsCollisionShape *collisionShape):MashPhysicsRigidBody(), m_node(node), 
		m_motionState(motionState), m_rigidBody(rigidBody), m_collisionShape(collisionShape)
	{
		if (m_node)
			m_node->Grab();

		if (m_collisionShape)
			m_collisionShape->Grab();
	}

	CMashPhysicsRigidBody::~CMashPhysicsRigidBody()
	{
		if (m_rigidBody)
		{
			MASH_DELETE_T(btRigidBody, m_rigidBody);
			m_rigidBody = 0;
		}
        
		if (m_node)
		{
			m_node->Drop();
			m_node = 0;
		}

		if (m_collisionShape)
		{
			m_collisionShape->Drop();
			m_collisionShape = 0;
		}

		if (m_motionState)
		{
			MASH_DELETE_T(CMashPhysicsMotionState, m_motionState);
			m_motionState = 0;
		}
	}

	void CMashPhysicsRigidBody::SetSleepState(bool enable)
	{
		if (enable)
			m_rigidBody->setActivationState(WANTS_DEACTIVATION);
		else
			m_rigidBody->setActivationState(DISABLE_DEACTIVATION);
	}

	void CMashPhysicsRigidBody::SetLinearVelocity(const mash::MashVector3 &velocity)
	{
		m_rigidBody->setLinearVelocity(btVector3(velocity.x, velocity.y, velocity.z));
	}

	void CMashPhysicsRigidBody::SetAngularVelocity(const mash::MashVector3 &velocity)
	{
		m_rigidBody->setAngularVelocity(btVector3(velocity.x, velocity.y, velocity.z));
	}

	void CMashPhysicsRigidBody::ApplyImpulse(const mash::MashVector3 &impulse, const mash::MashVector3 &relPos)
	{
		m_rigidBody->applyImpulse(btVector3(impulse.x, impulse.y, impulse.z), btVector3(relPos.x, relPos.y, relPos.z));
	}

	void CMashPhysicsRigidBody::ApplyForce(const mash::MashVector3 &force, const mash::MashVector3 &relPos)
	{
		m_rigidBody->applyForce(btVector3(force.x, force.y, force.z), btVector3(relPos.x, relPos.y, relPos.z));
	}

	void CMashPhysicsRigidBody::ApplyTorque(const mash::MashVector3 &torque)
	{
		m_rigidBody->applyTorque(btVector3(torque.x, torque.y, torque.z));
	}

	void CMashPhysicsRigidBody::ApplyTorqueImpulse(const mash::MashVector3 &torque)
	{
		m_rigidBody->applyTorqueImpulse(btVector3(torque.x, torque.y, torque.z));
	}
}