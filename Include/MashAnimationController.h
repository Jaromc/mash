//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_ANIMATION_CONTROLLER_H_
#define _MASH_ANIMATION_CONTROLLER_H_

#include "MashReferenceCounter.h"
#include "MashEnum.h"

namespace mash
{
	class MashKeySetInterface;
	class MashSceneNode;
	/*!
		The controllers are resposible for interpolating there data to a
		particular frame. They are accessed by the animation mixer and users
		shouldn't really need to use them directly.

		These hold animation key data for a node instance. So Each node will have a unique
		controller, but the keys held internally are shared to each controller.

		Controllers are commonly used for node interpolation. But others can be created
		to handle animations such as light colours.

		Once a key set has been assigned to a controller the key set should not be modified.
	*/
	class MashKeyController : public MashReferenceCounter
	{
	public:
		MashKeyController():MashReferenceCounter(){}
		virtual ~MashKeyController(){}

		//! Gets the key set.
		/*!
			\return Key set.
		*/
		virtual const MashKeySetInterface* GetKeySet()const = 0;

		//! Called before animating forward.
		/*!
			This allows the controller to cache any data needed to animate forward.
			Call AnimationEnd() when done.
		*/
		virtual void AnimationStart() = 0;

		//! Animates to a particular key.
		/*!
			This should be called between AnimationStart() and AnimationEnd().

			\param key Key to animate to (Not frame).
		*/
		virtual void AnimateToKey(uint32 key) = 0;

		//! Interpolates the animation.
		/*!
			This should be called between AnimationStart() and AnimationEnd().

			\param fromKey Start key (Not frame).
			\param toKey End key (Not frame).
			\param amount Normalized interpolate amount.
		*/
		virtual void AnimateForward(uint32 fromKey, uint32 toKey, f32 amount) = 0;

		//! Called when animation is finished for a frame.
		/*!
			Called after AnimationStart().
			
			\param blend How should the interpolation blend.
			\param blendAmount Blend weight between 0 - 1.
		*/
		virtual void AnimationEnd(eANIMATION_BLEND_MODE blend, f32 blendAmount) = 0;

		//! Returns the controller type.
		/*!
			\return Controller type.
		*/
		virtual eMASH_CONTROLLER_TYPE GetControllerType()const = 0;

		//! Returns the node these animations belong to.
		virtual MashSceneNode* GetOwner()const = 0;
	};

	/*!
		Controller responsible for node interpolation.
	*/
	class MashTransformationController : public MashKeyController
	{
	public:
		MashTransformationController():MashKeyController(){}
		virtual ~MashTransformationController(){}
	};
}

#endif