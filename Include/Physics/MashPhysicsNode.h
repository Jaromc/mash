//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_PHYSICS_NODE_H_
#define _MASH_PHYSICS_NODE_H_

#include "MashReferenceCounter.h"
#include "MashPhysicsCommon.h"

namespace mash
{
	class MashSceneNode;

    /*!
        Base class for physics objects.
    */
	class MashPhysicsNode : public MashReferenceCounter
	{
	public:
		MashPhysicsNode():MashReferenceCounter(){}
		virtual ~MashPhysicsNode(){}

        //! Gets the scene node that is affected by thie object.
		virtual MashSceneNode* GetSceneNode()const = 0;
        
        //! Gets the physics object type.
		virtual eMASH_PHYSICS_OBJECT_TYPE GetPhysicsObjectType()const = 0;
	};
}

#endif