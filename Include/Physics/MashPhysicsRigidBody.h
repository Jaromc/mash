//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_PHYSICS_RIGID_BODY_H_
#define _MASH_PHYSICS_RIGID_BODY_H_

#include "MashPhysicsNode.h"
#include "MashVector3.h"

namespace mash
{
    /*!
        This object is owned by one scene node. Movement applied to this object in the
        physics scene is transfered to its scene node.
    */
	class MashPhysicsRigidBody : public MashPhysicsNode
	{
	public:
		MashPhysicsRigidBody():MashPhysicsNode(){}
		virtual ~MashPhysicsRigidBody(){}

        //! Sets the linear velocity.
        /*!
            \param velocity Object linear velocity.
        */
		virtual void SetLinearVelocity(const MashVector3 &velocity) = 0;
        
        //! Sets the angular velocity.
        /*!
            \param velocity Object angular velocity.
        */
		virtual void SetAngularVelocity(const MashVector3 &velocity) = 0;

        //! Applies an impulse to the node.
        /*!
            Impulse instantaneously affects the linear and angular velocity.
         
            \param impulse Impulse to apply.
            \param relPos Relative position on the object to apply the impulse.
        */
		virtual void ApplyImpulse(const MashVector3 &impulse, 
			const MashVector3 &relPos = MashVector3(0.0f, 0.0f, 0.0f)) = 0;

        //! Applies a force to the node.
        /*!
            Force is accumulated and applied over following simulation steps.
         
            \param force Force to apply.
            \param relPos Relative position on the object to apply the force.
        */
		virtual void ApplyForce(const MashVector3 &force, 
			const MashVector3 &relPos = MashVector3(0.0f, 0.0f, 0.0f)) = 0;

        //! Applys torque to the node.
        /*!
            Torque is accumulated and applied over following simulation steps.
            
            \param torque Torque to apply.
        */
		virtual void ApplyTorque(const MashVector3 &torque) = 0;
        
        //! Applys a torque impulse to the node.
        /*!
            Impulse instantaneously affects the angular velocity.
         
            \param torque Torque to apply.
        */
		virtual void ApplyTorqueImpulse(const MashVector3 &torque) = 0;

        //! Stops any forces being applied to this object till its re-enabled.
        /*!
            \param enable Enables or disabled sleep.
        */
		virtual void SetSleepState(bool enable) = 0;
	};
}

#endif