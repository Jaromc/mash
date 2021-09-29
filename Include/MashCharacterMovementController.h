//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_CHARACTER_MOVEMENT_CONTROLLER_H_
#define _MASH_CHARACTER_MOVEMENT_CONTROLLER_H_

#include "MashSceneNodeCallback.h"

namespace mash
{
	class MashCharacterMovementController : public MashSceneNodeCallback
	{
	public:
		/*!
			These are of type eINPUT_KEY_MAP and define the mapping for a
			controllers movement.

			For example,
			moveX = aINPUTMAP_MOVE_HORIZONTAL
			moveZ = aINPUTMAP_MOVE_VERTICAL
			lookY = aINPUTMAP_LOOK_HORIZONTAL

			...though you will most likely be using your own custom
			actions.
		*/
		struct sInputActionMovement
		{
			int32 moveX;
			int32 moveZ;
			int32 lookY;
		};
    public:
        MashCharacterMovementController():MashSceneNodeCallback(){}
        virtual ~MashCharacterMovementController(){}
        
        //! Gets the linear speed.
        virtual f32 GetLinearSpeed()const = 0;
        
        //! Gets the rotation speed.
		virtual f32 GetRotationSpeed()const = 0;
        
        //! Returns true if input is enabled, false if input is disabled.
        virtual bool GetInputState()const = 0;
        
        //! Set true to enable user input control, false to disable it.
        virtual void SetInputState(bool enable) = 0;
        
        //! Sets the linear speed.
        virtual void SetLinearSpeed(f32 s) = 0;
        
        //! Sets the roation speed.
        virtual void SetRotationSpeed(f32 s) = 0;
	};
}

#endif