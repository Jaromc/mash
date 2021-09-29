//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_ELLIPSOID_COLLIDER_CONTROLLER_H_
#define _MASH_ELLIPSOID_COLLIDER_CONTROLLER_H_

#include "MashSceneNodeCallback.h"

namespace mash
{
	class MashEllipsoidColliderController : public MashSceneNodeCallback
	{
    public:
        MashEllipsoidColliderController():MashSceneNodeCallback(){}
        virtual ~MashEllipsoidColliderController(){}
        
        //! Sets the scene this collider will colide with.
        /*!
            The owning node may be apart of the collision scene.
         
            \param collisionScene Scene of objects with triangle colliders.
        */
        virtual void SetCollisionScene(MashSceneNode *collisionScene) = 0;
        
        //! Gets the collision scene for this object.
		virtual MashSceneNode* GetCollisionScene()const = 0;
        
	};
}

#endif