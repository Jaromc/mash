//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_PHYSICS_COLLISION_SHAPE_H_
#define _MASH_PHYSICS_COLLISION_SHAPE_H_

#include "MashReferenceCounter.h"

namespace mash
{
    /*!
        Base class for all collision shapes.
    */
	class MashPhysicsCollisionShape : public MashReferenceCounter
	{
	public:
		MashPhysicsCollisionShape():MashReferenceCounter(){}
		virtual ~MashPhysicsCollisionShape(){}

        //! Returns true if this is a complex collision shape.
		virtual bool IsConcave()const = 0;
	};
}

#endif